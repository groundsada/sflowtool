/*
 * decoder.c - Core packet decoder engine
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include "../include/decoder.h"
#include "../include/protocol.h"
#include "../include/json_loader.h"
#include "../json_protocols.h"

extern char *fieldExtract_toString(uint8_t *packetData, uint32_t packetLength, FieldDefinition *field);
extern uint64_t fieldExtract_toUint64(uint8_t *packetData, uint32_t packetLength, FieldDefinition *field);
extern int fieldExtract_checkBounds(uint8_t *packetData, uint32_t packetLength, FieldDefinition *field);

static DecodedField *decodeFields(ProtocolDefinition *protocol, uint8_t *packetData, uint32_t packetLength, uint32_t startOffset);

int decoder_init(DecoderContext *ctx, const char *protocolsFile) {
    if (!ctx) return -1;
    ctx->protocols = malloc(sizeof(ProtocolDatabase));
    if (!ctx->protocols) { snprintf(ctx->errorBuffer, sizeof(ctx->errorBuffer), "Memory allocation failed"); return -1; }
    protocolDatabase_init(ctx->protocols);
    if (protocolsFile) {
        if (jsonLoader_loadFile(ctx->protocols, protocolsFile) == 0) { ctx->initialized = true; return 0; }
    }
    if (json_protocols_load(ctx->protocols) == 0) { ctx->initialized = true; return 0; }
    snprintf(ctx->errorBuffer, sizeof(ctx->errorBuffer), "Failed to load protocols: %s", jsonLoader_getError());
    free(ctx->protocols);
    ctx->protocols = NULL;
    return -1;
}

void decoder_free(DecoderContext *ctx) {
    if (!ctx) return;
    if (ctx->protocols) { protocolDatabase_free(ctx->protocols); free(ctx->protocols); ctx->protocols = NULL; }
    ctx->initialized = false;
}

static DecodedField *decodeFields(ProtocolDefinition *protocol, uint8_t *packetData, uint32_t packetLength, uint32_t startOffset) {
    DecodedField *fields = NULL;
    uint32_t i;
    (void)startOffset;
    if (!protocol || !packetData) return NULL;
    if (protocol->fieldCount == 0) return NULL;
    fields = malloc(sizeof(DecodedField) * protocol->fieldCount);
    if (!fields) return NULL;
    for (i = 0; i < protocol->fieldCount; i++) {
        FieldDefinition *fieldDef = &protocol->fields[i];
        DecodedField *field = &fields[i];
        field->name = strdup(fieldDef->name);
        field->type = fieldDef->type;
        field->rawData = NULL;
        field->rawLength = 0;
        if (fieldExtract_checkBounds(packetData, packetLength, fieldDef)) {
            char *val = fieldExtract_toString(packetData, packetLength, fieldDef);
            field->value = val ? strdup(val) : strdup("<error>");
        } else {
            field->value = strdup("<truncated>");
        }
    }
    return fields;
}

DecodedProtocol *decoder_decodePacket(DecoderContext *ctx, uint8_t *packetData, uint32_t packetLength, uint32_t headerProtocol) {
    ProtocolDefinition *protocol = NULL;
    uint32_t startOffset = 0;
    if (!ctx || !ctx->initialized || !packetData) return NULL;
    switch (headerProtocol) {
        case 1: protocol = protocolDatabase_findByName(ctx->protocols, "ethernet"); break;
        case 2: protocol = protocolDatabase_findByName(ctx->protocols, "ipv4"); startOffset = 0; break;
        case 3: protocol = protocolDatabase_findByName(ctx->protocols, "ipv6"); startOffset = 0; break;
        default:
            protocol = protocolDatabase_findById(ctx->protocols, headerProtocol, PROTOCOL_LAYER_LINK);
            if (!protocol) protocol = protocolDatabase_findById(ctx->protocols, headerProtocol, PROTOCOL_LAYER_NETWORK);
            break;
    }
    if (!protocol) return NULL;
    return decoder_decodeFromProtocol(ctx, packetData, packetLength, protocol, startOffset);
}

DecodedProtocol *decoder_decodeFromProtocol(DecoderContext *ctx, uint8_t *packetData, uint32_t packetLength, ProtocolDefinition *protocol, uint32_t startOffset) {
    DecodedProtocol *result;
    DecodedField *fields;
    uint32_t i;
    if (!ctx || !packetData || !protocol) return NULL;
    result = malloc(sizeof(DecodedProtocol));
    if (!result) return NULL;
    memset(result, 0, sizeof(DecodedProtocol));
    result->protocolName = strdup(protocol->name);
    result->definition = protocol;
    result->startOffset = startOffset;
    fields = decodeFields(protocol, packetData, packetLength, startOffset);
    if (fields) {
        result->fields = fields;
        result->fieldCount = protocol->fieldCount;
        for (i = 0; i < protocol->fieldCount; i++) {
            uint32_t fieldEnd = protocol->fields[i].offset + protocol->fields[i].length;
            if (fieldEnd > result->length) result->length = fieldEnd;
        }
    }
    if (protocol->nextProtocolCount > 0 && fields) {
        uint32_t nextProtoId = 0;
        uint32_t nextStartOffset = result->length;
        for (i = 0; i < protocol->fieldCount; i++) {
            if (strcmp(protocol->fields[i].name, "ethernet_type") == 0 ||
                strcmp(protocol->fields[i].name, "protocol") == 0 ||
                strcmp(protocol->fields[i].name, "nextHeader") == 0) {
                nextProtoId = (uint32_t)fieldExtract_toUint64(packetData, packetLength, &protocol->fields[i]);
                break;
            }
        }
        if (nextProtoId > 0) {
            ProtocolLayer nextLayer = PROTOCOL_LAYER_NETWORK;
            if (protocol->layer == PROTOCOL_LAYER_LINK) nextLayer = PROTOCOL_LAYER_NETWORK;
            else if (protocol->layer == PROTOCOL_LAYER_NETWORK) nextLayer = PROTOCOL_LAYER_TRANSPORT;
            ProtocolDefinition *nextProto = protocolDatabase_findById(ctx->protocols, nextProtoId, nextLayer);
            if (nextProto) {
                DecodedProtocol *child = decoder_decodeFromProtocol(ctx, packetData, packetLength, nextProto, nextStartOffset);
                if (child) {
                    result->children = child;
                    result->childCount = 1;
                }
            }
        }
    }
    return result;
}

void decoder_freeResult(DecodedProtocol *result) {
    uint32_t i;
    if (!result) return;
    if (result->protocolName) free(result->protocolName);
    if (result->fields) {
        for (i = 0; i < result->fieldCount; i++) {
            if (result->fields[i].name) free(result->fields[i].name);
            if (result->fields[i].value) free(result->fields[i].value);
            if (result->fields[i].rawData) free(result->fields[i].rawData);
        }
        free(result->fields);
    }
    if (result->children) {
        for (i = 0; i < result->childCount; i++) decoder_freeResult(&result->children[i]);
        free(result->children);
    }
    free(result);
}

void decoder_outputResult(void *sample, DecodedProtocol *result) {
    uint32_t i;
    (void)sample;
    if (!result) return;
    if (result->protocolName) { /* sf_logf(sample, "protocol", result->protocolName); */ }
    for (i = 0; i < result->fieldCount; i++) {
        DecodedField *field = &result->fields[i];
        if (field->name && field->value) { /* sf_logf(sample, field->name, field->value); */ }
    }
    for (i = 0; i < result->childCount; i++) decoder_outputResult(sample, &result->children[i]);
}
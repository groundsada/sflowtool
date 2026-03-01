/*
 * decoder.h - Public API for JSON-driven packet decoder
 */

#ifndef SFLOW_DECODER_H
#define SFLOW_DECODER_H

#include <stdint.h>
#include <stdbool.h>
#include "protocol.h"

/* Decoded field value */
typedef struct {
    char *name;
    char *value;
    uint8_t *rawData;
    uint32_t rawLength;
    FieldType type;
} DecodedField;

/* Decoded protocol result */
typedef struct DecodedProtocol_ {
    char *protocolName;
    ProtocolDefinition *definition;
    DecodedField *fields;
    uint32_t fieldCount;
    struct DecodedProtocol_ *children;
    uint32_t childCount;
    uint32_t startOffset;
    uint32_t length;
} DecodedProtocol;

/* Decoder context */
typedef struct {
    ProtocolDatabase *protocols;
    bool initialized;
    char errorBuffer[256];
} DecoderContext;

/* Decoder API */
int decoder_init(DecoderContext *ctx, const char *protocolsFile);
void decoder_free(DecoderContext *ctx);
DecodedProtocol *decoder_decodePacket(DecoderContext *ctx, uint8_t *packetData, uint32_t packetLength, uint32_t headerProtocol);
DecodedProtocol *decoder_decodeFromProtocol(DecoderContext *ctx, uint8_t *packetData, uint32_t packetLength, ProtocolDefinition *protocol, uint32_t startOffset);
void decoder_freeResult(DecodedProtocol *result);
void decoder_outputResult(void *sample, DecodedProtocol *result);

#endif /* SFLOW_DECODER_H */
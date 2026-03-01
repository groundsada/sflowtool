/*
 * field_extract.c - Field extraction from raw packet data
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>
#include "../include/protocol.h"

#define MAX_STRING_LEN 256

char *fieldExtract_toString(uint8_t *packetData, uint32_t packetLength, FieldDefinition *field) {
    static char buffer[MAX_STRING_LEN];
    uint32_t offset, len;
    if (!packetData || !field) return NULL;
    offset = field->offset;
    len = field->length;
    if (offset + len > packetLength) return NULL;
    buffer[0] = '\0';
    switch (field->type) {
        case FIELD_TYPE_UINT8:
            snprintf(buffer, sizeof(buffer), "%u", packetData[offset]);
            break;
        case FIELD_TYPE_UINT16: {
            uint16_t val;
            if (len < 2) len = 2;
            memcpy(&val, packetData + offset, sizeof(val));
            if (field->networkByteOrder) val = ntohs(val);
            snprintf(buffer, sizeof(buffer), "%u", val);
            break;
        }
        case FIELD_TYPE_UINT32: {
            uint32_t val;
            if (len < 4) len = 4;
            memcpy(&val, packetData + offset, sizeof(val));
            if (field->networkByteOrder) val = ntohl(val);
            snprintf(buffer, sizeof(buffer), "%u", val);
            break;
        }
        case FIELD_TYPE_UINT64: {
            uint64_t val;
            if (len < 8) len = 8;
            memcpy(&val, packetData + offset, sizeof(val));
            snprintf(buffer, sizeof(buffer), "%llu", (unsigned long long)val);
            break;
        }
        case FIELD_TYPE_MAC:
            if (len < 6) len = 6;
            snprintf(buffer, sizeof(buffer), "%02x:%02x:%02x:%02x:%02x:%02x",
                packetData[offset + 0], packetData[offset + 1], packetData[offset + 2],
                packetData[offset + 3], packetData[offset + 4], packetData[offset + 5]);
            break;
        case FIELD_TYPE_IPV4:
            if (len < 4) len = 4;
            snprintf(buffer, sizeof(buffer), "%u.%u.%u.%u",
                packetData[offset + 0], packetData[offset + 1], packetData[offset + 2], packetData[offset + 3]);
            break;
        case FIELD_TYPE_IPV6:
            if (len < 16) len = 16;
            snprintf(buffer, sizeof(buffer),
                "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
                packetData[offset + 0], packetData[offset + 1], packetData[offset + 2], packetData[offset + 3],
                packetData[offset + 4], packetData[offset + 5], packetData[offset + 6], packetData[offset + 7],
                packetData[offset + 8], packetData[offset + 9], packetData[offset + 10], packetData[offset + 11],
                packetData[offset + 12], packetData[offset + 13], packetData[offset + 14], packetData[offset + 15]);
            break;
        case FIELD_TYPE_STRING: {
            uint32_t copyLen = (len < MAX_STRING_LEN - 1) ? len : (MAX_STRING_LEN - 1);
            memcpy(buffer, packetData + offset, copyLen);
            buffer[copyLen] = '\0';
            break;
        }
        case FIELD_TYPE_BYTES: {
            uint32_t i, maxBytes = (len < 16) ? len : 16;
            char *ptr = buffer;
            for (i = 0; i < maxBytes; i++) {
                if (i > 0) *ptr++ = ':';
                snprintf(ptr, 4, "%02x", packetData[offset + i]);
                ptr += 2;
            }
            *ptr = '\0';
            break;
        }
        default:
            break;
    }
    return buffer;
}

uint64_t fieldExtract_toUint64(uint8_t *packetData, uint32_t packetLength, FieldDefinition *field) {
    uint64_t val = 0;
    uint32_t offset, len;
    if (!packetData || !field) return 0;
    offset = field->offset;
    len = field->length;
    if (offset + len > packetLength) return 0;
    switch (field->type) {
        case FIELD_TYPE_UINT8:
            val = packetData[offset];
            break;
        case FIELD_TYPE_UINT16: {
            uint16_t v;
            memcpy(&v, packetData + offset, sizeof(v));
            val = field->networkByteOrder ? ntohs(v) : v;
            break;
        }
        case FIELD_TYPE_UINT32: {
            uint32_t v;
            memcpy(&v, packetData + offset, sizeof(v));
            val = field->networkByteOrder ? ntohl(v) : v;
            break;
        }
        case FIELD_TYPE_UINT64: {
            uint64_t v;
            memcpy(&v, packetData + offset, sizeof(v));
            val = v;
            break;
        }
        default:
            break;
    }
    return val;
}

int fieldExtract_checkBounds(uint8_t *packetData, uint32_t packetLength, FieldDefinition *field) {
    if (!packetData || !field) return 0;
    return (field->offset + field->length <= packetLength);
}
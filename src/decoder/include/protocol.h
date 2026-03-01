/*
 * protocol.h - Protocol definition structures for JSON-driven decoder
 */

#ifndef SFLOW_DECODER_PROTOCOL_H
#define SFLOW_DECODER_PROTOCOL_H

#include <stdint.h>
#include <stdbool.h>

/* Field types supported by the decoder */
typedef enum {
    FIELD_TYPE_UINT8,
    FIELD_TYPE_UINT16,
    FIELD_TYPE_UINT32,
    FIELD_TYPE_UINT64,
    FIELD_TYPE_MAC,
    FIELD_TYPE_IPV4,
    FIELD_TYPE_IPV6,
    FIELD_TYPE_STRING,
    FIELD_TYPE_BYTES
} FieldType;

/* Protocol layers */
typedef enum {
    PROTOCOL_LAYER_LINK,
    PROTOCOL_LAYER_NETWORK,
    PROTOCOL_LAYER_TRANSPORT,
    PROTOCOL_LAYER_APPLICATION
} ProtocolLayer;

/* sFlow protocol enumeration for mapping */
typedef enum {
    SFLOW_PROTOCOL_ETHERNET,
    SFLOW_PROTOCOL_IPV4,
    SFLOW_PROTOCOL_IPV6,
    SFLOW_PROTOCOL_TCP,
    SFLOW_PROTOCOL_UDP,
    SFLOW_PROTOCOL_ICMP,
    SFLOW_PROTOCOL_VXLAN,
    SFLOW_PROTOCOL_UNKNOWN
} SFlowProtocol;

/* Field definition from JSON */
typedef struct {
    char *name;
    uint32_t offset;
    uint32_t length;
    FieldType type;
    bool networkByteOrder;
    char *description;
} FieldDefinition;

/* Next protocol mapping rule */
typedef struct {
    char *fieldName;
    uint32_t expectedValue;
    char *targetProtocol;
    uint32_t offset;
} NextProtocolRule;

/* Protocol definition loaded from JSON */
typedef struct {
    char *name;
    uint32_t id;
    SFlowProtocol sflowProtocol;
    ProtocolLayer layer;
    FieldDefinition *fields;
    uint32_t fieldCount;
    NextProtocolRule *nextProtocols;
    uint32_t nextProtocolCount;
} ProtocolDefinition;

/* Protocol database */
typedef struct {
    ProtocolDefinition *protocols;
    uint32_t protocolCount;
} ProtocolDatabase;

/* Protocol definition API */
void protocolDatabase_init(ProtocolDatabase *db);
void protocolDatabase_free(ProtocolDatabase *db);
ProtocolDefinition *protocolDatabase_findById(ProtocolDatabase *db, uint32_t id, ProtocolLayer layer);
ProtocolDefinition *protocolDatabase_findByName(ProtocolDatabase *db, const char *name);
int protocolDatabase_addProtocol(ProtocolDatabase *db, ProtocolDefinition *protocol);

/* Helper functions for string to enum conversion */
FieldType fieldType_fromString(const char *typeStr);
ProtocolLayer protocolLayer_fromString(const char *layerStr);
SFlowProtocol sflowProtocol_fromString(const char *protoStr);

#endif /* SFLOW_DECODER_PROTOCOL_H */
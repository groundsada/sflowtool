/*
 * protocol.c - Protocol management implementation
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../include/protocol.h"

void protocolDatabase_init(ProtocolDatabase *db) {
    if (!db) return;
    db->protocols = NULL;
    db->protocolCount = 0;
}

void protocolDatabase_free(ProtocolDatabase *db) {
    uint32_t i, j;
    if (!db) return;
    if (db->protocols) {
        for (i = 0; i < db->protocolCount; i++) {
            ProtocolDefinition *proto = &db->protocols[i];
            if (proto->name) free(proto->name);
            if (proto->fields) {
                for (j = 0; j < proto->fieldCount; j++) {
                    if (proto->fields[j].name) free(proto->fields[j].name);
                    if (proto->fields[j].description) free(proto->fields[j].description);
                }
                free(proto->fields);
            }
            if (proto->nextProtocols) {
                for (j = 0; j < proto->nextProtocolCount; j++) {
                    if (proto->nextProtocols[j].fieldName) free(proto->nextProtocols[j].fieldName);
                    if (proto->nextProtocols[j].targetProtocol) free(proto->nextProtocols[j].targetProtocol);
                }
                free(proto->nextProtocols);
            }
        }
        free(db->protocols);
    }
    db->protocols = NULL;
    db->protocolCount = 0;
}

ProtocolDefinition *protocolDatabase_findById(ProtocolDatabase *db, uint32_t id, ProtocolLayer layer) {
    uint32_t i;
    if (!db) return NULL;
    for (i = 0; i < db->protocolCount; i++) {
        if (db->protocols[i].id == id && db->protocols[i].layer == layer) {
            return &db->protocols[i];
        }
    }
    return NULL;
}

ProtocolDefinition *protocolDatabase_findByName(ProtocolDatabase *db, const char *name) {
    uint32_t i;
    if (!db || !name) return NULL;
    for (i = 0; i < db->protocolCount; i++) {
        if (db->protocols[i].name && strcmp(db->protocols[i].name, name) == 0) {
            return &db->protocols[i];
        }
    }
    return NULL;
}

int protocolDatabase_addProtocol(ProtocolDatabase *db, ProtocolDefinition *protocol) {
    ProtocolDefinition *newProtocols;
    if (!db || !protocol) return -1;
    newProtocols = realloc(db->protocols, sizeof(ProtocolDefinition) * (db->protocolCount + 1));
    if (!newProtocols) return -1;
    db->protocols = newProtocols;
    memcpy(&db->protocols[db->protocolCount], protocol, sizeof(ProtocolDefinition));
    db->protocolCount++;
    return 0;
}

FieldType fieldType_fromString(const char *typeStr) {
    if (!typeStr) return FIELD_TYPE_UINT8;
    if (strcmp(typeStr, "uint8") == 0) return FIELD_TYPE_UINT8;
    if (strcmp(typeStr, "uint16") == 0) return FIELD_TYPE_UINT16;
    if (strcmp(typeStr, "uint32") == 0) return FIELD_TYPE_UINT32;
    if (strcmp(typeStr, "uint64") == 0) return FIELD_TYPE_UINT64;
    if (strcmp(typeStr, "mac") == 0) return FIELD_TYPE_MAC;
    if (strcmp(typeStr, "ipv4") == 0) return FIELD_TYPE_IPV4;
    if (strcmp(typeStr, "ipv6") == 0) return FIELD_TYPE_IPV6;
    if (strcmp(typeStr, "string") == 0) return FIELD_TYPE_STRING;
    if (strcmp(typeStr, "bytes") == 0) return FIELD_TYPE_BYTES;
    return FIELD_TYPE_UINT8;
}

ProtocolLayer protocolLayer_fromString(const char *layerStr) {
    if (!layerStr) return PROTOCOL_LAYER_LINK;
    if (strcmp(layerStr, "link") == 0) return PROTOCOL_LAYER_LINK;
    if (strcmp(layerStr, "network") == 0) return PROTOCOL_LAYER_NETWORK;
    if (strcmp(layerStr, "transport") == 0) return PROTOCOL_LAYER_TRANSPORT;
    if (strcmp(layerStr, "application") == 0) return PROTOCOL_LAYER_APPLICATION;
    return PROTOCOL_LAYER_LINK;
}

SFlowProtocol sflowProtocol_fromString(const char *protoStr) {
    if (!protoStr) return SFLOW_PROTOCOL_UNKNOWN;
    if (strcmp(protoStr, "ethernet") == 0) return SFLOW_PROTOCOL_ETHERNET;
    if (strcmp(protoStr, "ipv4") == 0) return SFLOW_PROTOCOL_IPV4;
    if (strcmp(protoStr, "ipv6") == 0) return SFLOW_PROTOCOL_IPV6;
    if (strcmp(protoStr, "tcp") == 0) return SFLOW_PROTOCOL_TCP;
    if (strcmp(protoStr, "udp") == 0) return SFLOW_PROTOCOL_UDP;
    if (strcmp(protoStr, "icmp") == 0) return SFLOW_PROTOCOL_ICMP;
    if (strcmp(protoStr, "vxlan") == 0) return SFLOW_PROTOCOL_VXLAN;
    return SFLOW_PROTOCOL_UNKNOWN;
}
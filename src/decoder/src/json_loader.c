/*
 * json_loader.c - JSON protocol definition loader using cJSON
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <cJSON.h>
#include "../include/protocol.h"
#include "../include/json_loader.h"

static char g_errorBuffer[256] = {0};

const char *jsonLoader_getError(void) {
    return g_errorBuffer;
}

static void setError(const char *msg) {
    strncpy(g_errorBuffer, msg, sizeof(g_errorBuffer) - 1);
    g_errorBuffer[sizeof(g_errorBuffer) - 1] = '\0';
}

static int parseField(cJSON *fieldJson, FieldDefinition *field) {
    cJSON *item;
    memset(field, 0, sizeof(FieldDefinition));
    item = cJSON_GetObjectItem(fieldJson, "name");
    if (!item || !cJSON_IsString(item)) { setError("Field missing 'name'"); return -1; }
    field->name = strdup(item->valuestring);
    item = cJSON_GetObjectItem(fieldJson, "offset");
    if (!item || !cJSON_IsNumber(item)) { setError("Field missing 'offset'"); return -1; }
    field->offset = item->valueint;
    item = cJSON_GetObjectItem(fieldJson, "length");
    if (!item || !cJSON_IsNumber(item)) { setError("Field missing 'length'"); return -1; }
    field->length = item->valueint;
    item = cJSON_GetObjectItem(fieldJson, "type");
    field->type = (item && cJSON_IsString(item)) ? fieldType_fromString(item->valuestring) : FIELD_TYPE_UINT8;
    item = cJSON_GetObjectItem(fieldJson, "networkByteOrder");
    field->networkByteOrder = (item && cJSON_IsBool(item) && cJSON_IsTrue(item));
    item = cJSON_GetObjectItem(fieldJson, "description");
    if (item && cJSON_IsString(item)) field->description = strdup(item->valuestring);
    return 0;
}

static int parseNextProtocol(cJSON *ruleJson, NextProtocolRule *rule) {
    cJSON *item;
    memset(rule, 0, sizeof(NextProtocolRule));
    item = cJSON_GetObjectItem(ruleJson, "fieldName");
    if (!item || !cJSON_IsString(item)) { setError("NextProtocolRule missing 'fieldName'"); return -1; }
    rule->fieldName = strdup(item->valuestring);
    item = cJSON_GetObjectItem(ruleJson, "expectedValue");
    if (!item || !cJSON_IsNumber(item)) { setError("NextProtocolRule missing 'expectedValue'"); return -1; }
    rule->expectedValue = item->valueint;
    item = cJSON_GetObjectItem(ruleJson, "targetProtocol");
    if (!item || !cJSON_IsString(item)) { setError("NextProtocolRule missing 'targetProtocol'"); return -1; }
    rule->targetProtocol = strdup(item->valuestring);
    item = cJSON_GetObjectItem(ruleJson, "offset");
    rule->offset = (item && cJSON_IsNumber(item)) ? item->valueint : 0;
    return 0;
}

static int parseProtocol(cJSON *protoJson, ProtocolDefinition *protocol) {
    cJSON *item, *fieldArray, *nextProtoArray;
    uint32_t i;
    memset(protocol, 0, sizeof(ProtocolDefinition));
    item = cJSON_GetObjectItem(protoJson, "name");
    if (!item || !cJSON_IsString(item)) { setError("Protocol missing 'name'"); return -1; }
    protocol->name = strdup(item->valuestring);
    item = cJSON_GetObjectItem(protoJson, "id");
    if (!item || !cJSON_IsNumber(item)) { setError("Protocol missing 'id'"); return -1; }
    protocol->id = item->valueint;
    item = cJSON_GetObjectItem(protoJson, "sflowProtocol");
    protocol->sflowProtocol = (item && cJSON_IsString(item)) ? sflowProtocol_fromString(item->valuestring) : SFLOW_PROTOCOL_UNKNOWN;
    item = cJSON_GetObjectItem(protoJson, "layer");
    protocol->layer = (item && cJSON_IsString(item)) ? protocolLayer_fromString(item->valuestring) : PROTOCOL_LAYER_LINK;
    fieldArray = cJSON_GetObjectItem(protoJson, "fields");
    if (fieldArray && cJSON_IsArray(fieldArray)) {
        protocol->fieldCount = cJSON_GetArraySize(fieldArray);
        if (protocol->fieldCount > 0) {
            protocol->fields = malloc(sizeof(FieldDefinition) * protocol->fieldCount);
            if (!protocol->fields) { setError("Memory allocation failed for fields"); return -1; }
            for (i = 0; i < protocol->fieldCount; i++) {
                if (parseField(cJSON_GetArrayItem(fieldArray, i), &protocol->fields[i]) != 0) return -1;
            }
        }
    }
    nextProtoArray = cJSON_GetObjectItem(protoJson, "nextProtocols");
    if (nextProtoArray && cJSON_IsArray(nextProtoArray)) {
        protocol->nextProtocolCount = cJSON_GetArraySize(nextProtoArray);
        if (protocol->nextProtocolCount > 0) {
            protocol->nextProtocols = malloc(sizeof(NextProtocolRule) * protocol->nextProtocolCount);
            if (!protocol->nextProtocols) { setError("Memory allocation failed for nextProtocols"); return -1; }
            for (i = 0; i < protocol->nextProtocolCount; i++) {
                if (parseNextProtocol(cJSON_GetArrayItem(nextProtoArray, i), &protocol->nextProtocols[i]) != 0) return -1;
            }
        }
    }
    return 0;
}

int jsonLoader_loadString(ProtocolDatabase *db, const char *jsonString) {
    cJSON *root, *protocolsArray, *protocolJson;
    uint32_t i;
    int result = -1;
    if (!db || !jsonString) { setError("Invalid parameters"); return -1; }
    root = cJSON_Parse(jsonString);
    if (!root) { snprintf(g_errorBuffer, sizeof(g_errorBuffer), "JSON parse error: %s", cJSON_GetErrorPtr()); return -1; }
    protocolsArray = cJSON_GetObjectItem(root, "protocols");
    if (!protocolsArray || !cJSON_IsArray(protocolsArray)) { setError("Missing or invalid 'protocols' array"); goto cleanup; }
    db->protocolCount = cJSON_GetArraySize(protocolsArray);
    if (db->protocolCount == 0) { setError("No protocols defined"); goto cleanup; }
    db->protocols = malloc(sizeof(ProtocolDefinition) * db->protocolCount);
    if (!db->protocols) { setError("Memory allocation failed"); goto cleanup; }
    for (i = 0; i < db->protocolCount; i++) {
        protocolJson = cJSON_GetArrayItem(protocolsArray, i);
        if (parseProtocol(protocolJson, &db->protocols[i]) != 0) goto cleanup;
    }
    result = 0;
cleanup:
    cJSON_Delete(root);
    return result;
}

int jsonLoader_loadFile(ProtocolDatabase *db, const char *filename) {
    FILE *fp;
    char *jsonString;
    long fileSize;
    int result;
    if (!db || !filename) { setError("Invalid parameters"); return -1; }
    fp = fopen(filename, "r");
    if (!fp) { snprintf(g_errorBuffer, sizeof(g_errorBuffer), "Failed to open file %s: %s", filename, strerror(errno)); return -1; }
    fseek(fp, 0, SEEK_END);
    fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (fileSize <= 0) { setError("Invalid file size"); fclose(fp); return -1; }
    jsonString = malloc(fileSize + 1);
    if (!jsonString) { setError("Memory allocation failed"); fclose(fp); return -1; }
    if (fread(jsonString, 1, fileSize, fp) != (size_t)fileSize) { setError("Failed to read file"); free(jsonString); fclose(fp); return -1; }
    jsonString[fileSize] = '\0';
    fclose(fp);
    result = jsonLoader_loadString(db, jsonString);
    free(jsonString);
    return result;
}
/*
 * json_loader.h - JSON loading interface for protocol definitions
 */

#ifndef SFLOW_DECODER_JSON_LOADER_H
#define SFLOW_DECODER_JSON_LOADER_H

#include "protocol.h"

int jsonLoader_loadFile(ProtocolDatabase *db, const char *filename);
int jsonLoader_loadString(ProtocolDatabase *db, const char *jsonString);
const char *jsonLoader_getError(void);

#endif /* SFLOW_DECODER_JSON_LOADER_H */
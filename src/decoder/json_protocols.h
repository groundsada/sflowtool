/*
 * json_protocols.h - Header for embedded JSON protocols
 */

#ifndef SFLOW_DECODER_JSON_PROTOCOLS_H
#define SFLOW_DECODER_JSON_PROTOCOLS_H

#include "include/protocol.h"

int json_protocols_load(ProtocolDatabase *db);
const char *json_protocols_getDefault(void);

#endif /* SFLOW_DECODER_JSON_PROTOCOLS_H */
/*
 * json_protocols.c - JSON protocol definitions embedded
 */

#include "include/protocol.h"
#include "include/json_loader.h"

static const char *default_protocols_json = R"({
  "protocols": [
    {"name": "ethernet", "id": 1, "sflowProtocol": "ethernet", "layer": "link",
      "fields": [
        {"name": "dstMAC", "offset": 0, "length": 6, "type": "mac"},
        {"name": "srcMAC", "offset": 6, "length": 6, "type": "mac"},
        {"name": "ethernet_type", "offset": 12, "length": 2, "type": "uint16", "networkByteOrder": true}
      ],
      "nextProtocols": [
        {"fieldName": "ethernet_type", "expectedValue": 2048, "targetProtocol": "ipv4", "offset": 14},
        {"fieldName": "ethernet_type", "expectedValue": 34525, "targetProtocol": "ipv6", "offset": 14}
      ]
    },
    {"name": "ipv4", "id": 2048, "sflowProtocol": "ipv4", "layer": "network",
      "fields": [
        {"name": "version_ihl", "offset": 0, "length": 1, "type": "uint8"},
        {"name": "tos", "offset": 1, "length": 1, "type": "uint8"},
        {"name": "total_length", "offset": 2, "length": 2, "type": "uint16", "networkByteOrder": true},
        {"name": "ttl", "offset": 8, "length": 1, "type": "uint8"},
        {"name": "protocol", "offset": 9, "length": 1, "type": "uint8"},
        {"name": "srcIP", "offset": 12, "length": 4, "type": "ipv4"},
        {"name": "dstIP", "offset": 16, "length": 4, "type": "ipv4"}
      ],
      "nextProtocols": [
        {"fieldName": "protocol", "expectedValue": 6, "targetProtocol": "tcp", "offset": 20},
        {"fieldName": "protocol", "expectedValue": 17, "targetProtocol": "udp", "offset": 20}
      ]
    },
    {"name": "ipv6", "id": 34525, "sflowProtocol": "ipv6", "layer": "network",
      "fields": [
        {"name": "version_class_flow", "offset": 0, "length": 4, "type": "uint32"},
        {"name": "payload_length", "offset": 4, "length": 2, "type": "uint16", "networkByteOrder": true},
        {"name": "nextHeader", "offset": 6, "length": 1, "type": "uint8"},
        {"name": "hopLimit", "offset": 7, "length": 1, "type": "uint8"},
        {"name": "srcIP6", "offset": 8, "length": 16, "type": "ipv6"},
        {"name": "dstIP6", "offset": 24, "length": 16, "type": "ipv6"}
      ],
      "nextProtocols": [
        {"fieldName": "nextHeader", "expectedValue": 6, "targetProtocol": "tcp", "offset": 40},
        {"fieldName": "nextHeader", "expectedValue": 17, "targetProtocol": "udp", "offset": 40}
      ]
    },
    {"name": "tcp", "id": 6, "sflowProtocol": "tcp", "layer": "transport",
      "fields": [
        {"name": "srcPort", "offset": 0, "length": 2, "type": "uint16", "networkByteOrder": true},
        {"name": "dstPort", "offset": 2, "length": 2, "type": "uint16", "networkByteOrder": true},
        {"name": "seqNumber", "offset": 4, "length": 4, "type": "uint32", "networkByteOrder": true},
        {"name": "ackNumber", "offset": 8, "length": 4, "type": "uint32", "networkByteOrder": true},
        {"name": "flags", "offset": 13, "length": 1, "type": "uint8"},
        {"name": "window", "offset": 14, "length": 2, "type": "uint16", "networkByteOrder": true}
      ],
      "nextProtocols": []
    },
    {"name": "udp", "id": 17, "sflowProtocol": "udp", "layer": "transport",
      "fields": [
        {"name": "srcPort", "offset": 0, "length": 2, "type": "uint16", "networkByteOrder": true},
        {"name": "dstPort", "offset": 2, "length": 2, "type": "uint16", "networkByteOrder": true},
        {"name": "length", "offset": 4, "length": 2, "type": "uint16", "networkByteOrder": true},
        {"name": "checksum", "offset": 6, "length": 2, "type": "uint16", "networkByteOrder": true}
      ],
      "nextProtocols": []
    },
    {"name": "icmp", "id": 1, "sflowProtocol": "icmp", "layer": "transport",
      "fields": [
        {"name": "type", "offset": 0, "length": 1, "type": "uint8"},
        {"name": "code", "offset": 1, "length": 1, "type": "uint8"},
        {"name": "checksum", "offset": 2, "length": 2, "type": "uint16", "networkByteOrder": true}
      ],
      "nextProtocols": []
    }
  ]
})";

int json_protocols_load(ProtocolDatabase *db) {
    return jsonLoader_loadString(db, default_protocols_json);
}

const char *json_protocols_getDefault(void) {
    return default_protocols_json;
}
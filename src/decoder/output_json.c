/*
 * output_json.c - JSON output formatting for decoded packets
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/decoder.h"

void decoder_outputJSON(void *sample, DecodedProtocol *result) {
    (void)sample;
    (void)result;
}

int decoder_formatJSON(DecodedProtocol *result, char **outputBuffer) {
    (void)result;
    (void)outputBuffer;
    return -1;
}
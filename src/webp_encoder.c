#include "webp_encoder.h"

#include "webp/encode.h"

#include <stdio.h>

int wpGetEncoderVersion() {
  return WebPGetEncoderVersion();
}

unsigned char* wpEncode(const char* raw_data, int width, int height, int* encoded_data_size) {
    uint8_t* encodede_buffer = NULL; 
    size_t raw_data_size = WebPEncodeRGBA((const uint8_t*) raw_data, width, height, width * 4, 50, &encodede_buffer);

    //printf("Encoded address %p %d\n", encodede_buffer, encodede_buffer);
    //printf("Encoded address %d\n", raw_data_size);

    *encoded_data_size = (int)raw_data_size;

    return encodede_buffer;
}

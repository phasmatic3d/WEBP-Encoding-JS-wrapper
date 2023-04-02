#include "webp_decoder.h"

#include "webp/decode.h"

#include <stdio.h>

int wpGetDecoderVersion() {
  return WebPGetDecoderVersion();
}

int wpGetWidth(const char* encoded_data, unsigned int encoded_data_size) {
  int width = -1;
  int height = -1;
  int ok = WebPGetInfo((const uint8_t*) encoded_data, (size_t) encoded_data_size, &width, &height);
  if (!ok) {
    return -1;
  }
  return width;
}

int wpGetHeight(const char* encoded_data, unsigned int encoded_data_size) {
  int width = -1;
  int height = -1;
  int ok = WebPGetInfo((const uint8_t*) encoded_data, (size_t) encoded_data_size, &width, &height);
  if (!ok) {
    return -1;
  }
  return height;
}

int wpDecode(const char* encoded_data, unsigned int encoded_data_size, unsigned char* decoded_data) {
  WebPBitstreamFeatures features = {0};
  VP8StatusCode status = VP8_STATUS_OK;
  int decoded_data_size;
  int ok = 0;
  int width = -1;
  int height = -1;
  
  ok = WebPGetInfo((const uint8_t*) encoded_data, (size_t) encoded_data_size, &width, &height);
  if (!ok) {
    return -1;
  }

  status = WebPGetFeatures((const uint8_t*) encoded_data, (size_t) encoded_data_size, &features);

  //printf("Width %d Height %d Alpha %d Animation %d Format %d\n", 
  //  features.width, features.height,
  //  features.has_alpha, features.has_animation, features.format
  //);

  decoded_data_size = width * height * 4;
  const uint8_t* ret = WebPDecodeRGBAInto(
      (const uint8_t*) encoded_data, encoded_data_size,
      decoded_data, decoded_data_size, width * 4);

  //printf("All well %p\n", ret);
  //const uint8_t* data = WebPDecodeRGBAInto((const uint8_t*)data, data_size,
  //                          output_buffer, 200 * 200 * 4, 200 *4);

  return width;
}
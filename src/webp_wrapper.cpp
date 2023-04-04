#include <emscripten/bind.h>
using namespace emscripten;

#include "webp/decode.h"
#include "webp/encode.h"

#include <stdio.h>

extern "C" int wpGetDecoderVersion() {
  return WebPGetDecoderVersion();
}

extern "C" int wpGetEncoderVersion() {
  return WebPGetEncoderVersion();
}

extern "C" int wpGetWidth(const uint8_t* encoded_data, unsigned int encoded_data_size) {
  int width = -1;
  int height = -1;
  int ok = WebPGetInfo(encoded_data, (size_t) encoded_data_size, &width, &height);
  if (!ok) {
    return -1;
  }
  return width;
}

extern "C" int wpGetHeight(const uint8_t* encoded_data, unsigned int encoded_data_size) {
  int width = -1;
  int height = -1;
  int ok = WebPGetInfo(encoded_data, (size_t) encoded_data_size, &width, &height);
  if (!ok) {
    return -1;
  }
  return height;
}

extern "C" uint8_t* wpEncode(const uint8_t* raw_data, int width, int height, int comps, float quality, int* encoded_data_size) {
  uint8_t* encodeded_buffer = NULL; 
  size_t raw_data_size = 0;
  if(comps == 3)
    raw_data_size = WebPEncodeRGB((const uint8_t*) raw_data, width, height, width * comps, quality, &encodeded_buffer);
  else if (comps == 4)
    raw_data_size = WebPEncodeRGBA((const uint8_t*) raw_data, width, height, width * comps, quality, &encodeded_buffer);
  else
    return encodeded_buffer;

  *encoded_data_size = (int)raw_data_size;

  return encodeded_buffer;
} 

extern "C" uint8_t* wpRescale(const uint8_t* raw_data, int width, int height, int comps, int scaled_width, int scaled_height) {
  uint8_t* raw_data_scaled = nullptr;
  int result = -1;
  WebPPicture pic;
  if (!WebPPictureInit(&pic)) return raw_data_scaled;  // version error
  pic.width = width;
  pic.height = height;
  pic.use_argb = 1;
  if (!WebPPictureAlloc(&pic)) return raw_data_scaled;   // memory error
  WebPPictureImportRGBA(&pic, raw_data, pic.argb_stride * comps);
  result = WebPPictureRescale(&pic, scaled_width, scaled_height);
  if (!result) return raw_data_scaled;
  
  raw_data_scaled = (uint8_t*)malloc(scaled_width * scaled_height * comps);
  const uint8_t* argb = (const uint8_t*)pic.argb;
  for (int y = 0; y < scaled_height; y++) {
    for (int x = 0; x < scaled_width; x++) {
      int index = comps * (y * scaled_width + x);
      raw_data_scaled[index + 0] = argb[index + 2];
      raw_data_scaled[index + 1] = argb[index + 1];
      raw_data_scaled[index + 2] = argb[index + 0];
      raw_data_scaled[index + 3] = argb[index + 3];
    }
  }

  WebPPictureFree(&pic);
  return raw_data_scaled;
} 

extern "C" int wpDecode(const uint8_t* encoded_data, unsigned int encoded_data_size, unsigned char* decoded_data) {
  WebPBitstreamFeatures features = {0};
  VP8StatusCode status = VP8_STATUS_OK;
  int decoded_data_size;
  int ok = 0;
  int width = -1;
  int height = -1;
  
  ok = WebPGetInfo(encoded_data, (size_t) encoded_data_size, &width, &height);
  if (!ok) {
    return -1;
  }

  status = WebPGetFeatures(encoded_data, (size_t) encoded_data_size, &features);

  decoded_data_size = width * height * 4;
  const uint8_t* ret = WebPDecodeRGBAInto(
      (const uint8_t*) encoded_data, encoded_data_size,
      decoded_data, decoded_data_size, width * 4);

  return width;
}
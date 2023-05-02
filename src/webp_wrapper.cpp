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

int getWidth(const emscripten::val& data) {
  std::vector<uint8_t> bytes{};
  bytes.resize(data["byteLength"].as<size_t>());

  emscripten::val memory = emscripten::val::module_property("HEAP8")["buffer"];
  emscripten::val memoryView = data["constructor"].new_(memory, reinterpret_cast<uintptr_t>(bytes.data()), data["length"].as<uint32_t>());
  memoryView.call<void>("set", data);
  int width = -1;
  int height = -1;
  int ok = WebPGetInfo(bytes.data(), bytes.size(), &width, &height);
  return (ok) ? width : -1;
}

int getHeight(const emscripten::val& data) {
  std::vector<uint8_t> bytes{};
  bytes.resize(data["byteLength"].as<size_t>());

  emscripten::val memory = emscripten::val::module_property("HEAP8")["buffer"];
  emscripten::val memoryView = data["constructor"].new_(memory, reinterpret_cast<uintptr_t>(bytes.data()), data["length"].as<uint32_t>());
  memoryView.call<void>("set", data);
  int width = -1;
  int height = -1;
  int ok = WebPGetInfo(bytes.data(), bytes.size(), &width, &height);
  return (ok) ? height : -1;
}

emscripten::val encode(const emscripten::val& data, int width, int height, int comps, float quality) {
  std::vector<uint8_t> bytes{};
  bytes.resize(data["byteLength"].as<size_t>());

  emscripten::val memory = emscripten::val::module_property("HEAP8")["buffer"];
  emscripten::val memoryView = data["constructor"].new_(memory, reinterpret_cast<uintptr_t>(bytes.data()), data["length"].as<uint32_t>());
  memoryView.call<void>("set", data);

  uint8_t* encoded_data = nullptr; 
  size_t encoded_data_size = 0;
  if(comps == 3)
    encoded_data_size = WebPEncodeRGB((const uint8_t*) bytes.data(), width, height, width * comps, quality, &encoded_data);
  else if (comps == 4)
    encoded_data_size = WebPEncodeRGBA((const uint8_t*) bytes.data(), width, height, width * comps, quality, &encoded_data);
  else
    return emscripten::val::array();

  return emscripten::val(
            emscripten::typed_memory_view(encoded_data_size,
                                          encoded_data));      
} 

emscripten::val rescale(const emscripten::val& data, int width, int height, int comps, int scaled_width, int scaled_height) {
  std::vector<uint8_t> bytes{};
  bytes.resize(data["byteLength"].as<size_t>());

  emscripten::val memory = emscripten::val::module_property("HEAP8")["buffer"];
  emscripten::val memoryView = data["constructor"].new_(memory, reinterpret_cast<uintptr_t>(bytes.data()), data["length"].as<uint32_t>());
  memoryView.call<void>("set", data);

  uint8_t* raw_data_scaled = nullptr;
  int result = -1;
  WebPPicture pic;
  if (!WebPPictureInit(&pic)) return emscripten::val::array();  // version error
  pic.width = width;
  pic.height = height;
  pic.use_argb = 1;
  if (!WebPPictureAlloc(&pic)) return emscripten::val::array();   // memory error
  WebPPictureImportRGBA(&pic, bytes.data(), pic.argb_stride * comps);
  result = WebPPictureRescale(&pic, scaled_width, scaled_height);
  if (!result) return emscripten::val::array();
  
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
  return emscripten::val(
            emscripten::typed_memory_view(scaled_width * scaled_height * comps,
                                          raw_data_scaled));      
} 

emscripten::val decode(const emscripten::val& data) {
  std::vector<uint8_t> bytes{};
  bytes.resize(data["byteLength"].as<size_t>());

  emscripten::val memory = emscripten::val::module_property("HEAP8")["buffer"];
  emscripten::val memoryView = data["constructor"].new_(memory, reinterpret_cast<uintptr_t>(bytes.data()), data["length"].as<uint32_t>());
  memoryView.call<void>("set", data);

  WebPBitstreamFeatures features = {0};
  VP8StatusCode status = VP8_STATUS_OK;
  int decoded_data_size;
  int ok = 0;
  int width = -1;
  int height = -1;
  
  ok = WebPGetInfo(bytes.data(), bytes.size(), &width, &height);
  if (!ok) {
    return emscripten::val::array();
  }

  status = WebPGetFeatures(bytes.data(), bytes.size(), &features);

  decoded_data_size = width * height * 4;
  uint8_t* decoded_data = (uint8_t*)malloc(decoded_data_size);
  const uint8_t* ret = WebPDecodeRGBAInto(
      (const uint8_t*) bytes.data(), bytes.size(),
      decoded_data, decoded_data_size, width * 4);
  return emscripten::val(
            emscripten::typed_memory_view(decoded_data_size,
                                          decoded_data));
}

namespace webp
{
  class texture
    {
      public:
        texture(texture&) = delete;
        texture(texture&& other) = default;
        
        static texture createFromMemory(const emscripten::val& data)
        {
            std::vector<uint8_t> bytes{};
            bytes.resize(data["byteLength"].as<size_t>());
            // Yes, this code IS copying the data. Sigh! According to Alon
            // Zakai:
            //     "There isn't a way to let compiled code access a new
            //     ArrayBuffer. The compiled code has hardcoded access to the
            //     wasm Memory it was instantiated with - all the pointers it
            //     can understand are indexes into that Memory. It can't refer
            //     to anything else, I'm afraid."
            //
            //     "In the future using different address spaces or techniques
            //     with reference types may open up some possibilities here."
            emscripten::val memory = emscripten::val::module_property("HEAP8")["buffer"];
            emscripten::val memoryView = data["constructor"].new_(memory, reinterpret_cast<uintptr_t>(bytes.data()), data["length"].as<uint32_t>());
            memoryView.call<void>("set", data);

             WebPBitstreamFeatures features = {0};
            VP8StatusCode status = VP8_STATUS_OK;
            int decoded_data_size;
            int ok = 0;
            int width = -1;
            int height = -1;
            
            ok = WebPGetInfo(bytes.data(), bytes.size(), &width, &height);
            if (!ok) {
              return texture();
            }

            status = WebPGetFeatures(bytes.data(), bytes.size(), &features);

            decoded_data_size = width * height * 4;
            uint8_t* decoded_data = (uint8_t*)malloc(decoded_data_size);
            const uint8_t* ret = WebPDecodeRGBAInto(
                (const uint8_t*) bytes.data(), bytes.size(),
                decoded_data, decoded_data_size, width * 4);

            return texture(decoded_data, width, height, 4);
        }

        uint32_t getWidth() const { return m_width; }
        uint32_t getHeight() const { return m_height; }
        uint32_t getComponentCount() const { return m_comps; }
        emscripten::val getData() const { 
          return emscripten::val(
            emscripten::typed_memory_view(m_width * m_height * m_comps,
                                          m_data));
        }
      private:
        texture() : texture(nullptr, -1, -1, 0) {}
        texture(uint8_t* data, uint32_t width, uint32_t height, uint32_t comps)
            : m_data { data }, m_width{ width }, m_height{ height }, m_comps { comps } { }
        uint32_t m_width;
        uint32_t m_height;
        uint32_t m_comps;
        uint8_t* m_data;
    };

}
EMSCRIPTEN_BINDINGS(WebpLibrary) {
  class_<webp::texture>("webpTexture")
        .constructor(&webp::texture::createFromMemory)
        .property("data", &webp::texture::getData)
        .property("width", &webp::texture::getWidth)
        .property("height", &webp::texture::getHeight)
        .property("comps", &webp::texture::getComponentCount)
    ;

    function("encode", &encode);
    function("rescale", &rescale);
    function("getWidth", &getWidth);
    function("getHeight", &getHeight);
}
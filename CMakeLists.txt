if(APPLE)
  cmake_minimum_required(VERSION 3.17)
else()
  cmake_minimum_required(VERSION 3.7)
endif()

if(POLICY CMP0072)
  cmake_policy(SET CMP0072 NEW)
endif()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libwebp ${CMAKE_CURRENT_SOURCE_DIR}/libwebp/build)

project(libwebp)

set(
  LIBWEP_EXPORTED_FUNCTIONS "_WebPFree,_WebPMalloc,_wpRescale,_wpEncode,_wpGetEncoderVersion,_wpDecode,_wpGetDecoderVersion,_wpGetHeight,_wpGetWidth,_malloc,_free" 
)

add_executable(libwebp src/webp_wrapper.cpp)
target_link_libraries(libwebp webp)
target_include_directories(libwebp PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_compile_definitions(libwebp PUBLIC EMSCRIPTEN)

set_target_properties(
    libwebp
    PROPERTIES LINK_FLAGS "-sWASM=1 \
       -sTOTAL_MEMORY=1024MB \
       -sEXPORTED_FUNCTIONS=${LIBWEP_EXPORTED_FUNCTIONS} -sINVOKE_RUN=0 \
       -sMODULARIZE=1 -sEXPORT_NAME=LIBWEBP \
       -sEXPORTED_RUNTIME_METHODS=cwrap,getValue")
set_target_properties(libwebp PROPERTIES OUTPUT_NAME libwebp)

add_custom_command(
        TARGET libwebp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory dist
                COMMENT "Creating 'dist' directory")

add_custom_command(
        TARGET libwebp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_BINARY_DIR}/libwebp.js
                ${CMAKE_CURRENT_SOURCE_DIR}/dist/libwebp.js
                COMMENT "Copying 'libwebp.js' library to 'dist/libwebp.js'")
add_custom_command(
        TARGET libwebp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_BINARY_DIR}/libwebp.wasm
                ${CMAKE_CURRENT_SOURCE_DIR}/dist/libwebp.wasm
                COMMENT "Copying 'libwebp.wasm' library to 'dist/libwebp.wasm'")


add_custom_command(
        TARGET libwebp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_BINARY_DIR}/libwebp.js
                ${CMAKE_CURRENT_SOURCE_DIR}/examples/libwebp.js
                COMMENT "Copying 'libwebp.js' library to 'examples/libwebp.js'")
add_custom_command(
        TARGET libwebp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_BINARY_DIR}/libwebp.wasm
                ${CMAKE_CURRENT_SOURCE_DIR}/examples/libwebp.wasm
                COMMENT "Copying 'libwebp.wasm' library to 'examples/libwebp.wasm'")

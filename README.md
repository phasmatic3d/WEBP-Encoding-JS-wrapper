# WEBP-Encoding-JS-wrapper

- git clone https://github.com/phasmatic3d/WEBP-Encoding-JS-wrapper.git
- git submodule update  --init --recursive
- emsdk activate latest
- emsdk_env.bat
- emcmake cmake -S . -B build -G "Unix Makefiles" -DCMAKE_MAKE_PROGRAM=D:\Tools\make\bin\make.exe -DCMAKBUILD_TYPE=Release -DWEBP_BUILD_WEBP_JS=1 -DWEBP_USE_THREAD=0
    * Make sure "cmake.exe" is in your path
- cmake.exe --build build --config Release

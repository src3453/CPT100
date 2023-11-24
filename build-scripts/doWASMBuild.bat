./lib/emsdk/upstream/emscripten/emcmake cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug
cd build
cmake --build . --parallel
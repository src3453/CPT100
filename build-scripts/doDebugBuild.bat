cd build
cmake -S .. -B . -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug
cmake --build . --parallel 
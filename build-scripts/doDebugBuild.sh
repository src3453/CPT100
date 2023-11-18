cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=./cmake/mingw-w64-x86_64.cmake -DCMAKE_BUILD_TYPE=Debug -G "Ninja"
ninja -C build
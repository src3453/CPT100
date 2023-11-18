cmake -S . -B build -DTOOLCHAIN_PREFIX=/usr/bin/x86_64-w64-mingw32 -DCMAKE_BUILD_TYPE=Debug -G "Ninja"
cmake --build build
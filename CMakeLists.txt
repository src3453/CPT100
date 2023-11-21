# CMakeのバージョンを設定
cmake_minimum_required(VERSION 3.16)
# プロジェクト名と使用する言語を設定
project(CPT100 CXX)
set(EXECUTABLE_NAME cpt100)
# ソースファイルを検索し，変数「MY_SOURCE」に設定
set(CMAKE_CXX_FLAGS "-mconsole")

set(CMAKE_CXX_STANDARD 17)

# Print processor and compiler 
message(STATUS "Detected processor is " ${CMAKE_SYSTEM_PROCESSOR})
message(STATUS "CMAKE_C_COMPILER: " ${CMAKE_C_COMPILER})
message(STATUS "CMAKE_CXX_COMPILER: " ${CMAKE_CXX_COMPILER})
message(STATUS "CMAKE_CXX_COMPILER_ID: " ${CMAKE_CXX_COMPILER_ID})


set(LUA_DIR ${PROJECT_SOURCE_DIR}/lib/lua)
set(SOL_DIR ${PROJECT_SOURCE_DIR}/lib/sol2)
set(SDL2_DIR ${PROJECT_SOURCE_DIR}/lib/SDL2)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
add_compile_options(-std=c++17)
endif()
#add_compile_options(-O2)

set(SOL_SRC)

add_subdirectory(${SDL2_DIR})
set(LUA_SRC
    ${LUA_DIR}/lapi.c
    ${LUA_DIR}/lcode.c
    ${LUA_DIR}/lctype.c
    ${LUA_DIR}/ldebug.c
    ${LUA_DIR}/ldo.c
    ${LUA_DIR}/ldump.c
    ${LUA_DIR}/lfunc.c
    ${LUA_DIR}/lgc.c
    ${LUA_DIR}/llex.c
    ${LUA_DIR}/lmem.c
    ${LUA_DIR}/lobject.c
    ${LUA_DIR}/lopcodes.c
    ${LUA_DIR}/lparser.c
    ${LUA_DIR}/lstate.c
    ${LUA_DIR}/lstring.c
    ${LUA_DIR}/ltable.c
    ${LUA_DIR}/ltm.c
    ${LUA_DIR}/lundump.c
    ${LUA_DIR}/lvm.c
    ${LUA_DIR}/lzio.c
    ${LUA_DIR}/lauxlib.c
    ${LUA_DIR}/lbaselib.c
    ${LUA_DIR}/lcorolib.c
    ${LUA_DIR}/ldblib.c
    ${LUA_DIR}/liolib.c
    ${LUA_DIR}/lmathlib.c
    ${LUA_DIR}/loslib.c
    ${LUA_DIR}/lstrlib.c
    ${LUA_DIR}/ltablib.c
    ${LUA_DIR}/lutf8lib.c
    ${LUA_DIR}/loadlib.c
    ${LUA_DIR}/linit.c
)

add_library(lua STATIC ${LUA_SRC})

file(GLOB, SOL_SRC, ${SOL_DIR}/include/sol/*.hpp)

add_library(sol STATIC ${SOL_SRC})


#実行ファイルを作成
add_executable(${EXECUTABLE_NAME} src/main.cpp)

target_compile_features(${EXECUTABLE_NAME} PUBLIC cxx_std_17)


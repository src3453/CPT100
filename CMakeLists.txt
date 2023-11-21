# CMakeのバージョンを設定
cmake_minimum_required(VERSION 3.16)

set(VERSION_MAJOR 0)
set(VERSION_MINOR 1)
set(VERSION_STATUS "-alpha")

find_package(Git)
if(Git_FOUND)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} status
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        ERROR_VARIABLE RESULT_STRING
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )


    string(LENGTH "${RESULT_STRING}" LENGTH_RESULT_STRING)

    if(${LENGTH_RESULT_STRING} EQUAL 0)

        execute_process(
            COMMAND ${GIT_EXECUTABLE} log -1 --format=%H
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE GIT_COMMIT_HASH
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )

        string(SUBSTRING ${GIT_COMMIT_HASH} 0 7 GIT_COMMIT_HASH)
        set(VERSION_HASH ${GIT_COMMIT_HASH} )

        execute_process(
            COMMAND ${GIT_EXECUTABLE} rev-list HEAD --count
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE VERSION_REVISION
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )

    endif()
endif()

# プロジェクト名と使用する言語を設定
project(CPTTracker VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_REVISION} LANGUAGES C CXX)
set(EXECUTABLE_NAME CPTTracker)

message("PROJECT_VERSION: ${PROJECT_VERSION}${VERSION_STATUS}")
message("VERSION_HASH: ${VERSION_HASH}")

# ソースファイルを検索し，変数「MY_SOURCE」に設定
if(WIN32)
set(CMAKE_CXX_FLAGS "-mconsole")
endif()


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
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
add_compile_options(-std=c++17)
endif()

if(MINGW)
    set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++)
endif()

set(SOL_SRC)

set(SDL_SHARED OFF CACHE BOOL "" FORCE)

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


file(GLOB_RECURSE SOL_SRC ${SOL_DIR}/include/sol/*.hpp)

file(GLOB_RECURSE SDL2_SRC ${SDL2_DIR}/include/*.h)

add_library(lua STATIC ${LUA_SRC})

add_library(sol STATIC ${SOL_SRC})

add_library(SDL2 STATIC ${SDL2_SRC})

set_target_properties(sol PROPERTIES LINKER_LANGUAGE CXX)

set_target_properties(SDL2 PROPERTIES LINKER_LANGUAGE CXX)


#実行ファイルを作成
add_executable(${EXECUTABLE_NAME} ${CMAKE_PROJECT_DIR}src/main.cpp)

target_compile_features(${EXECUTABLE_NAME} PRIVATE cxx_std_17)

target_include_directories(${EXECUTABLE_NAME} PRIVATE ${LUA_DIR})

target_include_directories(${EXECUTABLE_NAME} PRIVATE ${SDL2_DIR}/include)

target_include_directories(${EXECUTABLE_NAME} PRIVATE ${SOL_DIR}/include)

target_link_libraries(${EXECUTABLE_NAME} lua sol SDL2main SDL2-static)
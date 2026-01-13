# CMakeのバージョンを設定
cmake_minimum_required(VERSION 3.16)

option(WASM_BUILD "Build wasm" OFF) 
option(USE_LUAJIT "Use LuaJIT. if not, use Lua 5.4" ON)

if(EMSCRIPTEN)
# 共有ライブラリのサポートを有効にする
set_property(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS TRUE)

# 余計なフラグをつけないように全部リセット
set(CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS "")
set(CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS "")
endif()

# プロジェクト名と使用する言語を設定
set(VERSION_MAJOR 0)
set(VERSION_MINOR 2)
set(VERSION_STATUS "-dev")

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

configure_file(${CMAKE_SOURCE_DIR}/src/config.h.in ${CMAKE_SOURCE_DIR}/src/config.h @ONLY)

message("Configured version strings:")
message("- VERSION_MAJOR: ${VERSION_MAJOR}")
message("- VERSION_MINOR: ${VERSION_MINOR}")
message("- VERSION_REVISION: ${VERSION_REVISION}")
message("- VERSION_STATUS: ${VERSION_STATUS}")
message("- VERSION_HASH: ${VERSION_HASH}")

find_package(PythonInterp REQUIRED)
if(PythonInterp_FOUND)
    message("Python3 found: ${PYTHON_EXECUTABLE}")
    
    # Get all *.lua files in src/lua directory recursively
    file(GLOB_RECURSE LUA_FILES "${CMAKE_SOURCE_DIR}/src/lua/*.lua")
    
    # Process each file
    foreach(LUA_FILE ${LUA_FILES})
        message(STATUS "Processing: ${LUA_FILE}")
        execute_process(
            COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/toCFormat.py ${LUA_FILE}
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    endforeach()
else()
    message("Python3 not found.")
endif()

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

project(cpt200 LANGUAGES C CXX)
set(EXECUTABLE_NAME cpt200)
# ソースファイルを検索し，変数「MY_SOURCE」に設定

set(CMAKE_CXX_STANDARD 17)

# Print processor and compiler 
message(STATUS "Detected processor is " ${CMAKE_SYSTEM_PROCESSOR})
message(STATUS "CMAKE_C_COMPILER: " ${CMAKE_C_COMPILER})
message(STATUS "CMAKE_CXX_COMPILER: " ${CMAKE_CXX_COMPILER})
message(STATUS "CMAKE_CXX_COMPILER_ID: " ${CMAKE_CXX_COMPILER_ID})

if(USE_LUAJIT)
    message(STATUS "Using LuaJIT")
    set(LUA_DIR ${PROJECT_SOURCE_DIR}/lib/luajit/src)
else()
    message(STATUS "Using Lua 5.4")
    set(LUA_DIR ${PROJECT_SOURCE_DIR}/lib/lua)
endif()
set(SOL_DIR ${PROJECT_SOURCE_DIR}/lib/sol2)
set(SDL2_DIR ${PROJECT_SOURCE_DIR}/lib/SDL2)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
add_compile_options(-std=c++17)
endif()
#add_compile_options(-O2)
if(MINGW)
set(CMAKE_CXX_FLAGS "-mconsole")
set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")
endif()

set(SOL_SRC)

set(SDL_SHARED OFF CACHE BOOL "" FORCE)

add_subdirectory(${SDL2_DIR})

# LuaJIT sources are not compiled here, we use the prebuilt library.


file(GLOB_RECURSE SOL_SRC ${SOL_DIR}/include/sol/*.hpp)

file(GLOB_RECURSE SDL2_SRC ${SDL2_DIR}/include/*.h)

add_library(lua SHARED IMPORTED)
if(USE_LUAJIT)
    set_target_properties(lua PROPERTIES
        IMPORTED_LOCATION "${LUA_DIR}/lua51.dll"
        IMPORTED_IMPLIB "${LUA_DIR}/lua51.lib"
    )
else()
    set_target_properties(lua PROPERTIES
        IMPORTED_LOCATION "${LUA_DIR}/lua54.dll"
        IMPORTED_IMPLIB "${LUA_DIR}/lua54.lib"
    )
endif()

add_library(sol INTERFACE)
target_include_directories(sol INTERFACE ${SOL_DIR}/include)

add_library(SDL2 STATIC ${SDL2_SRC})

set_target_properties(SDL2 PROPERTIES LINKER_LANGUAGE CXX)


#実行ファイルを作成
add_executable(${EXECUTABLE_NAME} 

${CMAKE_PROJECT_DIR}src/main.cpp
)

if (EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".html") # デフォルトでは.js
    target_link_options(${EXECUTABLE_NAME} PRIVATE
    "SHELL:-s WASM=0"
    "SHELL:-s ENVIRONMENT=web"
    "SHELL:-s USE_SDL=2"
    "SHELL:-s USE_SDL_IMAGE=2"
    "SHELL:-s ALLOW_MEMORY_GROWTH=1")
endif()

target_compile_features(${EXECUTABLE_NAME} PRIVATE cxx_std_17)

if(USE_LUAJIT)
    target_compile_definitions(${EXECUTABLE_NAME} PRIVATE SOL_LUAJIT)
endif()

target_include_directories(${EXECUTABLE_NAME} PRIVATE ${LUA_DIR})

target_include_directories(${EXECUTABLE_NAME} PRIVATE ${SDL2_DIR}/include)

target_include_directories(${EXECUTABLE_NAME} PRIVATE ${SOL_DIR}/include)

target_link_libraries(${EXECUTABLE_NAME} lua SDL2main SDL2-static opengl32)

if(USE_LUAJIT)
    set(LUA_DLL_NAME "lua51.dll")
else()
    set(LUA_DLL_NAME "lua54.dll")
endif()

add_custom_command(TARGET ${EXECUTABLE_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${LUA_DIR}/${LUA_DLL_NAME}"
    "$<TARGET_FILE_DIR:${EXECUTABLE_NAME}>"
)


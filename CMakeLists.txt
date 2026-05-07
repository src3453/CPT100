# CMakeのバージョンを設定
cmake_minimum_required(VERSION 3.5)

option(WASM_BUILD "Build wasm" OFF) 
option(USE_LUAJIT "Use LuaJIT. if not, use Lua 5.4" ON)
option(MINIFY_SCRIPT "Minify script files with lua-minify. node.js module luamin required" ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

if (MINIFY_SCRIPT)
    message("Minifying Lua scripts...")
    
    # Get all *.lua files in src/lua directory recursively
    file(GLOB_RECURSE LUA_FILES "${CMAKE_SOURCE_DIR}/src/lua/*.lua")
    
    # Try to find npm executable
    find_program(NPM_EXECUTABLE npm)
    
    if(NPM_EXECUTABLE)
        message(STATUS "Found npm: ${NPM_EXECUTABLE}")
        
        # Install npm dependencies first
        message(STATUS "Installing npm dependencies...")
        if(WIN32)
            execute_process(
                COMMAND cmd /c "${NPM_EXECUTABLE}" install
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/build
                ERROR_VARIABLE npm_install_error
                RESULT_VARIABLE npm_install_code
            )
        else()
            execute_process(
                COMMAND ${NPM_EXECUTABLE} install
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/build
                ERROR_VARIABLE npm_install_error
                RESULT_VARIABLE npm_install_code
            )
        endif()
        
        if(NOT npm_install_code EQUAL 0)
            message(WARNING "npm install failed: ${npm_install_error}")
        else()
            message(STATUS "npm dependencies installed successfully")
        endif()
        
        # Process each file
        foreach(LUA_FILE ${LUA_FILES})
            message(STATUS "Minifying: ${LUA_FILE}")
            if(WIN32)
                # On Windows, use cmd /c to ensure proper environment inheritance
                execute_process(
                    COMMAND cmd /c "cd ${CMAKE_SOURCE_DIR}/build && npx luamin -f ${LUA_FILE}"
                    OUTPUT_FILE "${LUA_FILE}.min"
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/build
                    OUTPUT_STRIP_TRAILING_WHITESPACE
                    ERROR_VARIABLE error_output
                    RESULT_VARIABLE result_code
                )
            else()
                execute_process(
                    COMMAND "npx luamin -f ${LUA_FILE}"
                    OUTPUT_FILE ${LUA_FILE}.min
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/build
                    OUTPUT_STRIP_TRAILING_WHITESPACE
                    ERROR_VARIABLE error_output
                    RESULT_VARIABLE result_code
                )
            endif()
            if(NOT result_code EQUAL 0)
                message(WARNING "Minification failed for ${LUA_FILE}. Error: ${error_output}")
            else()
                message(STATUS "Successfully minified: ${LUA_FILE}")
            endif()
        endforeach()
    else()
        message(WARNING "npm not found in PATH. Skipping Lua script minification. Ensure Node.js is installed and in your PATH.")
    endif()
endif()

find_package(Python3 REQUIRED)
if(Python3_FOUND)
    message("Python3 found: ${Python3_EXECUTABLE}")
    
    # Get all *.lua files in src/lua directory recursively
    file(GLOB_RECURSE LUA_FILES "${CMAKE_SOURCE_DIR}/src/lua/*.lua")
    
    # Process each file
    foreach(LUA_FILE ${LUA_FILES})
        if(MINIFY_SCRIPT)
            # Use the minified version if minification is enabled
            set(_LUA_FILE "${LUA_FILE}.min")
        else()
            set(_LUA_FILE "${LUA_FILE}")
        endif()
        message(STATUS "Processing: ${_LUA_FILE}")
        execute_process(
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/toCFormat.py ${_LUA_FILE} ${LUA_FILE}.hpp
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

if(APPLE AND USE_LUAJIT)
    # macOS: use static library for LuaJIT
    add_library(lua STATIC IMPORTED)
    set_target_properties(lua PROPERTIES
        IMPORTED_LOCATION "${LUA_DIR}/libluajit.a"
    )
else()
    add_library(lua SHARED IMPORTED)
    if(USE_LUAJIT)
        if(UNIX AND NOT APPLE)
            set_target_properties(lua PROPERTIES
                IMPORTED_LOCATION "${LUA_DIR}/libluajit.so"
                IMPORTED_IMPLIB "${LUA_DIR}/libluajit.a"
            )
        elseif(WIN32)
            set_target_properties(lua PROPERTIES
                IMPORTED_LOCATION "${LUA_DIR}/lua51.dll"
                IMPORTED_IMPLIB "${LUA_DIR}/lua51.lib"
            )
        else()
            message(FATAL_ERROR "Unsupported platform for LuaJIT")
        endif()
    else()
        set_target_properties(lua PROPERTIES
            IMPORTED_LOCATION "${LUA_DIR}/lua54.dll"
            IMPORTED_IMPLIB "${LUA_DIR}/lua54.lib"
        )
    endif()
endif()

add_library(sol INTERFACE)
target_include_directories(sol INTERFACE ${SOL_DIR}/include)

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

target_compile_definitions(${EXECUTABLE_NAME} PRIVATE SOL_STD_OPTIONAL=0
SOL_ALL_SAFETIES_ON=1
)

target_include_directories(${EXECUTABLE_NAME} PRIVATE ${LUA_DIR})

target_include_directories(${EXECUTABLE_NAME} PRIVATE ${SDL2_DIR}/include)

target_include_directories(${EXECUTABLE_NAME} PRIVATE ${SOL_DIR}/include)

#target_include_directories(${EXECUTABLE_NAME} PRIVATE ${Boost_INCLUDE_DIRS})
if (WIN32)
    target_link_libraries(${EXECUTABLE_NAME} opengl32)
elseif(APPLE)
    find_library(OPENGL_LIBRARY OpenGL)
    target_link_libraries(${EXECUTABLE_NAME} ${OPENGL_LIBRARY})
elseif(UNIX)
    target_link_libraries(${EXECUTABLE_NAME} GL)
endif()
# Boost のリンク方法を柔軟に対応
if(TARGET Boost::system)
    target_link_libraries(${EXECUTABLE_NAME} lua SDL2main SDL2-static Boost::system)
else()
    # Boost::system ターゲットが無い場合は直接ライブラリをリンク
    target_link_libraries(${EXECUTABLE_NAME} lua SDL2main SDL2-static ${Boost_LIBRARIES})
endif()

if(USE_LUAJIT)
    set(LUA_DLL_NAME "lua51.dll")
else()
    set(LUA_DLL_NAME "lua54.dll")
endif()

if (WIN32)
add_custom_command(TARGET ${EXECUTABLE_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${LUA_DIR}/${LUA_DLL_NAME}"
    "$<TARGET_FILE_DIR:${EXECUTABLE_NAME}>"
)
elseif (UNIX OR APPLE)
endif()



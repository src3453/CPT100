# CMakeのバージョンを設定
cmake_minimum_required(VERSION 3.13)
# プロジェクト名と使用する言語を設定
project(CPT100 CXX)
set(EXECUTABLE_NAME cpt100)
# ソースファイルを検索し，変数「MY_SOURCE」に設定
set(CMAKE_CXX_FLAGS "-mconsole")

set(SDL2_DIR ${PROJECT_SOURCE_DIR}/lib/SDL2/x86_64-w64-mingw32/lib/cmake/SDL2)

find_package(SDL2 CONFIG REQUIRED)

#実行ファイルを作成
add_executable(${EXECUTABLE_NAME} src/main.cpp)

target_include_directories(${EXECUTABLE_NAME} PUBLIC ${SDL2_INCLUDE_DIRS})

# SDL2のライブラリをリンク
target_link_libraries(${EXECUTABLE_NAME} PRIVATE ${SDL2_LIBRARIES})

# SDL2.dllを実行ファイルと同じディレクトリにコピー
add_custom_command(TARGET ${EXECUTABLE_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PROJECT_SOURCE_DIR}/lib/SDL2/x86_64-w64-mingw32/bin/SDL2.dll"        # コピー元のファイルパス
        $<TARGET_FILE_DIR:${EXECUTABLE_NAME}>    # コピー先のディレクトリ
)
add_custom_command(TARGET ${EXECUTABLE_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PROJECT_SOURCE_DIR}/lib/libstdc++-6.dll"        # コピー元のファイルパス
        $<TARGET_FILE_DIR:${EXECUTABLE_NAME}>    # コピー先のディレクトリ
)
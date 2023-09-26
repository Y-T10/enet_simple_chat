#ライブラリを追加
add_library(console_io)

#プログラムファイルを追加
# - ${CMAKE_CURRENT_SOURCE_DIR}: このファイルがあるディレクトリ
target_sources(console_io PRIVATE console_io.cpp)
target_include_directories(console_io PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
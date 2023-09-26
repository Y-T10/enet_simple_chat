#ライブラリを追加
add_library(meditator INTERFACE)

#プログラムファイルを追加
# - ${CMAKE_CURRENT_SOURCE_DIR}: このファイルがあるディレクトリ
target_include_directories(meditator INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
#ライブラリを追加
add_library(enet_host)

#プログラムファイルを追加
# - ${CMAKE_CURRENT_SOURCE_DIR}: このファイルがあるディレクトリ
target_sources(enet_host PRIVATE enet_host.cpp)
target_include_directories(enet_host PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

#ライブラリを追加
target_link_libraries(
    enet_host PRIVATE
    cpp_common
    basic_enet
)
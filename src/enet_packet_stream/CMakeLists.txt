#ライブラリを追加
add_library(enet_packet_stream)

#プログラムファイルを追加
# - ${CMAKE_CURRENT_SOURCE_DIR}: このファイルがあるディレクトリ
target_sources(enet_packet_stream PRIVATE enet_packet_stream.cpp)
target_include_directories(enet_packet_stream PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

#ライブラリを追加
target_link_libraries(
    enet_packet_stream PRIVATE
    basic_enet
)

target_link_libraries(
    enet_packet_stream PUBLIC
    msgpack-cxx
)
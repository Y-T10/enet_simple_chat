#ライブラリを追加
add_library(enet_packet_stream)

#プログラムファイルを追加
# - ${CMAKE_CURRENT_SOURCE_DIR}: このファイルがあるディレクトリ
<<<<<<< HEAD
target_sources(enet_packet_stream PRIVATE enet_packet_stream.cpp)
=======
target_sources(enet_packet_stream PRIVATE console_io.cpp)
>>>>>>> 4c0df82 (ライブラリのビルドを追加)
target_include_directories(enet_packet_stream PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

#ライブラリを追加
target_link_libraries(
    enet_packet_stream PRIVATE
    cpp_common
<<<<<<< HEAD
    PkgConfig::libenet
=======
    PkgConfig::enet
>>>>>>> 4c0df82 (ライブラリのビルドを追加)
    msgpackc-cxx
)
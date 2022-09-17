#実行ファイルの設定
add_executable(server)
add_executable(client)
set_target_properties(server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set_target_properties(client PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

#プログラムファイルを追加
target_sources(server PRIVATE server.cpp)
target_sources(client PRIVATE client.cpp)
target_include_directories(server PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(client PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include(basic_enet.cmake)
include(console_io.cmake)
include(enet_client.cmake)
include(enet_host.cmake)
include(enet_packet_stream.cmake)
include(sig_event.cmake)

#ライブラリを追加
target_link_libraries(
    server PRIVATE
    cpp_common_flags
    enet_host
    enet_packet_stream
    sig_event
)
target_link_libraries(
    client PRIVATE
    cpp_common_flags
    console_io
    enet_client
    enet_packet_stream
)
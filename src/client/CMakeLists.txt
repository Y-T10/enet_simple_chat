add_executable(client)

set_target_properties(client PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

#プログラムファイルを追加
target_sources(client PRIVATE client.cpp)
target_include_directories(client PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

#ライブラリを追加
target_link_libraries(
    client PRIVATE
    console_io
    enet_client
    enet_packet_stream
)
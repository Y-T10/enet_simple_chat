#ライブラリを追加
add_library(enet_client)

#プログラムファイルを追加
# - ${CMAKE_CURRENT_SOURCE_DIR}: このファイルがあるディレクトリ
target_sources(enet_client PRIVATE enet_client.cpp)
target_include_directories(enet_client PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

#ライブラリを追加
target_link_libraries(
    enet_client PRIVATE
    basic_enet
)
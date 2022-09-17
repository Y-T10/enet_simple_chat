#ライブラリを追加
add_library(enet_client)

<<<<<<< HEAD
=======
include(basic_enet.cmake)

>>>>>>> 4c0df82 (ライブラリのビルドを追加)
#プログラムファイルを追加
# - ${CMAKE_CURRENT_SOURCE_DIR}: このファイルがあるディレクトリ
target_sources(enet_client PRIVATE enet_client.cpp)
target_include_directories(enet_client PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

#ライブラリを追加
target_link_libraries(
    enet_client PRIVATE
    cpp_common
    basic_enet
)
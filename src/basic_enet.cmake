#ライブラリを追加
add_library(basic_enet)

#プログラムファイルを追加
# - ${CMAKE_CURRENT_SOURCE_DIR}: このファイルがあるディレクトリ
target_sources(basic_enet PRIVATE basic_enet.cpp)
target_include_directories(basic_enet PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

#ライブラリを追加
target_link_libraries(
    basic_enet PRIVATE
    cpp_common
<<<<<<< HEAD
    PkgConfig::libenet
=======
    PkgConfig::enet
>>>>>>> 4c0df82 (ライブラリのビルドを追加)
    Boost::headers
)
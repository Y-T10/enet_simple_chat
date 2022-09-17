#ライブラリを追加
add_library(sig_event)

#プログラムファイルを追加
# - ${CMAKE_CURRENT_SOURCE_DIR}: このファイルがあるディレクトリ
<<<<<<< HEAD
target_sources(sig_event PRIVATE sig_event.cpp)
=======
target_sources(sig_event PRIVATE SigEvent.cpp)
>>>>>>> 4c0df82 (ライブラリのビルドを追加)
target_include_directories(sig_event PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

#ライブラリを追加
target_link_libraries(
    sig_event PRIVATE
    cpp_common
)
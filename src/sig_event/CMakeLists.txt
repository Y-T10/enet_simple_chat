#ライブラリを追加
add_library(sig_event)

#プログラムファイルを追加
# - ${CMAKE_CURRENT_SOURCE_DIR}: このファイルがあるディレクトリ
target_sources(sig_event PRIVATE sig_event.cpp)
target_include_directories(sig_event PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

#ライブラリを追加
target_link_libraries(
    sig_event PRIVATE
    cpp_common
)
#フラグ用インターフェィスを追加
add_library(cpp_common INTERFACE)
#共通のフラグを設定する
target_compile_options(
    cpp_common INTERFACE
    -Wall $<$<CONFIG:Debug>:-g3>
)
#C++の標準を指定
target_compile_features(cpp_common INTERFACE cxx_std_20)
target_link_libraries(cpp_common INTERFACE Boost::headers)
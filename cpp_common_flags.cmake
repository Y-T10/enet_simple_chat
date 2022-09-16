#フラグ用インターフェィスを追加
add_library(cpp_common_flags INTERFACE)
#共通のフラグを設定する
target_compile_options(
    cpp_common_flags INTERFACE
    -Wall $<$<CONFIG:Debug>:-g3>
)
#C++の標準を指定
target_compile_features(cpp_common_flags INTERFACE cxx_std_20)
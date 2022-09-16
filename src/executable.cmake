#実行ファイルの設定
# - ${CMAKE_SOURCE_DIR}: ルートのCMakeListsがある場所
add_executable("exec name")
set_target_properties("exec name" PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

#プログラムファイルを追加
# - ${CMAKE_CURRENT_SOURCE_DIR}: このファイルがあるディレクトリ
target_sources("exec name" PRIVATE "src0.cpp src1.cpp src2.cpp ...")
target_include_directories("exec name" PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

#ライブラリを追加
target_link_libraries("exec name" PRIVATE
    cpp_common_flags "libname::libname PkgConfig::libname project_libname")
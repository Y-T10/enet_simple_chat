#ライブラリを追加
add_library("lib name")

#プログラムファイルを追加
# - ${CMAKE_CURRENT_SOURCE_DIR}: このファイルがあるディレクトリ
target_sources("lib name" PRIVATE "src0.cpp src1.cpp src2.cpp ...")
target_include_directories(window PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

#ライブラリを追加
target_link_libraries("exec name" PRIVATE
    cpp_common_flags "libname::libname PkgConfig::libname project_libname")
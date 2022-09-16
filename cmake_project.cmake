#要求するCMakeのバージョンを指定
cmake_minimum_required(VERSION 3.20)

#プロジェクトの設定
# - cmake_policyでtarget_sourcesのパスを自動で調節する
project(fwf VERSION 0.0.1 LANGUAGES CXX)
cmake_policy(SET CMP0076 NEW)

#必要なモジュールの読み込み
# - cpp_common_flags.cmake: C++の設定
# - find_pkg_conf.cmake: pc読み込みライブラリ
include(cpp_common_flags.cmake)
include(find_pkg_conf.cmake)

#必要なライブラリを探す
# - CMake対応ならfind_package("pkg name" REQUIRED)
# - pkgconfig対応ならfind_pkg_conf("pkg name")

#サブディレクトリ
#add_subdirectory("sub dir")
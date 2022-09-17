#要求するCMakeのバージョンを指定
cmake_minimum_required(VERSION 3.20)

#プロジェクトの設定
# - cmake_policyでtarget_sourcesのパスを自動で調節する
project(simple_enet_chat VERSION 0.0.1 LANGUAGES CXX)
cmake_policy(SET CMP0076 NEW)

#必要なモジュールの読み込み
include(cpp_common_flags.cmake)
include(find_pkg_conf.cmake)

#必要なライブラリを探す
find_package(Boost REQUIRED)
find_package(msgpack REQUIRED)
find_pkg_conf(enet)

#サブディレクトリ
add_subdirectory(src)
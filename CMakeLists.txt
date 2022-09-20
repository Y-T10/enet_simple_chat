#要求するCMakeのバージョンを指定
cmake_minimum_required(VERSION 3.20)

#プロジェクトの設定
# - cmake_policyでtarget_sourcesのパスを自動で調節する
project(simple_enet_chat VERSION 0.0.1 LANGUAGES CXX)
cmake_policy(SET CMP0076 NEW)

#必要なライブラリを探す
include(find_pkg_conf.cmake)
find_package(Boost REQUIRED)
find_package(msgpack REQUIRED)
find_package(Angelscript REQUIRED)
find_pkg_conf(libenet)

#C++の共通設定を追加
include(cpp_common.cmake)

#サブディレクトリ
add_subdirectory(src)
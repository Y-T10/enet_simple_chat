#要求するCMakeのバージョンを指定
cmake_minimum_required(VERSION 3.20)

#プロジェクトの設定
# - cmake_policyでtarget_sourcesのパスを自動で調節する
project(simple_enet_chat VERSION 0.0.1 LANGUAGES CXX)
cmake_policy(SET CMP0076 NEW)
cmake_policy(SET CMP0128 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0128 NEW)
cmake_policy(SET CMP0074 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0074 NEW)
cmake_policy(SET CMP0077 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(CMAKE_BUILD_TYPE Release)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

#C++の共通設定を追加
add_compile_options(-Wall $<$<CONFIG:Debug>:-g3>)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#Gitのサブモジュールからライブラリを探す
add_subdirectory(lib)

#必要なライブラリを探す
find_package(Boost REQUIRED)

#サブディレクトリ
add_subdirectory(src)
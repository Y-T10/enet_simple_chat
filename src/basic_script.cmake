#ライブラリを追加
add_library(basic_script)

#標準ライブラリを追加
add_subdirectory(scriptarray)
add_subdirectory(scriptstdstring)

#プログラムファイルを追加
# - ${CMAKE_CURRENT_SOURCE_DIR}: このファイルがあるディレクトリ
target_sources(basic_script PRIVATE basic_script.cpp)
target_include_directories(basic_script PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

#ライブラリを追加
target_link_libraries(
    basic_script PRIVATE
    cpp_common
    Boost::headers
    Angelscript::angelscript
    asStdString
    asArray
)
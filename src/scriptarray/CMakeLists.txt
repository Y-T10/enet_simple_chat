add_library(asArray)
target_sources(asArray PRIVATE scriptarray.cpp)
target_include_directories(asArray PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(asArray PRIVATE cpp_common_flags Angelscript::angelscript)
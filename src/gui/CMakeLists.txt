add_executable(gui)

set_target_properties(gui PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

#プログラムファイルを追加
target_sources(gui PRIVATE main.cpp)
target_include_directories(gui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(
    gui PRIVATE
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    SDL2::SDL2
    SDL2_ttf::SDL2_ttf
)
#ライブラリの設定
set(MSGPACK_BUILD_DOCS OFF CACHE  BOOL "ドキュメントの生成を禁止する")
set(MSGPACK_CXX20 ON CACHE  BOOL "c++20でコンパイルする")

add_subdirectory(enet)
add_subdirectory(msgpack)
add_subdirectory(SDL)
add_subdirectory(SDL_ttf)
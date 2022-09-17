#pkgconfファイル読み取りライブラリ

#pkg-confを使用してパッケージのインポートターゲットを作る
find_package(PkgConfig REQUIRED)

#pkgconfファイル読み取り関数
function(find_pkg_conf pkg_name)
    pkg_check_modules(${pkg_name} REQUIRED IMPORTED_TARGET GLOBAL ${pkg_name})
endfunction()
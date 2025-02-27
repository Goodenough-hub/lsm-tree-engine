-- 定义项目
set_project("lsm-tree")
set_version("0.0.1")
set_languages("c++20")

add_rules("mode.debug", "mode.release")
add_requires("gtest")

target("skiplist")
    set_kind("static") -- 静态库
    add_files("src/skiplist/*.cpp")
    add_includedirs("src/skiplist", {public = true})

target("test_skiplist")
    set_kind("binary")
    set_group("tests")
    add_files("test/test_skiplist.cpp")
    add_deps("skiplist")
    add_packages("gtest")
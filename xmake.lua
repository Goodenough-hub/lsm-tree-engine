-- 定义项目
set_project("lsm-tree")
set_version("0.0.1")
set_languages("c++20")

add_rules("mode.debug", "mode.release")
add_requires("gtest")

target("utils")
    set_kind("static") -- 静态库
    add_files("src/utils/*.cpp")
    add_includedirs("include", {public = true})

target("skiplist")
    set_kind("static") -- 静态库
    add_files("src/skiplist/*.cpp")
    add_includedirs("include", {public = true})

target("memtable")
    set_kind("static") -- 静态库
    add_deps("skiplist")
    add_files("src/memtable/*.cpp")
    add_includedirs("include", {public = true})

target("block")
    set_kind("static") -- 静态库
    add_files("src/block/*.cpp")
    add_includedirs("include", {public = true})

target("sst")
    set_kind("static") -- 静态库
    add_files("src/sst/*.cpp")
    add_includedirs("include", {public = true})

--- 单元测试

target("test_skiplist")
    set_kind("binary")
    set_group("tests")
    add_files("test/test_skiplist.cpp")
    add_deps("skiplist")
    add_packages("gtest")

target("test_memtable")
    set_kind("binary")
    set_group("tests")
    add_files("test/test_memtable.cpp")
    add_deps("memtable")
    add_packages("gtest")

target("test_utils")
    set_kind("binary")
    set_group("tests")
    add_files("test/test_utils.cpp")
    add_deps("utils")
    add_packages("gtest")
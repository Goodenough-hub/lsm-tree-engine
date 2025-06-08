-- 定义项目
set_project("lsm-tree")
set_version("0.0.1")
set_languages("c++20")

add_rules("mode.debug", "mode.release")
add_requires("gtest")
add_requires("muduo")

-- 全局启用 -fPIC（对所有 target 生效）
-- add_rules("mode.release", "mode.debug")
-- set_policy("build.warning", true)
-- add_requires("xxx") -- 你的依赖项

-- -- 关键配置：强制所有目标（包括静态库）使用 -fPIC
-- set_policy("build.merge_archive", true) -- 合并静态库（可选）
-- add_defines("_FORTIFY_SOURCE=2")        -- 安全编译选项（可选）
-- set_symbols("hidden")                   -- 符号隐藏（可选）

-- -- 对所有 C/C++ 代码启用 -fPIC
-- if is_plat("linux", "macosx") then
--     add_cxflags("-fPIC")
--     add_mxflags("-fPIC")
--     add_ldflags("-fPIC")
-- end

target("utils")
    set_kind("static") -- 静态库
    add_files("src/utils/*.cpp")
    add_includedirs("include", {public = true})

target("iterator")
    set_kind("static") -- 静态库
    add_files("src/iterator/*.cpp")
    add_includedirs("include", {public = true})

target("skiplist")
    set_kind("static") -- 静态库
    add_files("src/skiplist/*.cpp")
    add_includedirs("include", {public = true})

target("memtable")
    set_kind("static") -- 静态库
    add_deps("skiplist", "iterator", "sst")
    add_files("src/memtable/*.cpp")
    add_includedirs("include", {public = true})

target("block")
    set_kind("static") -- 静态库
    add_files("src/block/*.cpp")
    add_includedirs("include", {public = true})

target("sst")
    set_kind("static") -- 静态库
    add_files("src/sst/*.cpp")
    add_deps("block", "utils")
    add_includedirs("include", {public = true})

target("wal")
    set_kind("static")  -- 生成静态库
    add_deps("sst", "memtable")
    add_files("src/wal/*.cpp")
    add_includedirs("include", {public = true})

target("engine")
     set_kind("static") -- 静态库
     add_files("src/engine/*.cpp")
     add_deps("block", "memtable", "sst", "wal", "utils", "iterator")
     add_includedirs("include", {public = true})

target("redis_wrapper")
     set_kind("static") -- 静态库
     add_files("src/redis_wrapper/*.cpp")
     add_deps("engine")
     add_includedirs("include", {public = true})
 
 target("lsm_shared")
     set_kind("shared") -- 动态链接库
     add_files("src/**.cpp")
     add_includedirs("include", {public = true})
     set_targetdir("$(buildir)/lib") -- 指定编译输出到build/lib目录下
 
     -- 安装头文件和动态链接库
     on_install(function (target) 
         os.cp("include", path.join(target:installdir(), "include/lsm")) -- 安装头文件到include/lsm
         os.cp(target:targetfile(), path.join(target:installdir(), "lib")) -- 安装动态库到lib目录
     end)

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
    add_deps("memtable", "skiplist", "iterator", "sst", "block", "utils")
    add_packages("gtest")

target("test_utils")
    set_kind("binary")
    set_group("tests")
    add_files("test/test_utils.cpp")
    add_deps("utils")
    add_packages("gtest")

target("test_block")
     set_kind("binary")
     set_group("tests")
     add_files("test/test_block.cpp")
     add_deps("block")
     add_packages("gtest")
 
target("test_sst")
     set_kind("binary")
     set_group("tests")
     add_files("test/test_sst.cpp")
     add_deps("sst", "block", "iterator", "utils")
     add_packages("gtest")

target("test_block_cache")
    set_kind("binary")
    set_group("tests")
    add_files("test/test_block_cache.cpp")
    add_deps("block")
    add_packages("gtest")

target("test_engine")
    set_kind("binary")
    set_group("tests")
    add_files("test/test_engine.cpp")
    add_deps("engine")
    add_packages("gtest")

target("server")
    set_kind("binary")
    add_files("server/src/*.cpp")
    add_deps("redis_wrapper")
    add_packages("muduo")
    set_targetdir("$(buildir)/bin")
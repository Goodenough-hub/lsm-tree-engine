{
    depfiles = "build/.objs/test_block_cache/linux/x86_64/debug/test/test_block_cache.cpp.o:  test/test_block_cache.cpp test/../include/block/block_cache.h  test/../include/block/block_iterator.h test/../include/block/block.h\
",
    files = {
        "test/test_block_cache.cpp"
    },
    depfiles_format = "gcc",
    values = {
        "/usr/bin/gcc",
        {
            "-m64",
            "-g",
            "-O0",
            "-std=c++20",
            "-Iinclude",
            "-isystem",
            "/home/fangming/.xmake/packages/g/gtest/v1.15.2/aea99fd8f63a47f0a0b9fe284bee1bfa/include"
        }
    }
}
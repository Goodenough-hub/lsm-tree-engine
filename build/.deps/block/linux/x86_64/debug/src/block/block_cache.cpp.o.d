{
    depfiles = "block_cache.o: src/block/block_cache.cpp  src/block/../../include/block/block_cache.h  src/block/../../include/block/block_iterator.h\
",
    depfiles_format = "gcc",
    values = {
        "/usr/bin/gcc",
        {
            "-m64",
            "-g",
            "-O0",
            "-std=c++20",
            "-Iinclude"
        }
    },
    files = {
        "src/block/block_cache.cpp"
    }
}
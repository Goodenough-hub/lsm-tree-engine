{
    files = {
        "src/block/block_cache.cpp"
    },
    depfiles = "block_cache.o: src/block/block_cache.cpp  src/block/../../include/block/block_cache.h  src/block/../../include/block/block_iterator.h  src/block/../../include/block/../iterator/iterator.h\
",
    depfiles_format = "gcc",
    values = {
        "/usr/bin/gcc",
        {
            "-m64",
            "-fPIC",
            "-O3",
            "-std=c++20",
            "-Iinclude",
            "-DNDEBUG"
        }
    }
}
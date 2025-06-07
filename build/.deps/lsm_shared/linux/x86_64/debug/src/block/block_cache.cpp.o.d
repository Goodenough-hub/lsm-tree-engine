{
    files = {
        "src/block/block_cache.cpp"
    },
    depfiles_format = "gcc",
    depfiles = "block_cache.o: src/block/block_cache.cpp  src/block/../../include/block/block_cache.h  src/block/../../include/block/block_iterator.h  src/block/../../include/block/../iterator/iterator.h\
",
    values = {
        "/usr/bin/gcc",
        {
            "-m64",
            "-fvisibility=hidden",
            "-fvisibility-inlines-hidden",
            "-O0",
            "-std=c++20",
            "-Iinclude",
            "-D_FORTIFY_SOURCE=2",
            "-fPIC"
        }
    }
}
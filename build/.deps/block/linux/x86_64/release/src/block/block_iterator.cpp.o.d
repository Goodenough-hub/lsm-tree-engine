{
    files = {
        "src/block/block_iterator.cpp"
    },
    depfiles = "block_iterator.o: src/block/block_iterator.cpp  src/block/../../include/block/block_iterator.h  src/block/../../include/block/../iterator/iterator.h  src/block/../../include/block/block.h\
",
    depfiles_format = "gcc",
    values = {
        "/usr/bin/gcc",
        {
            "-m64",
            "-fvisibility=hidden",
            "-fvisibility-inlines-hidden",
            "-O3",
            "-std=c++20",
            "-Iinclude",
            "-DNDEBUG"
        }
    }
}
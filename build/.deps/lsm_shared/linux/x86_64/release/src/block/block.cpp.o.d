{
    files = {
        "src/block/block.cpp"
    },
    depfiles = "block.o: src/block/block.cpp src/block/../../include/block/block.h  src/block/../../include/block/block_iterator.h  src/block/../../include/block/../iterator/iterator.h\
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
{
    depfiles = "block.o: src/block/block.cpp src/block/../../include/block/block.h  src/block/../../include/block/block_iterator.h\
",
    depfiles_format = "gcc",
    values = {
        "/usr/bin/gcc",
        {
            "-m64",
            "-fPIC",
            "-g",
            "-O0",
            "-std=c++20",
            "-Iinclude"
        }
    },
    files = {
        "src/block/block.cpp"
    }
}
{
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
        "src/block/block_iterator.cpp"
    },
    depfiles = "block_iterator.o: src/block/block_iterator.cpp  src/block/../../include/block/block_iterator.h  src/block/../../include/block/../iterator/iterator.h  src/block/../../include/block/block.h\
",
    depfiles_format = "gcc"
}
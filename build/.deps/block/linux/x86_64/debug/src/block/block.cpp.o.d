{
    depfiles = "build/.objs/block/linux/x86_64/debug/src/block/block.cpp.o:  src/block/block.cpp src/block/../../include/block/block.h  src/block/../../include/block/block_iterator.h\
",
    depfiles_format = "gcc",
    files = {
        "src/block/block.cpp"
    },
    values = {
        "/usr/bin/gcc",
        {
            "-m64",
            "-g",
            "-O0",
            "-std=c++20",
            "-Iinclude"
        }
    }
}
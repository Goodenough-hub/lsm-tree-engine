{
    depfiles = "build/.objs/block/linux/x86_64/debug/src/block/block_iterator.cpp.o:  src/block/block_iterator.cpp  src/block/../../include/block/block_iterator.h  src/block/../../include/block/block.h\
",
    depfiles_format = "gcc",
    files = {
        "src/block/block_iterator.cpp"
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
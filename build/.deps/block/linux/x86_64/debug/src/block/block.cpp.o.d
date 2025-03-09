{
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
    },
    depfiles = "block.o: src/block/block.cpp src/block/../../include/block/block.h\
"
}
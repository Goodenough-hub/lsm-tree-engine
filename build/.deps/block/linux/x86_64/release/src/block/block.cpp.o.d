{
    files = {
        "src/block/block.cpp"
    },
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
    },
    depfiles = "block.o: src/block/block.cpp src/block/../../include/block/block.h\
"
}
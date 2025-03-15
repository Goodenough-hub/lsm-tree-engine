{
    depfiles_format = "gcc",
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
    files = {
        "src/block/blockmeta.cpp"
    },
    depfiles = "blockmeta.o: src/block/blockmeta.cpp  src/block/../../include/block/blockmeta.h\
"
}
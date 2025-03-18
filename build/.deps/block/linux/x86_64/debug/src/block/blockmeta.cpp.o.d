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
    depfiles = "blockmeta.o: src/block/blockmeta.cpp  src/block/../../include/block/blockmeta.h\
",
    files = {
        "src/block/blockmeta.cpp"
    }
}
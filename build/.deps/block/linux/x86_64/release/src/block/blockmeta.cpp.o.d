{
    files = {
        "src/block/blockmeta.cpp"
    },
    depfiles = "blockmeta.o: src/block/blockmeta.cpp  src/block/../../include/block/blockmeta.h\
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
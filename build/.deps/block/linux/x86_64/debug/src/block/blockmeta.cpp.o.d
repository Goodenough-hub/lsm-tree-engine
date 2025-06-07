{
    files = {
        "src/block/blockmeta.cpp"
    },
    depfiles_format = "gcc",
    depfiles = "blockmeta.o: src/block/blockmeta.cpp  src/block/../../include/block/blockmeta.h\
",
    values = {
        "/usr/bin/gcc",
        {
            "-m64",
            "-fvisibility=hidden",
            "-fvisibility-inlines-hidden",
            "-O0",
            "-std=c++20",
            "-Iinclude",
            "-D_FORTIFY_SOURCE=2",
            "-fPIC"
        }
    }
}
{
    files = {
        "src/wal/record.cpp"
    },
    depfiles_format = "gcc",
    depfiles = "record.o: src/wal/record.cpp src/wal/../../include/wal/record.h\
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
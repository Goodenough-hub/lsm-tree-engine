{
    files = {
        "src/wal/record.cpp"
    },
    depfiles = "record.o: src/wal/record.cpp src/wal/../../include/wal/record.h\
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
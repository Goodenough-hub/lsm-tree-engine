{
    files = {
        "src/wal/wal.cpp"
    },
    depfiles_format = "gcc",
    depfiles = "wal.o: src/wal/wal.cpp src/wal/../../include/wal/wal.h  src/wal/../../include/wal/../utils/file.h  src/wal/../../include/wal/../utils/mmap_file.h  src/wal/../../include/wal/../utils/std_file.h  src/wal/../../include/wal/record.h\
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
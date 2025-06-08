{
    files = {
        "src/wal/wal.cpp"
    },
    depfiles = "wal.o: src/wal/wal.cpp src/wal/../../include/wal/wal.h  src/wal/../../include/wal/../utils/file.h  src/wal/../../include/wal/../utils/mmap_file.h  src/wal/../../include/wal/../utils/std_file.h  src/wal/../../include/wal/record.h\
",
    depfiles_format = "gcc",
    values = {
        "/usr/bin/gcc",
        {
            "-m64",
            "-fPIC",
            "-O3",
            "-std=c++20",
            "-Iinclude",
            "-DNDEBUG"
        }
    }
}
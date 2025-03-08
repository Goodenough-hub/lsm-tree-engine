{
    depfiles = "file.o: src/utils/file.cpp src/utils/../../include/utils/file.h  src/utils/../../include/utils/mmap_file.h\
",
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
    files = {
        "src/utils/file.cpp"
    },
    depfiles_format = "gcc"
}
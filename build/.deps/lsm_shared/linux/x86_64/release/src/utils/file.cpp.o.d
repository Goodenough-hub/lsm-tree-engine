{
    files = {
        "src/utils/file.cpp"
    },
    depfiles = "file.o: src/utils/file.cpp src/utils/../../include/utils/file.h  src/utils/../../include/utils/mmap_file.h  src/utils/../../include/utils/std_file.h\
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
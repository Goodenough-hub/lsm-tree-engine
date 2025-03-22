{
    depfiles = "file.o: src/utils/file.cpp src/utils/../../include/utils/file.h  src/utils/../../include/utils/mmap_file.h\
",
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
        "src/utils/file.cpp"
    }
}
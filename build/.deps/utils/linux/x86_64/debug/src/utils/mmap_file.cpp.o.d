{
    depfiles_format = "gcc",
    files = {
        "src/utils/mmap_file.cpp"
    },
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
    depfiles = "mmap_file.o: src/utils/mmap_file.cpp  src/utils/../../include/utils/mmap_file.h\
"
}
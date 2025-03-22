{
    depfiles = "mmap_file.o: src/utils/mmap_file.cpp  src/utils/../../include/utils/mmap_file.h\
",
    depfiles_format = "gcc",
    values = {
        "/usr/bin/gcc",
        {
            "-m64",
            "-fPIC",
            "-g",
            "-O0",
            "-std=c++20",
            "-Iinclude"
        }
    },
    files = {
        "src/utils/mmap_file.cpp"
    }
}
{
    files = {
        "src/utils/mmap_file.cpp"
    },
    depfiles = "mmap_file.o: src/utils/mmap_file.cpp  src/utils/../../include/utils/mmap_file.h\
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
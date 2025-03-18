{
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
    depfiles = "bloom_filter.o: src/utils/bloom_filter.cpp  src/utils/../../include/utils/bloom_filter.h\
",
    files = {
        "src/utils/bloom_filter.cpp"
    }
}
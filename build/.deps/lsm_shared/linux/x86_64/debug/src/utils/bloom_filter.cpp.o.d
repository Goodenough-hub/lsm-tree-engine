{
    depfiles_format = "gcc",
    files = {
        "src/utils/bloom_filter.cpp"
    },
    depfiles = "bloom_filter.o: src/utils/bloom_filter.cpp  src/utils/../../include/utils/bloom_filter.h\
",
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
    }
}
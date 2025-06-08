{
    files = {
        "src/utils/bloom_filter.cpp"
    },
    depfiles = "bloom_filter.o: src/utils/bloom_filter.cpp  src/utils/../../include/utils/bloom_filter.h\
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
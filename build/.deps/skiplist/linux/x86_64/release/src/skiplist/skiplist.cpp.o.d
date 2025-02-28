{
    depfiles = "skiplist.o: src/skiplist/skiplist.cpp  src/skiplist/../../include/skiplist/skiplist.h\
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
    },
    files = {
        "src/skiplist/skiplist.cpp"
    }
}
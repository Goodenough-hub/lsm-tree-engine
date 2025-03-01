{
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
    depfiles_format = "gcc",
    files = {
        "src/skiplist/skiplist.cpp"
    },
    depfiles = "skiplist.o: src/skiplist/skiplist.cpp  src/skiplist/../../include/skiplist/skiplist.h\
"
}
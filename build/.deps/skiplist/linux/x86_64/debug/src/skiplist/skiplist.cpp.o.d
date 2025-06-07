{
    files = {
        "src/skiplist/skiplist.cpp"
    },
    depfiles_format = "gcc",
    depfiles = "skiplist.o: src/skiplist/skiplist.cpp  src/skiplist/../../include/skiplist/skiplist.h  src/skiplist/../../include/skiplist/../iterator/iterator.h\
",
    values = {
        "/usr/bin/gcc",
        {
            "-m64",
            "-fvisibility=hidden",
            "-fvisibility-inlines-hidden",
            "-O0",
            "-std=c++20",
            "-Iinclude",
            "-D_FORTIFY_SOURCE=2",
            "-fPIC"
        }
    }
}
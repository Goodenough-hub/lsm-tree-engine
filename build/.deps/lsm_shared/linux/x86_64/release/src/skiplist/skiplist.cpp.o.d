{
    files = {
        "src/skiplist/skiplist.cpp"
    },
    depfiles = "skiplist.o: src/skiplist/skiplist.cpp  src/skiplist/../../include/skiplist/skiplist.h  src/skiplist/../../include/skiplist/../iterator/iterator.h\
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
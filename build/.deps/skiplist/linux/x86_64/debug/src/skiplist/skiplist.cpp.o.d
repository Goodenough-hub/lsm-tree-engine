{
    depfiles = "skiplist.o: src/skiplist/skiplist.cpp  src/skiplist/../../include/skiplist/skiplist.h\
",
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
    files = {
        "src/skiplist/skiplist.cpp"
    }
}
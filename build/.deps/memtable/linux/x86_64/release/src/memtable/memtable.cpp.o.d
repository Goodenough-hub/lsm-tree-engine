{
    files = {
        "src/memtable/memtable.cpp"
    },
    depfiles_format = "gcc",
    depfiles = "memtable.o: src/memtable/memtable.cpp  src/memtable/../../include/memtable/memtable.h  src/memtable/../../include/memtable/../skiplist/skiplist.h\
",
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
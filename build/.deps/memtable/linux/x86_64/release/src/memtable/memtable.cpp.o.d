{
    depfiles_format = "gcc",
    files = {
        "src/memtable/memtable.cpp"
    },
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
    depfiles = "memtable.o: src/memtable/memtable.cpp  src/memtable/../../include/memtable/memtable.h  src/memtable/../../include/memtable/../skiplist/skiplist.h\
"
}
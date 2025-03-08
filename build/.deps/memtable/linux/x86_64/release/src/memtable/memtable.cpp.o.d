{
    depfiles = "memtable.o: src/memtable/memtable.cpp  src/memtable/../../include/memtable/memtable.h  src/memtable/../../include/memtable/../skiplist/skiplist.h  src/memtable/../../include/memtable/../sst/sst.h  src/memtable/../../include/memtable/../sst/../block/block.h  src/memtable/../../include/memtable/../sst/../block/blockmeta.h  src/memtable/../../include/memtable/../sst/../utils/file.h  src/memtable/../../include/memtable/../sst/../utils/mmap_file.h\
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
    },
    files = {
        "src/memtable/memtable.cpp"
    },
    depfiles_format = "gcc"
}
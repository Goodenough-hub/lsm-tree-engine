{
    files = {
        "src/memtable/memtable.cpp"
    },
    depfiles = "memtable.o: src/memtable/memtable.cpp  src/memtable/../../include/memtable/memtable.h  src/memtable/../../include/memtable/../skiplist/skiplist.h  src/memtable/../../include/memtable/../skiplist/../iterator/iterator.h  src/memtable/../../include/memtable/../sst/sst.h  src/memtable/../../include/memtable/../sst/../block/block.h  src/memtable/../../include/memtable/../sst/../block/block_iterator.h  src/memtable/../../include/memtable/../sst/../block/blockmeta.h  src/memtable/../../include/memtable/../sst/../block/block_cache.h  src/memtable/../../include/memtable/../sst/../utils/file.h  src/memtable/../../include/memtable/../sst/../utils/mmap_file.h  src/memtable/../../include/memtable/../sst/../utils/std_file.h  src/memtable/../../include/memtable/../sst/../utils/bloom_filter.h  src/memtable/../../include/memtable/../sst/sst_iterator.h\
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
    }
}
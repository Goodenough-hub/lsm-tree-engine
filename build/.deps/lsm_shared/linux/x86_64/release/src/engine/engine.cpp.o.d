{
    files = {
        "src/engine/engine.cpp"
    },
    depfiles = "engine.o: src/engine/engine.cpp src/engine/../../include/engine/engine.h  src/engine/../../include/engine/../memtable/memtable.h  src/engine/../../include/engine/../memtable/../skiplist/skiplist.h  src/engine/../../include/engine/../memtable/../skiplist/../iterator/iterator.h  src/engine/../../include/engine/../memtable/../sst/sst.h  src/engine/../../include/engine/../memtable/../sst/../block/block.h  src/engine/../../include/engine/../memtable/../sst/../block/block_iterator.h  src/engine/../../include/engine/../memtable/../sst/../block/blockmeta.h  src/engine/../../include/engine/../memtable/../sst/../block/block_cache.h  src/engine/../../include/engine/../memtable/../sst/../utils/file.h  src/engine/../../include/engine/../memtable/../sst/../utils/mmap_file.h  src/engine/../../include/engine/../memtable/../sst/../utils/std_file.h  src/engine/../../include/engine/../memtable/../sst/../utils/bloom_filter.h  src/engine/../../include/engine/../memtable/../sst/sst_iterator.h  src/engine/../../include/engine/two_merge_iterator.h  src/engine/../../include/engine/transaction.h  src/engine/../../include/engine/../wal/record.h  src/engine/../../include/const.h  src/engine/../../include/sst/concat_iterator.h\
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
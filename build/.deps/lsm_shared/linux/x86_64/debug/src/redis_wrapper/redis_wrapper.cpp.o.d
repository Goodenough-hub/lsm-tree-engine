{
    depfiles = "redis_wrapper.o: src/redis_wrapper/redis_wrapper.cpp  src/redis_wrapper/../../include/redis_wrapper/redis_wrapper.h  src/redis_wrapper/../../include/redis_wrapper/../engine/engine.h  src/redis_wrapper/../../include/redis_wrapper/../engine/../memtable/memtable.h  src/redis_wrapper/../../include/redis_wrapper/../engine/../memtable/../skiplist/skiplist.h  src/redis_wrapper/../../include/redis_wrapper/../engine/../memtable/../../include/iterator/iterator.h  src/redis_wrapper/../../include/redis_wrapper/../engine/../memtable/../sst/sst.h  src/redis_wrapper/../../include/redis_wrapper/../engine/../memtable/../sst/../block/block.h  src/redis_wrapper/../../include/redis_wrapper/../engine/../memtable/../sst/../block/block_iterator.h  src/redis_wrapper/../../include/redis_wrapper/../engine/../memtable/../sst/../block/blockmeta.h  src/redis_wrapper/../../include/redis_wrapper/../engine/../memtable/../sst/../block/block_cache.h  src/redis_wrapper/../../include/redis_wrapper/../engine/../memtable/../sst/../utils/file.h  src/redis_wrapper/../../include/redis_wrapper/../engine/../memtable/../sst/../utils/mmap_file.h  src/redis_wrapper/../../include/redis_wrapper/../engine/../memtable/../sst/../utils/bloom_filter.h  src/redis_wrapper/../../include/redis_wrapper/../engine/../memtable/../sst/sst_iterator.h  src/redis_wrapper/../../include/const.h\
",
    values = {
        "/usr/bin/gcc",
        {
            "-m64",
            "-fPIC",
            "-g",
            "-O0",
            "-std=c++20",
            "-Iinclude"
        }
    },
    files = {
        "src/redis_wrapper/redis_wrapper.cpp"
    },
    depfiles_format = "gcc"
}
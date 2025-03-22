{
    depfiles = "handler.o: server/src/handler.cpp server/src/../include/handler.h  server/src/../include/../../include/redis_wrapper/redis_wrapper.h  server/src/../include/../../include/redis_wrapper/../engine/engine.h  server/src/../include/../../include/redis_wrapper/../engine/../memtable/memtable.h  server/src/../include/../../include/redis_wrapper/../engine/../memtable/../skiplist/skiplist.h  server/src/../include/../../include/redis_wrapper/../engine/../memtable/../../include/iterator/iterator.h  server/src/../include/../../include/redis_wrapper/../engine/../memtable/../sst/sst.h  server/src/../include/../../include/redis_wrapper/../engine/../memtable/../sst/../block/block.h  server/src/../include/../../include/redis_wrapper/../engine/../memtable/../sst/../block/block_iterator.h  server/src/../include/../../include/redis_wrapper/../engine/../memtable/../sst/../block/blockmeta.h  server/src/../include/../../include/redis_wrapper/../engine/../memtable/../sst/../block/block_cache.h  server/src/../include/../../include/redis_wrapper/../engine/../memtable/../sst/../utils/file.h  server/src/../include/../../include/redis_wrapper/../engine/../memtable/../sst/../utils/mmap_file.h  server/src/../include/../../include/redis_wrapper/../engine/../memtable/../sst/../utils/bloom_filter.h  server/src/../include/../../include/redis_wrapper/../engine/../memtable/../sst/sst_iterator.h\
",
    values = {
        "/usr/bin/gcc",
        {
            "-m64",
            "-g",
            "-O0",
            "-std=c++20",
            "-Iinclude",
            "-isystem",
            "/home/fangming/.xmake/packages/m/muduo/2022.11.01/e9382a25649e4e43bf04f01f925d9c2f/include"
        }
    },
    files = {
        "server/src/handler.cpp"
    },
    depfiles_format = "gcc"
}
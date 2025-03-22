{
    depfiles = "test_engine.o: test/test_engine.cpp test/../include/engine/engine.h  test/../include/engine/../memtable/memtable.h  test/../include/engine/../memtable/../skiplist/skiplist.h  test/../include/engine/../memtable/../../include/iterator/iterator.h  test/../include/engine/../memtable/../sst/sst.h  test/../include/engine/../memtable/../sst/../block/block.h  test/../include/engine/../memtable/../sst/../block/block_iterator.h  test/../include/engine/../memtable/../sst/../block/blockmeta.h  test/../include/engine/../memtable/../sst/../block/block_cache.h  test/../include/engine/../memtable/../sst/../utils/file.h  test/../include/engine/../memtable/../sst/../utils/mmap_file.h  test/../include/engine/../memtable/../sst/../utils/bloom_filter.h  test/../include/engine/../memtable/../sst/sst_iterator.h\
",
    depfiles_format = "gcc",
    values = {
        "/usr/bin/gcc",
        {
            "-m64",
            "-g",
            "-O0",
            "-std=c++20",
            "-Iinclude",
            "-isystem",
            "/home/fangming/.xmake/packages/g/gtest/v1.15.2/aea99fd8f63a47f0a0b9fe284bee1bfa/include"
        }
    },
    files = {
        "test/test_engine.cpp"
    }
}
{
    files = {
        "test/test_memtable.cpp"
    },
    depfiles = "test_memtable.o: test/test_memtable.cpp  test/../include/memtable/memtable.h  test/../include/memtable/../skiplist/skiplist.h  test/../include/memtable/../skiplist/../iterator/iterator.h  test/../include/memtable/../sst/sst.h  test/../include/memtable/../sst/../block/block.h  test/../include/memtable/../sst/../block/block_iterator.h  test/../include/memtable/../sst/../block/blockmeta.h  test/../include/memtable/../sst/../block/block_cache.h  test/../include/memtable/../sst/../utils/file.h  test/../include/memtable/../sst/../utils/mmap_file.h  test/../include/memtable/../sst/../utils/std_file.h  test/../include/memtable/../sst/../utils/bloom_filter.h  test/../include/memtable/../sst/sst_iterator.h\
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
            "-isystem",
            "/home/fangming/.xmake/packages/g/gtest/v1.15.2/aea99fd8f63a47f0a0b9fe284bee1bfa/include",
            "-DNDEBUG"
        }
    }
}
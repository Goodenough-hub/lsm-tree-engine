{
    depfiles_format = "gcc",
    depfiles = "test_memtable.o: test/test_memtable.cpp  test/../include/memtable/memtable.h  test/../include/memtable/../skiplist/skiplist.h  test/../include/memtable/../sst/sst.h  test/../include/memtable/../sst/../block/block.h  test/../include/memtable/../sst/../block/blockmeta.h  test/../include/memtable/../sst/../utils/file.h  test/../include/memtable/../sst/../utils/mmap_file.h  test/../include/memtable/../sst/sst_iterator.h  test/../include/memtable/../sst/../../include/block/block_iterator.h\
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
            "/home/fangming/.xmake/packages/g/gtest/v1.15.2/aea99fd8f63a47f0a0b9fe284bee1bfa/include"
        }
    },
    files = {
        "test/test_memtable.cpp"
    }
}
{
    files = {
        "test/test_sst.cpp"
    },
    depfiles = "test_sst.o: test/test_sst.cpp test/../include/const.h  test/../include/sst/sst.h test/../include/sst/../block/block.h  test/../include/sst/../block/block_iterator.h  test/../include/sst/../block/blockmeta.h  test/../include/sst/../block/block_cache.h  test/../include/sst/../utils/file.h  test/../include/sst/../utils/mmap_file.h  test/../include/sst/../utils/bloom_filter.h  test/../include/sst/sst_iterator.h\
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
    }
}
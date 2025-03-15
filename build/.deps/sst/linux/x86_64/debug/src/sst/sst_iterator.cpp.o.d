{
    depfiles = "sst_iterator.o: src/sst/sst_iterator.cpp  src/sst/../../include/sst/sst_iterator.h  src/sst/../../include/sst/../../include/block/block_iterator.h  src/sst/../../include/sst/sst.h  src/sst/../../include/sst/../block/block.h  src/sst/../../include/sst/../block/blockmeta.h  src/sst/../../include/sst/../block/block_cache.h  src/sst/../../include/sst/../utils/file.h  src/sst/../../include/sst/../utils/mmap_file.h  src/sst/../../include/sst/../utils/bloom_filter.h\
",
    files = {
        "src/sst/sst_iterator.cpp"
    },
    depfiles_format = "gcc",
    values = {
        "/usr/bin/gcc",
        {
            "-m64",
            "-g",
            "-O0",
            "-std=c++20",
            "-Iinclude"
        }
    }
}
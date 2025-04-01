{
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
        "src/sst/sst_iterator.cpp"
    },
    depfiles = "sst_iterator.o: src/sst/sst_iterator.cpp  src/sst/../../include/sst/sst_iterator.h  src/sst/../../include/sst/../../include/block/block_iterator.h  src/sst/../../include/sst/../../include/block/../iterator/iterator.h  src/sst/../../include/sst/sst.h  src/sst/../../include/sst/../block/block.h  src/sst/../../include/sst/../block/blockmeta.h  src/sst/../../include/sst/../block/block_cache.h  src/sst/../../include/sst/../utils/file.h  src/sst/../../include/sst/../utils/mmap_file.h  src/sst/../../include/sst/../utils/bloom_filter.h\
",
    depfiles_format = "gcc"
}
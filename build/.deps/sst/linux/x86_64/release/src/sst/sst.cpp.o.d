{
    files = {
        "src/sst/sst.cpp"
    },
    depfiles = "sst.o: src/sst/sst.cpp src/sst/../../include/sst/sst.h  src/sst/../../include/sst/../block/block.h  src/sst/../../include/sst/../block/block_iterator.h  src/sst/../../include/sst/../block/../iterator/iterator.h  src/sst/../../include/sst/../block/blockmeta.h  src/sst/../../include/sst/../block/block_cache.h  src/sst/../../include/sst/../utils/file.h  src/sst/../../include/sst/../utils/mmap_file.h  src/sst/../../include/sst/../utils/std_file.h  src/sst/../../include/sst/../utils/bloom_filter.h  src/sst/../../include/sst/sst_iterator.h src/sst/../../include/const.h\
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
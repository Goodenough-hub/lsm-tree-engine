{
    depfiles_format = "gcc",
    depfiles = "sst.o: src/sst/sst.cpp src/sst/../../include/sst/sst.h  src/sst/../../include/sst/../block/block.h  src/sst/../../include/sst/../block/blockmeta.h  src/sst/../../include/sst/../utils/file.h  src/sst/../../include/sst/../utils/mmap_file.h  src/sst/../../include/sst/sst_iterator.h  src/sst/../../include/sst/../../include/block/block_iterator.h\
",
    values = {
        "/usr/bin/gcc",
        {
            "-m64",
            "-g",
            "-O0",
            "-std=c++20",
            "-Iinclude"
        }
    },
    files = {
        "src/sst/sst.cpp"
    }
}
{
    depfiles_format = "gcc",
    files = {
        "test/test_memtable.cpp"
    },
    depfiles = "build/.objs/test_memtable/linux/x86_64/release/test/test_memtable.cpp.o:  test/test_memtable.cpp test/../include/memtable/memtable.h  test/../include/memtable/../skiplist/skiplist.h\
",
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
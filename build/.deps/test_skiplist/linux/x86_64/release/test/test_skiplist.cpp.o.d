{
    depfiles_format = "gcc",
    depfiles = "test_skiplist.o: test/test_skiplist.cpp  test/../include/skiplist/skiplist.h\
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
    },
    files = {
        "test/test_skiplist.cpp"
    }
}
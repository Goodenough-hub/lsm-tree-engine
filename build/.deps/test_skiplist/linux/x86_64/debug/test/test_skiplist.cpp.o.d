{
    depfiles = "test_skiplist.o: test/test_skiplist.cpp  test/../include/skiplist/skiplist.h  test/../include/skiplist/../iterator/iterator.h\
",
    files = {
        "test/test_skiplist.cpp"
    },
    depfiles_format = "gcc",
    values = {
        "/usr/bin/gcc",
        {
            "-m64",
            "-fvisibility=hidden",
            "-fvisibility-inlines-hidden",
            "-O0",
            "-std=c++20",
            "-Iinclude",
            "-D_FORTIFY_SOURCE=2",
            "-isystem",
            "/home/fangming/.xmake/packages/g/gtest/v1.15.2/aea99fd8f63a47f0a0b9fe284bee1bfa/include",
            "-fPIC"
        }
    }
}
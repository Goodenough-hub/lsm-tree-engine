{
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
    },
    depfiles = "test_skiplist.o: test/test_skiplist.cpp  test/../include/skiplist/skiplist.h\
",
    files = {
        "test/test_skiplist.cpp"
    }
}
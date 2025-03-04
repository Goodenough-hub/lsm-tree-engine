{
    depfiles_format = "gcc",
    depfiles = "test_utils.o: test/test_utils.cpp test/../include/utils/file.h  test/../include/utils/mmap_file.h\
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
        "test/test_utils.cpp"
    }
}
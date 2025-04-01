{
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
    depfiles = "test_block.o: test/test_block.cpp test/../include/block/block.h  test/../include/block/block_iterator.h  test/../include/block/../iterator/iterator.h test/../include/const.h\
",
    files = {
        "test/test_block.cpp"
    },
    depfiles_format = "gcc"
}
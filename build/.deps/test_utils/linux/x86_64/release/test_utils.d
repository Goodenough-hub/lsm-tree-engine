{
    files = {
        "build/.objs/test_utils/linux/x86_64/release/test/test_utils.cpp.o",
        "build/linux/x86_64/release/libutils.a"
    },
    values = {
        "/usr/bin/g++",
        {
            "-m64",
            "-L/home/fangming/.xmake/packages/g/gtest/v1.15.2/aea99fd8f63a47f0a0b9fe284bee1bfa/lib",
            "-Lbuild/linux/x86_64/release",
            "-s",
            "-lgtest",
            "-lgmock",
            "-lutils",
            "-lpthread"
        }
    }
}
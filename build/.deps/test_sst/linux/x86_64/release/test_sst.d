{
    files = {
        "build/.objs/test_sst/linux/x86_64/release/test/test_sst.cpp.o",
        "build/linux/x86_64/release/libutils.a",
        "build/linux/x86_64/release/libiterator.a",
        "build/linux/x86_64/release/libblock.a",
        "build/linux/x86_64/release/libsst.a"
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
            "-lsst",
            "-lblock",
            "-literator",
            "-lutils",
            "-lpthread"
        }
    }
}
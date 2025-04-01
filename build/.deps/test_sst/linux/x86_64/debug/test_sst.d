{
    values = {
        "/usr/bin/g++",
        {
            "-m64",
            "-L/home/fangming/.xmake/packages/g/gtest/v1.15.2/aea99fd8f63a47f0a0b9fe284bee1bfa/lib",
            "-Lbuild/linux/x86_64/debug",
            "-lgtest",
            "-lgmock",
            "-lsst",
            "-lblock",
            "-lutils",
            "-lpthread"
        }
    },
    files = {
        "build/.objs/test_sst/linux/x86_64/debug/test/test_sst.cpp.o",
        "build/linux/x86_64/debug/libutils.a",
        "build/linux/x86_64/debug/libblock.a",
        "build/linux/x86_64/debug/libsst.a"
    }
}
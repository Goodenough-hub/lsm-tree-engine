{
    files = {
        "build/.objs/test_memtable/linux/x86_64/debug/test/test_memtable.cpp.o",
        "build/linux/x86_64/debug/libutils.a",
        "build/linux/x86_64/debug/libblock.a",
        "build/linux/x86_64/debug/libsst.a",
        "build/linux/x86_64/debug/libiterator.a",
        "build/linux/x86_64/debug/libskiplist.a",
        "build/linux/x86_64/debug/libmemtable.a"
    },
    values = {
        "/usr/bin/g++",
        {
            "-m64",
            "-L/home/fangming/.xmake/packages/g/gtest/v1.15.2/aea99fd8f63a47f0a0b9fe284bee1bfa/lib",
            "-Lbuild/linux/x86_64/debug",
            "-lgtest",
            "-lgmock",
            "-lmemtable",
            "-lskiplist",
            "-literator",
            "-lsst",
            "-lblock",
            "-lutils",
            "-lpthread"
        }
    }
}
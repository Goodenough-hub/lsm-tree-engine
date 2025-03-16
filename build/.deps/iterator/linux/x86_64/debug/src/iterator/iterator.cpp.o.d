{
    depfiles = "build/.objs/iterator/linux/x86_64/debug/src/iterator/iterator.cpp.o:  src/iterator/iterator.cpp src/iterator/../../include/iterator/iterator.h\
",
    depfiles_format = "gcc",
    files = {
        "src/iterator/iterator.cpp"
    },
    values = {
        "/usr/bin/gcc",
        {
            "-m64",
            "-g",
            "-O0",
            "-std=c++20",
            "-Iinclude"
        }
    }
}
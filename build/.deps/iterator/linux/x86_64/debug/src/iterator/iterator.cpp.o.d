{
    depfiles_format = "gcc",
    values = {
        "/usr/bin/gcc",
        {
            "-m64",
            "-g",
            "-O0",
            "-std=c++20",
            "-Iinclude"
        }
    },
    depfiles = "iterator.o: src/iterator/iterator.cpp  src/iterator/../../include/iterator/iterator.h\
",
    files = {
        "src/iterator/iterator.cpp"
    }
}
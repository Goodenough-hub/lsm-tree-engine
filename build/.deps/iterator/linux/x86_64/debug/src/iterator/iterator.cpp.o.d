{
    depfiles_format = "gcc",
    files = {
        "src/iterator/iterator.cpp"
    },
    depfiles = "iterator.o: src/iterator/iterator.cpp  src/iterator/../../include/iterator/iterator.h\
",
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
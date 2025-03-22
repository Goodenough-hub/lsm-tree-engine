{
    depfiles = "iterator.o: src/iterator/iterator.cpp  src/iterator/../../include/iterator/iterator.h\
",
    depfiles_format = "gcc",
    values = {
        "/usr/bin/gcc",
        {
            "-m64",
            "-fPIC",
            "-g",
            "-O0",
            "-std=c++20",
            "-Iinclude"
        }
    },
    files = {
        "src/iterator/iterator.cpp"
    }
}
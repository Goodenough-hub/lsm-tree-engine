{
    files = {
        "src/iterator/iterator.cpp"
    },
    depfiles = "iterator.o: src/iterator/iterator.cpp  src/iterator/../../include/iterator/iterator.h\
",
    depfiles_format = "gcc",
    values = {
        "/usr/bin/gcc",
        {
            "-m64",
            "-fvisibility=hidden",
            "-fvisibility-inlines-hidden",
            "-O3",
            "-std=c++20",
            "-Iinclude",
            "-DNDEBUG"
        }
    }
}
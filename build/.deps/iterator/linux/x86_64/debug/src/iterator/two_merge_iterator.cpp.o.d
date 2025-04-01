{
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
    files = {
        "src/iterator/two_merge_iterator.cpp"
    },
    depfiles_format = "gcc",
    depfiles = "two_merge_iterator.o: src/iterator/two_merge_iterator.cpp  src/iterator/../../include/engine/two_merge_iterator.h  src/iterator/../../include/engine/../iterator/iterator.h\
"
}
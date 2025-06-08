{
    files = {
        "src/engine/two_merge_iterator.cpp"
    },
    depfiles = "two_merge_iterator.o: src/engine/two_merge_iterator.cpp  src/engine/../../include/engine/two_merge_iterator.h  src/engine/../../include/engine/../iterator/iterator.h\
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
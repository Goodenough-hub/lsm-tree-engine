{
    files = {
        "src/utils/std_file.cpp"
    },
    depfiles_format = "gcc",
    depfiles = "std_file.o: src/utils/std_file.cpp  src/utils/../../include/utils/std_file.h\
",
    values = {
        "/usr/bin/gcc",
        {
            "-m64",
            "-fvisibility=hidden",
            "-fvisibility-inlines-hidden",
            "-O0",
            "-std=c++20",
            "-Iinclude",
            "-D_FORTIFY_SOURCE=2",
            "-fPIC"
        }
    }
}
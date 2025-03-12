{
    depfiles_format = "gcc",
    files = {
        "src/utils/std_file.cpp"
    },
    depfiles = "std_file.o: src/utils/std_file.cpp  src/utils/../../include/utils/std_file.h\
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
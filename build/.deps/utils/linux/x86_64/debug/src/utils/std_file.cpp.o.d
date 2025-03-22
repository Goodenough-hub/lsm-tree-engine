{
    depfiles = "std_file.o: src/utils/std_file.cpp  src/utils/../../include/utils/std_file.h\
",
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
    files = {
        "src/utils/std_file.cpp"
    }
}
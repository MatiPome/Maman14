cmake_minimum_required(VERSION 3.28.3)
project(maman14 C)

set(CMAKE_C_STANDARD 90)

add_executable(maman14 main.c
        macros.c
        data_struct.c
        util.c
        first_pass.c
        errors.h
        errors.c
        second_pass.c
        code_conversion.c
        code_conversion.h
        globals.h
        table.h
        table.c)

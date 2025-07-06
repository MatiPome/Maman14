cmake_minimum_required(VERSION 3.30)
project(maman14 C)

set(CMAKE_C_STANDARD 90)

add_executable(maman14 main.c
        macros.c
        data_struct.c
        util.c
        symbol_table.c
        first_pass.c
        errors.h
        errors.c)

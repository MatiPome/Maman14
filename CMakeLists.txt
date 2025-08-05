cmake_minimum_required(VERSION 3.10)
project(mmn14_assembler C)

set(CMAKE_C_STANDARD 90)

add_executable(mmn14_assembler
        main.c
        macros.c
        first_pass.c
        second_pass.c
        code_conversion.c
        data_struct.c
        errors.c
        globals.c
        table.c
        util.c
)

# Makefile for MMN14 Assembler (C90 / ANSI C)
CC = gcc
CFLAGS = -ansi -pedantic -Wall -Wextra

# List all your source files here (except main.o)
SRCS = main.c macros.c first_pass.c second_pass.c table.c code_conversion.c data_struct.c errors.c util.c globals.c

OBJS = $(SRCS:.c=.o)

# Name of the final executable
TARGET = assembler

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) *.am *.ob *.ent *.ext

#ifndef GLOBALS_H
#define GLOBALS_H

#define MAX_INSTRUCTIONS 1024
#define MAX_DATA_SIZE 1024
#define MAX_LINE_LENGTH 100
#define MAX_LABEL_LENGTH 32

extern int IC;
extern int DC;
extern int instruction_memory[MAX_INSTRUCTIONS];
extern int data_memory[MAX_DATA_SIZE];

#endif

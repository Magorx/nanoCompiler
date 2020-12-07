CUR_PROG = general

ifndef VERBOSE
.SILENT:
endif

CC  = gcc
CPP = g++

WARNINGS = -Wall -Wno-multichar
STANDARD =  
CFLAGS = $(STANDARD) $(WARNINGS) -lm

all: main_c main_cpp

c: main_c

cpp: main_cpp

main_c: main.c general_c.h
	$(CC) $(CFLAGS) main.c -o out
	./out

main_cpp: main.cpp general_cpp.h
	$(CPP) $(CFLAGS) main.cpp -o out
	./out

run: all
	./$(CUR_PROG)

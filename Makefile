CUR_PROG = gr

ifndef VERBOSE
.SILENT:
endif

GENERAL_PREFIX = general
GC = $(GENERAL_PREFIX)/c
GCPP = $(GENERAL_PREFIX)/cpp

CC = gcc
CPP = g++

WARNINGS = -Wall -Wextra -Wno-multichar
STANDARD =  
CFLAGS = $(STANDARD) $(WARNINGS) -lm -std=c++17

all: gr

gr: main.cpp compiler.h compiler_options.h recursive_parser.h lexical_parser.h lex_token.h code_node.h $(GC)/announcement.h
	$(CPP) $(CFLAGS) main.cpp -o gr

run: all
	./$(CUR_PROG)

valg: all
	valgrind --leak-check=full --show-leak-kinds=all -s ./$(CUR_PROG)

CUR_PROG = ncc

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

all: ncc

ncc: main.cpp compiler.h id_table_scope.h id_table.h compiler_options.h recursive_parser.h lexical_parser.h lex_token.h code_node.h $(GC)/announcement.h
	$(CPP) $(CFLAGS) main.cpp -o ncc

run: all
	./$(CUR_PROG)

run_: all
	./$(CUR_PROG)
	kasm out.kc out.tf
	kspu out.tf

comp: all
	kasm out.kc out.tf

comp_run: comp out.kc
	kspu out.tf

crun:
	kspu out.tf

valg: all
	valgrind --leak-check=full --show-leak-kinds=all -s ./$(CUR_PROG)

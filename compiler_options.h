#ifndef COMPILER_OPTIONS
#define COMPILER_OPTIONS

const int GLOBAL_VARS_OFFSET = 100;
const int GLOBAL_VARS_MAX_COUNT = 100;
const int INIT_RVX_OFFSET = GLOBAL_VARS_OFFSET + GLOBAL_VARS_MAX_COUNT;
const int INIT_RMX_OFFSET = 1000;

enum CYCLES_TYPE {
	CYCLE_TYPE_WHILE = 1,
	CYCLE_TYPE_FOR   = 2
};

struct Cycle {
	int type;
	int number;

	Cycle() :
	type(0),
	number(0)
	{}

	Cycle(int type_, int number_) :
	type(type_),
	number(number_)
	{}
};

#define OPDEF(name, code, ign1, ign2, ign3) name = code,

enum OPERATION_CODE {
	#include "opcodes.h"
};

#undef OPDEF
#define OPDEF(name, code, ign1, ign2, ign3) case code : return #name + 7;

const char *OPERATION_NAME(const int opcode) {
	switch(opcode) {
		#include "opcodes.h"

		default : return nullptr;
	}
}

#undef OPDEF

bool is_normal_op(const int op) {
	return op == '+' || op == '-' ||
		   op == '*' || op == '/' ||
		   op == '^' || op == '=' ||
		   op == '<' || op == '>' ||
		   op == '?' || op == ':';
}

bool is_bracket(const int op) {
	return op == '(' || op == ')' ||
		   op == '{' || op == '}' ||
		   op == '[' || op == ']';
}

bool is_splitting_op(const int op) {
	return op == '{' || op == ';';
}

bool is_printable_op(const int op) {
	return isalpha(op) || isdigit(op) || is_normal_op(op) || is_bracket(op) || is_splitting_op(op);
}

bool is_loggable_op(const int op) {
	return !is_splitting_op(op) && is_printable_op(op);
}

bool is_compiling_loggable_op(const int op) {
	return !is_splitting_op(op);
}

#endif // COMPILER_OPTIONS
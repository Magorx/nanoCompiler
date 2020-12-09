#ifndef COMPILER_OPTIONS
#define COMPILER_OPTIONS

enum OPERATION_CODE {
	OPCODE_IF    = '?',
	OPCODE_WHILE = 'w',
	OPCODE_FOR   = 'f',

	OPCODE_LE    = 5,
	OPCODE_GE    = 6,
	OPCODE_EQ    = 7,
	OPCODE_NEQ   = 8,
	OPCODE_OR    = 10,
	OPCODE_AND   = 11,

	OPCODE_VAR_DEF  = 20,

	OPCODE_IF_CONDITION   = 30,
	OPCODE_COND_DEPENDENT = 31,

	OPCODE_WHILE_CONDITION   = 34,
	OPCODE_WHILE_BODY = 35,

	OPCODE_ELEM_PUTN  = 100,
	OPCODE_ELEM_PUTC  = 101,
	OPCODE_ELEM_INPUT = 102,
};

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
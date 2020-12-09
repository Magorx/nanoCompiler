#ifndef COMPILER_OPTIONS
#define COMPILER_OPTIONS

enum OPERATION_CODE {
	OPCODE_IF    = 'i',
	OPCODE_WHILE = 'w',
	OPCODE_FOR   = 'f'
};

bool is_normal_op(const int op) {
	return op == '+'   || op == '-'   ||
		   op == '*'   || op == '/'   ||
		   op == '^'   || op == '=';
}

bool is_bracket(const int op) {
	return op == '('   || op == ')'   ||
		   op == '{'   || op == '}'   ||
		   op == '['   || op == ']';
}

bool is_printable_op(const int op) {
	return isalpha(op) || isdigit(op) || is_normal_op(op) || is_bracket(op);
}

bool is_splitting_op(const int op) {
	return op == '{' || op == ';';
}

bool is_loggable_op(const int op) {
	return !is_splitting_op(op) && is_printable_op(op);
}

bool is_compiling_loggable_op(const int op) {
	return !is_splitting_op(op);
}

#endif // COMPILER_OPTIONS
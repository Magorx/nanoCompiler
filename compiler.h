#ifndef COMPILER
#define COMPILER

#include "general/c/announcement.h"
#include "general/cpp/file.hpp"

#include <cassert>

#include "compiler_options.h"

#include "lexical_parser.h"
#include "recursive_parser.h"
#include "id_table.h"

//=============================================================================
// Compiler ==================================================================

class Compiler {
private:
// data =======================================================================
	char *prog_text;
	RecursiveParser rec_parser;
	LexicalParser   lex_parser;
	IdTable 		id_table;


	int if_cnt;
	int while_cnt;
	int for_cnt;
//=============================================================================
	void compile_operation(const CodeNode *node, FILE *file) {
		assert(node);
		assert(file);

		#define LOG_ERROR_LINE_POS(node) RAISE_ERROR("line [%d] | pos [%d]\n", node->line, node->pos);

		#define COMPILE_L() if (node->L) compile(node->L, file)
		#define COMPILE_R() if (node->R) compile(node->R, file)
		#define COMPILE_L_COMMENT() if (node->L && is_compiling_loggable_op(node->L->get_op())) { fprintf(file, "\n; "); node->L->space_dump(file); fprintf(file, "\n");} COMPILE_L()
		#define COMPILE_R_COMMENT() if (node->R && is_compiling_loggable_op(node->R->get_op())) { fprintf(file, "\n; "); node->R->space_dump(file); fprintf(file, "\n");} COMPILE_R()
		#define COMPILE_LR() do {COMPILE_L(); COMPILE_R();} while (0)

		switch (node->get_op()) {
			case '+' : {
				if (node->L) {
					COMPILE_LR();
					fprintf(file, "add\n");
				} else {
					COMPILE_R();
				}
				break;
			}

			case '-' : {
				if (node->L) {
					COMPILE_LR();
				} else {
					fprintf(file, "push 0\n");
					COMPILE_R();
				}
				fprintf(file, "sub\n");
				break;
			}

			case '/' : {
				COMPILE_LR();
				fprintf(file, "div\n");
				break;
			}

			case '*' : {
				COMPILE_LR();
				fprintf(file, "mul\n");
				break;
			}

			case '^' : {
				COMPILE_LR();
				fprintf(file, "pow\n");
				break;
			}

			case '=' : {
				COMPILE_R();
				fprintf(file, "pop ");
				compile_variable(node->L, file);
				fprintf(file, "\n");

				break;
			}

			case '<' : {
				COMPILE_LR();
				fprintf(file, "lt\n");
				break;
			}

			case '>' : {
				COMPILE_LR();
				fprintf(file, "gt\n");
				break;
			}

			case OPCODE_LE  : {
				COMPILE_LR();
				fprintf(file, "le\n");
				break;
			}
			case OPCODE_GE  : {
				COMPILE_LR();
				fprintf(file, "ge\n");
				break;
			}

			case OPCODE_EQ  : {
				COMPILE_LR();
				fprintf(file, "eq\n");
				break;
			}

			case OPCODE_NEQ : {
				COMPILE_LR();
				fprintf(file, "neq\n");
				break;
			}

			case OP_VAR_DEF : {
				if (!node->L || !node->L->is_id()) {
					RAISE_ERROR("bad variable definition [\n");
					node->space_dump();
					printf("]\n");
					LOG_ERROR_LINE_POS(node);
					break;
				}

				bool ret = id_table.declare(ID_TYPE_VAR, node->L->get_id());
				if (!ret) {
					RAISE_ERROR("Redefinition of the id [\n");
					node->L->get_id()->print();
					printf("]\n");
					LOG_ERROR_LINE_POS(node);
					break;
				}

				if (node->R) {
					COMPILE_R();
					fprintf(file, "pop ");
					compile_variable(node->L, file);
					fprintf(file, "\n");
				}

				break;
			}

			case OP_CONDITION : {
				int cur_if_cnt = if_cnt;
				fprintf(file, "if_%d_cond:\n", cur_if_cnt);
				COMPILE_L();
				fprintf(file, "\npush 0\n");
				fprintf(file, "jne if_%d_true:\n", cur_if_cnt);
				COMPILE_R();
				fprintf(file, "\njne if_%d_end:\n", cur_if_cnt);

				++if_cnt;
				break;
			}

			case OP_COND_DEPENDENT : {
				int cur_if_cnt = if_cnt;
				fprintf(file, "if_%d_false:\n", cur_if_cnt);
				COMPILE_L_COMMENT();
				fprintf(file, "\nif_%d_true:\n", cur_if_cnt);
				COMPILE_R_COMMENT();
				break;
			}

			case '{' : {
				id_table.add_scope();
				COMPILE_L_COMMENT();
				COMPILE_R_COMMENT();
				id_table.remove_scope();

				break;
			}

			case ';' : {
				COMPILE_L_COMMENT();
				COMPILE_R_COMMENT();

				break;
			}

			default : {
				RAISE_ERROR("bad operation: [");
				node->space_dump();
				printf("]\n");
				LOG_ERROR_LINE_POS(node);
				break;
			}
		}
	}

	void compile_push(const CodeNode *node, FILE *file) {
		assert(node);
		assert(file);

		fprintf(file, "push ");
		if (node->type == VALUE) {
			compile_value(node, file);
		} else if (node->type == ID) {
			compile_variable(node, file);
		}
		fprintf(file, "\n");
	}

	void compile_value(const CodeNode *node, FILE *file) {
		assert(node);
		assert(file);

		fprintf(file, " %lg", node->get_val());
	}

	void compile_variable(const CodeNode *node, FILE *file) {
		assert(node);
		assert(file);

		if (!node->is_id()) {
			RAISE_ERROR("bad compiling type, node is [%d]\n", node->get_type());
			LOG_ERROR_LINE_POS(node);
			return;
		}

		int offset = id_table.find(ID_TYPE_VAR, node->get_id());
		if (!offset) {
			RAISE_ERROR("variable does not exist [");
			node->get_id()->print();
			printf("]\n");
			LOG_ERROR_LINE_POS(node);
			return;
		}

		fprintf(file, "[rvx + %d]", offset);
	}

	void compile_id(const CodeNode *node, FILE *file) {
		assert(node);
		assert(file);

		fprintf(file, "[%d]", node->get_var_from_id());
	}

	void compile(const CodeNode *node, FILE *file) {
		if (!node) {
			return;
		}

		switch (node->type) {
			case VALUE : {
				compile_push(node, file);
				break;
			}

			case OPERATION : {
				compile_operation(node, file);
				break;
			}

			case VARIABLE : {
				compile_variable(node, file);
				break;
			}

			case ID : {
				compile_push(node, file);
				break;
			}

			default: {
				printf("A strange node detected... [");
				node->space_dump();
				printf("]\n");
				return;
			}
		}
	}


public:
	Compiler            (const Compiler&) = delete;
	Compiler &operator= (const Compiler&) = delete;

	Compiler():
	prog_text(nullptr),
	rec_parser(),
	lex_parser(),
	id_table(),
	if_cnt(0),
	while_cnt(0),
	for_cnt(0)
	{}

	~Compiler() {}

	void ctor() {
		prog_text = nullptr;
		rec_parser.ctor();
		lex_parser.ctor();

		if_cnt    = 0;
		while_cnt = 0;
		for_cnt   = 0;
	}

	static Compiler *NEW() {
		Compiler *cake = (Compiler*) calloc(1, sizeof(Compiler));
		if (!cake) {
			return nullptr;
		}

		cake->ctor();
		return cake;
	}

	void dtor() {

	}

	static void DELETE(Compiler *compiler) {
		if (!compiler) {
			return;
		}

		compiler->dtor();
		free(compiler);
	}

//=============================================================================

	CodeNode *read_to_nodes(const File *file) {
		Vector<Token> *tokens = lex_parser.parse(file->data);
		for (size_t i = 0; i < tokens->size(); ++i) {
			(*tokens)[i].dump(stdout, false);
			printf(" ");
		}
		printf("\n");

		CodeNode *ret = rec_parser.parse(tokens);

		Vector<Token>::DELETE(tokens);

		return ret;
	}

	bool compile(const CodeNode *prog, const char *filename) {
		if (filename == nullptr) {
			RAISE_ERROR("[filename](nullptr)\n");
			return false;
		}

		FILE *file = fopen(filename, "w");
		if (!file) {
			RAISE_ERROR("[filename](%s) can't be opened\n", filename);
			return false;
		}

		if_cnt    = 0;
		while_cnt = 0;
		for_cnt   = 0;
		id_table.dtor();
		id_table.ctor();

		compile(prog, file);

		if (ANNOUNCEMENT_ERROR) {
			fprintf(file, "AN ERROR OCCURED DURING COMPILATION IUCK\n");
			ANNOUNCE("ERR", "compiler", "An error occured during compilation\n");
		}
		fclose(file);

		return true;
	}

};

#endif // COMPILER
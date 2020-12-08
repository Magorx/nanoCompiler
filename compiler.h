#ifndef COMPILER
#define COMPILER

#include "general/c/announcement.h"
#include "general/cpp/file.hpp"

#include <cassert>

#include "compiler_options.h"
#include "lexical_parser.h"
#include "recursive_parser.h"

//=============================================================================
// Compiler ==================================================================

class Compiler {
private:
// data =======================================================================
	char *prog_text;
	RecursiveParser rec_parser;
	LexicalParser   lex_parser;

	int if_cnt;
	int while_cnt;
	int for_cnt;
//=============================================================================
	void compile_operation(const CodeNode *node, FILE *file) {
		assert(node);
		assert(file);

		#define COMPILE_L() compile(node->L, file)
		#define COMPILE_R() compile(node->R, file)
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
				compile_id(node->L, file);
				fprintf(file, "\n");
				break;
			}

			case ';' : {
				if (node->R) {
					if (!node->R->is_op(';')) {
						fprintf(file, "\n; ");
						node->R->space_dump(file);
						fprintf(file, "\n");
					}
					COMPILE_R();
				}

				if (node->L) {
					if (!node->L->is_op(';')) {
						fprintf(file, "\n; ");
						node->L->space_dump(file);
						fprintf(file, "\n");
					}
					COMPILE_L();
				}
				break;
			}

			default : {
				printf("A strange operation detected... [");
				node->space_dump();
				printf("]\n");
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
			compile_id(node, file);
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

		fprintf(file, "[%d]", node->get_var_from_id());
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
			(*tokens)[i].dump(false);
			printf(" ");
		}
		printf("\n");

		CodeNode *ret = rec_parser.parse(tokens);

		Vector<Token>::DELETE(tokens);

		return ret;
	}

	bool compile(const CodeNode *prog, const char *filename) {
		if (filename == nullptr) {
			RAISE_ERROR("[filename](nullptr)");
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

		compile(prog, file);
		fclose(file);

		return true;
	}

};

#endif // COMPILER
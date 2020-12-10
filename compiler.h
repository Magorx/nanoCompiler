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

		#define DUMP_L() if (node->L) {printf("L] "); node->L->full_dump(); printf("\n");}
		#define DUMP_R() if (node->R) {printf("R] "); node->R->full_dump(); printf("\n");}
		#define COMPILE_L() if (node->L) compile(node->L, file)
		#define COMPILE_R() if (node->R) compile(node->R, file)
		#define COMPILE_L_COMMENT() if (node->L && (is_compiling_loggable_op(node->L->get_op()))) { fprintf(file, "\n; "); node->L->space_dump(file); fprintf(file, "\n");} COMPILE_L()
		#define COMPILE_R_COMMENT() if (node->R && (is_compiling_loggable_op(node->R->get_op()))) { fprintf(file, "\n; "); node->R->space_dump(file); fprintf(file, "\n");} COMPILE_R()
		#define COMPILE_LR() do {COMPILE_L(); COMPILE_R();} while (0)

		#define CHECK_ERROR() do {if (ANNOUNCEMENT_ERROR) {return;}} while (0)

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

				fprintf(file, "push ");
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

			case OPCODE_OR : {
				COMPILE_LR();
				fprintf(file, "l_or\n");
				break;
			}

			case OPCODE_AND : {
				COMPILE_LR();
				fprintf(file, "l_and\n");
				break;
			}

			case OPCODE_EXPR : {
				COMPILE_L();
				fprintf(file, "pop rzx\n");
				break;
			}

			case OPCODE_VAR_DEF : {
				if (!node->L || !node->L->is_id()) {
					RAISE_ERROR("bad variable definition [\n");
					node->space_dump();
					printf("]\n");
					LOG_ERROR_LINE_POS(node);
					break;
				}

				if (node->R) {
					COMPILE_R();
				}

				bool ret = id_table.declare(ID_TYPE_VAR, node->L->get_id());
				if (!ret) {
					RAISE_ERROR("Redefinition of the id [");
					node->L->get_id()->print();
					printf("]\n");
					LOG_ERROR_LINE_POS(node);
					break;
				}

				if (node->R) {
					fprintf(file, "pop ");
					compile_variable(node->L, file);
					fprintf(file, "\n");
				}

				break;
			}

			case OPCODE_WHILE : {
				int cur_while_cnt = ++while_cnt;
				fprintf(file, "while_%d_cond:\n", cur_while_cnt);
				COMPILE_L();
				fprintf(file, "\npush 0\n");
				fprintf(file, "je while_%d_end\n", cur_while_cnt);
				COMPILE_R();
				fprintf(file, "jmp while_%d_cond\n", cur_while_cnt);
				fprintf(file, "\nwhile_%d_end:\n", cur_while_cnt);
				break;
			}

			case OPCODE_IF : {
				int cur_if_cnt = ++if_cnt;
				fprintf(file, "if_%d_cond:\n", cur_if_cnt);
				COMPILE_L();
				fprintf(file, "\npush 0\n");
				fprintf(file, "jne if_%d_true\n", cur_if_cnt);
				COMPILE_R();
				fprintf(file, "\nif_%d_end:\n", cur_if_cnt);
				break;
			}

			case OPCODE_COND_DEPENDENT : {
				int cur_if_cnt = if_cnt;
				fprintf(file, "if_%d_false:\n", cur_if_cnt);
				COMPILE_L_COMMENT();
				fprintf(file, "\njmp if_%d_end\n", cur_if_cnt);
				fprintf(file, "\nif_%d_true:\n", cur_if_cnt);
				COMPILE_R_COMMENT();
				break;
			}

			case OPCODE_ELEM_EXIT : {
				fprintf(file, "halt\n");
				break;
			}

			case OPCODE_ELEM_PUTN : {
				if (node->R) {
					COMPILE_R();
					fprintf(file, "dup\n");
					fprintf(file, "out\n");
				} else {
					fprintf(file, "push %d\n", '\n');
					fprintf(file, "dup\n");
					fprintf(file, "out_c\n");
				}

				break;
			}

			case OPCODE_ELEM_PUTC : {
				if (node->R) {
					COMPILE_R();
				} else {
					fprintf(file, "push %d\n", ' ');
				}
				fprintf(file, "dup\n");
				fprintf(file, "out_c\n");

				break;
			}

			case OPCODE_ELEM_INPUT : {
				fprintf(file, "in\n");

				break;
			}

			case OPCODE_RET : {
				if (!node->R) {
					fprintf(file, "push 0\n");
				} else {
					COMPILE_R();
				}

				fprintf(file, "swp\n");
				fprintf(file, "ret\n");

				break;
			}

			case OPCODE_FUNC_DECL : {
				if (!node->L) {
					RAISE_ERROR("bad func decl node, func info node id is absent\n");
					LOG_ERROR_LINE_POS(node);
					break;
				}

				if (!node->L->R) {
					RAISE_ERROR("bad func info node, func id id is absent\n");
					LOG_ERROR_LINE_POS(node);
					break;
				}

				StringView *id = node->L->R->get_id();
				fprintf(file, "jmp _func_");
				id->print(file);
				fprintf(file, "_END\n");
				fprintf(file, "_func_");
				id->print(file);
				fprintf(file, "_BEGIN:\n");

				id_table.declare(ID_TYPE_FUNC, id, node->L->L);

				id_table.add_scope();
				COMPILE_L();
				COMPILE_R();
				id_table.remove_scope();
				fprintf(file, "ret\n");
				fprintf(file, "_func_");
				id->print(file);
				fprintf(file, "_END:\n");
				break;
			}

			case OPCODE_FUNC_INFO : {
				COMPILE_L();

				node->R->get_id()->print(file);
				fprintf(file, ":\n");
				break;
			}

			case OPCODE_FUNC_ARG_DECL : {
				if (!node->L) {
					if (node->R) {
						RAISE_ERROR("bad argument node, arg is absent\n");
						LOG_ERROR_LINE_POS(node);
					}
					break;
				}

				if (node->L->is_op(OPCODE_VAR_DEF)) {
					if (!node->L->L) {
						RAISE_ERROR("bad argument node, name of arg is absent\n");
						LOG_ERROR_LINE_POS(node);
						break;
					}

					id_table.declare(ID_TYPE_VAR, node->L->L->get_id());
				} else if (node->L->is_id()) {
					id_table.declare(ID_TYPE_VAR, node->L->get_id());
				} 

				COMPILE_R();
				break;
			}

			case OPCODE_FUNC_CALL : {
				compile_func_call(node, file);
				break;
			}

			case '{' : {
				id_table.add_scope();
				COMPILE_L_COMMENT();
				id_table.remove_scope();
				COMPILE_R_COMMENT();

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

	void compile_func_call(const CodeNode *node, FILE *file) {
		assert(node);
		assert(file);

		if (false && !node->L) {
			RAISE_ERROR("bad func call, arglist is absent\n");
			LOG_ERROR_LINE_POS(node);
			return;
		}

		const StringView *id = nullptr;
		if (!node->R) {
			if (node->is_id()) {
				id = node->get_id();
			} else {
				RAISE_ERROR("bad func call, func name is absent\n");
				LOG_ERROR_LINE_POS(node);
				return;
			}
		} else {
			if (!node->R->is_id()) {
				RAISE_ERROR("bad func call, func name is not a name lol\n");
				LOG_ERROR_LINE_POS(node);
				return;
			}
			id = node->R->get_id();
		}

		const CodeNode *arglist = node->L;

		if (id_table.find(ID_TYPE_FUNC, id) == NOT_FOUND) {
			RAISE_ERROR("bad func call, func not declared [");
			id->print();
			printf("]\n");
			LOG_ERROR_LINE_POS(node);
			return;
		}

		const CodeNode *func_arglist = id_table.get_arglist(id);
		if (!func_arglist) {
			RAISE_ERROR("bad func call, declared func arglist is absent\n");
			LOG_ERROR_LINE_POS(node);
			return;
		}

		//=====================================================================
		// here we definetly will compile a function

		id_table.add_scope();

		while (arglist && func_arglist && arglist->L && func_arglist->L) {
			const CodeNode *arg  = arglist->L;
			const CodeNode *prot = func_arglist->L;

			if (arg->is_op(OPCODE_DEFAULT_ARG)) {
				compile_default_arg(prot, file);
			} else if (arg->is_op(OPCODE_CONTEXT_ARG)) {
				compile_context_arg(prot, file);
			} else if (arg->is_op(OPCODE_EXPR)) {
				if (!arg->L) {
					RAISE_ERROR("bad func call, expr node has no expression inside\n");
					LOG_ERROR_LINE_POS(node);
					return;
				}
				CodeNode *expr = arg->L;
				id_table.shift_backward();
				compile(expr, file);
				id_table.shift_forward();

				if (prot->is_id()) {
					id_table.declare(ID_TYPE_VAR, prot->get_id());
					fprintf(file, "pop [rvx + %d]\n", id_table.find(ID_TYPE_VAR, prot->get_id()));
				} else if (prot->is_op(OPCODE_VAR_DEF)) {
					id_table.declare(ID_TYPE_VAR, prot->L->get_id());
					fprintf(file, "pop [rvx + %d]\n", id_table.find(ID_TYPE_VAR, prot->L->get_id()));
				} else {
					RAISE_ERROR("bad func call, unexpected PROT type [");
					printf("%d]\n", prot->get_op());
					LOG_ERROR_LINE_POS(node);
					return;
				}
			}

			arglist = arglist->R;
			func_arglist = func_arglist->R;
			CHECK_ERROR();
		}

		while (func_arglist && func_arglist->L) {
			compile_default_arg(func_arglist->L, file);
			func_arglist = func_arglist->R;
			CHECK_ERROR();
		}

		id_table.remove_scope();

		fprintf(file, "push rvx\n");
		fprintf(file, "push %d\n", id_table.get_upper_offset());
		fprintf(file, "add\n");
		fprintf(file, "pop rvx\n");

		fprintf(file, "call ");
		id->print(file);
		fprintf(file, "\n");
	}

	void compile_default_arg(const CodeNode *prot, FILE *file) {
		if (prot->is_id()) {
			id_table.shift_backward();
			compile_push(prot, file);
			id_table.shift_forward();

			id_table.declare(ID_TYPE_VAR, prot->get_id());
			fprintf(file, "pop [rvx + %d]\n", id_table.find(ID_TYPE_VAR, prot->get_id()));
		} else if (prot->is_op(OPCODE_VAR_DEF)) {
			if (!prot->R) {
				RAISE_ERROR("bad func call, required arg [");
				prot->L->get_id()->print();
				printf("] has no default set\n");
				LOG_ERROR_LINE_POS(prot);
				return;
			}

			id_table.shift_backward();
			compile(prot->R, file);
			id_table.shift_forward();

			id_table.declare(ID_TYPE_VAR, prot->L->get_id());
			fprintf(file, "pop [rvx + %d]\n", id_table.find(ID_TYPE_VAR, prot->L->get_id()));
		} else {
			RAISE_ERROR("bad func call, unexpected PROT type [");
			printf("%d]\n", prot->get_op());
			LOG_ERROR_LINE_POS(prot);
			return;
		}
	}

	void compile_context_arg(const CodeNode *prot, FILE *file) {
		if (prot->is_id()) {
			id_table.shift_backward();
			compile_push(prot, file);
			id_table.shift_forward();

			id_table.declare(ID_TYPE_VAR, prot->get_id());
			fprintf(file, "pop [rvx + %d]\n", id_table.find(ID_TYPE_VAR, prot->get_id()));
		} else if (prot->is_op(OPCODE_VAR_DEF)) {
			id_table.shift_backward();
			compile_push(prot->L, file);
			id_table.shift_forward();

			id_table.declare(ID_TYPE_VAR, prot->L->get_id());
			fprintf(file, "pop [rvx + %d]\n", id_table.find(ID_TYPE_VAR, prot->L->get_id()));
		} else {
			RAISE_ERROR("bad func call, unexpected PROT type [");
			printf("%d]\n", prot->get_op());
			LOG_ERROR_LINE_POS(prot);
			return;
		}
	}

	void compile_push(const CodeNode *node, FILE *file) {
		assert(node);
		assert(file);

		fprintf(file, "push ");
		if (node->type == VALUE) {
			if (node->get_val() < 0) {
				fprintf(file, "0\npush ");
				
				CodeNode tmp = {};
				tmp.ctor(VALUE, fabs(node->get_val()), nullptr, nullptr, node->line, node->pos);
				compile_value(&tmp, file);
				tmp.dtor();

				fprintf(file, "\nsub\n");
			} else {
				compile_value(node, file);
			}
		} else if (node->type == ID) {
			compile_variable(node, file);
		}
		fprintf(file, "\n");
	}

	void compile_value(const CodeNode *node, FILE *file) {
		assert(node);
		assert(file);

		fprintf(file, "%lf", node->get_val());
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

		// node->get_id()->print();
		// printf("| finding\n");
		// id_table.dump();

		if (offset == NOT_FOUND) {
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

		//node->gv_dump();

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
				if (id_table.find(ID_TYPE_FUNC, node->get_id()) != NOT_FOUND) {
					compile_func_call(node, file);
				} else {
					compile_push(node, file);
				}
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
		// for (size_t i = 0; i < tokens->size(); ++i) {
		// 	(*tokens)[i].dump(stdout, false);
		// 	printf(" ");
		// }
		// printf("\n");

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

		fprintf(file, "call MAIN\n");
		fprintf(file, "halt\n");

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
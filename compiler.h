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
				compile_lvalue(node->L, file);
				fprintf(file, "\n");

				fprintf(file, "push ");
				compile_lvalue(node->L, file);
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
				compile_expr(node, file, true);
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

				bool ret = id_table.declare_var(node->L->get_id(), 1);
				if (!ret) {
					RAISE_ERROR("Redefinition of the id [");
					node->L->get_id()->print();
					printf("]\n");
					LOG_ERROR_LINE_POS(node);
					break;
				}

				if (node->R) {
					fprintf(file, "pop ");
					compile_lvalue(node->L, file);
					fprintf(file, "\n");
				}

				break;
			}

			case OPCODE_ARR_DEF : {
				CodeNode *arr_name = node->L->R;
				if (!node->L || !node->L->is_op(OPCODE_ARR_INFO)) {
					RAISE_ERROR("bad variable definition [\n");
					node->space_dump();
					printf("]\n");
					LOG_ERROR_LINE_POS(node);
					break;
				}
				
				bool ret = id_table.declare_var(arr_name->get_id(), (int) node->L->L->get_val());
				if (!ret) {
					RAISE_ERROR("Redefinition of the id [");
					arr_name->get_id()->print();
					printf("]\n");
					LOG_ERROR_LINE_POS(node);
					break;
				}

				int offset = 0;
				id_table.find_var(arr_name->get_id(), &offset);
				fprintf(file, "push %d\n", offset);
				fprintf(file, "push rvx\n");
				fprintf(file, "add\n");
				fprintf(file, "pop [rvx + %d]\n", offset);

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

			case OPCODE_FOR : {
				int cur_for_cnt = ++for_cnt;
				if (!node->L || !node->R || !node->L->L || !node->L->R || !node->L->L->L || !node->L->L->R) {
					RAISE_ERROR("bad for node, something is missing\n");
					break;
				}
				
				id_table.add_scope();
				fprintf(file, "\nfor_%d_init_block:\n", cur_for_cnt);
				compile(node->L->L->L, file);

				fprintf(file, "\nfor_%d_start:\n", cur_for_cnt);
				fprintf(file, "\nfor_%d_cond:\n", cur_for_cnt);
				compile(node->L->L->R, file);
				fprintf(file, "\npush 0\n");
				fprintf(file, "je for_%d_end\n", cur_for_cnt);

				compile(node->R, file);

				compile_expr(node->L->R, file, true);
				fprintf(file, "jmp for_%d_cond\n", cur_for_cnt);
				fprintf(file, "\nfor_%d_end:\n", cur_for_cnt);

				id_table.remove_scope();
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

				id_table.declare_func(id, node->L->L);

				id_table.add_scope(true);
				COMPILE_L();
				COMPILE_R();
				id_table.remove_scope();
				fprintf(file, "push 0\n");
				fprintf(file, "swp\n");
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

					id_table.declare_var(node->L->L->get_id(), 1);
				} else if (node->L->is_id()) {
					id_table.declare_var(node->L->get_id(), 1);
				} 

				COMPILE_R();
				break;
			}

			case OPCODE_FUNC_CALL : {
				if (id_table.find_func(node->R->get_id()) == NOT_FOUND) {
					compile_arr_call(node, file);
				} else {
					compile_func_call(node, file);
				}
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

	void compile_expr(const CodeNode *node, FILE *file, const bool to_pop = false) {
		if (node->is_op(OPCODE_EXPR)) {
			COMPILE_L();
		} else {
			compile(node, file);
		}

		if (to_pop) {
			fprintf(file, "pop rzx\n");
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

		if (id_table.find_func(id) == NOT_FOUND) {
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
				compile_default_arg(arg, prot, file);
			} else if (arg->is_op(OPCODE_CONTEXT_ARG)) {
				compile_context_arg(arg, prot, file);
			} else if (arg->is_op(OPCODE_EXPR)) {
				compile_expr_arg(arg, prot, file);
			}

			arglist = arglist->R;
			func_arglist = func_arglist->R;
			CHECK_ERROR();
		}

		while (func_arglist && func_arglist->L) {
			compile_default_arg(node, func_arglist->L, file);
			func_arglist = func_arglist->R;
			CHECK_ERROR();
		}

		id_table.remove_scope();

		fprintf(file, "push rvx\n");
		fprintf(file, "push %d\n", id_table.get_func_offset());
		fprintf(file, "add\n");
		fprintf(file, "pop rvx\n");

		fprintf(file, "call ");
		id->print(file);
		fprintf(file, "\n");

		fprintf(file, "push rvx\n");
		fprintf(file, "push %d\n", id_table.get_func_offset());
		fprintf(file, "sub\n");
		fprintf(file, "pop rvx\n");
	}

	void compile_default_arg(const CodeNode *arg, const CodeNode *prot, FILE *file) {
		if (prot->is_id()) {
			id_table.shift_backward();
			bool ret = compile_push(prot, file);
			id_table.shift_forward();

			if (!ret) {
				RAISE_ERROR("func call arg position:\n");
				LOG_ERROR_LINE_POS(arg);
				return;
			}

			id_table.declare_var(prot->get_id(), 1);
			fprintf(file, "pop ");
			compile_lvalue(prot, file);
			fprintf(file, "\n");
		} else if (prot->is_op(OPCODE_VAR_DEF)) {
			if (!prot->R) {
				RAISE_ERROR("bad func call, required arg [");
				prot->L->get_id()->print();
				printf("] has no default set\n");
				LOG_ERROR_LINE_POS(arg);
				return;
			}

			id_table.shift_backward();
			compile(prot->R, file);
			id_table.shift_forward();

			id_table.declare_var(prot->L->get_id(), 1);
			fprintf(file, "pop ");
			compile_lvalue(prot->L, file);
			fprintf(file, "\n");
		} else {
			RAISE_ERROR("bad func call, unexpected PROT type [");
			printf("%d]\n", prot->get_op());
			LOG_ERROR_LINE_POS(prot);
			return;
		}
	}

	void compile_context_arg(const CodeNode *arg, const CodeNode *prot, FILE *file) {
		if (prot->is_id()) {
			id_table.shift_backward();
			bool ret = compile_push(prot, file);
			id_table.shift_forward();

			if (!ret) {
				RAISE_ERROR("func call arg position:\n");
				LOG_ERROR_LINE_POS(arg);
				return;
			}

			id_table.declare_var(prot->get_id(), 1);
			fprintf(file, "pop ");
			compile_lvalue(prot, file);
			fprintf(file, "\n");
		} else if (prot->is_op(OPCODE_VAR_DEF)) {
			id_table.shift_backward();
			bool ret = compile_push(prot->L, file);
			id_table.shift_forward();

			if (!ret) {
				RAISE_ERROR("func call arg position:\n");
				LOG_ERROR_LINE_POS(arg);
				return;
			}

			id_table.declare_var(prot->L->get_id(), 1);
			fprintf(file, "pop ");
			compile_lvalue(prot->L, file);
			fprintf(file, "\n");
		} else {
			RAISE_ERROR("bad func call, unexpected PROT type [");
			printf("%d]\n", prot->get_op());
			LOG_ERROR_LINE_POS(arg);
			return;
		}
	}

	void compile_expr_arg(const CodeNode *arg, const CodeNode *prot, FILE *file) {
		if (!arg->L) {
			RAISE_ERROR("bad func call, expr node has no expression inside\n");
			LOG_ERROR_LINE_POS(arg);
			return;
		}
		CodeNode *expr = arg->L;
		id_table.shift_backward();
		compile(expr, file);
		id_table.shift_forward();

		if (prot->is_id()) {
			id_table.declare_var(prot->get_id(), 1);
			fprintf(file, "pop ");
			compile_lvalue(prot, file);
			fprintf(file, "\n");
		} else if (prot->is_op(OPCODE_VAR_DEF)) {
			id_table.declare_var(prot->L->get_id(), 1);
			fprintf(file, "pop ");
			compile_lvalue(prot->L, file);
			fprintf(file, "\n");
		} else {
			RAISE_ERROR("bad func call, unexpected PROT type [");
			printf("%d]\n", prot->get_op());
			LOG_ERROR_LINE_POS(arg);
			return;
		}
	}

	void compile_arr_call(const CodeNode *node, FILE *file) {
		if (!node->R) {
			RAISE_ERROR("bad arr call, where is name, you are worthless [");
			printf("%d]\n", node->get_op());
			LOG_ERROR_LINE_POS(node);
			return;
		}

		CodeNode *id = node->R;
		CodeNode *arg = node->L->L;
		if (!arg->is_op(OPCODE_EXPR)) {
			RAISE_ERROR("bad arr call, argument is not an expr [");
			printf("%d]\n", node->get_op());
			LOG_ERROR_LINE_POS(node);
			return;
		}

		int offset = 0;
		int ret = id_table.find_var(id->get_id(), &offset);
		if (ret == NOT_FOUND) {
			RAISE_ERROR("variable does not exist [");
			id->get_id()->print();
			printf("]\n");
			LOG_ERROR_LINE_POS(node);
			return;
		}

		compile_expr(arg, file);
		fprintf(file, "push [rvx + %d]\n", offset);
		fprintf(file, "add\n");
		fprintf(file, "pop rax\n");
		fprintf(file, "push [rax]\n");
	}

	bool compile_push(const CodeNode *node, FILE *file) {
		assert(node);
		assert(file);

		fprintf(file, "push ");
		bool result = false;
		if (node->type == VALUE) {
			if (node->get_val() < 0) {
				fprintf(file, "0\npush ");
				
				CodeNode tmp = {};
				tmp.ctor(VALUE, fabs(node->get_val()), nullptr, nullptr, node->line, node->pos);
				compile_value(&tmp, file);
				tmp.dtor();

				fprintf(file, "\nsub\n");
			} else {
				result = compile_value(node, file);
			}
		} else if (node->type == ID) {
			result = compile_lvalue(node, file);
		}
		fprintf(file, "\n");
		return result;
	}

	bool compile_value(const CodeNode *node, FILE *file) {
		assert(node);
		assert(file);

		fprintf(file, "%lf", node->get_val());
		return true;
	}

	bool compile_lvalue(const CodeNode *node, FILE *file, const bool for_asgn_dup = false) {
		assert(node);
		assert(file);

		if (node->is_id()) {
			// printf("\n\n==================\n");
			// printf("compile var |");
			// node->get_id()->print();
			// printf("|\n");
			// printf("~~~~~~~~~~~~~~~~~~\n");
			// printf("cur id_table:\n");
			// id_table.dump();

			int offset = 0;
			int is_found = id_table.find_var(node->get_id(), &offset);

			// printf("\nfound = %d", offset);
			// printf("\n==================\n");



			if (is_found == NOT_FOUND) {
				RAISE_ERROR("variable does not exist [");
				node->get_id()->print();
				printf("]\n");
				LOG_ERROR_LINE_POS(node);
				return false;
			}

			if (is_found == ID_TYPE_GLOBAL) {
				fprintf(file, "[%d]\n", GLOBAL_VARS_OFFSET + offset);
			} else {
				fprintf(file, "[rvx + %d]", offset);
			}
			return true;
		} else if (node->is_op(OPCODE_FUNC_CALL) && id_table.find_func(node->R->get_id()) == NOT_FOUND) { // so that's an array
			CodeNode *id  = node->R;
			CodeNode *arg = node->L->L;
			if (!arg || !arg->is_op(OPCODE_EXPR)) {
				RAISE_ERROR("bad arr call, argument is not an expr [");
				printf("%d]\n", node->get_op());
				LOG_ERROR_LINE_POS(node);
				return false;
			}

			int offset = 0;
			int ret = id_table.find_var(id->get_id(), &offset);
			if (ret == NOT_FOUND) {
				RAISE_ERROR("variable does not exist [");
				id->get_id()->print();
				printf("]\n");
				LOG_ERROR_LINE_POS(node);
				return false;
			}

			// we are called by assign, so there's 'pop ' already written in assembler, let's fix it
			if (for_asgn_dup) {
				fprintf(file, "0\n");
				fprintf(file, "pop rzx\n");
				fprintf(file, "push [rax + %d]\n", offset);
				return true;
			}

			fprintf(file, "rbx\n");
			fprintf(file, "push rbx\n");

			compile_expr(arg, file);
			fprintf(file, "push [rvx + %d]\n", offset);
			fprintf(file, "add\n");
			fprintf(file, "pop rax\n");
			fprintf(file, "pop [rax]\n");
			return true;
		} else {
			RAISE_ERROR("bad compiling type, node is [%d]\n", node->get_type());
			LOG_ERROR_LINE_POS(node);
			return false;
		}
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
				compile_lvalue(node, file);
				break;
			}

			case ID : {
				if (id_table.find_func(node->get_id()) != NOT_FOUND) {
					compile_func_call(node, file);
				} else if (node->R) {
					compile_arr_call(node, file);
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
		id_table.dtor();
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

		fprintf(file, "push %d\n", INIT_RVX_OFFSET);
		fprintf(file, "pop rvx\n");

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
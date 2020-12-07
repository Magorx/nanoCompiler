#include <cstdlib>
#include <cstdio>

#include "general/constants.h"
#include "general/warnings.h"

#include "compiler.h"

int main() {
	const char *file_name = "prog.wzr";

	File file = {};
	file.ctor(file_name);
	if (!file.data) {
		ANNOUNCE("ERR", "compiler", "problems with input file [%s]", file_name);
		return 0;
	}

	Compiler comp = {};
	CodeNode *prog = comp.read_to_nodes(&file);

	prog->space_dump();
	printf("\n");

	if (!prog) {
	} else {
		const int var_tabe_size = 257;
		double var_table[var_tabe_size];
		for (int i = 0; i < var_tabe_size; ++i) {
			var_table[i] = KCTF_POISON;
		}

		//================== vars here
		//================== vars here

		printf("+---+------------+\n");
		for (int i = 0; i < var_tabe_size; ++i) {
			if (fabs(var_table[i] - KCTF_POISON) > GENERAL_EPS) {
				printf("| %c | % 11lf|\n", i, var_table[i]);
			}
		}
		printf("+---+------------+\n");
		printf("    |\n    Y\n");

		printf("res = [%10lf]\n", prog->evaluate_expr(var_table, var_tabe_size));
		printf("+---+------------+\n");
		for (int i = 0; i < var_tabe_size; ++i) {
			if (fabs(var_table[i] - KCTF_POISON) > GENERAL_EPS) {
				printf("| %c | % 11lf|\n", i, var_table[i]);
			}
		}
		printf("+---+------------+\n");
	}

	CodeNode::DELETE(prog, true, true);
	file.dtor();

	printf(".doned.\n");
	return 0;
}
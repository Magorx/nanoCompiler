#include <cstdlib>
#include <cstdio>

// TODO GLOBAL VARIABLES
// TODO ARRAYS
// TODO FOR
// TODO 

#include "general/c/debug.h"

#include "general/constants.h"
#include "general/warnings.h"

#include "compiler.h"

//#define TEST
int test();

int main() {
#ifdef TEST
	test();
#else
	const char *file_name = "prog.wzr";

	File file = {};
	file.ctor(file_name);
	if (!file.data) {
		ANNOUNCE("ERR", "compiler", "problems with input file [%s]", file_name);
		return 0;
	}

	Compiler comp = {};
	comp.ctor();
	CodeNode *prog = comp.read_to_nodes(&file);

	// prog->gv_dump();

	comp.compile(prog, "out.kc");

	CodeNode::DELETE(prog, true, true);
	file.dtor();
	comp.dtor();

	printf(".doned.\n");
#endif
	return 0;
}

int test() {
	return 0;
}
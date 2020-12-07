#ifndef COMPILER
#define COMPILER

#include "general/c/announcement.h"
#include "general/cpp/file.hpp"

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
//=============================================================================


public:
	Compiler            (const Compiler&) = delete;
	Compiler &operator= (const Compiler&) = delete;

	Compiler():
	prog_text(nullptr),
	rec_parser(),
	lex_parser()
	{}

	~Compiler() {}

	void ctor() {
		prog_text = nullptr;
		rec_parser.ctor();
		lex_parser.ctor();
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

};

#endif // COMPILER
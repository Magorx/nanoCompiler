#ifndef RECURSIVE_PARSER
#define RECURSIVE_PARSER

/* Grammar ================================\

G ::= PROG'\0'

PROG       ::= CODE_BLOCK;{CODE_BLOCK;}*
CODE_BLOCK ::= EXPR; | { CODE_BLOCK;{CODE_BLOCK;}* }

EXPR ::= CALC{=CALC}*

CALC ::= TERM{[+-]TERM}*
TERM ::= FACT{{/|*}FACT}*
FACT ::= [+-]FACT | UNIT^FACT | UNIT
UNIT ::= ID(EXPR) | ID | (EXPR) | NUMB
NUMB ::= [+-]?[0-9]+{.[0-9]+}?{[eE][0-9]+}?
ID   ::= [a-zA-Z_][a-zA-Z_0-9]*

*///=======================================/

#include "general/c/announcement.h"
#include "general/cpp/stringview.hpp"
#include "general/cpp/vector.hpp"

#include "general/c/debug.h"

#include <cmath>

#include "code_node.h"
#include "lex_token.h"

typedef CodeNode ParseNode;

enum PARSER_ERROR {
	OK = 0,

	ERROR_SYNTAX = 100,
};

//=============================================================================
// RecursiveParser ============================================================

class RecursiveParser {
private:
// data =======================================================================
	Vector<Token> *expr;
	int            cur_index;
	Token         *cur;

	int            ERROR;
	const Token   *ERRPOS;
//=============================================================================
	#define NEXT() ++cur_index; cur = &(*expr)[cur_index]

	#define REQUIRE_OP(op)                                  \
		do {                                                \
			if (cur->type != T_OP || cur->data.op != op) {  \
				ERROR  = ERROR_SYNTAX;                      \
				ERRPOS = cur;                               \
				return nullptr;                             \
			} else {                                        \
				NEXT();                                     \
			}                                               \
		} while (0)

	#define IF_PARSED(index, ret_name, code)          \
		ParseNode *ret_name = (code);                 \
		if (ERROR) {                                  \
			cur_index = index;                        \
			cur = &(*expr)[cur_index];                \
			SET_ERR(0, &(*expr)[0]);                  \
		} else

	#define SET_ERR(errcode, errpos) do {ERROR = errcode; ERRPOS = errpos;} while (0)

	bool is_id_char(const char c) {
		return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
	}

	bool is_digit(const char c) {
		return ('0' <= c && c <= '9');
	}

	bool is_sign(const char c) {
		return c == '+' || c == '-';
	}

	bool is_sign(const Token *t) {
		return t->is_op('+') || t->is_op('-');
	}

	bool is_multiplicative(const char c) {
		return c == '*' || c == '/';
	}

	bool is_multiplicative(const Token *t) {
		return t->is_op('*') || t->is_op('/');
	}

	ParseNode *parse_ID() {
		if (!cur->is_id()) {
			SET_ERR(ERROR_SYNTAX, cur);
			return nullptr;
		}

		CodeNode *ret = ParseNode::NEW(ID, cur->data.id);
		NEXT();
		return ret;
	}

	ParseNode *parse_NUMB() {
		if (!cur->is_number()) {
			SET_ERR(ERROR_SYNTAX, cur);
			return nullptr;
		}
		CodeNode *ret = ParseNode::NEW(VALUE, cur->data.num);
		NEXT();
		return ret;
	}

	ParseNode *parse_UNIT() {
		IF_PARSED (cur_index, unit_id, parse_ID()) {
			if (cur->is_op('(')) {
				NEXT();
				IF_PARSED (cur_index, bracket_expr, parse_EXPR()) {
					if (cur->is_op(')')) {
						NEXT();
						unit_id->set_R(bracket_expr);
						return unit_id;
					}
				}
				NEXT();
				SET_ERR(ERROR_SYNTAX, cur);
				ParseNode::DELETE(unit_id);
				return nullptr;
			} else {
				return unit_id;
			}
		}

		if (cur->is_op('(')) {
			NEXT();
			IF_PARSED (cur_index, expr_in_brackets, parse_EXPR()) {
				if (cur->is_op(')')) {
					NEXT();
					return expr_in_brackets;
				} else {
					ParseNode::DELETE(expr_in_brackets);
				}
			}
			NEXT();
			SET_ERR(ERROR_SYNTAX, cur);
			return nullptr;
		}

		IF_PARSED (cur_index, number, parse_NUMB()) {
			return number;
		}

		SET_ERR(ERROR_SYNTAX, cur);
		return nullptr;
	}

	ParseNode *parse_FACT() {
		if (is_sign(cur)) {
			int sign = cur->get_op();
			NEXT();
			IF_PARSED (cur_index, fact, parse_FACT()) {
				return ParseNode::NEW(OPERATION, sign, nullptr, fact);
			}
			SET_ERR(ERROR_SYNTAX, cur);
			return nullptr;
		}

		IF_PARSED (cur_index, unit, parse_UNIT()) {
			if (cur->is_op('^')) {
				NEXT();
				IF_PARSED (cur_index, fact, parse_FACT()) {
					return ParseNode::NEW(OPERATION, '^', unit, fact);
				}
				ParseNode::DELETE(unit);
				SET_ERR(ERROR_SYNTAX, cur);
				return nullptr;
			} else {
				return unit;
			}
		}
		
		SET_ERR(ERROR_SYNTAX, cur);
		return nullptr;
	}

	ParseNode *parse_TERM() {
		IF_PARSED (cur_index, fact, parse_FACT()) {
			while (is_multiplicative(cur)) {
				int op = cur->get_op();
				NEXT();

				IF_PARSED (cur_index, next_fact, parse_FACT()) {
					fact = ParseNode::NEW(OPERATION, op, fact, next_fact);
					continue;
				}

				ParseNode::DELETE(fact, true);
				SET_ERR(ERROR_SYNTAX, cur);
				return nullptr;
			}

			return fact;
		}

		SET_ERR(ERROR_SYNTAX, cur);
		return nullptr;
	}

	ParseNode *parse_CALC() {
		IF_PARSED (cur_index, term, parse_TERM()) {
			while (is_sign(cur)) {
				int op = cur->get_op();
				NEXT();

				IF_PARSED (cur_index, next_term, parse_TERM()) {
					term = ParseNode::NEW(OPERATION, op, term, next_term);
					continue;
				}

				ParseNode::DELETE(term, true);
				SET_ERR(ERROR_SYNTAX, cur);
				return nullptr;
			}

			return term;
		}

		SET_ERR(ERROR_SYNTAX, cur);
		return nullptr;
	} 

	ParseNode *parse_EXPR() {
		IF_PARSED (cur_index, calc, parse_CALC()) {
			while (cur->is_op('=')) {
				int op = cur->get_op();
				NEXT();

				IF_PARSED (cur_index, next_calc, parse_EXPR()) {
					calc = ParseNode::NEW(OPERATION, op, calc, next_calc);
					continue;
				}

				ParseNode::DELETE(calc, true);
				SET_ERR(ERROR_SYNTAX, cur);
				return nullptr;
			}

			return calc;
		}

		SET_ERR(ERROR_SYNTAX, cur);
		return nullptr;		
	}

	ParseNode *parse_CODE_BLOCK() {
		if (cur->is_op('{')) {
			NEXT();
			IF_PARSED (cur_index, code_block, parse_CODE_BLOCK()) {
				while (!cur->is_op('}')) {
					IF_PARSED(cur_index, next_block, parse_CODE_BLOCK()) {
						code_block = ParseNode::NEW(OPERATION, ';', code_block, next_block);
						continue;
					}

					ParseNode::DELETE(code_block, true);
					SET_ERR(ERROR_SYNTAX, cur);
					return nullptr;
				}

				NEXT();
				return code_block;
			}

			ParseNode::DELETE(code_block, true);
			SET_ERR(ERROR_SYNTAX, cur);
			return nullptr;
		}

		IF_PARSED (cur_index, expr_node, parse_EXPR()) {
			if (!cur->is_op(';')) {
				ParseNode::DELETE(expr_node, true);
				SET_ERR(ERROR_SYNTAX, cur);
				return nullptr;
			}

			NEXT();
			return expr_node;
		}

		SET_ERR(ERROR_SYNTAX, cur);
		return nullptr;
	}

	ParseNode *parse_PROG() {
		return parse_CODE_BLOCK();
	}      

	ParseNode *parse_G() {
		IF_PARSED (cur_index, prog, parse_PROG()) {
			if (cur->type == T_END) {
				return prog;
			} else {
				ParseNode::DELETE(prog, true);
			}
		}

		SET_ERR(ERROR_SYNTAX, cur);

		return nullptr;
	}

public:
	RecursiveParser            (const RecursiveParser&) = delete;
	RecursiveParser &operator= (const RecursiveParser&) = delete;

	RecursiveParser():
	expr(nullptr),
	cur_index(0),
	cur(nullptr),
	ERROR(0),
	ERRPOS(nullptr)
	{}

	~RecursiveParser() {}

	void ctor() {
		expr = nullptr;
		cur_index = 0;
		cur = nullptr;
		ERROR = 0;
		ERRPOS = nullptr;
	}

	static RecursiveParser *NEW() {
		RecursiveParser *cake = (RecursiveParser*) calloc(1, sizeof(RecursiveParser));
		if (!cake) {
			return nullptr;
		}

		cake->ctor();
		return cake;
	}

	void dtor() {
		expr = nullptr;
		cur_index = 0;
		cur = nullptr;

		ERROR  = 0;
		ERRPOS = 0;
	}

	static void DELETE(RecursiveParser *classname) {
		if (!classname) {
			return;
		}

		classname->dtor();
		free(classname);
	}

//=============================================================================

	ParseNode *parse(Vector<Token> *expression) {
		expr      = expression;
		cur_index = 0;
		cur       = &(*expr)[cur_index];

		ParseNode *res = parse_G();
		if (!ERROR) {
			return res;
		} else {
			//RAISE_ERROR_SYNTAX(init_expr_ptr, ERRPOS - init_expr_ptr);
			ANNOUNCE("ERR", "parser", "grammar unfit expression, check pos [%ld]", ERRPOS - expr->get_buffer());
			printf(">>> ");
			ERRPOS->dump();
			printf(" <<<\n");
			return nullptr;
		}
	}

};

#endif // RECURSIVE_PARSER
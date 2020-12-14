#ifndef LEXICAL_PARSER
#define LEXICAL_PARSER

#include "general/cpp/stringview.hpp"
#include "general/cpp/vector.hpp"

#include <cctype>
#include <cmath>

#include "compiler_options.h"
#include "lex_token.h"

//=============================================================================
// LexicalParser ==================================================================

class LexicalParser {
private:
	#define CUR_POS (int)(cur - cur_line)
	#define ADD_TOKEN(type, data) Token token = {}; token.ctor(type, data, line, CUR_POS); tokens->push_back(token)

// data =======================================================================
	const char *cur_expr;
	const char *cur_line;
	const char *cur;
	Vector<Token> *tokens;
	char skip_mode;
	int line;
//=============================================================================

	bool is_id_char(const char c) {
		return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
	}

	bool is_sign(const char c) {
		return c == '+' || c == '-';
	}

	bool two_available() {
		return *cur && *(cur + 1);
	}

	bool three_available() {
		return *cur && *(cur + 1) && *(cur + 2);
	}

	void collect_number() {
		Token token = {};
		token.ctor(T_NUMBER, 0, line, CUR_POS);

		char sign = '+';
		if (is_sign(*cur)) {
			sign = *cur;
			++cur;
		}

		double val = 0;
		while (isdigit(*cur)) {
			val = val * 10 + (*cur - '0');
			++cur;
		}

		if (two_available() && *cur == '.' && isdigit(*(cur + 1))) {
			++cur;

			double frac = 0;
			double len_frac = 1;
			while (isdigit(*cur)) {
				len_frac *= 10;
				frac = frac * 10 + (*cur - '0');
				++cur;
			}

			val = val + frac / (len_frac);
		}

		if (two_available() && (*cur == 'e' || *cur == 'E') && (isdigit(*(cur + 1)) || (three_available() && is_sign(*(cur + 1)) && isdigit(*(cur + 2))))) {
			++cur;

			char exp_sign = '+';
			if (is_sign(*cur)) {
				exp_sign = *cur;
				++cur;
			}

			double ten_pow = 0;
			while (isdigit(*cur)) {
				ten_pow = ten_pow * 10 + (*cur - '0');
				++cur;
			}

			if (exp_sign == '-') {
				ten_pow *= -1;
			}

			val = val * pow(10, ten_pow);
		}

		if (sign == '-') {
			val *= -1;
		}

		token.data.num = val;
		tokens->push_back(token);
	}

	void collect_id() {
		StringView *id = StringView::NEW(cur);
		size_t len = 0;
		while (is_id_char(*cur) || isdigit(*cur)) {
			++len;
			++cur;
		}
		id->set_length(len);

		ADD_TOKEN(T_ID, id);
	}

	bool try_collect_long_op() {
		if (*cur == '\'' && three_available() && (*(cur + 2) == '\'')) {
			ADD_TOKEN(T_NUMBER, (double) *(cur + 1));
			cur += 3;
			return true;
		} else if (StringView::starts_with(cur, "?")) {
			cur += 1;
			ADD_TOKEN(T_OP, OPCODE_IF);
			return true;
		} else if (*cur == '@') {
			cur += 1;
			ADD_TOKEN(T_OP, OPCODE_ELEM_INPUT);
			return true;
		} else if (StringView::starts_with(cur, ">|")) {
			cur += 2;
			ADD_TOKEN(T_OP, OPCODE_WHILE);
			return true;
		} else if (StringView::starts_with(cur, ">>")) {
			cur += 2;
			ADD_TOKEN(T_OP, OPCODE_FOR);
			return true;
		} else if (StringView::starts_with(cur, "for ")) {
			cur += 4;
			ADD_TOKEN(T_OP, OPCODE_FOR);
			return true;
		} else if (StringView::starts_with(cur, "<=")) {
			cur += 2;
			ADD_TOKEN(T_OP, OPCODE_LE);
			return true;
		} else if (StringView::starts_with(cur, ">=")) {
			cur += 2;
			ADD_TOKEN(T_OP, OPCODE_GE);
			return true;
		} else if (StringView::starts_with(cur, "==")) {
			cur += 2;
			ADD_TOKEN(T_OP, OPCODE_EQ);
			return true;
		} else if (StringView::starts_with(cur, "!=")) {
			cur += 2;
			ADD_TOKEN(T_OP, OPCODE_NEQ);
			return true;
		} else if (StringView::starts_with(cur, "||")) {
			cur += 2;
			ADD_TOKEN(T_OP, OPCODE_OR);
			return true;
		} else if (StringView::starts_with(cur, "&&")) {
			cur += 2;
			ADD_TOKEN(T_OP, OPCODE_AND);
			return true;
		} else if (StringView::starts_with(cur, "var ")) {
			cur += 4;
			ADD_TOKEN(T_OP, OPCODE_VAR);
			return true;
		} else if (StringView::starts_with(cur, "func ")) {
			cur += 5;
			ADD_TOKEN(T_OP, OPCODE_FUNC);
			return true;
		} else if (StringView::starts_with(cur, "ret ")|| StringView::starts_with(cur, "ret;")) {
			cur += 3;
			ADD_TOKEN(T_OP, OPCODE_RET);
			return true;
		} else if (StringView::starts_with(cur, "exit ")|| StringView::starts_with(cur, "exit;")) {
			cur += 4;
			ADD_TOKEN(T_OP, OPCODE_ELEM_EXIT);
			return true;
		} else if (StringView::starts_with(cur, "__PUT_NUMBER__ ") || StringView::starts_with(cur, "__PUT_NUMBER__;")) {
			cur += 14;
			ADD_TOKEN(T_OP, OPCODE_ELEM_PUTN);
			return true;
		} else if (StringView::starts_with(cur, "__PUT_CHAR__ ") || StringView::starts_with(cur, "__PUT_CHAR__;")) {
			cur += 12;
			ADD_TOKEN(T_OP, OPCODE_ELEM_PUTC);
			return true;
		} else {
			return false;
		}
	}

	void parse() {
		while(*cur) {
			if (isspace(*cur)) { // skip spaces
				if (*cur == '\n') {
					++line;
					cur_line = cur + 1;
				}
				++cur;
				continue;
			}

			if (skip_mode) {
				if (two_available()) { // check the end of the comment
					if (*cur == '*' && *(cur + 1) == '/') {
						++cur;
						skip_mode = 0;
					}
				}
				++cur;
				continue;
			}

			if (two_available()) {
				if (*cur == '/' && *(cur + 1) == '*') {
					++cur;
					skip_mode = 1;
					continue;
				}
			}

			if (isdigit(*cur) || (two_available() && is_sign(*cur) && isdigit(*(cur + 1)))) {
				collect_number();
				continue;
			}

			if (try_collect_long_op()) {
				continue;
			}

			if (is_id_char(*cur)) {
				collect_id();
				continue;
			}

			ADD_TOKEN(T_OP, *cur);
			++cur;
		}
	}

public:
	LexicalParser            (const LexicalParser&) = delete;
	LexicalParser &operator= (const LexicalParser&) = delete;

	LexicalParser():
	cur_expr(nullptr),
	cur_line(nullptr),
	cur(nullptr),
	tokens(nullptr),
	skip_mode(0),
	line(0)
	{}

	~LexicalParser() {}

	void ctor() {
		cur_expr  = nullptr;
		cur_line  = nullptr;
		cur       = nullptr;
		tokens    = nullptr;
		skip_mode = 0;
		line      = 1;
	}

	static LexicalParser *NEW() {
		LexicalParser *cake = (LexicalParser*) calloc(1, sizeof(LexicalParser));
		if (!cake) {
			return nullptr;
		}

		cake->ctor();
		return cake;
	}

	void dtor() {

	}

	static void DELETE(LexicalParser *parser) {
		if (!parser) {
			return;
		}

		parser->dtor();
		free(parser);
	}

//=============================================================================

	Vector<Token> *parse(const char *expression) {
		cur_expr = expression;
		cur_line = expression;
		cur      = expression;

		tokens = Vector<Token>::NEW();
		parse();
		ADD_TOKEN(T_END, 0);

		Vector<Token> *ret = tokens;

		cur    = nullptr;
		tokens = nullptr;
		return ret;
	}

	#undef ADD_TOKEN
};

#endif // LEXICAL_PARSER
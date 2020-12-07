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
	#define ADD_TOKEN(type, data) do {Token token = {}; token.ctor(type, data); tokens->push_back(token);} while (0)

// data =======================================================================
	const char *cur;
	Vector<Token> *tokens;
	char skip_mode;
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
		token.ctor(T_NUMBER, 0);

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

	void collect_id_or_long_op() {
		if (StringView::starts_with(cur, "if")) {
			cur += 2;
			ADD_TOKEN(T_OP, OPCODE_IF);
		} else if (StringView::starts_with(cur, "while")) {
			cur += 5;
			ADD_TOKEN(T_OP, OPCODE_WHILE);
		} else if (StringView::starts_with(cur, "for")) {
			cur += 3;
			ADD_TOKEN(T_OP, OPCODE_FOR);
		} else {
			collect_id();
		}
	}

	void parse() {
		while(*cur) {
			if (isspace(*cur)) { // skip spaces
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

			if (is_id_char(*cur)) {
				collect_id_or_long_op();
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
	cur(nullptr),
	tokens(nullptr),
	skip_mode(0)
	{}

	~LexicalParser() {}

	void ctor() {
		cur = nullptr;
		tokens = nullptr;
		skip_mode = 0;
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
		cur    = expression;
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
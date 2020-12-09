#ifndef LEX_TOKEN
#define LEX_TOKEN

#include "compiler_options.h"

//=============================================================================
// Token ======================================================================

enum TOKEN_TYPE {
	T_END    = 0,
	T_NUMBER = 1,
	T_ID     = 2,
	T_OP     = 3,
};

union TokenData {
	int         op;
	double      num;
	StringView *id;
};

struct Token {
// data =======================================================================
	int type;
	TokenData data;
	int line;
	int pos;
//=============================================================================

	// Token            (const Token&) = delete;
	// Token &operator= (const Token&) = delete;

	Token():
	type(0),
	data(),
	line(0),
	pos(0)
	{}

	~Token() {}

	void ctor() {
		type = 0;
		data.num = 0;
		line = 0;
		pos = 0;
	}

	void ctor(int type_, int line_, int pos_) {
		type = type_;
		data.num = 0;
		line = line_;
		pos = pos_;
	}

	void ctor(int type_, int op_, int line_, int pos_) {
		type = type_;
		data.op = op_;
		line = line_;
		pos = pos_;
	}

	void ctor(int type_, double num_, int line_, int pos_) {
		type = type_;
		data.num = num_;
		line = line_;
		pos = pos_;
	}

	void ctor(int type_, StringView *id_, int line_, int pos_) {
		type = type_;
		data.id = id_;
		line = line_;
		pos = pos_;
	}

	static Token *NEW() {
		Token *cake = (Token*) calloc(1, sizeof(Token));
		if (!cake) {
			return nullptr;
		}

		cake->ctor();
		return cake;
	}

	static Token *NEW(int type_, int line_, int pos_) {
		Token *cake = (Token*) calloc(1, sizeof(Token));
		if (!cake) {
			return nullptr;
		}

		cake->ctor(type_, line_, pos_);
		return cake;
	}

	static Token *NEW(int type_, int op_, int line_, int pos_) {
		Token *cake = (Token*) calloc(1, sizeof(Token));
		if (!cake) {
			return nullptr;
		}

		cake->ctor(type_, op_, line_, pos_);
		return cake;
	}

	static Token *NEW(int type_, double num_, int line_, int pos_) {
		Token *cake = (Token*) calloc(1, sizeof(Token));
		if (!cake) {
			return nullptr;
		}

		cake->ctor(type_, num_, line_, pos_);
		return cake;
	}

	static Token *NEW(int type_, StringView *id_, int line_, int pos_) {
		Token *cake = (Token*) calloc(1, sizeof(Token));
		if (!cake) {
			return nullptr;
		}

		cake->ctor(type_, id_, line_, pos_);
		return cake;
	}

	void dtor(bool to_delete_id = false) {
		if (type == T_ID) {
			if (to_delete_id) {
				StringView::DELETE(data.id);
			}
		}
	}

	static void DELETE(Token *classname, bool to_delete_id = false) {
		if (!classname) {
			return;
		}

		classname->dtor(to_delete_id);
		free(classname);
	}

	int get_op() const {
		return data.op;
	}

	StringView *get_id() const {
		return data.id;
	}

	bool is_op() const {
		return type == T_OP;
	}

	bool is_op(const int op) const {
		return type == T_OP && data.op == op;
	}

	bool is_number() const {
		return type == T_NUMBER;
	}

	bool is_id() const {
		return type == T_ID;
	}

	bool is_end() const {
		return type == T_END;
	}

	void dump(FILE *file = stdout, bool bracked = true) const {
		if (bracked) {
			fprintf(file, "[");
		}
		if (type == T_END) {
			fprintf(file, "0[END]0");
		} else if (type == T_NUMBER) {
			fprintf(file, "%lg", data.num);
		} else if (type == T_OP) {
			if (is_printable_op(data.op)) {
				fputc(data.op, file);
			} else {
				fprintf(file, "[op_%d]", data.op);
			}
		} else {
			data.id->print();
		}
		if (bracked) {
			fprintf(file, "]");
		}
	}
};

#endif // LEX_TOKEN
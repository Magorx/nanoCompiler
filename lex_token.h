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
//=============================================================================

	// Token            (const Token&) = delete;
	// Token &operator= (const Token&) = delete;

	Token():
	type(0),
	data()
	{}

	~Token() {}

	void ctor() {
		type = 0;
		data.num = 0;
	}

	void ctor(int type_) {
		type = type_;
		data.num = 0;
	}

	void ctor(int type_, int op_) {
		type = type_;
		data.op = op_;
	}

	void ctor(int type_, double num_) {
		type = type_;
		data.num = num_;
	}

	void ctor(int type_, StringView *id_) {
		type = type_;
		data.id = id_;
	}

	static Token *NEW() {
		Token *cake = (Token*) calloc(1, sizeof(Token));
		if (!cake) {
			return nullptr;
		}

		cake->ctor();
		return cake;
	}

	static Token *NEW(int type_) {
		Token *cake = (Token*) calloc(1, sizeof(Token));
		if (!cake) {
			return nullptr;
		}

		cake->ctor(type_);
		return cake;
	}

	static Token *NEW(int type_, int op_) {
		Token *cake = (Token*) calloc(1, sizeof(Token));
		if (!cake) {
			return nullptr;
		}

		cake->ctor(type_, op_);
		return cake;
	}

	static Token *NEW(int type_, double num_) {
		Token *cake = (Token*) calloc(1, sizeof(Token));
		if (!cake) {
			return nullptr;
		}

		cake->ctor(type_, num_);
		return cake;
	}

	static Token *NEW(int type_, StringView *id_) {
		Token *cake = (Token*) calloc(1, sizeof(Token));
		if (!cake) {
			return nullptr;
		}

		cake->ctor(type_, id_);
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

	void dump(bool bracked = true) const {
		if (bracked) {
			printf("[");
		}
		if (type == T_END) {
			printf("0{END}0");
		} else if (type == T_NUMBER) {
			printf("%lg", data.num);
		} else if (type == T_OP) {
			switch (data.op) {
				case OPCODE_IF : {
					printf("if");
					break;
				}

				case OPCODE_WHILE : {
					printf("while");
					break;
				}

				case OPCODE_FOR : {
					printf("for");
					break;
				}

				default: {
					printf("%c", data.op);
					break;
				}
			}
		} else {
			data.id->print();
		}
		if (bracked) {
			printf("]");
		}
	}
};

#endif // LEX_TOKEN
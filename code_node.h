#ifndef CODENODE_H
#define CODENODE_H

#include "general/cpp/stringview.hpp"
#include "general/constants.h"

#include <cstdio>

//=============================================================================
// CodeNode ===================================================================

enum CODE_NODE_TYPE {
	NONE        = 0,
	OPERATION   = 1,
	VARIABLE    = 2,
	VALUE       = 3,
	ID          = 4,
};

union CodeNodeData {
	int op;
	int var;
	double val;
	StringView *id;
};

struct CodeNode {
// data =======================================================================
	char type;
	CodeNodeData data;
	CodeNode *L;
	CodeNode *R;
//=============================================================================

public:
	CodeNode            (const CodeNode&) = delete;
	CodeNode &operator= (const CodeNode&) = delete;

	CodeNode():
	type(0),
	data(),
	L(nullptr),
	R(nullptr)
	{}

	~CodeNode() {}

	void ctor() {
		type     = NONE;
		data.val = 0;
		L        = nullptr;
		R        = nullptr;
	}

	static CodeNode *NEW() {
		CodeNode *cake = (CodeNode*) calloc(1, sizeof(CodeNode));
		if (!cake) {
			return nullptr;
		}

		cake->ctor();
		return cake;
	}

	void ctor(const char type_, const int op_or_var, CodeNode *L_=nullptr, CodeNode *R_=nullptr) {
		type = type_;
		if (type == OPERATION) {
			data.op = op_or_var;
		} else {
			data.var = op_or_var;
		}
		L = L_;
		R = R_;
	}

	static CodeNode *NEW(const char type_, const int op_or_var, CodeNode *L_=nullptr, CodeNode *R_=nullptr) {
		CodeNode *cake = (CodeNode*) calloc(1, sizeof(CodeNode));
		if (!cake) {
			return nullptr;
		}

		cake->ctor(type_, op_or_var, L_, R_);
		return cake;
	}

	void ctor(const char type_, const double val_, CodeNode *L_=nullptr, CodeNode *R_=nullptr) {
		type     = type_;
		data.val = val_;
		L        = L_;
		R        = R_;
	}

	static CodeNode *NEW(const char type_, const double val_, CodeNode *L_=nullptr, CodeNode *R_=nullptr) {
		CodeNode *cake = (CodeNode*) calloc(1, sizeof(CodeNode));
		if (!cake) {
			return nullptr;
		}

		cake->ctor(type_, val_, L_, R_);
		return cake;
	}

	void ctor(const char type_, StringView *id_, CodeNode *L_=nullptr, CodeNode *R_=nullptr) {
		type    = type_;
		data.id = id_;
		L       = L_;
		R       = R_;
	}

	static CodeNode *NEW(const char type_, StringView *id_, CodeNode *L_=nullptr, CodeNode *R_=nullptr) {
		CodeNode *cake = (CodeNode*) calloc(1, sizeof(CodeNode));
		if (!cake) {
			return nullptr;
		}

		cake->ctor(type_, id_, L_, R_);
		return cake;
	}

	void dtor() {
		type     = NONE;
		data.val = 0;
		L        = nullptr;
		R        = nullptr;
	}

	static void DELETE(CodeNode *node, bool recursive = false, bool to_delete_id = false) {
		if (!node) {
			return;
		}

		if (recursive) {
			if (node->L) DELETE(node->L, recursive, to_delete_id);
			if (node->R) DELETE(node->R, recursive, to_delete_id);
		}

		if (to_delete_id && node->type == ID) {
			StringView::DELETE(node->data.id);
		}

		node->dtor();
		free(node);
	}

//=============================================================================
// Setters & Getters ==========================================================
//=============================================================================

	void set_L(CodeNode *L_) {
		L = L_;
	}

	void set_R(CodeNode *R_) {
		R = R_;
	}

	void set_LR(CodeNode *L_, CodeNode *R_) {
		L = L_;
		R = R_;
	}

	void set_type(const char type_) {
		type = type_;
	}

	void set_op(const int op_) {
		set_type(OPERATION);
		data.op = op_;
	}

	void set_var(const int var_) {
		set_type(VARIABLE);
		data.var = var_;
	}

	void set_val(const double val_) {
		set_type(VALUE);
		data.val = val_;
	}

	void set_id(StringView *id_) {
		set_type(ID);
		data.id = id_;
	}

	char get_type() const {
		return type;
	}

	int get_op() const {
		return data.op;
	}

	int get_var() const {
		return data.var;
	}

	double get_val() const {
		return data.val;
	}

	StringView *get_id() const {
		return data.id;
	}

	int get_var_from_id() const {
		return (*data.id)[0];
	}

	bool is_op() const {
		return type == OPERATION;
	}

	bool is_op(const int op) const {
		return type == OPERATION && data.op == op;
	}

	bool is_var() const {
		return type == VARIABLE;
	}

	bool is_val() const {
		return type == VALUE;
	}

	bool is_id() const {
		return type == ID;
	}

//=============================================================================
// Executing ==================================================================
//=============================================================================

	double evaluate_expr(double *var_table = nullptr, const size_t var_table_len = 0, void **ret_var = nullptr) {
		if (is_val()) {
			return data.val;
		} else if (is_var()) {
			return evaluate_variable(var_table, var_table_len, ret_var);
		} else if (is_op()) {
			return evaluate_operation(var_table, var_table_len, ret_var);
		} else if (is_id()) {
			return evaluate_id(var_table, var_table_len, ret_var);
		} else {
			return 0;
		}
	}

	double evaluate_operation(double *var_table, const size_t var_table_len, void **ret_var = nullptr) {
		int op = data.op;
		void *l_var = nullptr;
		void *r_var = nullptr;
		double L_res = L ? L->evaluate_expr(var_table, var_table_len, &l_var) : 0;
		double R_res = R ? R->evaluate_expr(var_table, var_table_len, &r_var) : 0;

		switch (op) {
			case '+' : {
				if (ret_var) *ret_var = nullptr;
				return L_res + R_res;
			}

			case '-' : {
				if (ret_var) *ret_var = nullptr;
				return L_res - R_res;
			}

			case '*' : {
				if (ret_var) *ret_var = nullptr;
				return L_res * R_res;
			}

			case '/' : {
				if (ret_var) *ret_var = nullptr;
				return L_res / R_res;
			}

			case '^' : {
				if (ret_var) *ret_var = nullptr;
				return pow(L_res, R_res);
			}

			case '=' : {
				//printf("setting %lf to %lf\n", *((double*) l_var), R_res);
				*((double*) l_var) = R_res;
				if (ret_var) *ret_var = r_var;
				return R_res;
			}

			case ';' : {
				return R_res;
			}

			default: {
				return 0;
			}
		}
	}

	double evaluate_id(double *var_table, const size_t var_table_len, void **ret_var = nullptr) {
		if (!R && !L) {
			return evaluate_variable(var_table, var_table_len, ret_var);
		} else {
			StringView &id = *data.id;
			double R_res = R ? R->evaluate_expr(var_table, var_table_len, ret_var) : 0;

			if (id.starts_with("sin")) {
				return sin(R_res);
			} else if (id.starts_with("cos")) {
				return cos(R_res);
			} else if (id.starts_with("ln")) {
				return log(R_res);
			} else if (id.starts_with("log")) {
				return log10(R_res);
			} else {
				return R_res;
			}
		}
	}

	double evaluate_variable(double *var_table, const size_t var_table_len, void **ret_var = nullptr) {
		int var = data.id->get_buffer()[0];
		//printf("going to get %d [%c]: %lf\n", var, var, var_table[var]);
		if (var_table_len <= (size_t) var) {
			if (ret_var) *ret_var = &var_table[var];
			return (double) KCTF_POISON;
		} else  {
			if (ret_var) *ret_var = &var_table[var];
			return var_table[var];
		}
	}
	
// ============================================================================

	void space_dump(FILE *file = stdout) const {
		if (is_op(';')) {
			//fprintf(file, ";");
			return;
		}

		if (L) {
			fprintf(file, "(");
			L->space_dump(file);
			fprintf(file, ")");
		}

		//fprintf(file, "(");
		if (is_op()) {
			fputc(data.op, file);
		} else if (is_var()) {
			fprintf(file, "{%d}", data.var);
		} else if (is_val()) {
			fprintf(file, "%lg", data.val);
		} else if (is_id()) {
			data.id->print(file);
		} else {
			fprintf(file, "ERR");
		}
		//fprintf(file, ")");

		if (R) {
			fprintf(file, "(");
			R->space_dump(file);
			fprintf(file, ")");
		}
	}

};

#endif // CODENODE_H
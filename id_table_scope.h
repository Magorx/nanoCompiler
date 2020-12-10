#ifndef ID_TABLE_SCOPE
#define ID_TABLE_SCOPE

#include "general/cpp/stringview.hpp"
#include "general/cpp/vector.hpp"

#include "code_node.h"

enum ID_TYPE {
	ID_TYPE_VAR  = 1,
	ID_TYPE_FUNC = 2,
	NOT_FOUND    = -999999999,
};

struct IdData {
	int type;
	const StringView *id;
	int offset;
	const CodeNode *arglist;

	IdData():
	type(0),
	id(nullptr),
	offset(0),
	arglist(nullptr)
	{}

	IdData& operator=(const IdData& other) {
		type    = other.type;
		id      = other.id;
		offset  = other.offset;
		arglist = other.arglist;

		return *this;
	}

	void ctor(int type_, const StringView *id_, const int offset_, const CodeNode *arglist_ = nullptr) {
		type    = type_;
		id      = id_;
		offset  = offset_;
		arglist = arglist_;
	}

	bool equal(const IdData &other) {
		return type == other.type && id->equal(other.id);
	}
};

//=============================================================================
// IdTableScope ==============================================================

class IdTableScope {
private:
// data =======================================================================
	Vector<IdData> data;
	int var_cnt;
//=============================================================================


public:
	int offset;

	IdTableScope():
	data(),
	var_cnt(0),
	offset(0)
	{}

	~IdTableScope() {}

	void ctor(const int offset_ = 0) {
		data.ctor();
		offset = offset_;
		var_cnt = 0;
	}

	static IdTableScope *NEW(const int offset_ = 0) {
		IdTableScope *cake = (IdTableScope*) calloc(1, sizeof(IdTableScope));
		if (!cake) {
			return nullptr;
		}

		cake->ctor(offset_);
		return cake;
	}

	void dtor() {
		data.dtor();
	}

	static void DELETE(IdTableScope *scope) {
		if (!scope) {
			return;
		}

		scope->dtor();
		free(scope);
	}

//=============================================================================

	bool find_id(const StringView *id) const {
		size_t data_size = data.size();
		for (size_t i = 0; i < data_size; ++i) {
			if (id->equal(data[i].id)) {
				return true;
			}
		}

		return false;
	}

	int find(const int type, const StringView *id) const {
		IdData idat = {};
		idat.ctor(type, id, 0);

		size_t data_size = data.size();
		int func_cnt = 0;
		for (size_t i = 0; i < data_size; ++i) {
			if (idat.equal(data[i])) {
				return data[i].offset - func_cnt;
			} else if (data[i].type == ID_TYPE_FUNC) {
				++func_cnt;
			}
		}

		return NOT_FOUND;
	}

	int get_var_cnt() const {
		return var_cnt;
	}

	bool declare(const int type, const StringView *id, const CodeNode *arglist_ = nullptr) {
		IdData idat = {};
		idat.ctor(type, id, (int)data.size(), arglist_);

		if (find_id(id)) {
			return false;
		} else {
			var_cnt += type == ID_TYPE_VAR;
			data.push_back(idat);
			return true;
		}
	}

	const CodeNode *get_arglist(const StringView *id) {
		IdData idat = {};
		idat.ctor(ID_TYPE_FUNC, id, 0);

		size_t data_size = data.size();
		for (size_t i = 0; i < data_size; ++i) {
			if (idat.equal(data[i])) {
				return data[i].arglist;
			}
		}

		return nullptr;
	}

	int size() {
		return (int)data.size();
	}

	void dump() {
		for (size_t i = 0; i < data.size(); ++i) {
			printf("[%lu] ", i);
			data[i].id->print();
			printf("\n");
		}
	}

};

#endif // ID_TABLE_SCOPE

#ifndef ID_TABLE_SCOPE
#define ID_TABLE_SCOPE

#include "general/cpp/stringview.hpp"
#include "general/cpp/vector.hpp"

#include "code_node.h"

enum ID_TYPE {
	ID_TYPE_NONE   = 0,
	ID_TYPE_VAR    = 1,
	ID_TYPE_FUNC   = 2,
	ID_TYPE_STRUCT = 3,
	ID_TYPE_GLOBAL = 4,
	ID_TYPE_FOUND  = 5,
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
		if (id == nullptr || other.id == nullptr) {
			return false;
		}

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
	int functive;
//=============================================================================


public:
	int offset;

	IdTableScope():
	data(),
	var_cnt(0),
	functive(0),
	offset(0)
	{}

	~IdTableScope() {}

	void ctor(const int offset_ = 0, const int functive_ = 0) {
		data.ctor();
		offset = offset_;
		var_cnt = 0;
		functive = functive_;
	}

	static IdTableScope *NEW(const int offset_ = 0, const int functive_ = 0) {
		IdTableScope *cake = (IdTableScope*) calloc(1, sizeof(IdTableScope));
		if (!cake) {
			return nullptr;
		}

		cake->ctor(offset_, functive_);
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
		if (!id) {
			return false;
		}

		size_t data_size = data.size();
		for (size_t i = 0; i < data_size; ++i) {
			if (data[i].id && id->equal(data[i].id)) {
				return true;
			}
		}

		return false;
	}

	int find(const int type, const StringView *id) const {
		IdData idat = {};
		idat.ctor(type, id, 0);

		size_t data_size = data.size();
		int id_offset = 0;
		for (size_t i = 0; i < data_size; ++i) {
			if (idat.equal(data[i])) {
				return id_offset;
			} else {
				id_offset += data[i].offset;
			}
		}

		return NOT_FOUND;
	}

	int get_var_cnt() const {
		return offset;
	}

	bool declare(const int type, const StringView *id, const int size, const CodeNode *arglist_ = nullptr) {
		IdData idat = {};
		idat.ctor(type, id, size, arglist_);

		if (find_id(id)) {
			return false;
		} else {
			data.push_back(idat);
			offset += size;
			return true;
		}
	}

	bool add_buffer_zone(const int zone_size) {
		IdData idat = {};
		idat.ctor(ID_TYPE_NONE, nullptr, zone_size);
		data.push_back(idat);
		offset += zone_size;
		return true;
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

	int is_functive() {
		return functive;
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

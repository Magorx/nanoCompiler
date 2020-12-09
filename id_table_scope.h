#ifndef ID_TABLE_SCOPE
#define ID_TABLE_SCOPE

#include "general/cpp/stringview.hpp"
#include "general/cpp/vector.hpp"

enum ID_TYPE {
	ID_TYPE_VAR  = 1,
	ID_TYPE_FUNC = 2
};

struct IdData {
	int type;
	const StringView *id;
	int offset;

	IdData():
	type(0),
	id(nullptr),
	offset(0)
	{}

	IdData& operator=(const IdData& other) {
		type   = other.type;
		id     = other.id;
		offset = other.offset;

		return *this;
	}

	void ctor(int type_, const StringView *id_, const int offset_) {
		type   = type_;
		id     = id_;
		offset = offset_;
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
//=============================================================================


public:
	int offset;

	IdTableScope():
	data(),
	offset(0)
	{}

	~IdTableScope() {}

	void ctor(const int offset_ = 0) {
		data.ctor();
		data.push_back(IdData());
		offset = offset_;
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
		for (size_t i = 1; i < data_size; ++i) {
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
		for (size_t i = 1; i < data_size; ++i) {
			if (idat.equal(data[i])) {
				return data[i].offset;
			}
		}

		return 0;
	}

	bool declare(const int type, const StringView *id) {
		IdData idat = {};
		idat.ctor(type, id, offset + (int)data.size());

		if (find_id(id)) {
			return false;
		} else {
			data.push_back(idat);
			return true;
		}
	}

	int size() {
		return (int)data.size();
	}

};

#endif // ID_TABLE_SCOPE

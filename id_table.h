#ifndef ID_TABLE
#define ID_TABLE

#include "id_table_scope.h"
#include "general/cpp/vector.hpp"

//=============================================================================
// IdTable ===================================================================

class IdTable {
private:
// data =======================================================================
	Vector<IdTableScope*> data;
//=============================================================================

	void add_scope(const int offset) {
		IdTableScope *scope = IdTableScope::NEW(offset);
		data.push_back(scope);
	}

public:
	IdTable():
	data()
	{}

	~IdTable() {}

	void ctor() {
		data.ctor();
		add_scope(100);
	}

	static IdTable *NEW() {
		IdTable *cake = (IdTable*) calloc(1, sizeof(IdTable));
		if (!cake) {
			return nullptr;
		}

		cake->ctor();
		return cake;
	}

	void dtor() {
		for (int i = (int) data.size() - 1; i >= 0; --i) {
			IdTableScope::DELETE(data[i]);
			data.pop_back();
		}
		data.dtor();
	}

	static void DELETE(IdTable *table) {
		if (!table) {
			return;
		}

		table->dtor();
		free(table);
	}

//=============================================================================

	bool find_id(const StringView *id) const {
		for (int i = (int) data.size() - 1; i >= 0; --i) {
			if (data[i]->find_id(id)) {
				return true;
			}
		}

		return false;
	}

	int find(const int type, const StringView *id) const {
		for (int i = (int) data.size() - 1; i >= 0; --i) {
			if (int offset = data[i]->find(type, id)) {
				return offset;
			}
		}

		return 0;
	}

	int find_in_upper_scope(const int type, const StringView *id) const {
		if (!data.size()) {
			return 0;
		}

		return data[data.size() - 1]->find(type, id);
	}

	bool declare(const int type, const StringView *id) {
		if (!data.size()) {
			RAISE_ERROR("no scope to declare a variable in\n");
			return false;
		}

		return data[data.size() - 1]->declare(type, id);
	}

	void add_scope() {
		if (!data.size()) {
			add_scope(100);
		} else {
			add_scope(data[data.size() - 1]->offset + data[data.size() - 1]->size() + 1);
		}
	}

	void remove_scope() {
		if (!data.size()) {
			RAISE_ERROR("removing unexistant scope\n");
			return;
		} else {
			data[data.size() - 1]->dtor();
			data.pop_back();
		}
	}

};

#endif // ID_TABLE

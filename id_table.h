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
	int cur_scope;
	int var_cnt;
//=============================================================================

	void add_scope(const int offset) {
		IdTableScope *scope = IdTableScope::NEW(offset);
		data.push_back(scope);
	}

public:
	IdTable():
	data(),
	cur_scope(0),
	var_cnt(0)
	{}

	~IdTable() {}

	void ctor() {
		data.ctor();
		cur_scope = 0;
		var_cnt = 0;
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
		for (int i = cur_scope; i >= 0; --i) {
			if (data[i]->find_id(id)) {
				return true;
			}
		}

		return false;
	}

	int find(const int type, const StringView *id) const {
		for (int i = cur_scope; i >= 0; --i) {
			int offset = data[i]->find(type, id);
			if (offset != NOT_FOUND) {
				return offset;
			}
		}

		return NOT_FOUND;
	}

	int find_in_upper_scope(const int type, const StringView *id) const {
		if (!data.size()) {
			return NOT_FOUND;
		}

		return data[cur_scope]->find(type, id);
	}

	int find_from_prev(const int type, const StringView *id) const {
		for (int i = cur_scope - 1; i >= 0; --i) {
			int offset = data[i]->find(type, id);
			if (offset != NOT_FOUND) {
				return offset;
			}
		}

		return NOT_FOUND;
	}

	int get_upper_offset() const {
		if (!data.size()) {
			return NOT_FOUND;
		}

		return data[data.size() - 1]->get_var_cnt();
	}

	const CodeNode *get_arglist(const StringView *id) {
		for (int i = cur_scope; i >= 0; --i) {
			if (const CodeNode *arglist = data[i]->get_arglist(id)) {
				return arglist;
			}
		}

		return nullptr;
	}

	bool declare(const int type, const StringView *id, const CodeNode *arglist = nullptr) {
		if (!data.size()) {
			RAISE_ERROR("no scope to declare a variable in\n");
			return false;
		}

		return data[cur_scope]->declare(type, id, arglist);
	}

	void add_scope() {
		if (!data.size()) {
			add_scope(0);
		} else {
			add_scope(data[data.size() - 1]->offset + data[data.size() - 1]->size());
		}
		cur_scope = (int)data.size() - 1;
	}

	void remove_scope() {
		if (!data.size()) {
			RAISE_ERROR("removing unexistant scope\n");
			return;
		} else {
			IdTableScope::DELETE(data[data.size() - 1]);
			data.pop_back();
		}
		cur_scope = (int)data.size() - 1;
	}

	bool shift_backward() {
		if (cur_scope > 0) {
			--cur_scope;
			return true;
		} else {
			return false;
		}
	}

	bool shift_forward() {
		if (cur_scope < (int)data.size() - 1) {
			++cur_scope;
			return true;
		} else {
			return false;
		}
	}

	void dump() {
		for (size_t i = 0; i < data.size(); ++i) {
			printf("-----[%lu]\n", i);
			data[i]->dump();
		}
	}

};

#endif // ID_TABLE

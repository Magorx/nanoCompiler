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

	void add_scope(const int offset, const int functive) {
		IdTableScope *scope = IdTableScope::NEW(offset, functive);
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

	int find_var(const StringView *id, int *res) const {
		if (!data.size()) {
			return NOT_FOUND;
		}

		int offset = NOT_FOUND;
		int found_index = -1;

		for (int i = cur_scope; i >= 1; --i) {
			offset = data[i]->find(ID_TYPE_VAR, id);
			
			if (data[i]->is_functive() || offset != NOT_FOUND) {
				found_index = i;
				break;
			}
		}

		if (offset == NOT_FOUND) { // let's try to find global variable
			size_t data_size = data.size();
			for (size_t i = 0; i < data_size; ++i) {
				if (data[i]->is_functive()) {
					break;
				}

				int ret = data[i]->find(ID_TYPE_VAR, id);
				if (ret != NOT_FOUND) {
					offset = ret;
				}
			}

			*res = offset;
			return offset == NOT_FOUND ? NOT_FOUND : ID_TYPE_GLOBAL;
		}

		if (data[found_index]->is_functive()) {
			*res = offset;
			return offset == NOT_FOUND ? NOT_FOUND : ID_TYPE_FOUND;
		}

		for (int i = found_index - 1; i >= 1; --i) {
			offset += data[i]->get_var_cnt();
			if (data[i]->is_functive()) {
				break;
			}
		}

		*res = offset;
		return offset == NOT_FOUND ? NOT_FOUND : ID_TYPE_FOUND;
	}

	int find_func(const StringView *id) const {
		for (int i = cur_scope; i >= 0; --i) {
			int offset = data[i]->find(ID_TYPE_FUNC, id);
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

	int get_func_offset() const {
		int offset = 0;
		for (int i = cur_scope; i >= 0; --i) {
			offset += data[i]->get_var_cnt();
			if (data[i]->is_functive()) {
				return offset;
			}
		}

		return offset;
	}

	const CodeNode *get_arglist(const StringView *id) {
		for (int i = cur_scope; i >= 0; --i) {
			if (const CodeNode *arglist = data[i]->get_arglist(id)) {
				return arglist;
			}
		}

		return nullptr;
	}

	bool declare(const int type, const StringView *id, const int size, const CodeNode *arglist = nullptr) {
		if (!data.size()) {
			RAISE_ERROR("no scope to declare a variable in\n");
			return false;
		}

		return data[cur_scope]->declare(type, id, size, arglist);
	}

	bool declare_func(const StringView *id, const CodeNode *arglist) {
		return declare(ID_TYPE_FUNC, id, 0, arglist);
	}

	bool declare_var(const StringView *id, const int size, const CodeNode *fields = nullptr) {
		return declare(ID_TYPE_VAR, id, size, fields);
	}

	bool declare_struct(const StringView *id, const CodeNode *fields) {
		return declare(ID_TYPE_STRUCT, id, 0, fields);
	}

	bool add_buffer_zone(const int zone_size) {
		if (!data.size()) {
			RAISE_ERROR("adding buffer zone with on scopes alive\n");
			return false;
		}
		return data[cur_scope]->add_buffer_zone(zone_size);
	}

	void add_scope(int functive = 0) {
		add_scope(0, functive);
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

	void dump() const {
		for (size_t i = 0; i < data.size(); ++i) {
			printf("-----[%lu]\n", i);
			data[i]->dump();
		}
	}

};

#endif // ID_TABLE

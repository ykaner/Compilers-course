#include "SymbolTable.h"
#include "Record.h"
#include "Pointer.h"


bool SymbolTable::isPrimitive(string type) {
	return (type == "int") || (type == "real") || (type == "bool");
}

int SymbolTable::size_of(string type) {
	if (isPrimitive(type)) {
		return 1;
	}
	else if (type == "pointer") {
		return 1;
	}
	else
		return vars[type]->getSize();
}

void getDimsList(AST *p, vector<pair<int, int>>& dimsList) {
	if (p == nullptr)
		return;
	string op = p->getValue();
	AST *R = p->getRight();
	AST *L = p->getLeft();

	if (op == "array") {
		getDimsList(L, dimsList);
	}
	else if (op == "rangeList") {
		getDimsList(L, dimsList);
		getDimsList(R, dimsList);
	}
	else if (op == "range") {
		dimsList.push_back(pair<int, int>(stoi(L->getLeft()->getValue()), stoi(R->getLeft()->getValue())));
	}
}

void SymbolTable::get_st(AST *p) {
	if (p == nullptr)
		return;
	string op = p->getValue();
	AST *R = p->getRight();
	AST *L = p->getLeft();

	if (op == "declarationsList") {
		get_st(L);
		get_st(R);
		return;
	}

	else if (op == "var") {
		string name = L->getLeft()->getValue();
		string type = R->getValue();
		int addr = this->global_addr;

		if (isPrimitive(type)) { // primitive
			vars[name] = new Variable(type, name, addr, 1);
			this->global_addr++;
		}
		else if (type == "array") {
			string subtype = R->getRight()->getValue();
			if (subtype == "identifier") {
				subtype = R->getRight()->getLeft()->getValue();
			}
			int size = size_of(subtype);
			vector<pair<int, int>> dimsList;
			getDimsList(R, dimsList);
			vars[name] = new Array(type, name, addr, size, dimsList, subtype);
			this->global_addr += vars[name]->getSize();
		}
		else if (type == "record") {
			vars[name] = new Record(type, name, addr, R->getLeft(), this);
		}
		else if (type == "pointer") {
			string subtype = R->getLeft()->getValue();
			if (subtype == "identifier") {
				subtype = R->getLeft()->getLeft()->getValue();
			}

			vars[name] = new Pointer(type, name, addr, 1, subtype);
			this->global_addr++;
		}

	}

}


SymbolTable SymbolTable::generateSymbolTable(AST* p) {
	SymbolTable st = SymbolTable();
	st.get_st(p->getRight()->getLeft()->getLeft());
	return st;
}


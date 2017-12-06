#pragma once

#include <iostream>
#include <map>
#include <vector>

#include "AST.h"
#include "Variable.h"
#include "Array.h"

using namespace std;

class SymbolTable {
public:
	int global_addr;
	int start_addr;
	map<string, Variable *> vars;

public:

	SymbolTable(int start_addr=5) : vars(), global_addr(start_addr), start_addr(start_addr) {}

	Variable* operator[](string id) {
		return this->vars[id];
	}

	static SymbolTable generateSymbolTable(AST* p);
	void get_st(AST *p);

	int tableSize(map<string, Variable *> table) {
		int size = 0;
		for (auto x : table) {
			size += x.second->getSize();
		}
		return size;
	}

	static bool isPrimitive(string type);
	int size_of(string);
};



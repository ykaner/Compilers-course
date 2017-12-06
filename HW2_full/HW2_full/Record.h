#pragma once

#include "Variable.h"
#include "SymbolTable.h"

class Record :
	public Variable
{
public:
	//map<string, Variable *> vars;

public:
	Record(string type, string name, int addr, AST *p, SymbolTable *st);

	/*
	int getSize() {
		int size = 0;
		for (auto x : vars) {
			size += x.second->getSize();
		}
		return size;
	}
	*/
};


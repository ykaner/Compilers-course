#pragma once
#include "Variable.h"
#include "SymbolTable.h"

class Pointer :
	public Variable
{
public:
	string point_to;

public:
	Pointer(string type, string name, int addr, int size, string pointedT) :
		Variable(type, name, addr, size), point_to(pointedT) {

	}
};


#pragma once
#include <iostream>
#include <string>
#include <map>
#include <vector>

using namespace std;


class Variable {
public:
	string type;
	string name;
	int addr;
	int size;


public:
	Variable()//:type("None"), name("None"), size(0), addr(-1)
	{}

	Variable(string type, string name, int addr, int size) {
		this->type = type;
		this->name = name;
		this->addr = addr;
		this->size = size;
	}


	string getType() {
		return this->type;
	}

	string getName() {
		return this->name;
	}

	virtual int getAddr() {
		return this->addr;
	}

	virtual int getSize() {
		return this->size;
	}

	bool operator==(Variable other) {
		return this->name == other.name;
	}

};

/*ostream& operator<<(ostream& out, Variable var) {
	return cout << "( " << var.name << ", " << var.type << ", " << var.addr << ", " << var.size << ") ";
}
*/

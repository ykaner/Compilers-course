#pragma once
#include <numeric>

#include "Variable.h"

class Array :
	public Variable
{
public:
	int baseSize;
	string baseType;
	vector<pair<int, int> > dimsList;
	vector<int> lenList;
	int subpart;

public:
	Array(string type, string name, int addr, int size,
		vector<pair<int, int> > dimsList, string baseType):
	Variable(type, name, addr, size), dimsList(dimsList), baseSize(size), baseType(baseType)
	{
		for (auto r : dimsList) {
			this->lenList.push_back(r.second - r.first + 1);
		}

		this->size = accumulate(lenList.begin(), lenList.end(), this->baseSize, multiplies<double>());

		for (int i = 0; i < dimsList.size(); i++) {
			this->subpart += dimsList[i].first * this->baseSize *
				accumulate(lenList.begin() + i + 1, lenList.end(), 1, multiplies<double>());
		}
	}

	int getSize() {
		return this->size;
	}

};


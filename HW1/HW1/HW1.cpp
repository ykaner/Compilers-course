#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>
#include <map>

using namespace std;


template<class T>
int find(vector<T> list, T x) {
	for (int i = 0; i < list.size(); i++) {
		if (list[i] == x) return i;
	}
	return -1;
}

template<class T>
ostream& operator<<(ostream& out, vector<T>& list) {
	for (int i = 0; i < list.size(); i++) {
		out << i << ": " << list[i] << ", ";
	}
	out << endl;
	return out;
}


// Abstract Syntax Tree
class AST
{
private:

	string _value;
	AST* _left; // can be null
	AST* _right; // can be null

public:

	string getValue() {
		return _value;
	}

	AST* getLeft() {
		return _left;
	}

	AST* getRight() {
		return _right;
	}

public:

	AST(string value, AST* right, AST* left) {
		_value = value;
		_left = left;
		_right = right;
	}
	static AST* createAST(ifstream& input) {
		if (!(input))
			return nullptr;

		string line;
		getline(input, line);
		if (line == "~")
			return nullptr;
		AST* left = createAST(input);
		AST* right = createAST(input);
		return new AST(line, right, left);
	}


	vector<AST *> getByValue(string value) {
		if (this == nullptr) {
			return vector<AST *>();
		}
		if (this->_value == value) {
			vector<AST *> res;
			res.push_back(this);
			return res;
		}

		vector<AST *> lefts = this->_left->getByValue(value);
		vector<AST *> rights = this->_right->getByValue(value);
		
		vector<AST *> res;
		for (auto i : lefts) {
			res.push_back(i);
		}
		for (auto i : rights) {
			res.push_back(i);
		}
		
		return res;
	}

};




class Variable {
	string type;
	string name;
public:
	Variable(string type, string name) {
		this->type = type;
		this->name = name;
	}
	static Variable* creatVar(AST* ast) {
		if (ast->getValue() != "var"){
			return nullptr;
		}
		if (ast->getLeft() == nullptr || ast->getLeft()->getValue() != "identifier" ||
			ast->getLeft()->getLeft() == nullptr) return nullptr;
		string name = ast->getLeft()->getLeft()->getValue();
		if (ast->getRight() == nullptr)
			return nullptr;
		string type = ast->getRight()->getValue();

		return new Variable(type, name);
	}
	
	string getType() {
		return this->type;
	}

	string getName() {
		return this->name;
	}



	bool operator==(Variable other) {
		return this->name == other.name;
	}

};


int getSize(Variable var) {
	return 1; // TODO: genralize!
}



class SymbolTable {
	vector<Variable> vars;
	vector<int> addrs;
	vector<int> sizes;

public:

	const vector<Variable>& getVars() {
		return this->vars;
	}

	const vector<int>& getAddrs() {
		return this->addrs;
	}

	const vector<int>& getSizes() {
		return this->sizes;
	}

public:

	SymbolTable() {

	}

	SymbolTable(vector<Variable> v, vector<int> add, vector<int> size) {
		this->vars = v;
		this->addrs = add;
		this->sizes = size;
	}

	static SymbolTable generateSymbolTable(AST* tree) {
		vector<AST *> varns = tree->getByValue("var");
		
		vector<Variable> vars;
		vector<int> addrs;
		vector<int> sizes;
		int curAddr = 5;
		int curSize = 1;

		for (auto i = varns.begin(); i != varns.end(); i++) {
			vars.push_back(*Variable::creatVar(*i));
			addrs.push_back(curAddr);
			curSize = getSize(vars.back());
			sizes.push_back(curSize);
			curAddr += curSize;
		}
		
		return * new SymbolTable(vars, addrs, sizes);
	}

	int addrOfVar(string name) {
		int idx = find(this->vars, Variable("NoneType", name));
		return this->addrs[idx];
	}

};





//the types:
//0: helper
//1: unOp
//2: binOp
//3: scope
vector<vector<string>> types = {
	{
		"statementsList",
		"content",
		"program"
	},
	{
		"identifier",
		"constInt",
		"constReal",
		"false",
		"true",
		"not",
		"negative",
		"print"
	},
	{
		"plus",
		"minus",
		"multiply",
		"divide",
		"assignment",
		"and",
		"or",
		"lessOrEquals",
		"greaterOrEquals",
		"greaterThan",
		"lessThan",
		"equals",
		"notEquals",
	},
	{
		"if",
		"while"
	}
};

map<string, string> opTable = {
	{ "plus", "add" },
	{ "minus", "sub" },
	{ "multiply", "mul" },
	{ "divide", "div" },
	{"assignment", "sto"},
	{ "negative", "neg" },
	{ "and", "and" },
	{ "or", "or" },
	{ "lessOrEquals", "leq" },
	{ "greaterOrEquals", "geq" },
	{ "greaterThan", "grt" },
	{ "lessThan", "les" },
	{ "equals", "equ" },
	{ "notEquals", "neq" },
	{ "not", "not" },

	{ "print", "print" },
	{ "true", "ldc 1" },
	{ "false", "ldc 0" }

};


int getType(AST *ast) {
	string value = ast->getValue();


	for (int i = 0; i < types.size(); i++) {
		if (find(types[i], value) != -1) return i;
	}
	return -1;
}


int lbl_i = 0;

void generatePCode(AST* ast, SymbolTable st) {

	if (ast == nullptr)
		return;

	int type = getType(ast);
	string value = ast->getValue();
	switch (type){
	case 0:
		generatePCode(ast->getLeft(), st);
		generatePCode(ast->getRight(), st);
		break;
	case 1:
		if (value == "constInt" || value == "constReal") {
			cout << "ldc " << ast->getLeft()->getValue() << endl;
			break;
		}
		if (value == "identifier") {
			cout << "ldc " << st.addrOfVar(ast->getLeft()->getValue()) << endl;
			break;
		}
		else {
			generatePCode(ast->getLeft(), st);
			if (value == "print" && ast->getLeft()->getValue() == "identifier") {
				cout << "ind" << endl;
			}
			cout << opTable[value] << endl;
		}


		break;
	case 2:
		if (ast->getLeft() != nullptr) {
			generatePCode(ast->getLeft(), st);
			if (ast->getLeft()->getValue() == "identifier" && ast->getValue() != "assignment") {
				cout << "ind" << endl;
			}
		}
		if (ast->getRight() != nullptr) {
			generatePCode(ast->getRight(), st);
			if (ast->getRight()->getValue() == "identifier" && ast->getValue() != "assignment") {
				cout << "ind" << endl;
			}
		}

		cout << opTable[value] << endl;
		break;
	case 3:
		bool isElse = ast->getRight() != nullptr && ast->getRight()->getValue() == "else";
		string start_lbl, else_lbl, end_lbl;

		if (value == "while") {
			start_lbl = "L" + to_string(lbl_i);
			lbl_i++;
			end_lbl = "L" + to_string(lbl_i);
			lbl_i++;

			cout << start_lbl << ":" << endl;

			generatePCode(ast->getLeft(), st);
			cout << "fjp " << end_lbl << endl;
			generatePCode(ast->getRight(), st);
			cout << "ujp " << start_lbl << endl;
			cout << end_lbl << ":" << endl;
			break;
		}
		if (value == "if" && !isElse) {
			end_lbl = "L" + to_string(lbl_i);
			lbl_i++;

			generatePCode(ast->getLeft(), st);
			cout << "fjp " << end_lbl << endl;
			generatePCode(ast->getRight(), st);
			cout << end_lbl << ":" << endl;
			break;
		}
		if (value == "if" && isElse) {
			else_lbl = "L" + to_string(lbl_i);
			lbl_i++;
			end_lbl = "L" + to_string(lbl_i);
			lbl_i++;
			
			generatePCode(ast->getLeft(), st);
			cout << "fjp " << else_lbl << endl;
			generatePCode(ast->getRight()->getLeft(), st);
			cout << "ujp " << end_lbl << endl;
			cout << else_lbl << ":" << endl;
			generatePCode(ast->getRight()->getRight(), st);
			cout << end_lbl << ":" << endl;
			break;
		}
		break;
	}

}


ostream& operator<<(ostream& out, AST* ast) {
	if (ast == nullptr)
		return out;
	return out << ast->getValue() << " ( " << ast->getLeft() << " ,  " << ast->getRight() << " )";
}

static ostream& operator<<(ostream& out, Variable var) {
	out << var.getType() << " " << var.getName();
	return out;
}

static ostream& operator<<(ostream& out, SymbolTable table) {
	for (int i = 0; i < table.getVars().size(); i++) {
		out << table.getVars()[i] << " , " << table.getAddrs()[i] << " , " << table.getSizes()[i] << endl;
	}
	return out;
}


int main()
{
	AST* ast;
	SymbolTable symbolTable;
	ifstream myfile("C:/Users/ykane/Downloads/SamplesTxt/tree8.txt");
	if (myfile.is_open())
	{
		ast = AST::createAST(myfile);
		myfile.close();
		symbolTable = SymbolTable::generateSymbolTable(ast);
		generatePCode(ast, symbolTable);
	}
	else cout << "Unable to open file";

	return 0;
}
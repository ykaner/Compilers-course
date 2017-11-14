#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>

using namespace std;

// Abstract Syntax Tree
class AST
{
public:

	string _value;
	AST* _left; // can be null
	AST* _right; // can be null

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
		if (this == nullptr)
			return vector<AST *>();
		if (this->_value == value)
			return vector<AST *>(1, this);

		vector<AST *> lefts = this->_left->getByValue(value);
		vector<AST *> rights = this->_right->getByValue(value);
		
		vector<AST *> res(lefts.size() + rights.size());
		set_union(lefts.begin(), lefts.end(), rights.begin(), rights.end(), res.begin());
		
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
		if (ast->_value != "var"){
			return nullptr;
		}
		if (ast->_left == nullptr || ast->_left->_value != "identifier" ||
			ast->_left->_left == nullptr) return nullptr;
		string name = ast->_left->_left->_value;
		if (ast->_right == nullptr)
			return nullptr;
		string type = ast->_right->_value;

		return new Variable(type, name);
	}
	
	string getType() {
		return this->type;
	}

	string getName() {
		return this->name;
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

	const vector<Variable>& getVars() {
		return this->vars;
	}

	const vector<int>& getAddrs() {
		return this->addrs;
	}

	const vector<int>& getSizes() {
		return this->sizes;
	}

};

void generatePCode(AST* ast, SymbolTable symbolTable) {
	// TODO: go over AST and print code
}


ostream& operator<<(ostream& out, AST* ast) {
	if (ast == nullptr)
		return out;
	out << ast->_value << " ( " << ast->_left << " ,  " << ast->_right << " )";
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
	ifstream myfile("C:/Users/ykane/Downloads/SamplesTxt/tree5.txt");
	if (myfile.is_open())
	{
		ast = AST::createAST(myfile);
		cout << ast << endl;
		myfile.close();
		symbolTable = SymbolTable::generateSymbolTable(ast);
		cout << symbolTable;
		generatePCode(ast, symbolTable);
	}
	else cout << "Unable to open file";

	cin.get();
	return 0;
}
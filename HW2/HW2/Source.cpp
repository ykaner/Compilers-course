#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>
#include <map>
#include <set>
#include <numeric>


using namespace std;


/*template<class T>
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
*/


/***********class declarations***************/
class AST;
class SymbolTable;
class Variable;


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


int size_of(string  type) {
	return 1; // TODO: genralize!
}


class Variable {
public:
	string type;
	string name;
	int addr;
	int size;

	//array stuff:
	vector<pair<int, int> > dimsList;
	vector<int> lenList;
	int subpart;

public:
	Variable()//:type("None"), name("None"), size(0), addr(-1)
	{

	}

	Variable(string type, string name, int addr, int size) {
		this->type = type;
		this->name = name;
		this->addr = addr;
		this->size = size;
	}

	Variable(string type, string name, int addr, int size,
		vector<pair<int, int> > dimsList, vector<int> lenList, int subpart) {
		this->type = type;
		this->name = name;
		this->addr = addr;
		this->size = size;
		this->dimsList = dimsList;
		this->lenList = lenList;
		this->subpart = subpart;
	}


	string getType() {
		return this->type;
	}

	string getName() {
		return this->name;
	}

	int getAddr() {
		return this->addr;
	}

	int getSize() {
		return this->size;
	}

	bool operator==(Variable other) {
		return this->name == other.name;
	}

};

ostream& operator<<(ostream& out, Variable var) {
	return cout << "( " << var.name << ", " << var.type << ", " << var.addr << ", " << var.size << ") ";
}


/********************************************/
/*********symbol table generation************/
/********************************************/
SymbolTable get_st(AST *p);
string st_type(AST *p);
string st_id(AST *p);


class SymbolTable {
public:
	map<string, Variable> vars;

public:

	SymbolTable() : vars() {

	}

	static SymbolTable generateSymbolTable(AST* p) {
		return get_st(p->getRight()->getLeft());
	}

	Variable& operator[](string id) {
		return this->vars[id];
	}

};
ostream& operator<<(ostream& out, SymbolTable st) {
	for (auto a : st.vars) {
		out << "(" << a.first << ", " << a.second << "), ";
	}
	return out << endl;
}




/***********global variables*****************/
SymbolTable st;



string st_type(AST *p) {
	if (p == nullptr)
		return "";
	string op = p->getValue();
	AST *R = p->getRight();
	AST *L = p->getLeft();

	if (op == "var") {
		return st_type(R);
	}
	else if (op == "int") {
		return "int";
	}
	else if (op == "real") {
		return "real";
	}
	else if (op == "bool") {
		return "bool";
	}
	else if (op == "array") {
		return "array of " + st_type(R);
	}

	return "";
}

string st_id(AST *p) {
	if (p == nullptr)
		return "";
	string op = p->getValue();
	AST *R = p->getRight();
	AST *L = p->getLeft();

	if (op == "var") {
		return st_id(L);
	}
	else if (op == "identifier") {
		return L->getValue();
	}

	return nullptr;
}

int glob_addr = 5;
int st_addr() {
	int ret = glob_addr;
	return glob_addr;
}

void st_arr(AST *p, vector<pair<int, int>>& list) {
	if (p == nullptr)
		return;
	string op = p->getValue();
	AST *R = p->getRight();
	AST *L = p->getLeft();

	if (op == "rangeList") {
		st_arr(L, list);
		st_arr(R, list);
		return;
	}
	else if (op == "range") {
		list.push_back(pair<int, int>(stoi(L->getLeft()->getValue()), stoi(R->getLeft()->getValue())));
		return;
	}

}

SymbolTable st_var(AST *p) {
	if (p == nullptr)
		return st;
	string op = p->getValue();
	AST *R = p->getRight();
	AST *L = p->getLeft();

	string name = st_id(L);

	string type = st_type(R);
	int addr = st_addr();
	int size = size_of(type);
	vector<pair<int, int>> rangeList;
	vector<int> lengthList;
	int subpart = 0;
	if (type.find("array") == 0) {
		int single_size = size;

		st_arr(L, rangeList);
		for (auto r : rangeList) {
			lengthList.push_back(r.second - r.first + 1);
			int sum = std::accumulate(lengthList.begin(), lengthList.end(), 0);
			size *= sum;
		}

		for (int i = 0; i < rangeList.size(); i++) {
			int ds = 1;
			for (int j = i + 1; j < rangeList.size(); j++) {
				ds *= lengthList[j];
			}
			subpart += rangeList[i].first * single_size * ds;
		}
	}

	glob_addr += size;
	

	st[name] = Variable(type, name, addr, size, rangeList, lengthList, subpart);
	return st;
}

SymbolTable get_st(AST *p) {
	if (p == nullptr)
		return st;
	string op = p->getValue();
	AST *R = p->getRight();
	AST *L = p->getLeft();

	if (op == "scope") {
		get_st(L);
		return st;
	}
	else if (op == "declarationsList") {
		get_st(R);
		get_st(L);
		return st;
	}
	else if (op == "var") {
		return st_var(p);
	}

}














/********************************************/
/************code generation*****************/
/********************************************/
void code(AST *p);
void codel(AST *p); // code left value
void coder(AST *p); // code right value
void codear(AST *p, string ar_name, int x_i=0); // array indexing
void codec(AST *p, int l_switch); // code case
int _codec(AST *p, int l_switch, int l_case); // private recursive function for cases



/***********global variables*****************/
int LAB = 0;
int l_while_out = 0;
int l_switch_end = 0;
int l_case = 0;


void coder(AST *p) {
	if (p == nullptr)
		return;
	string op = p->getValue();
	AST *R = p->getRight();
	AST *L = p->getLeft();

	if (op == "plus") {
		coder(L);
		coder(R);
		cout << "add" << endl;
		return;
	}
	else if (op == "minus") {
		coder(L);
		coder(R);
		cout << "sub" << endl;
		return;
	}
	else if (op == "multiply") {
		coder(L);
		coder(R);
		cout << "mul" << endl;
		return;
	}
	else if (op == "divide") {
		coder(L);
		coder(R);
		cout << "div" << endl;
		return;
	}
	else if (op == "and") {
		coder(L);
		coder(R);
		cout << "and" << endl;
		return;
	}
	else if (op == "or") {
		coder(L);
		coder(R);
		cout << "or" << endl;
		return;
	}
	else if (op == "lessOrEquals") {
		coder(L);
		coder(R);
		cout << "leq" << endl;
		return;
	}
	else if (op == "greaterOrEquals") {
		coder(L);
		coder(R);
		cout << "geq" << endl;
		return;
	}
	else if (op == "greaterThan") {
		coder(L);
		coder(R);
		cout << "grt" << endl;
		return;
	}
	else if (op == "lessThan") {
		coder(L);
		coder(R);
		cout << "les" << endl;
		return;
	}
	else if (op == "equals") {
		coder(L);
		coder(R);
		cout << "equ" << endl;
		return;
	}
	else if (op == "notEquals") {
		coder(L);
		coder(R);
		cout << "neq" << endl;
		return;
	}

	else if (op == "negative") {
		coder(L);
		cout << "neg" << endl;
		return;
	}
	else if (op == "not") {
		coder(L);
		cout << "neg" << endl;
		return;
	}


	else if (op == "false") {
		cout << "ldc 0" << endl;
		return;
	}
	else if (op == "true") {
		cout << "ldc 1" << endl;
		return;
	}

	else if (op == "constInt" || op == "constReal") {
		cout << "ldc " << L->getValue() << endl;
		return;
	}
	else if (op == "identifier") {
		codel(p);
		cout << "ind" << endl;
		return;
	}
	else if (op == "array") {
		codel(L);
		codear(R, L->getLeft()->getValue());

		return;
	}
	else if (op == "pointer") {
		//TODO:
		return;
	}

}

void codel(AST *p) {
	if (p == nullptr)
		return;
	string op = p->getValue();
	AST *R = p->getRight();
	AST *L = p->getLeft();

	if (op == "identifier") {
		cout << "ldc " << st[L->getValue()].getAddr() << endl;
		return;
	}

}

void codear(AST *p, string ar_name, int x_i) {
	return; // the function crashs
	if (p == nullptr)
		return;
	string op = p->getValue();
	AST *R = p->getRight();
	AST *L = p->getLeft();

	if (op == "indexList") {
		codear(L, ar_name, x_i + 1);
		coder(R);
		auto lenList = st[ar_name].lenList;
		cout << "ixa " << accumulate(lenList.end() - x_i, lenList.end(), 1, multiplies<double>()) << endl;
		return;
	}

}

void codec(AST *p, int l_switch) {
	int case_cnt = _codec(p, l_switch, 0);
	for (int i = case_cnt; i > 0; i--) {
		cout << "ujp case_" << l_switch << "_" << i << endl;
	}
}

int _codec(AST *p, int l_switch, int l_case) {
	if (p == nullptr)
		return l_case;
	string op = p->getValue();
	AST *R = p->getRight();
	AST *L = p->getLeft();

	if (op == "caseList") {
		l_case = _codec(L, l_switch, l_case);
		l_case = _codec(R, l_switch, l_case);
		return l_case;
	}
	else if (op == "case") {
		l_case++;
		cout << "case_" << l_switch << "_" << l_case << ":" << endl;
		code(R);
		cout << "ujp " << "switch_end_" << l_switch << endl;
		return l_case;
	}


}

void code(AST *p) {
	if (p == nullptr)
		return;
	string op = p->getValue();
	AST *R = p->getRight();
	AST *L = p->getLeft();

	if (op == "statementsList") {
		code(L);
		code(R);
		return;
	}

	else if (op == "assignment") {
		codel(L);
		coder(R);
		cout << "sto" << endl;
		return;
	}
	else if (op == "print") {
		coder(L);
		cout << "print" << endl;
		return;
	}
	else if (op == "if" && R->getValue() != "else") {
		int la = LAB;
		LAB++;
		coder(L);
		cout << "fjp L" << la << endl;
		code(R);
		cout << "L" << la << ":" << endl;
		return;
	}
	else if (op == "if" && R->getValue() == "else") {
		int la = LAB;
		LAB++;
		int lb = LAB;
		LAB++;
		coder(L);
		cout << "fjp L" << la << endl;
		code(R->getLeft());
		cout << "ujp L" << lb << endl;
		cout << "L" << la << ":" << endl;
		code(R->getRight());
		cout << "L" << lb << ":" << endl;
		return;
	}
	else if (op == "while") {
		int la = LAB;
		LAB++;
		int lb = LAB;
		l_while_out = lb;
		LAB++;
		cout << "while_" << la << ":" << endl;
		coder(L);
		cout << "fjp while_out_" << lb << endl;
		code(R);
		cout << "ujp while_" << la << endl;
		cout << "while_out_" << lb << ":" << endl;
		return;
 	}
	else if (op == "break") {
		cout << "ujp while_out_" << l_while_out << endl;
		return;
	}
	
	else if (op == "switch") {
		int la = LAB++;
		coder(L);
		cout << "neg" << endl;
		cout << "ixj switch_end_" << la << endl;
		codec(R, la); // codec - caseList
		cout << "switch_end_" << la << ":" << endl;
		return;
	}


}

void generatePCode(AST *ast, SymbolTable symbolTable) {
	st = symbolTable;
	if (ast == nullptr)
		return;
	
	code(ast->getRight()->getRight());
}



int main()
{
	AST* ast;
	SymbolTable symbolTable;
	ifstream myfile("C:/Users/ykane/Downloads/TestsHW2/tree3.txt");
	if (myfile.is_open())
	{
		ast = AST::createAST(myfile);
		myfile.close();
		symbolTable = SymbolTable::generateSymbolTable(ast);
		generatePCode(ast, symbolTable);
	}
	else cout << "Unable to open file";
	cin.get();


	return 0;
}
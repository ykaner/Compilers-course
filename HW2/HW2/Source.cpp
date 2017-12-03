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
		if (ast->getValue() != "var") {
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

		return *new SymbolTable(vars, addrs, sizes);
	}

	int addrOfVar(string name) {
		int idx = find(this->vars, Variable("NoneType", name));
		return this->addrs[idx];
	}

};












/********************************************/
/************code generation*****************/
/********************************************/
void code(AST *p);
void codel(AST *p); // code left value
void coder(AST *p); // code right value
void codec(AST *p, int l_switch); // code case
int _codec(AST *p, int l_switch, int l_case); // private recursive function for cases


/***********global variables*****************/
SymbolTable st;

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

}

void codel(AST *p) {
	if (p == nullptr)
		return;
	string op = p->getValue();
	AST *R = p->getRight();
	AST *L = p->getLeft();

	if (op == "identifier") {
		cout << "ldc " << st.addrOfVar(L->getValue()) << endl;
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
	ifstream myfile("C:/Users/ykane/Downloads/TestsHW2/tree11.txt");
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
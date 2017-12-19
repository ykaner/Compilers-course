#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <stack>
#include <algorithm>
#include <map>
#include <set>

using namespace std;


int my_accumulate(vector<int>::iterator start, vector<int>::iterator end, int init) {
	int res = init;
	for (; start < end; start++) {
		res *= *start;
	}
	return res;
}


/***********class declarations*************/
class AST;
class SymbolTable;
class Varable;
class Record;
class Pointer;
class Array;



class Variable {
public:
	string type;
	string name;
	int addr;
	int size;


public:
	Variable()//:type("None"), name("None"), size(0), addr(-1)
	{}
	virtual ~Variable() {

	}

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
		vector<pair<int, int> > dimsList, string baseType) :
		Variable(type, name, addr, size), baseSize(size), baseType(baseType), dimsList(dimsList), lenList(), subpart(0)
	{
		for (auto r : dimsList) {
			this->lenList.push_back(r.second - r.first + 1);
		}

		this->size = my_accumulate(lenList.begin(), lenList.end(), this->baseSize);

		this->subpart = 0;
		for (unsigned int i = 0; i < dimsList.size(); i++) {
			this->subpart += dimsList[i].first * this->baseSize *
				my_accumulate(lenList.begin() + i + 1, lenList.end(), 1);
		}
	}

	int getSize() {
		return this->size;
	}

};




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


};



class SymbolTable {
public:
	int global_addr;
	int start_addr;
	map<string, Variable *> vars;

public:

	SymbolTable(int start_addr = 5) : global_addr(start_addr), start_addr(start_addr), vars() {}

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


Record::Record(string type, string name, int addr, AST *p, SymbolTable *st) : Variable(type, name, addr, 1)
{
	int start_addr = st->global_addr;
	st->global_addr = 0;
	st->get_st(p);
	this->size = st->global_addr;
	st->global_addr += start_addr;
}









/***********code generation*****************/
void code(AST *p, SymbolTable* st);
string codel(AST *p, SymbolTable* st); // code left value
void coder(AST *p, SymbolTable* st); // code right value
void codear(AST *p, string ar_name, SymbolTable *st, int x_i = 0); // array indexing
void codec(AST *p, int l_switch, SymbolTable* st); // code case
int _codec(AST *p, int l_switch, int l_case, SymbolTable* st); // private recursive function for cases



															   /***********global variables*****************/
int LAB = 0;
int l_while_out = 0;
int l_switch_end = 0;
int l_case = 0;

string l_break = "";


void coder(AST *p, SymbolTable* st) {
	if (p == nullptr)
		return;
	string op = p->getValue();
	AST *R = p->getRight();
	AST *L = p->getLeft();

	if (op == "plus") {
		coder(L, st);
		coder(R, st);
		cout << "add" << endl;
		return;
	}
	else if (op == "minus") {
		coder(L, st);
		coder(R, st);
		cout << "sub" << endl;
		return;
	}
	else if (op == "multiply") {
		coder(L, st);
		coder(R, st);
		cout << "mul" << endl;
		return;
	}
	else if (op == "divide") {
		coder(L, st);
		coder(R, st);
		cout << "div" << endl;
		return;
	}
	else if (op == "and") {
		coder(L, st);
		coder(R, st);
		cout << "and" << endl;
		return;
	}
	else if (op == "or") {
		coder(L, st);
		coder(R, st);
		cout << "or" << endl;
		return;
	}
	else if (op == "lessOrEquals") {
		coder(L, st);
		coder(R, st);
		cout << "leq" << endl;
		return;
	}
	else if (op == "greaterOrEquals") {
		coder(L, st);
		coder(R, st);
		cout << "geq" << endl;
		return;
	}
	else if (op == "greaterThan") {
		coder(L, st);
		coder(R, st);
		cout << "grt" << endl;
		return;
	}
	else if (op == "lessThan") {
		coder(L, st);
		coder(R, st);
		cout << "les" << endl;
		return;
	}
	else if (op == "equals") {
		coder(L, st);
		coder(R, st);
		cout << "equ" << endl;
		return;
	}
	else if (op == "notEquals") {
		coder(L, st);
		coder(R, st);
		cout << "neq" << endl;
		return;
	}

	else if (op == "negative") {
		coder(L, st);
		cout << "neg" << endl;
		return;
	}
	else if (op == "not") {
		coder(L, st);
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
		codel(p, st);
		cout << "ind" << endl;
		return;
	}
	else if (op == "array") {
		codel(p, st);
		cout << "ind" << endl;
		return;
	}
	else if (op == "record") {
		codel(p, st);
		cout << "ind" << endl;
		return;
	}
	else if (op == "pointer") {
		codel(p, st);
		cout << "ind" << endl;
		return;
	}

}
/*
void codem(AST *p, SymbolTable* st) {
if (p == nullptr)
return;
string op = p->getValue();
AST *R = p->getRight();
AST *L = p->getLeft();

if (op == "identifier") {
codel(p, st);
st = dynamic_cast<Record *>((*st)[L->getValue()])->vars;
return;
}
else if (op == "array") {
codel(L, st);
codear(R, L->getLeft()->getValue(), st);
Array *cur = dynamic_cast<Array *>(st[L->getLeft()->getValue()]);
cout << "dec " << cur->subpart << endl;
if (!SymbolTable::isPrimitive(cur->baseType) && cur->baseType != "array") {
st = dynamic_cast<Record *>(st[cur->baseType])->vars;
}
return;
}
else if (op == "record") {
codem(L, st);
cout << "inc " << st[R->getLeft()->getValue()]->addr << endl;
st = dynamic_cast<Record *>(st[R->getLeft()->getValue()])->vars;
return;
}
else if (op == "pointer") {
codel(p, st);
return;
}

}
*/
string codel(AST *p, SymbolTable* st) {
	if (p == nullptr)
		return nullptr;
	string op = p->getValue();
	AST *R = p->getRight();
	AST *L = p->getLeft();

	if (op == "identifier") {
		cout << "ldc " << (*st)[L->getValue()]->getAddr() << endl;
		return L->getValue();
	}
	else if (op == "array") {
		string ar_name = codel(L, st);
		codear(R, ar_name, st);
		Array *cur = dynamic_cast<Array *>((*st)[ar_name]);
		cout << "dec " << cur->subpart << endl;
		return cur->baseType;
	}
	else if (op == "record") {
		codel(L, st);
		cout << "inc " << (*st)[R->getLeft()->getValue()]->addr << endl;
		return R->getLeft()->getValue();
	}
	else if (op == "pointer") {
		string point_to = codel(L, st);
		cout << "ind " << endl;
		//coder(L, st);

		string pname = L->getValue();
		if (pname == "identifier") {
			pname = L->getLeft()->getValue();
		}
		else if (pname == "record") {
			pname = L->getRight()->getLeft()->getValue();
		}
		Pointer *curr = dynamic_cast<Pointer *>((*st)[point_to]);
		point_to = curr->point_to;
		//cout << "inc " << curr->getAddr() << endl;*/
		return point_to;
	}
	return nullptr;
}

void codear(AST *p, string ar_name, SymbolTable *st, int x_i) {
	if (p == nullptr)
		return;
	string op = p->getValue();
	AST *R = p->getRight();
	AST *L = p->getLeft();

	if (op == "indexList") {
		codear(L, ar_name, st, x_i + 1);
		coder(R, st);
		Array *curr = dynamic_cast<Array *>((*st)[ar_name]);
		auto lenList = curr->lenList;
		cout << "ixa " << my_accumulate(lenList.end() - x_i, lenList.end(), curr->baseSize) << endl;
		return;
	}

}

void codec(AST *p, int l_switch, SymbolTable* st) {
	int case_cnt = _codec(p, l_switch, 0, st);
	for (int i = case_cnt; i > 0; i--) {
		cout << "ujp case_" << l_switch << "_" << i << endl;
	}
}

int _codec(AST *p, int l_switch, int l_case, SymbolTable* st) {
	if (p == nullptr)
		return l_case;
	string op = p->getValue();
	AST *R = p->getRight();
	AST *L = p->getLeft();

	if (op == "caseList") {
		l_case = _codec(L, l_switch, l_case, st);
		l_case = _codec(R, l_switch, l_case, st);
		return l_case;
	}
	else if (op == "case") {
		l_case++;
		cout << "case_" << l_switch << "_" << l_case << ":" << endl;
		code(R, st);
		cout << "ujp " << "switch_end_" << l_switch << endl;
		return l_case;
	}
	return l_case;

}

void code(AST *p, SymbolTable* st) {
	if (p == nullptr)
		return;
	string op = p->getValue();
	AST *R = p->getRight();
	AST *L = p->getLeft();

	if (op == "statementsList") {
		code(L, st);
		code(R, st);
		return;
	}

	else if (op == "assignment") {
		codel(L, st);
		coder(R, st);
		cout << "sto" << endl;
		return;
	}
	else if (op == "print") {
		coder(L, st);
		cout << "print" << endl;
		return;
	}
	else if (op == "if" && R->getValue() != "else") {
		int la = LAB;
		LAB++;
		coder(L, st);
		cout << "fjp skip_if_" << la << endl;
		code(R, st);
		cout << "skip_if_" << la << ":" << endl;
		return;
	}
	else if (op == "if" && R->getValue() == "else") {
		int la = LAB;
		LAB++;
		int lb = LAB;
		LAB++;
		coder(L, st);
		cout << "fjp skip_if_" << la << endl;
		code(R->getLeft(), st);
		cout << "ujp skip_else_" << lb << endl;
		cout << "skip_if_" << la << ":" << endl;
		code(R->getRight(), st);
		cout << "skip_else_" << lb << ":" << endl;
		return;
	}
	else if (op == "while") {
		int la = LAB;
		LAB++;
		int lb = LAB;
		l_while_out = lb;
		string prev_l_break = l_break;
		l_break = "while";
		LAB++;
		cout << "while_" << la << ":" << endl;
		coder(L, st);
		cout << "fjp while_out_" << lb << endl;
		code(R, st);
		cout << "ujp while_" << la << endl;
		cout << "while_out_" << lb << ":" << endl;
		l_break = prev_l_break;
		return;
	}
	else if (op == "break") {
		if (l_break == "while") {
			cout << "ujp while_out_" << l_while_out << endl;
		}
		else if (l_break == "switch") {
			cout << "ujp switch_end_" << l_switch_end << endl;
		}
		return;
	}

	else if (op == "switch") {
		int la = LAB++;
		l_switch_end = la;
		string prev_l_reak = l_break;
		l_break = "switch";
		coder(L, st);
		cout << "neg" << endl;
		cout << "ixj switch_end_" << la << endl;
		codec(R, la, st); // codec - caseList
		cout << "switch_end_" << la << ":" << endl;
		l_break = prev_l_reak;
		return;
	}
}


void generatePCode(AST *ast, SymbolTable symbolTable) {
	if (ast == nullptr)
		return;

	code(ast->getRight()->getRight(), &symbolTable);
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

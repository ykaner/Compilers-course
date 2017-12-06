#include "Record.h"


Record::Record(string type, string name, int addr, AST *p, SymbolTable *st): Variable(type, name, addr, 1)
{
	int start_addr = st->global_addr;
	st->global_addr = 0;
	st->get_st(p);
	this->size = st->global_addr;
	st->global_addr += start_addr;
}



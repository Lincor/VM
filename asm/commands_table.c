#include "commands_table.h"

const cmd_info cmd_table[CMD_COUNT] = {
	{"nop", 0},
	{"ldw", 3},
	{"ldb", 2},
	{"llb", 2},
	{"lhb", 2},
	{"add", 2},
	{"sub", 2},
	{"mul", 2},
	{"div", 2},
	{"mod", 2},
	{"jeq", 3},
	{"jne", 3},
	{"jlt", 3},
	{"jgt", 3},
	{"jle", 3},
	{"jge", 3},
	{"in",  2},
	{"shl", 2},
	{"shr", 2},
	{"or",  2},
	{"and", 2},
	{"xor", 2},
	{"not", 2}
};

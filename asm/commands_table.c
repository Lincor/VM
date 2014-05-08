#include "commands_table.h"

const cmd_info cmd_table[CMD_COUNT] = {
	{"nop",   0, 0, {0}},
	{"hlt",   0, 0, {0}},

	{"cpy",   2, 1, {AT_REG_REG, 0}},
	{"ldw",   2, 3, {AT_REG, AT_WORD, 0}},
	{"ldb",   2, 2, {AT_REG, AT_BYTE, 0}},
	{"llb",   2, 2, {AT_REG, AT_BYTE, 0}},
	{"lhb",   2, 2, {AT_REG, AT_BYTE, 0}},

	{"add",   2, 1, {AT_REG_REG, 0}},
	{"sub",   2, 1, {AT_REG_REG, 0}},
	{"mul",   2, 1, {AT_REG_REG, 0}},
	{"div",   2, 1, {AT_REG_REG, 0}},
	{"mod",   2, 1, {AT_REG_REG, 0}},
	{"shl",   2, 1, {AT_REG_REG, 0}},
	{"shr",   2, 1, {AT_REG_REG, 0}},
	{"or",    2, 1, {AT_REG_REG, 0}}, 
	{"and",   2, 1, {AT_REG_REG, 0}},
	{"xor",   2, 1, {AT_REG_REG, 0}},
	{"not",   1, 1, {AT_REG, 0}},

	{"jeq",   3, 4, {AT_REG_REG, AT_SEG_WORD, 0}},
	{"jne",   3, 4, {AT_REG_REG, AT_SEG_WORD, 0}},
	{"jlt",   3, 4, {AT_REG_REG, AT_SEG_WORD, 0}},
	{"jgt",   3, 4, {AT_REG_REG, AT_SEG_WORD, 0}},
	{"jle",   3, 4, {AT_REG_REG, AT_SEG_WORD, 0}},
	{"jge",   3, 4, {AT_REG_REG, AT_SEG_WORD, 0}},
	{"jmp",   1, 3, {AT_SEG_WORD, 0}},
	{"jpr",   1, 1, {AT_REG, 0}},
	{"callr", 1, 1, {AT_REG, 0}},
	{"callv", 1, 3, {AT_SEG_WORD, 0}},

	{"pushr", 2, 2, {AT_BYTE, AT_REG, 0}},
	{"pushv", 2, 3, {AT_BYTE, AT_WORD, 0}},
	{"pop",   2, 2, {AT_BYTE, AT_REG, 0}},

	{"in",    2, 2, {AT_BYTE, AT_REG, 0}}, 
	{"out",   2, 2, {AT_REG, AT_BYTE, 0}} 
};

#pragma once

#include "assembler.h"

/*------------------------------------------*
*                  DEFINES                  *
*------------------------------------------*/
#define CMD_COUNT 23

/*------------------------------------------*
*                  TYPEDEFS                 *
*------------------------------------------*/
typedef struct cmd_info_st {
	char *mnemonics;
	uint8_t argc;
} cmd_info;

/*------------------------------------------*
*                  CONSTS                   *
*------------------------------------------*/
const cmd_info cmd_table[CMD_COUNT] = {
	{"nop", 0},
	{"ldw", 3},
	{"ldb", 2},
	{"llb", 2},
	{"lhb", 2},
	{"add", 1},
	{"sub", 1},
	{"mul", 1},
	{"div", 1},
	{"mod", 1},
	{"jeq", 1},
	{"jne", 1},
	{"jlt", 1},
	{"jgt", 1},
	{"jle", 1},
	{"jge", 1},
	{"in",  2},
	{"shl", 1},
	{"shr", 1},
	{"or",  1},
	{"and", 1},
	{"xor", 1},
	{"not", 1}
};

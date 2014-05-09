#pragma once

#include "assembler.h"

/*------------------------------------------*
*                  DEFINES                  *
*------------------------------------------*/
#define CMD_COUNT 35

/*------------------------------------------*
*                  TYPEDEFS                 *
*------------------------------------------*/
typedef struct cmd_info_st {
	char *mnemonics;
	uint8_t argc;
	uint8_t argl; // length of a cmd's arguments in bytes
	uint8_t argt[4];
} cmd_info;

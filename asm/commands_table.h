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
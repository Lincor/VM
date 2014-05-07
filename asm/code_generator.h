#pragma once

#include "assembler.h"

/*------------------------------------------*
*                  TYPEDEFS                 *
*------------------------------------------*/
typedef struct pair_st {
	char *str;
	int num;
	struct pair_st *next;
} pair;

/*------------------------------------------*
*            FUNCTION PROTOTYPES            *
*------------------------------------------*/
uint8_t code_generator(FILE *stream, line* lines);

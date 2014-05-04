#pragma once

#include "assembler.h"

/*------------------------------------------*
*                  DEFINES                  *
*------------------------------------------*/
#define MAX_LINE_LEN 128

/*------------------------------------------*
*            FUNCTION PROTOTYPES            *
*------------------------------------------*/
uint8_t lexical_analyzer(FILE *file, token_list **list);

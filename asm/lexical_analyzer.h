#pragma once

#include <stdio.h>
#include <stdbool.h>
#include "assembler.h"

/*------------------------------------------*
*                  DEFINES                  *
*------------------------------------------*/
#define MAX_LINE_LEN 128

/*------------------------------------------*
*            FUNCTION PROTOTYPES            *
*------------------------------------------*/
bool lexical_analyzer(FILE *file, token_list **list);

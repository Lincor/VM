#pragma once

#include <stdio.h>
#include <stdbool.h>
#include "assembler.h"

/*------------------------------------------*
*                  DEFINES                  *
*------------------------------------------*/

/*------------------------------------------*
*            FUNCTION PROTOTYPES            *
*------------------------------------------*/
uint8_t semantic_analyzer(token_list *list, line **lines);

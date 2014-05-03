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
bool semantic_analyzer(token_list *list, line **lines);

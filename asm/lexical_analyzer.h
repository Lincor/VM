#pragma once

/*------------------------------------------*
*                  DEFINES                  *
*------------------------------------------*/
#define MAX_LINE_LEN 128

/*------------------------------------------*
*            FUNCTION PROTOTYPES            *
*------------------------------------------*/
bool lexical_analyzer(FILE *file, token_list **list);

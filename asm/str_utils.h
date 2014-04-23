#pragma once

#include "assembler.h"

/*------------------------------------------*
*                  TYPEDEFS                 *
*------------------------------------------*/

typedef struct string_list_st {
	char *string;
	uint32_t code_column;
	struct string_list_st *next;
} string_list;

/*------------------------------------------*
*           FUNCTION PROTOTYPES             *
*------------------------------------------*/
void print_string_list(string_list *list);

/* splits a string into tokens */
string_list *string_tokenizer(char *str, const char *delim, const char *opr);

bool str_to_reg(const char *str, uint8_t *reg);

bool str_to_uint16_t(const char *str, uint16_t *num);

bool str_to_cmd(const char *str, uint8_t *cmd);

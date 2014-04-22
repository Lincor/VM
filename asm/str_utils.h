#pragma once

/*------------------------------------------*
*                  TYPEDEFS                 *
*------------------------------------------*/

typedef struct string_list_st {
	char *string;
	struct string_list_st *next;
} string_list;

/*------------------------------------------*
*           FUNCTION PROTOTYPES             *
*------------------------------------------*/
/* prints a string list */
void print_string_list(string_list *list);

/* splits a string into tokens */
string_list *string_tokenizer(char *str, const char *delim, const char *opr);

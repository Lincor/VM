#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "str_utils.h"

static void string_tokenizer_opr(string_list *list, const char *opr);

string_list *string_tokenizer(char *str, const char *delim, const char *opr)
{
	string_list *list = NULL;
	string_list *cur_list, *next_list;

	char *tok;
	tok = strtok(str, delim);

	while (tok)  {
		if (!list) {
			list = malloc(sizeof(string_list));
			cur_list = list;
		} else {
			next_list = malloc(sizeof(string_list));
			cur_list->next = next_list;
			cur_list = next_list;
		}

		cur_list->string = strdup(tok);
		cur_list->next = NULL;

		tok = strtok(NULL, delim);
	}
	
	string_tokenizer_opr(list, opr);

	return list;
}

static void string_tokenizer_opr(string_list *list, const char *opr)
{
	string_list *temp1, *temp2;

	while (list) {
		int i;
		for (i = 0; list->string[i]; i++)
			if (strchr(opr, list->string[i]) && !(i == 0 && list->string[i + 1] == '\0')) {
				if (i == 0) { // the operator is in the beginning of the line
					temp1 = malloc(sizeof(string_list));
					temp1->string = strdup(list->string + 1);

					temp1->next = list->next;
					list->next = temp1;
				} else if (list->string[i + 1] == '\0') { // the operator is in the end of the line
					temp1 = malloc(sizeof(string_list));
					temp1->string = malloc(sizeof(char) * 2);
					temp1->string[0] = list->string[i];
					temp1->string[1] = '\0';

					temp1->next = list->next;
					list->next = temp1;

					list->string[i] = '\0';
				} else { // the operator is inside the line
					temp1 = malloc(sizeof(string_list));
					temp1->string = malloc(sizeof(char) * 2);
					temp1->string[0] = list->string[i];
					temp1->string[1] = '\0';

					temp1->next = list->next;
					list->next = temp1;
					
					temp2 = malloc(sizeof(string_list));
					temp2->string = strdup(list->string + i + 1);
					temp2->next = temp1->next;
					temp1->next = temp2;

					list->string[i] = '\0';
				}
				break;
			}
		list = list->next;	
	}
}

void print_string_list(string_list *list)
{
	while (list) {
		printf("'%s'\n", list->string);
		list = list->next;
	}
}

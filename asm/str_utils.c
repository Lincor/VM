#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "str_utils.h"
#include "assembler.h"
#include "commands_table.h"

static void string_tokenizer_opr(string_list *list, const char *opr);
static bool isbindigit(char c);
static bool isoctdigit(char c);
static bool isdecdigit(char c);
static bool ishexdigit(char c);
static uint8_t char_to_digit(char c);

extern cmd_info cmd_table[];

string_list *string_tokenizer(char *str, const char *delim, const char *opr)
{
	string_list *list = NULL;
	string_list *cur_list, *next_list;

	char *tok;
	char *first_s;

	first_s = tok = strtok(str, delim);

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
		cur_list->code_column = tok - first_s + 1;
		cur_list->next = NULL;

		tok = strtok(NULL, delim);
	}
	
	string_tokenizer_opr(list, opr);

	return list;
}

static void string_tokenizer_opr(string_list *list, const char *opr)
{
	if (!list)
		return;
	string_list *temp1, *temp2;
	string_list *next = list->next;
	int i;
	for (i = 0; list->string[i]; i++)
		if (strchr(opr, list->string[i]) && !(i == 0 && list->string[i + 1] == '\0')) {
			if (i == 0) { // the operator is at the beginning of the line
				temp1 = malloc(sizeof(string_list));
				temp1->string = strdup(list->string + 1);
				temp1->code_column = list->code_column + 1;

				list->string[1] = '\0';

				temp1->next = list->next;
				list->next = temp1;
				
				next = temp1;
			} else if (list->string[i + 1] == '\0') { // the operator at in the end of the line
				temp1 = malloc(sizeof(string_list));
				temp1->string = malloc(sizeof(char) * 2);
				temp1->string[0] = list->string[i];
				temp1->string[1] = '\0';
				temp1->code_column = list->code_column + i;

				temp1->next = list->next;
				list->next = temp1;

				list->string[i] = '\0';

				next = temp1->next;
			} else { // the operator is inside the line
				temp1 = malloc(sizeof(string_list));
				temp1->string = malloc(sizeof(char) * 2);
				temp1->string[0] = list->string[i];
				temp1->string[1] = '\0';
				temp1->code_column = list->code_column + i;

				temp1->next = list->next;
				list->next = temp1;
				
				temp2 = malloc(sizeof(string_list));
				temp2->string = strdup(list->string + i + 1);
				temp2->code_column = temp1->code_column + 1;

				temp2->next = temp1->next;
				temp1->next = temp2;

				list->string[i] = '\0';

				next = temp2;
			}
			break;
		}
		string_tokenizer_opr(next, opr);
}

void print_string_list(string_list *list)
{
	while (list) {
		printf("%s %d\n", list->string, list->code_column);
		list = list->next;
	}
}

bool str_to_reg(const char *str, uint8_t *reg)
{
	if (str[1] == '\0') {
		if (str[0] >= '0' && str[0] <= '9')
			*reg = str[0] - '0';
		else if (str[0] == 'a')
			*reg = 0xa;
		else if (str[0] == 'b')
			*reg = 0xb;
		else
			return false;
	} else if (str[2] == '\0') {
		if (!strcmp(str, "AC"))
			*reg = 0xc;
		else if (!strcmp(str, "BP"))
			*reg = 0xd;
		else if (!strcmp(str, "SP"))
			*reg = 0xe;
		else if (!strcmp(str, "PC"))
			*reg = 0xf;
		else
			return false;
	}
	return true;
}

bool str_to_uint16_t(const char *str, uint16_t *num)
{
	uint8_t radix = 10;
	bool (*exam)(char) = isdecdigit;
	if (strlen(str) > 2) {
		if (!strncmp(str, "0b", 2)) {
			radix = 2;
			exam = isbindigit;
			str += 2;
		} else if (!strncmp(str, "0o", 2)) {
			radix = 8;
			exam = isoctdigit;
			str += 2;
		} else if (!strncmp(str, "0d", 2))
			str += 2;
		else if (!strncmp(str, "0x", 2)) {
			radix = 16;
			exam = ishexdigit;
			str += 2;
		}
	}

	int i;
	*num = 0;
	for (i = 0; str[i]; i++)
		if (exam(str[i])) {
			*num *= radix;
			*num += char_to_digit(str[i]);;
		} else
			return false;
	return true;
}

bool str_to_cmd(const char *str, uint8_t *cmd)
{
	int i;
	for (i = 0; i < CMD_COUNT; i++)
		if (!strcmp(cmd_table[i].mnemonics, str)) {
			*cmd = i;
			return true;
		}
	return false;
}

static bool isbindigit(char c)
{
	return (c == '0') || (c == '1');
}

static bool isoctdigit(char c)
{
	return (c >= '0') && (c <= '7');
}

static bool isdecdigit(char c)
{
	return (c >= '0') && (c <= '9');
}

static bool ishexdigit(char c)
{
	return (c >= '0' && c <= '9')
		|| (c >= 'a' && c <= 'f');
}

static uint8_t char_to_digit(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	else if (c >= 'a' && c <= 'f')
		return c - 'a';
	else
		return 0;
}

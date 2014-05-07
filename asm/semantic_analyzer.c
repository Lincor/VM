#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "semantic_analyzer.h"
#include "commands_table.h"

extern cmd_info cmd_table[];

uint8_t semantic_analyzer(token_list *list, line** lines)
{
	line *first_line = NULL, *cur_line = NULL, *next_line;
	arg *first_arg = NULL, *cur_arg = NULL, *next_arg;
	token *t;

	*lines = NULL;

	while (list) {

		if (!first_line) {
			cur_line = malloc(sizeof(line));
			first_line = cur_line;
		} else {
			next_line = malloc(sizeof(line));
			cur_line->next = next_line;
			cur_line = next_line;
		}
		cur_line->code_line = list->code_line;


		t = list->first_token;

		if (!t)
			goto next;

		if (t->type == TK_LABEL) {
			cur_line->type = CL_LABEL;
			cur_line->label = strdup(t->value_s);
			goto next;
		} else if (t->type = TK_CMD) {
			cur_line->type = CL_CMD;
			cur_line->command = malloc(sizeof(cmd));
			cur_line->command->cmd_i = t->value;
		} else
			return ERR_EXP_LBL_CMD_AT_BEG;

		bool new_arg = true;
		t = t->next;
		while (t) { // CL_CMD
			
			if (!first_arg) {
				cur_arg = malloc(sizeof(arg));
				first_arg = cur_arg;
			} else {
				if (new_arg) {
					next_arg = malloc(sizeof(arg));
					cur_arg->next = next_arg;
					cur_arg = next_arg;
				} else
					new_arg = true;
			}

	
			if (t->type == TK_COMMA) {
				new_arg = false;
				goto next_token;
			} else if (t->type == TK_SYMBOL) {
				cur_arg->type = CA_SYMBOL;
				cur_arg->value_s = strdup(t->value_s);
			} else if (t->type == TK_SYMBOL_ADR) {
				cur_arg->type = CA_SYMBOL_ADR;
				cur_arg->value_s = strdup(t->value_s);
			} else if (t->type == TK_REG) {
				cur_arg->type = CA_REG;
				cur_arg->v1 = t->value;
			} else if (t->type == TK_IMM) {
				cur_arg->type = CA_IMM;
				cur_arg->v1 = t->value & 0x000000ff;
				cur_arg->v2 = (t->value & 0x0000ff00) >> 8;
			} else
				asm_error(ERR_INV_TK, cur_line->code_line, t->code_column);

next_token:
			t = t->next;
		}

		cur_line->command->args = first_arg;
		first_arg = NULL;

		if (cur_arg)
			cur_arg->next = NULL;
		
next:
		list = list->next;
	}

	*lines = first_line;
	if (cur_line)
		cur_line->next = NULL;

	return ERR_NO_ERROR;
}

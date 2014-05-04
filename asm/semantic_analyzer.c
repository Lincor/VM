#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "semantic_analyzer.h"
#include "commands_table.h"

extern cmd_info cmd_table[];

static uint8_t parse_address(token *t, arg *a, token** out);

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
			} else if ((t->type == TK_IMM && t->next && t->next->type == TK_OBRACKET)
				|| (t->type == TK_OBRACKET)) {
				uint8_t err = parse_address(t, cur_arg, &t);
				if (err != ERR_NO_ERROR)
					return err;
			} else if (t->type == TK_IMM) {
				cur_arg->type = CA_IMM;
				cur_arg->v1 = t->value & 0x000000ff;
				cur_arg->v2 = (t->value & 0x0000ff00) >> 8;
			} else
				return ERR_INV_TK;

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

static uint8_t parse_address(token *t, arg *a, token** out) // parse structures of the form $a(%b,%c,$d)
{
	uint8_t v1 = 0, v2 = 0, v3 = 0, v4 = 1;
	
	if (t->type == TK_IMM) {
		v1 = t->value & 0xff;
		t = t->next;
	}

	if ((t = t->next) && (t->type == TK_REG)) {
		v2 = t->value;
		if (!(t = t->next))
			return ERR_EXP_CBRC_COM_AFT_REG;
		if (t->type == TK_CBRACKET)
			goto end;
	} else
		return ERR_EXP_REG_AFT_OBRC;

	if (t->type != TK_COMMA)
		return ERR_EXP_REG_COM_AFT_OBRC;

	if (!(t = t->next) || (t->type != TK_REG))
		return ERR_EXP_REG_AFT_COM;
	else
		v3 = t->value;

	if (!(t = t->next))
		return ERR_EXP_CBRC_COM_AFT_REG;
	if (t->type == TK_CBRACKET)
		goto end;
	else if (t->type != TK_COMMA)
		return ERR_EXP_CBRC_COM_AFT_REG;

	if (!(t = t->next) || (t->type != TK_IMM))
		return ERR_EXP_IMM_AFT_COM;
	else
		if ((t->value == 1) || (t->value == 2) || (t->value) == 4) {
			v4 = t->value;
			t = t->next;
		} else
			return ERR_INV_IMM_1_2_4;

end:

	a->type = CA_ADDRESS;
	a->v1 = v1;
	a->v2 = v2;
	a->v3 = v3;
	a->v4 = v4;

	*out = t;

	return ERR_NO_ERROR;
}

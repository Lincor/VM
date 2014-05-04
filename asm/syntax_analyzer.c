#include <stdbool.h>
#include <stdio.h>

#include "assembler.h"
#include "commands_table.h"

extern cmd_info cmd_table[];

uint8_t syntax_analyzer(token_list *list)
{
	token_list *tl = list;
	token *t = NULL;

	while (tl) {
		t = tl->first_token;

		if (!t)
			goto next;

		if (t->type == TK_LABEL)
			if (t->next)
				return ERR_UNEXP_TK_AFT_LBL;
			else
				goto next;
		
		if (t->type == TK_CMD) {
			uint8_t argc = cmd_table[t->value].argc;
			uint8_t count = 0;
			
			t = t->next;
			int brackets = 0;
			while (t) {
				if (t->type == TK_OBRACKET)
					brackets++;
				else if (t->type == TK_CBRACKET)
					brackets--;
				if (brackets == 0 &&
					((t->type != TK_COMMA && t->next && t->next->type == TK_COMMA) ||
					(t->type != TK_COMMA && !t->next)))
					count++;
				if (t->next && t->type == TK_COMMA && t->next->type == TK_COMMA) 
					return ERR_MANY_COM;
				if (t->type == TK_COMMA && !t->next)
					return ERR_EXP_TK_AFT_COM;
				t = t->next;
			}

			if (count > argc)
				return ERR_MANY_ARGS;
			else if (count < argc)
				return ERR_FEW_ARGS;
		} else
			return ERR_EXP_LBL_CMD_AT_BEG;
next:
		tl = tl->next;
	}
	return ERR_NO_ERROR;
}

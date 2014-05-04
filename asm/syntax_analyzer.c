#include <stdbool.h>
#include <stdio.h>

#include "assembler.h"
#include "commands_table.h"

extern cmd_info cmd_table[];

uint8_t syntax_analyzer(token_list *list)
{
	token_list *tl = list;
	token *t = NULL, *pt;

	while (tl) {
		t = tl->first_token;
		pt = t;

		if (!t)
			goto next;

		if (t->type == TK_LABEL)
			if (t->next)
				asm_error(ERR_UNEXP_TK_AFT_LBL, tl->code_line, t->code_column);
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
					asm_error(ERR_MANY_COM, tl->code_line, t->code_column);
				if (t->type == TK_COMMA && !t->next)
					asm_error(ERR_EXP_TK_AFT_COM, tl->code_line, t->code_column);
				pt = t;
				t = t->next;
			}

			if (count > argc)
				asm_error(ERR_MANY_ARGS, tl->code_line, pt->code_column);
			else if (count < argc)
				asm_error(ERR_FEW_ARGS, tl->code_line, pt->code_column);
		} else
			asm_error(ERR_EXP_LBL_CMD_AT_BEG, tl->code_line, t->code_column);
next:
		tl = tl->next;
	}
	return ERR_NO_ERROR;
}

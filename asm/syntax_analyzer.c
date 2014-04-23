#include <stdbool.h>

#include "assembler.h"
#include "commands_table.h"

extern cmd_info cmd_table[];

bool syntax_analyzer(token_list *list)
{
	token_list *tl = list;
	while (tl) {
		token *t = tl->first_token;

		if (t->type == TK_LABEL)
			if (t->next)
				return false; //TODO
			else
				goto next;
		
		if (t->type == TK_CMD) {
			uint8_t argc = cmd_table[t->value].argc;
			uint8_t count = 0;
			
			t = t->next;
			while (t) {
				if ((t->type != TK_COMMA && t->next && t->next->type == TK_COMMA) ||
					(t->type != TK_COMMA && !t->next))
					count++;
				if (t->next && t->type == TK_COMMA && t->next->type == TK_COMMA) 
					return false; //TODO
				if (t->type == TK_COMMA && !t->next)
					return false; //TODO
				t = t->next;
			}
			if (count > argc)
				return false; //TODO
			else if (count < argc)
				return false; //TODO
		} else
			return false; //TODO
next:
		tl = tl->next;
	}
	return true;
}
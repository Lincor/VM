#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "assembler.h"
#include "str_utils.h"
#include "lexical_analyzer.h"
#include "syntax_analyzer.h"

static bool classify_token(char *tok, token *token_item);

const char *delim = " \n\t\v";
const char *opr = ",:#()";

bool lexical_analyzer(FILE *file, token_list **list)
{
	uint32_t code_line;
	char line[MAX_LINE_LEN + 1];
	string_list *line_tokens;
	token *first_token = NULL, *cur_token = NULL, *next_token;
	token_list *first_token_list = NULL, *cur_token_list = NULL, *next_token_list;
	int braces;

	code_line = 1;
	braces = 0;

	while(fgets(line, sizeof(line), file)) {
		line_tokens = string_tokenizer(line, delim, opr);

		if (!first_token_list) {
			first_token_list = malloc(sizeof(token_list));
			cur_token_list = first_token_list;
		} else {
			next_token_list = malloc(sizeof(token_list));
			cur_token_list->next = next_token_list;
			cur_token_list = next_token_list;
		}

		while (line_tokens) {
			if (line_tokens->string[0] == ':') {
				if (cur_token)
					if (cur_token->type == TK_SYMBOL)
						cur_token->type = TK_LABEL;
					else
						return false; //TODO
				else
					return false; //TODO
			}

			else if (line_tokens->string[0] == '(') {
				braces++;
				if (line_tokens->next) {
					if (!((line_tokens->next->next && line_tokens->next->next->string[0] == ')') ||  //(base)
						(line_tokens->next->next && line_tokens->next->next->next &&
						 line_tokens->next->next->next->next && line_tokens->next->next->next->next->next &&
						 (line_tokens->next->next->next->next->next->string[0] == ')' ||  //(,index,multiplier)
						  (line_tokens->next->next->next->next->next->next &&
						   line_tokens->next->next->next->next->next->next->string[0] == ')'))))) //(base,index,nultiplier)
						return false; //TODO

				} else
					return false; //TODO
			}

			else if (line_tokens->string[0] == ')') {
				braces--;
				if (braces < 0)
					return false; //TODO
			}

			else if (line_tokens->string[0] == '#')
				break;

			else {
				if (!first_token) {
					cur_token = malloc(sizeof(token));
					first_token = cur_token;
				} else {
					next_token = malloc(sizeof(token));
					cur_token->next = next_token;
					cur_token = next_token;
				}

				if (!classify_token(line_tokens->string, cur_token))
					return false; //TODO
				cur_token->code_column = line_tokens->code_column;
			}


			line_tokens = line_tokens->next;
		}

		if (braces > 0)
			return false; //TODO

		cur_token_list->first_token = first_token;
		cur_token_list->code_line = code_line;

		cur_token->next = NULL;
		first_token = NULL;

		code_line++;
	}
	cur_token_list->next = NULL;

	*list = first_token_list;
	
	return true;
}

static bool classify_token(char *tok, token *token_item)
{
	/* IMMEDIATE */
	if (tok[0] == '$') {
		uint16_t imm;
		if (str_to_uint16_t(tok + 1, &imm)) {
			token_item->type = TK_IMM;
			token_item->value = imm;
		} else {
			return false;
		}
	}

	/* COMMA */	
	else if (tok[0] == ',') {
		token_item->type = TK_COMMA;
	}

	/* REGISTER */
	else if (tok[0] == '%') {
		uint8_t reg;
		if (str_to_reg(tok + 1, &reg)) {
			token_item->type = TK_REG;
			token_item->value = reg;
		} else {
			return false;
		}
	}

	else {
		/* COMMAND */
		uint8_t cmd;
		if (str_to_cmd(tok, &cmd)) {
			token_item->type = TK_CMD;
			token_item->value = cmd;
			token_item->value_s = strdup(tok);
		}
		
		/* SYMBOL */
		else {
			if (strlen(tok) <= MAX_SYMBOL_SIZE) {
				int i;
				for (i = 0; tok[i]; i++)
					if (tok[i] != '_' && !isalnum(tok[i]))
						return false;

				token_item->type = TK_SYMBOL;
				token_item->value_s = strdup(tok);
			} else
				return false;
		}
	}


	return true;
}

/*int main(void)*/
/*{*/
	/*FILE *f = fopen("test.asm", "r");*/
	/*token_list *tl;*/
	/*lexical_analyzer(f, &tl);*/
	/*printf("Syntax: %s\n", syntax_analyzer(tl) ? "OK" : "FAIL");*/
	/*while (tl) {*/
		/*token *t;*/
		/*t = tl->first_token;*/
		/*while (t) {*/
			/*switch (t->type) {*/
				/*case 1:*/
				/*case 2:*/
				/*case 3:*/
					/*printf(" %d %s;", t->type, t->value_s);*/
					/*break;*/
				/*case 4:*/
				/*case 5:*/
					/*printf(" %d %d;", t->type, t->value);*/
					/*break;*/
				/*case 6:*/
					/*printf(" comma;");*/
					/*break;*/
			/*}*/
			/*t = t->next;*/
		/*}*/
		/*printf("\n");*/
		/*tl = tl->next;*/
	/*}*/

	/*return 0;*/
/*}*/

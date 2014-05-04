#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "assembler.h"
#include "str_utils.h"
#include "lexical_analyzer.h"
#include "syntax_analyzer.h"

static uint8_t classify_token(char *tok, token *token_item);

const char *delim = " \n\t\v";
const char *opr = ",:#()";

uint8_t lexical_analyzer(FILE *file, token_list **list)
{
	uint32_t code_line;
	char line[MAX_LINE_LEN + 1];
	string_list *line_tokens;
	token *first_token = NULL,*cur_token = NULL, *next_token;
	token_list *first_token_list = NULL, *prev_token_list = NULL, *cur_token_list = NULL, *next_token_list;
	int brackets;

	*list = NULL;

	code_line = 1;
	brackets = 0;

	while(fgets(line, sizeof(line), file)) {
		line_tokens = string_tokenizer(line, delim, opr);

		if (!first_token_list) {
			first_token_list = malloc(sizeof(token_list));
			cur_token_list = first_token_list;
		} else {
			if (cur_token_list->first_token) {
				prev_token_list = cur_token_list;
				next_token_list = malloc(sizeof(token_list));
				cur_token_list->next = next_token_list;
				cur_token_list = next_token_list;
			}
		}

		while (line_tokens) {
			if (line_tokens->string[0] == ':') {
				if (cur_token)
					if (cur_token->type == TK_SYMBOL)
						cur_token->type = TK_LABEL;
					else
						return ERR_EXP_LBL_BEF_COL;
				else
					return ERR_EXP_LBL_BEF_COL;
			}

			else if (line_tokens->string[0] == '#')
				break;

			else {
				if (line_tokens->string[0] == '(') {
					brackets++;
					if (line_tokens->next) {
						if (!((line_tokens->next->next && line_tokens->next->next->string[0] == ')') ||  //(base)
							(line_tokens->next->next && line_tokens->next->next->next &&
							 line_tokens->next->next->next->string[0] == ')') || //(,index)
							(line_tokens->next->next && line_tokens->next->next->next &&
							 line_tokens->next->next->next->next &&
							 line_tokens->next->next->next->next->string[0] == ')') || //(base,index)
							(line_tokens->next->next && line_tokens->next->next->next &&
							 line_tokens->next->next->next->next && line_tokens->next->next->next->next->next &&
							 (line_tokens->next->next->next->next->next->string[0] == ')' ||  //(,index,multiplier)
							  (line_tokens->next->next->next->next->next->next &&
							   line_tokens->next->next->next->next->next->next->string[0] == ')'))))) //(base,index,multiplier)
							return ERR_INV_ADR;
					} else
						return ERR_EXP_REG_COM_AFT_OBRC;
				} else if (line_tokens->string[0] == ')') {
					brackets--;
					if (brackets < 0)
						return ERR_MANY_CBRC;
				}

				if (!first_token) {
					cur_token = malloc(sizeof(token));
					first_token = cur_token;
				} else {
					next_token = malloc(sizeof(token));
					cur_token->next = next_token;
					cur_token = next_token;
				}

				uint8_t err = classify_token(line_tokens->string, cur_token);
				if (err != ERR_NO_ERROR)
					return err;

				cur_token->code_column = line_tokens->code_column;
			}


			line_tokens = line_tokens->next;
		}

		if (brackets > 0)
			return ERR_MANY_OBRC;

		cur_token_list->first_token = first_token;
		cur_token_list->code_line = code_line;

		if (cur_token)
			cur_token->next = NULL;
		first_token = NULL;

		code_line++;
	}

	if (cur_token_list->first_token)
		cur_token_list->next = NULL;
	else
		prev_token_list->next = NULL;

	*list = first_token_list;
	
	return ERR_NO_ERROR;
}

static uint8_t classify_token(char *tok, token *token_item)
{
	/* IMMEDIATE OR SYMBOL_ADR */
	if (tok[0] == '$') {
		uint16_t imm;
		if (!isdigit(tok[1])) {
			token_item->type = TK_SYMBOL_ADR;
			token_item->value_s = strdup(tok);
		} else if (str_to_uint16_t(tok + 1, &imm)) {
			token_item->type = TK_IMM;
			token_item->value = imm;
		} else {
			return ERR_INV_DLR;
		}
	}

	/* COMMA */	
	else if (tok[0] == ',')
		token_item->type = TK_COMMA;

	/* BRACKETS */
	else if (tok[0] == '(')
		token_item->type = TK_OBRACKET;
	else if (tok[0] == ')')
		token_item->type = TK_CBRACKET;

	/* REGISTER */
	else if (tok[0] == '%') {
		uint8_t reg;
		if (str_to_reg(tok + 1, &reg)) {
			token_item->type = TK_REG;
			token_item->value = reg;
		} else {
			return ERR_INV_REG;
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
						return ERR_ILL_CHR_SYM;

				token_item->type = TK_SYMBOL;
				token_item->value_s = strdup(tok);
			} else
				return ERR_LONG_SYM;
		}
	}


	return ERR_NO_ERROR;
}

#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "assembler.h"
#include "str_utils.h"
#include "lexical_analyzer.h"
#include "syntax_analyzer.h"
#include "asm_error.h"

static uint8_t classify_token(char *tok, token *token_item, int code_line);

const char *delim = " \n\t\v";
const char *opr = ",:#";

uint8_t lexical_analyzer(FILE *file, token_list **list) {
	uint32_t code_line;
	char line[MAX_LINE_LEN + 1];
	string_list *line_tokens, *prev_tokens;
	token *first_token = NULL,*cur_token = NULL, *next_token;
	token_list *first_token_list = NULL, *prev_token_list = NULL, *cur_token_list = NULL, *next_token_list;

	*list = NULL;

	code_line = 1;

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

		prev_tokens = line_tokens;
		while (line_tokens) {
			if (line_tokens->string[0] == ':') {
				if (cur_token)
					if (cur_token->type == TK_SYMBOL)
						cur_token->type = TK_LABEL;
					else if (cur_token->type == TK_IMM)
						if (cur_token->value >= FIRST_SEG && cur_token->value <= LAST_SEG)
							cur_token->type = TK_SEG;
						else
							asm_error(ERR_INV_SEG_RANGE, code_line, prev_tokens->code_column);
					else
						asm_error(ERR_EXP_LBL_IMM_BEF_COL, code_line, line_tokens->code_column);
				else
					asm_error(ERR_EXP_LBL_IMM_BEF_COL, code_line, line_tokens->code_column);
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

				cur_token->code_column = line_tokens->code_column;

				uint8_t err = classify_token(line_tokens->string, cur_token, code_line);
				if (err != ERR_NO_ERROR)
					asm_error(err, code_line, line_tokens->code_column);
			}

			prev_tokens = line_tokens;
			line_tokens = line_tokens->next;
		}

		cur_token_list->first_token = first_token;
		cur_token_list->code_line = code_line;

		if (cur_token)
			cur_token->next = NULL;
		first_token = NULL;

		code_line++;
	}

	if (first_token_list->first_token) {
		if (cur_token_list->first_token)
			cur_token_list->next = NULL;
		else
			prev_token_list->next = NULL;

		*list = first_token_list;
	} else
		list = NULL;

	return ERR_NO_ERROR;
}

static uint8_t classify_token(char *tok, token *token_item, int code_line) {
	/* IMMEDIATE OR SYMBOL_ADR */
	if (tok[0] == '$') {
		uint16_t imm;
		uint8_t err = ERR_INV_DLR;
		if (!isdigit(tok[1])) {
			token_item->type = TK_SYMBOL_ADR;
			token_item->value_s = strdup(tok);
		} else if ((err = str_to_uint16_t(tok + 1, &imm)) == ERR_NO_ERROR) {
			token_item->type = TK_IMM;
			token_item->value = imm;
		} else {
			asm_error(err, code_line, token_item->code_column);
		}
	}

	/* COMMA */
	else if (tok[0] == ',')
		token_item->type = TK_COMMA;

	/* REGISTER */
	else if (tok[0] == '%') {
		uint8_t reg;
		if (str_to_reg(tok + 1, &reg)) {
			token_item->type = TK_REG;
			token_item->value = reg;
		} else {
			asm_error(ERR_INV_REG, code_line, token_item->code_column);
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
						asm_error(ERR_ILL_CHR_SYM, code_line, token_item->code_column);

				token_item->type = TK_SYMBOL;
				token_item->value_s = strdup(tok);
			} else
				asm_error(ERR_LONG_SYM, code_line, token_item->code_column);
		}
	}


	return ERR_NO_ERROR;
}

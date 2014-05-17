#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "assembler.h"
#include "code_generator.h"
#include "commands_table.h"

extern cmd_info cmd_table[];

static void get_labels(pair **labels, line *lines);
static bool add_label(pair **labels, const char *label, int adr);
static int get_label(pair **labels, const char *label);

uint8_t code_generator(FILE *stream, line* lines)
{
	pair *labels = NULL;
	get_labels(&labels, lines);

	while (lines) {
		if (lines->type != CL_CMD)
			goto next_line;

		cmd *c = lines->command;
		arg *a = c->args;

		fputc(c->cmd_i, stream);

		int i;

		for (i = 0; cmd_table[c->cmd_i].argt[i]; i++) { // first, writing register's bytes
			uint8_t t = cmd_table[c->cmd_i].argt[i];
			int l;

			if (!a)
				asm_error(ERR_FEW_ARGS, lines->code_line, 0);

			switch (t) {
				case AT_REG:
					switch (a->type) {
						case CA_REG:
							fputc(a->v1, stream);
							break;
						default:
							asm_error(ERR_EXP_REG, lines->code_line, a->code_column);
							exit(1);
							break;
					}
					break;
				case AT_REG_REG:
					switch (a->type) {
						case CA_REG:
							if (!a->next)
								asm_error(ERR_FEW_ARGS, lines->code_line, a->code_column);
							if (a->next->type != CA_REG) {
								asm_error(ERR_EXP_REG, lines->code_line, a->next->code_column);
								exit(1);
							}
							fputc((a->v1 << 4) | a->next->v1, stream);
							a = a->next;
							break;
						default:
							asm_error(ERR_EXP_REG, lines->code_line, a->code_column);
							exit(1);
							break;
					}
					break;
			}
			a = a->next;
		}

		a = c->args;
		for (i = 0; cmd_table[c->cmd_i].argt[i]; i++) { // second, writing others
			uint8_t t = cmd_table[c->cmd_i].argt[i];
			int l;

			if (!a)
				asm_error(ERR_FEW_ARGS, lines->code_line, 0);

			switch (t) {
				case AT_BYTE:
					switch (a->type) {
						case CA_SYMBOL:
							asm_error(ERR_VAR, lines->code_line, a->code_column);
							break;
						case CA_SYMBOL_ADR:
							if ((l = get_label(&labels, a->value_s + 1)) == -1)
								asm_error(ERR_UNDEF_LBL, lines->code_line, a->code_column);
							fputc(l & 0xff, stream);
							break;
						case CA_IMM:
							fputc(a->v2, stream);
							break;
						default:
							asm_error(ERR_EXP_SYM_ADR_IMM, lines->code_line, a->code_column);
							break;
					}
					break;
				case AT_WORD:
					switch (a->type) {
						case CA_SYMBOL:
							asm_error(ERR_VAR, lines->code_line, a->code_column);
							break;
						case CA_SYMBOL_ADR:
							if ((l = get_label(&labels, a->value_s + 1)) == -1)
								asm_error(ERR_UNDEF_LBL, lines->code_line, a->code_column);
							fputc((l & 0xff00) >> 8, stream);
							fputc(l & 0xff, stream);
							break;
						case CA_IMM:
							fputc(a->v1, stream);
							fputc(a->v2, stream);
							break;
						default:
							asm_error(ERR_EXP_SYM_ADR_IMM, lines->code_line, a->code_column);
							exit(1);
							break;
					}
					break;
				case AT_SEG_WORD:
					fputc(a->v1, stream);
					switch (a->type) {
						case CA_SEG:
							switch (a->v4) {
								case TK_SYMBOL:
									asm_error(ERR_VAR, lines->code_line, a->code_column);
									break;
								case TK_SYMBOL_ADR:
									if ((l = get_label(&labels, a->value_s + 1)) == -1)
										asm_error(ERR_UNDEF_LBL, lines->code_line, a->code_column);
									fputc((l & 0xff00) >> 8, stream);
									fputc(l & 0xff, stream);
									break;
								case TK_IMM:
									fputc(a->v2, stream);
									fputc(a->v3, stream);
									break;
							}
							break;
						default:
							asm_error(ERR_EXP_SEG, lines->code_line, a->code_column);
							exit(1);
							break;
					}
					break;
			}
			a = a->next;
		}

next_line:
		lines = lines->next;
	}

	return ERR_NO_ERROR;
}

static void get_labels(pair **labels, line *lines)
{
	int byte = 0;

	while (lines) {
		if (lines->type == CL_LABEL) {
			if (!add_label(labels, lines->label, byte))
				asm_error(ERR_ALDEF_LBL, lines->code_line, 0);
		} else if (lines->type == CL_CMD)
			byte += cmd_table[lines->command->cmd_i].argl + 1;

		lines = lines->next;
	}
}

static bool add_label(pair **labels, const char *label, int adr)
{
	pair *t;

	if (!*labels) {
		t = *labels = malloc(sizeof(pair));
	} else if (!(*labels)->next) {
		if (!strcmp((*labels)->str, label))
			return false;
		t = (*labels)->next = malloc(sizeof(pair));
	} else {
		if (!strcmp((*labels)->str, label))
			return false;
		return add_label(&((*labels)->next), label, adr);
	}

	t->str = strdup(label);
	t->num = adr;
	t->next = NULL;

	return true;
}

static int get_label(pair **labels, const char *label)
{
	if (!*labels)
		return -1;
	else if (!strcmp((*labels)->str, label))
		return (*labels)->num;
	else
		return get_label(&((*labels)->next), label);
}

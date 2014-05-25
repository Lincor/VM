#include <stdio.h>
#include <stdlib.h>

#include "assembler.h"
#include "str_utils.h"
#include "lexical_analyzer.h"
#include "syntax_analyzer.h"
#include "code_generator.h"


int main(int argc, char **argv) {

	FILE *f = fopen(argc > 1 ? argv[1] : "test.asm", "r");

	token_list *tl, *ttl;
	uint8_t err;
	if ((err = lexical_analyzer(f, &tl)) != ERR_NO_ERROR) {
		printf("There's a lexical error in the code: %d.\n", err);

		return EXIT_FAILURE;
	}

	if ((err = syntax_analyzer(tl)) != ERR_NO_ERROR)
		printf("There's a syntax error in the code: %d.\n", err);

	//ttl = tl;
	//while (tl) {
	//token *t;
	//t = tl->first_token;
	//while (t) {
	//switch (t->type) {
	//case TK_LABEL:
	//case TK_SYMBOL:
	//case TK_SYMBOL_ADR:
	//case TK_CMD:
	//printf(" %d %s;", t->type, t->value_s);
	//break;
	//case TK_REG:
	//case TK_IMM:
	//case TK_SEG:
	//printf(" %d %d;", t->type, t->value);
	//break;
	//case TK_COMMA:
	//printf(" comma;");
	//break;
	//}
	//t = t->next;
	//}
	//printf("\n");
	//tl = tl->next;
	//}
	//tl = ttl;

	line *lines, *tlines;
	if ((err = semantic_analyzer(tl, &lines)) != ERR_NO_ERROR) {
		printf("There's a semantic error in the code: %d.\n", err);

		return EXIT_FAILURE;
	}

	//tlines = lines;
	//while (lines) {
	//switch (lines->type) {
	//case CL_LABEL:
	//printf("CL_LABEL: %s\n", lines->label);
	//break;
	//case CL_CMD:
	//printf("CL_CMD: \n");
	//printf("\tcmd: %d\n", lines->command->cmd_i);
	//arg *a = lines->command->args;
	//if (a)
	//printf("\targs:\n");
	//while (a) {
	//switch (a->type) {
	//case CA_SYMBOL:
	//printf("\t\tsymbol: %s\n", a->value_s);
	//break;
	//case CA_SYMBOL_ADR:
	//printf("\t\tsymbol_adr: %s\n", a->value_s);
	//break;
	//case CA_REG:
	//printf("\t\treg: %d\n", a->v1);
	//break;
	//case CA_IMM:
	//printf("\t\tconst: %d\n", (a->v1 << 8) | a->v2);
	//break;
	//case CA_SEG:
	//switch (a->v4) {
	//case TK_SYMBOL:
	//printf("\t\tseg: %d:(symbol %s)\n", a->v1, a->value_s);
	//break;
	//case TK_SYMBOL_ADR:
	//printf("\t\tseg: %d:(symbol_adr %s)\n", a->v1, a->value_s);
	//break;
	//case TK_IMM:
	//printf("\t\tseg: %d:(const %d)\n", a->v1, (a->v2 << 8) | a->v3);
	//break;
	//}
	//break;
	//}
	//a = a->next;
	//}
	//break;
	//}

	//lines = lines->next;
	//}
	//lines = tlines;

	code_generator(stdout, lines);

	return EXIT_SUCCESS;
}

#include <stdio.h>
#include <stdlib.h>

#include "assembler.h"
#include "str_utils.h"
#include "lexical_analyzer.h"
#include "syntax_analyzer.h"


int main(int argc, char **argv)
{

	FILE *f = fopen(argc > 1 ? argv[1] : "test.asm", "r");

	token_list *tl, *ttl;
	if (!lexical_analyzer(f, &tl)) {
		printf("There's a lexical error in the code.\n");

		return EXIT_FAILURE;
	}

	printf("Syntax: %s\n", syntax_analyzer(tl) ? "OK" : "FAIL");

	ttl = tl;
	while (tl) {
		token *t;
		t = tl->first_token;
		while (t) {
			switch (t->type) {
				case 0:
				case 1:
				case 2:
					printf(" %d %s;", t->type, t->value_s);
					break;
				case 3:
				case 4:
					printf(" %d %d;", t->type, t->value);
					break;
				case 5:
					printf(" comma;");
					break;
			}
			t = t->next;
		}
		printf("\n");
		tl = tl->next;
	}
	tl = ttl;

	line *lines;
	if (!semantic_analyzer(tl, &lines)) {
		printf("There's a semantic error in the code.\n");

		return EXIT_FAILURE;
	}

	while (lines) {
		switch (lines->type) {
			case CL_LABEL:
				printf("CL_LABEL: %s\n", lines->label);
				break;
			case CL_CMD:
				printf("CL_CMD: \n");
				printf("\tcmd: %d\n", lines->command->cmd_i);
				arg *a = lines->command->args;
				if (a)
					printf("\targs:\n");
				while (a) {
					switch (a->type) {
						case CA_SYMBOL:
							printf("\t\tsymbol: %s\n", a->value_s);
							break;
						case CA_SYMBOL_ADR:
							printf("\t\tsymbol_adr: %s\n", a->value_s);
							break;
						case CA_REG:
							printf("\t\treg: %d\n", a->v1);
							break;
						case CA_IMM:
							printf("\t\tconst: %d\n", (a->v1 << 8) | a->v2);
							break;
						case CA_ADDRESS:
							printf("\t\taddress: %d(%d,%d,%d)\n", a->v1, a->v2, a->v3, a->v4);
							break;
					}
					a = a->next;
				}
				break;
		}

		lines = lines->next;
	}

	return EXIT_SUCCESS;
}

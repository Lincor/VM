#include <stdio.h>

#include "assembler.h"
#include "str_utils.h"
#include "lexical_analyzer.h"
#include "syntax_analyzer.h"


int main(int argc, char **argv)
{
	FILE *f = fopen(argc > 1 ? argv[1] : "test.asm", "r");

	token_list *tl;
	if (!lexical_analyzer(f, &tl))
		printf("Error in the lexer\n");

	printf("Syntax: %s\n", syntax_analyzer(tl) ? "OK" : "FAIL");

	while (tl) {
		token *t;
		t = tl->first_token;
		while (t) {
			switch (t->type) {
				case 1:
				case 2:
				case 3:
					printf(" %d %s;", t->type, t->value_s);
					break;
				case 4:
				case 5:
					printf(" %d %d;", t->type, t->value);
					break;
				case 6:
					printf(" comma;");
					break;
			}
			t = t->next;
		}
		printf("\n");
		tl = tl->next;
	}

	//char A[] = "token1, token2, 4(%%cat,%%hair,123),MEOW!";
	//string_list *sl = string_tokenizer(A, " \t\v\n", ",:#()");
	//print_string_list(sl);

	return 0;
}

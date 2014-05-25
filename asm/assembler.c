#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "assembler.h"

static void print_help();

int main(int argc, char **argv) {
	char *input = NULL, *output = NULL;
	int c;

	opterr = 0;

	while ((c = getopt(argc, argv, "ho:")) != -1)
		switch (c) {
		case 'h':
			print_help();
			return EXIT_SUCCESS;
		case 'o':
			output = optarg;
			break;
		case '?':
			if (optopt == 'c')
				fprintf(stderr, "Option '-%c' requires an argument.\n", optopt);
			else if (isprint (optopt))
				fprintf(stderr, "Unknown option '-%c'.\n", optopt);
			else
				fprintf(stderr, "Unknown option character '\\x%x'.\n", optopt);
			return EXIT_FAILURE;
		default:
			abort();
		}

	if (optind < argc)
		input = argv[optind];
	if (++optind < argc) {
		fprintf(stderr, "Too many arguments.\n");
		return EXIT_FAILURE;
	}

	FILE *inp, *out;

	if (input) {
		if (!(inp = fopen(input, "r"))) {
			fprintf(stderr, "Can't open for reading: %s.\n", input);
			return EXIT_FAILURE;
		}
	} else
		inp = stdin;

	if (output) {
		if (!(out = fopen(output, "wb"))) {
			fprintf(stderr, "Can't open for writing: %s\n", output);
			return EXIT_FAILURE;
		}
	} else
		out = stdout;

	token_list *tl;
	lexical_analyzer(inp, &tl);

	if (inp != stdin)
		fclose(inp);

	syntax_analyzer(tl);

	line *lines;
	semantic_analyzer(tl, &lines);

	code_generator(out, lines);

	if (out != stdout)
		fclose(out);

	return EXIT_SUCCESS;
}

static void print_help() {
	printf("Usage: asm [-h] [-o OUTPUT] [INPUT]\n");
	printf("Translate INPUT to OUTPUT.\n");
	printf("INPUT is stdin if it is not specified.\n");
	printf("OUTPUT is stdout if it is not specified.\n");
}

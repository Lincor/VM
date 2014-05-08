default:
	gcc assembler.c lexical_analyzer.c syntax_analyzer.c semantic_analyzer.c code_generator.c commands_table.c str_utils.c asm_error.c -o asm

r:
	make clean
	make

debug:
	gcc -g assembler.c lexical_analyzer.c syntax_analyzer.c semantic_analyzer.c code_generator.c commands_table.c str_utils.c asm_error.c -o asm

test:
	gcc -g test.c lexical_analyzer.c syntax_analyzer.c semantic_analyzer.c code_generator.c commands_table.c str_utils.c asm_error.c -o test

rtest:
	make clean
	make test

clean:
	rm -rf test asm a.out *.o *~ *.bak core

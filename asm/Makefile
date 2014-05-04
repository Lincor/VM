test:
	gcc -g test.c lexical_analyzer.c syntax_analyzer.c semantic_analyzer.c commands_table.c str_utils.c asm_error.c -o test

rtest:
	make clean
	make test

clean:
	rm -rf test main a.out *.o *~ *.bak core

all:
	dos2unix bf_template.c || true
	ld -r -b binary -o res.o bf_template.c
	gcc -o bf main.c brainfuck.c bf_compile.c res.o

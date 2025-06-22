all:
	dos2unix bf_template.c || sed -i 's/\r//g' bf_template.c
	dos2unix bf_template.s || sed -i 's/\r//g' bf_template.s
	ld -r -b binary -o res.o bf_template.c bf_template.s
	gcc -o bf main.c brainfuck.c bf_compile.c res.o

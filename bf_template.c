#include <stdio.h>
#include <stdlib.h>

long long ptr;
char *a_mem;
unsigned long long a_mem_size;
char *b_mem;
unsigned long long b_mem_size;

char tmp_c;
int tmp_n;

int init()
{
	ptr = 0;
	a_mem_size = 8;
	a_mem = (char *) calloc(a_mem_size, sizeof(char));
	if (a_mem == NULL) {
		fprintf(stderr, "[bf kernel] create a_memory failed!\n");
		a_mem_size = 0;
		return -1;
	}
	b_mem_size = 8;
	b_mem = (char *) calloc(b_mem_size, sizeof(char));
	if (b_mem == NULL) {
		fprintf(stderr, "[bf kernel] create b_memory failed!\n");
		b_mem_size = 0;
		free(a_mem);
		a_mem_size = 0;
		return -1;
	}

	return 0;
}

void destroy()
{
	free(a_mem);
	a_mem_size = 0;
	free(b_mem);
	b_mem_size = 0;
}

int check_expand()
{
	unsigned long long new_size;
	char *mem_tmp;
	unsigned long long i;
	if (ptr >= 0) {
		if (ptr < a_mem_size) {
			return 0;
		}
		new_size = a_mem_size * 2;
		while (new_size <= ptr) {
			new_size *= 2;
		}
		mem_tmp = (char *) realloc(a_mem, new_size * sizeof(char));
		if (mem_tmp == NULL) {
			fprintf(stderr, "[bf kernel] expand memory failed!\n");
			return -1;
		}
		a_mem = mem_tmp;
		for (i = a_mem_size; i < new_size; ++i) {
			a_mem[i] = 0;
		}
		a_mem_size = new_size;
	}
	else {
		if (-ptr <= b_mem_size) {
			return 0;
		}
		new_size = b_mem_size * 2;
		while (new_size < -ptr) {
			new_size *= 2;
		}
		mem_tmp = (char *) realloc(b_mem, new_size * sizeof(char));
		if (mem_tmp == NULL) {
			fprintf(stderr, "[bf kernel] expand memory failed!\n");
			return -1;
		}
		b_mem = mem_tmp;
		for (i = b_mem_size; i < new_size; ++i) {
			b_mem[i] = 0;
		}
		b_mem_size = new_size;
	}
	return 0;
}

char *get_ptr_mem()
{
	check_expand();
	if (ptr >= 0) {
		return a_mem + ptr;
	}
	else {
		return b_mem + (-ptr - 1);
	}
}

// +
#define BF_ADD			\
	++(*(get_ptr_mem()));

// -
#define BF_SUB			\
	--(*(get_ptr_mem()));

// >
#define BF_MOVE			\
	++ptr;

// <
#define BF_BACK			\
	--ptr;

// ,
#define BF_INPUT					\
	tmp_n = scanf("%c", &tmp_c);		\
	if (tmp_n == 1) {				\
		*(get_ptr_mem()) = tmp_c;	\
	}						\
	else {					\
		*(get_ptr_mem()) = 0;		\
	}

// .
#define BF_OUTPUT					\
	printf("%c", *(get_ptr_mem()));

// [
#define BF_LOOP					\
	while (*(get_ptr_mem())) {

// ]
#define BF_ENDLOOP				\
	}

int main()
{
	if (init() != 0) {
		fprintf(stderr, "[bf kernel] init failed!\n");
		return 1;
	}

// here insert your code

	destroy();

	return 0;
}

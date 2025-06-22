#include "bf_compile.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32) && !defined(_WIN64)
	#define _binary_bf_template_c_start	binary_bf_template_c_start
	#define _binary_bf_template_c_end	binary_bf_template_c_end
#endif

extern char _binary_bf_template_c_start[];
extern char _binary_bf_template_c_end[];

static const char *bf_code_insert_label = "// here insert your code\n";

int bf_tr_start(FILE *tmp_file)
{
	char *p, *p1;
	int label_len;
	int i;

	if (tmp_file == NULL) {
		fprintf(stderr, "need tmp_file\n");
		return -1;
	}

	label_len = strlen(bf_code_insert_label);

	for (p = _binary_bf_template_c_start; p < _binary_bf_template_c_end - label_len; ++p) {
		for (i = 0; i < label_len; ++i) {
			if (*(p + i) != bf_code_insert_label[i]) {
				break;
			}
		}
		if (i == label_len) {
			break;
		}
	}
	if (i < label_len) {
		fprintf(stderr, "bf template cannot find insert label!\n");
		return -1;
	}

	for (p1 = _binary_bf_template_c_start; p1 < p; ++p1) {
		if (fwrite(p1, 1, 1, tmp_file) != 1) {
			fprintf(stderr, "bf write template head failed!\n");
			return -1;
		}
	}

	return 0;
}

int bf_tr_add(FILE *tmp_file, char bf_code)
{
	int n;

	if (tmp_file == NULL) {
		fprintf(stderr, "need tmp_file\n");
		return -1;
	}

	switch (bf_code) {
	case '+':
		n = fprintf(tmp_file, "BF_ADD\n");
		break;
	case '-':
		n = fprintf(tmp_file, "BF_SUB\n");
		break;
	case '>':
		n = fprintf(tmp_file, "BF_MOVE\n");
		break;
	case '<':
		n = fprintf(tmp_file, "BF_BACK\n");
		break;
	case ',':
		n = fprintf(tmp_file, "BF_INPUT\n");
		break;
	case '.':
		n = fprintf(tmp_file, "BF_OUTPUT\n");
		break;
	case '[':
		n = fprintf(tmp_file, "BF_LOOP\n");
		break;
	case ']':
		n = fprintf(tmp_file, "BF_ENDLOOP\n");
		break;
	}

	if (n < 0) {
		fprintf(stderr, "bf write template body failed!\n");
		return -1;
	}

	return 0;
}

int bf_tr_end(FILE *tmp_file)
{
	char *p;
	int label_len;
	int i;

	if (tmp_file == NULL) {
		fprintf(stderr, "need tmp_file\n");
		return -1;
	}

	label_len = strlen(bf_code_insert_label);

	for (p = _binary_bf_template_c_start; p < _binary_bf_template_c_end - label_len; ++p) {
		for (i = 0; i < label_len; ++i) {
			if (*(p + i) != bf_code_insert_label[i]) {
				break;
			}
		}
		if (i == label_len) {
			break;
		}
	}
	if (i < label_len) {
		fprintf(stderr, "bf template cannot find insert label!\n");
		return -1;
	}

	for (p += label_len; p < _binary_bf_template_c_end; ++p) {
		if (fwrite(p, 1, 1, tmp_file) != 1) {
			fprintf(stderr, "bf write template tail failed!\n");
			return -1;
		}
	}

	return 0;
}

int c_compile(const char *c_file, const char *out_file)
{
	char compile_cmd[1024];
	sprintf(compile_cmd, "gcc -o %s %s", out_file, c_file);
	return system(compile_cmd);
}

#if defined(__x86_64__) || defined(_M_X64)
extern char _binary_bf_template_s_start[];
extern char _binary_bf_template_s_end[];

static const char *asm_bf_code_insert_label = "# here insert your code\n";

#define BF_LOOP_STACK_STEP 1024

static int cur_bf_loop_num = 0;
static int *bf_loop_stack = NULL;
static int bf_loop_stack_size = 0;
static int bf_loop_stack_idx = 0;

static int bf_asm_tr_init()
{
	cur_bf_loop_num = 0;
	bf_loop_stack = (int *) malloc(BF_LOOP_STACK_STEP * sizeof(int));
	if (bf_loop_stack == NULL) {
		return -1;
	}
	bf_loop_stack_size = BF_LOOP_STACK_STEP;
	bf_loop_stack_idx = 0;
	return 0;
}

static int bf_asm_tr_destroy()
{
	cur_bf_loop_num = 0;
	free(bf_loop_stack);
	bf_loop_stack_size = 0;
	bf_loop_stack_idx = 0;
}

static int bf_loop_stack_push()
{
	int *tmp_addr = NULL;

	if (bf_loop_stack_idx >= bf_loop_stack_size) {
		tmp_addr = (int *) realloc(bf_loop_stack, bf_loop_stack_size * 2 * sizeof(int));
		if (tmp_addr == NULL) {
			fprintf(stderr, "realloc compile stack failed!\n");
			return -1;
		}
		bf_loop_stack = tmp_addr;
	}

	return bf_loop_stack[bf_loop_stack_idx++] = cur_bf_loop_num++;
}

static int bf_loop_stack_pop()
{
	if (bf_loop_stack_idx <= 0) {
		fprintf(stderr, "syntax error, unexpected ']' symbol\n");
		return -1;
	}

	return bf_loop_stack[--bf_loop_stack_idx];
}

static int bf_loop_stack_top()
{
	return bf_loop_stack[bf_loop_stack_idx];
}

int bf_asm_tr_start(FILE *tmp_file)
{
	char *p, *p1;
	int label_len;
	int i;

	if (tmp_file == NULL) {
		fprintf(stderr, "need tmp_file\n");
		return -1;
	}

	if (bf_asm_tr_init() < 0) {
		fprintf(stderr, "init compile stack failed!\n");
		return -1;
	}

	label_len = strlen(asm_bf_code_insert_label);

	for (p = _binary_bf_template_s_start; p < _binary_bf_template_s_end - label_len; ++p) {
		for (i = 0; i < label_len; ++i) {
			if (*(p + i) != asm_bf_code_insert_label[i]) {
				break;
			}
		}
		if (i == label_len) {
			break;
		}
	}
	if (i < label_len) {
		fprintf(stderr, "bf template cannot find insert label!\n");
		return -1;
	}

	for (p1 = _binary_bf_template_s_start; p1 < p; ++p1) {
		if (fwrite(p1, 1, 1, tmp_file) != 1) {
			fprintf(stderr, "bf write template head failed!\n");
			return -1;
		}
	}

	return 0;
}

int bf_asm_tr_add_loop_in(FILE *tmp_file)
{
	// aka: int peer_bf_loop_num = (bf_loop_stack[bf_loop_stack_idx++] = cur_bf_loop_num++);
	int peer_bf_loop_num = bf_loop_stack_push();
	int n;

	if (peer_bf_loop_num < 0) {
		fprintf(stderr, "push compile stack failed!\n");
		return -1;
	}

	n = fprintf(tmp_file,	".loop_%d_in:\n"
					"mov mem_addr, %%r12\n"
					"mov mem_ptr, %%r13\n"
					"cmpb $0, (%%r12, %%r13, 1)\n"
					"je .loop_%d_out\n",
					peer_bf_loop_num, peer_bf_loop_num);

	if (n < 0) {
		return -1;
	}

	return 0;
}

int bf_asm_tr_add_loop_out(FILE *tmp_file)
{
	// aka: int peer_bf_loop_num = bf_loop_stack[--bf_loop_stack_idx];
	int peer_bf_loop_num = bf_loop_stack_pop();
	int n;

	if (peer_bf_loop_num < 0) {
		fprintf(stderr, "pop compile stack failed!\n");
		return -1;
	}

	n = fprintf(tmp_file,	"jmp .loop_%d_in\n"
					".loop_%d_out:\n",
					peer_bf_loop_num, peer_bf_loop_num);

	if (n < 0) {
		return -1;
	}

	return 0;
}

int bf_asm_tr_add(FILE *tmp_file, char bf_code)
{
	int n;

	if (tmp_file == NULL) {
		fprintf(stderr, "need tmp_file\n");
		return -1;
	}

	switch (bf_code) {
	case '+':
		n = fprintf(tmp_file, "call do_bf_add\n");
		break;
	case '-':
		n = fprintf(tmp_file, "call do_bf_sub\n");
		break;
	case '>':
		n = fprintf(tmp_file, "call do_bf_move\n");
		break;
	case '<':
		n = fprintf(tmp_file, "call do_bf_back\n");
		break;
	case ',':
		n = fprintf(tmp_file, "call do_bf_input\n");
		break;
	case '.':
		n = fprintf(tmp_file, "call do_bf_output\n");
		break;
	case '[':
		n = bf_asm_tr_add_loop_in(tmp_file);
		break;
	case ']':
		n = bf_asm_tr_add_loop_out(tmp_file);
		break;
	}

	if (n < 0) {
		fprintf(stderr, "bf write template body failed!\n");
		return -1;
	}

	return 0;
}

int bf_asm_tr_end(FILE *tmp_file)
{
	char *p;
	int label_len;
	int i;

	if (tmp_file == NULL) {
		fprintf(stderr, "need tmp_file\n");
		return -1;
	}

	bf_asm_tr_destroy();

	label_len = strlen(asm_bf_code_insert_label);

	for (p = _binary_bf_template_s_start; p < _binary_bf_template_s_end - label_len; ++p) {
		for (i = 0; i < label_len; ++i) {
			if (*(p + i) != asm_bf_code_insert_label[i]) {
				break;
			}
		}
		if (i == label_len) {
			break;
		}
	}
	if (i < label_len) {
		fprintf(stderr, "bf template cannot find insert label!\n");
		return -1;
	}

	for (p += label_len; p < _binary_bf_template_s_end; ++p) {
		if (fwrite(p, 1, 1, tmp_file) != 1) {
			fprintf(stderr, "bf write template tail failed!\n");
			return -1;
		}
	}

	return 0;
}

int asm_compile(const char *s_file, const char *tmp_o_file, const char *out_file)
{
	char compile_cmd[1024];
	sprintf(compile_cmd, "as -o %s %s && ld -o %s %s", tmp_o_file, s_file, out_file, tmp_o_file);
	return system(compile_cmd);
}
#endif

#if 0
int main()
{
	FILE *tmp_fp;
	FILE *bf_fp;
	char ch;

	tmp_fp = fopen("test_tr.c", "wb");

	bf_tr_start(tmp_fp);

	bf_fp = fopen("bfcodes/helloworld.bf", "rb");
	while (fscanf(bf_fp, "%c", &ch) > 0) {
		bf_tr_add(tmp_fp, ch);
	}
	fclose(bf_fp);

	bf_tr_end(tmp_fp);
	
	fclose(tmp_fp);

	c_compile("test_tr.c", "test_tr");

	return 0;
}
#endif

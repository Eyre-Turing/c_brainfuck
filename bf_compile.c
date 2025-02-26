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

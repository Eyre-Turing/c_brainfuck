#ifndef __BF_COMPILE_H__
#define __BF_COMPILE_H__

#include <stdio.h>

int bf_tr_start(FILE *tmp_file);
int bf_tr_add(FILE *tmp_file, char bf_code);
int bf_tr_end(FILE *tmp_file);
int c_compile(const char *c_file, const char *out_file);

#if defined(__x86_64__) || defined(_M_X64)
int bf_asm_tr_start(FILE *tmp_file);
int bf_asm_tr_add(FILE *tmp_file, char bf_code);
int bf_asm_tr_end(FILE *tmp_file);
int asm_compile(const char *s_file, const char *tmp_o_file, const char *out_file);
#else
#define bf_asm_tr_start(...)	(-1)
#define bf_asm_tr_add(...)	(-1)
#define bf_asm_tr_end(...)	(-1)
#define asm_compile(...)		(-1)
#endif

#endif

#ifndef __BF_COMPILE_H__
#define __BF_COMPILE_H__

#include <stdio.h>

int bf_tr_start(FILE *tmp_file);
int bf_tr_add(FILE *tmp_file, char bf_code);
int bf_tr_end(FILE *tmp_file);
int c_compile(const char *c_file, const char *out_file);

#endif

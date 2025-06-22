#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "brainfuck.h"
#include "bf_compile.h"

#ifdef _WIN32
#include <io.h>
#define WEXITSTATUS(status) status
#else
#include <sys/types.h>
#include <sys/wait.h>
#endif

void usage(char *self)
{
	printf("Usage:\n"
			"  %s <brainfuck-code-file> [-o <out>]\n"
			"  %s -c <brainfuck-code-text> [-o <out>]\n"
			"  %s [-o <out>]\n"
			"\n"
			"  A brainfuck code runner.\n"
			"Param:\n"
			"  -h | --help | -?          show this message and exit.\n"
			"  -c <brainfuck-code-text>  exec a brainfuck code as command.\n"
			"  <brainfuck-code-file>     run a brainfuck code file.\n"
			"  -o <out>                  compile into an executable file.\n"
			#if defined(__x86_64__) || defined(_M_X64)
			"  -S | --asm                use assemble code to compile (x86_64 Linux only)\n"
			#endif
			, self, self, self);
}

int main(int argc, char *argv[])
{
	int i;
	char code_filename[260] = "";
	char *code_text = NULL;
	int code_text_len = 0;
	FILE *fp = stdin;
	char b;
	brainfuck_runner *runner = NULL;
	int execflag = 0;
	int ret = 0;

	char *compile_out = NULL;
	int compile_flag = 0;
	int asm_flag = 0;
	char tmp_c_file[260] = "tmp_XXXXXX.c";
	char tmp_s_file[260] = "tmp_XXXXXX.s";
	char tmp_o_file[260] = "tmp_XXXXXX.o";
	FILE *tmp_c_fp, *tmp_s_fp;

	for (i = 1; i < argc; ++i) {
		if (execflag) {
			execflag = 0;
			code_text = argv[i];
			continue;
		}
		if (compile_flag) {
			compile_flag = 0;
			compile_out = argv[i];
			continue;
		}
		if (argv[i][0] != '-') {
			strcpy(code_filename, argv[i]);
			continue;
		}
		if (strcmp(argv[i], "-c") == 0) {
			execflag = 1;
		} else if (strcmp(argv[i], "-o") == 0) {
			compile_flag = 1;
		} else if (strcmp(argv[i], "-S") == 0 || strcmp(argv[i], "--asm") == 0) {
			#if defined(__x86_64__) || defined(_M_X64)
			asm_flag = 1;
			#endif
		} else if (strcmp(argv[i], "-h") == 0 || 
			strcmp(argv[i], "--help") == 0 || 
			strcmp(argv[i], "-?") == 0 ) {
			usage(argv[0]);
			return 0;
		}
	}
	if (execflag) {
		fprintf(stderr, "-c need a code text!\n");
		return 1;
	}
	if (compile_flag) {
		fprintf(stderr, "-o need specify out filename!\n");
		return 1;
	}
	if (code_text) {
		// exec code_text
		if (compile_out == NULL) {
			return brainfuck_exec(code_text);
		}
	}
	if (strcmp(code_filename, "")) {
		// read from file
		fp = fopen(code_filename, "rb");
		if (!fp) {
			fprintf(stderr, "open codefile(%s) failed!\n", code_filename);
			return 1;
		}
	}

	if (compile_out) {
		if (asm_flag) {
			#if defined(__x86_64__) || defined(_M_X64)

			mkstemps(tmp_s_file, 2);
			mkstemps(tmp_o_file, 2);

			tmp_s_fp = fopen(tmp_s_file, "wb");

			if (bf_asm_tr_start(tmp_s_fp) != 0) {
				fprintf(stderr, "bf_asm_tr_start failed!\n");
				fclose(tmp_s_fp);
				remove(tmp_s_file);
				remove(tmp_o_file);
				return 1;
			}

			if (code_text) {
				code_text_len = strlen(code_text);
				for (i = 0; i < code_text_len; ++i) {
					if (bf_asm_tr_add(tmp_s_fp, code_text[i]) != 0) {
						fprintf(stderr, "bf_asm_tr_add failed!\n");
						fclose(tmp_s_fp);
						remove(tmp_s_file);
						remove(tmp_o_file);
						return 1;
					}
				}
			}
			else {
				while (~fscanf(fp, "%c", &b)) {
					if (bf_asm_tr_add(tmp_s_fp, b) != 0) {
						fprintf(stderr, "bf_asm_tr_add failed!\n");
						fclose(tmp_s_fp);
						remove(tmp_s_file);
						remove(tmp_o_file);
						if (fp != stdin) {
							fclose(fp);
						}
						return 1;
					}
				}
			}
			if (bf_asm_tr_end(tmp_s_fp) != 0) {
				fprintf(stderr, "bf_asm_tr_end failed!\n");
				fclose(tmp_s_fp);
				remove(tmp_s_file);
				remove(tmp_o_file);
				return 1;
			}

			fclose(tmp_s_fp);

			ret = asm_compile(tmp_s_file, tmp_o_file, compile_out);
			if (ret != 0) {
				fprintf(stderr, "compile failed!\n");
			}

			remove(tmp_s_file);
			remove(tmp_o_file);

			return WEXITSTATUS(ret);

			#endif
		}
		else {
			#ifdef _WIN32
			strcpy(tmp_c_file, "tmp_XXXXXX");
			_mktemp(tmp_c_file);
			strcat(tmp_c_file, ".c");
			#else
			mkstemps(tmp_c_file, 2);
			#endif

			tmp_c_fp = fopen(tmp_c_file, "wb");

			if (bf_tr_start(tmp_c_fp) != 0) {
				fprintf(stderr, "bf_tr_start failed!\n");
				fclose(tmp_c_fp);
				remove(tmp_c_file);
				return 1;
			}
			if (code_text) {
				code_text_len = strlen(code_text);
				for (i = 0; i < code_text_len; ++i) {
					if (bf_tr_add(tmp_c_fp, code_text[i]) != 0) {
						fprintf(stderr, "bf_tr_add failed!\n");
						fclose(tmp_c_fp);
						remove(tmp_c_file);
						return 1;
					}
				}
			}
			else {
				while (~fscanf(fp, "%c", &b)) {
					if (bf_tr_add(tmp_c_fp, b) != 0) {
						fprintf(stderr, "bf_tr_add failed!\n");
						fclose(tmp_c_fp);
						remove(tmp_c_file);
						if (fp != stdin) {
							fclose(fp);
						}
						return 1;
					}
				}
			}
			if (bf_tr_end(tmp_c_fp) != 0) {
				fprintf(stderr, "bf_tr_end failed!\n");
				fclose(tmp_c_fp);
				remove(tmp_c_file);
				return 1;
			}

			fclose(tmp_c_fp);

			ret = c_compile(tmp_c_file, compile_out);
			if (ret != 0) {
				fprintf(stderr, "compile failed!\n");
			}

			remove(tmp_c_file);

			return WEXITSTATUS(ret);
		}
	}

	runner = brainfuck_new();
	if (!runner) {
		fprintf(stderr, "create brainfuck runner failed!\n");
		if (fp != stdin) {
			fclose(fp);
		}
		return 1;
	}
	while(~fscanf(fp ,"%c", &b)) {
		if (ret = runner->add(runner, b)) {	// run this part failed
			break;
		}
	}
	if (fp != stdin) {
		fclose(fp);
	}
	if (ret == 0) {
		ret = runner->quit(runner);
	}
	runner->destroy(runner);
	return ret;
}


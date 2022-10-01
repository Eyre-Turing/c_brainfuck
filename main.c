#include <stdio.h>
#include <string.h>
#include "brainfuck.h"

void usage()
{
	printf("Usage:\n"
			"  A brainfuck code runner.\n"
			"Param:\n"
			"  -h | --help | -?          show this message and exit.\n"
			"  -c <brainfuck-code-text>  exec a brainfuck code as command.\n"
			"  <brainfuck-code-file>     run a brainfuck code file.\n");
}

int main(int argc, char *argv[])
{
	int i;
	char code_filename[260] = "";
	char *code_text = NULL;
	FILE *fp = stdin;
	char b;
	brainfuck_runner *runner = NULL;
	int ret = 0;
	for (i = 1; i < argc; ++i) {
		if (argv[i][0] != '-') {
			strcpy(code_filename, argv[i]);
			continue;
		}
		if (strcmp(argv[i], "-c") == 0) {
			if (++i >= argc) {
				fprintf(stderr, "-c need a code text!\n");
				return 1;
			}
			code_text = argv[i];
		} else if (strcmp(argv[i], "-h") == 0 || 
			strcmp(argv[i], "--help") == 0 || 
			strcmp(argv[i], "-?") == 0 ) {
			usage();
			return 0;
		}
	}
	if (code_text) {
		// exec code_text
		return brainfuck_exec(code_text);
	}
	if (strcmp(code_filename, "")) {
		// read from file
		fp = fopen(code_filename, "rb");
		if (!fp) {
			fprintf(stderr, "open codefile(%s) failed!\n", code_filename);
			return 1;
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


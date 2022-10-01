#ifndef __BRAINFUCK_H__
#define __BRAINFUCK_H__

int brainfuck_exec(const char *code);	// exec a brainfuck code

typedef struct brainfuck_context *context;

typedef struct brainfuck_runner {
	context ctx;
	int (*add)(struct brainfuck_runner *runner, char c);	// run a part
	void (*destroy)(struct brainfuck_runner *runner);
	int (*quit)(struct brainfuck_runner *runner);	// get exit code, 0 is succeed, otherwise failed
} brainfuck_runner;

struct brainfuck_runner *brainfuck_new();

#endif


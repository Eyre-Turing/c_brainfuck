#include "brainfuck.h"
#include <stdio.h>
#include <stdlib.h>

struct brainfuck_context {
	char *a_memory;		// from 0 to +Inf
	unsigned long long a_mem_used;
	unsigned long long a_mem_size;
	char *b_memory;		// from -Inr to -1
	unsigned long long b_mem_used;
	unsigned long long b_mem_size;
	long long ptr;		// current pointer position
	
	char *code;
	unsigned long long code_used;
	unsigned long long code_size;
	long long end_depth;	// like [] or [][] is 1, [[]] is 2, while add code will use this
	unsigned long long code_pos;	// current code position to run

	unsigned long long *call_stack;	// [ position
	unsigned long long call_used;
	unsigned long long call_size;
};

static void brainfuck_delete(struct brainfuck_runner *runner)
{
	free(runner->ctx->a_memory);
	free(runner->ctx->b_memory);
	free(runner->ctx->code);
	free(runner->ctx->call_stack);
	
	free(runner->ctx);
	
	free(runner);
}

static int code_add(struct brainfuck_context *ctx, char c)
{
	char *code = NULL;
	if (ctx->code_size - ctx->code_used < 1) {
		code = (char *) realloc(ctx->code, (ctx->code_size + 1) * 2);
		if (!code) {
			fprintf(stderr, "[bf kernel] expand code size failed!\n");
			return 1;
		}
		ctx->code = code;
		ctx->code_size = (ctx->code_size + 1) * 2;
	}
	// ctx->code_pos = ctx->code_used;	// don't set code_pos here, because if c is [ need read until peer ]
	ctx->code[(ctx->code_used)++] = c;
	return 0;
}

static int find_peer_end(struct brainfuck_context *ctx, unsigned long long *end)
{
	unsigned long long pos;
	unsigned long long end_depth = 0;
	for (pos = ctx->code_pos; pos < ctx->code_used; ++pos) {
		if (ctx->code[pos] == '[') {
			++end_depth;
			continue;
		} else if (ctx->code[pos] != ']') {
			continue;
		}
		if (--end_depth == 0) {
			*end = pos;
			return 0;
		}
	}
	
	fprintf(stderr, "[bf kernel] find peer end failed!\n");
	return 1;
}

static int call_stack_add(struct brainfuck_context *ctx, unsigned long long pos)
{
	unsigned long long *stack_tmp = NULL;
	if (ctx->call_size - ctx->call_used < 1) {
		stack_tmp = (unsigned long long *)
			realloc(ctx->call_stack, 
			(ctx->call_size + 1) * 2 * sizeof(unsigned long long));
		if (!stack_tmp) {
			fprintf(stderr, "[bf kernel] expand call_stack failed!\n");
			return 1;
		}
		ctx->call_stack = stack_tmp;
		ctx->call_size = (ctx->call_size + 1) * 2;
	}
	ctx->call_stack[(ctx->call_used)++] = pos;
	return 0;
}

static int memory_expand(char **memory, unsigned long long *size, unsigned long long ptr)
{
	char *mem_tmp = NULL;
	if (ptr < *size) {
		return 0;
	}
	mem_tmp = (char *) realloc(*memory, (*size + 1) * 2);
	if (!mem_tmp) {
		fprintf(stderr, "[bf kernel] expand memory failed!\n");
		return 1;
	}
	*memory = mem_tmp;
	*size = (*size + 1) * 2;
	return 0;
}

static int code_run_tail(struct brainfuck_context *ctx)
{
	char ptrv;
	unsigned long long peer_end_tmp;
	int ret = 0;
	switch(ctx->code[ctx->code_pos]) {
	case '>':
		++(ctx->ptr);
		if (ctx->ptr < 0) {
			break;
		}
		if (memory_expand(&(ctx->a_memory), &(ctx->a_mem_size), ctx->ptr)) {
			fprintf(stderr, "[bf kernel] expand a_memory failed!\n");
			return 1;
		}
		if (ctx->ptr >= ctx->a_mem_used) {
			ctx->a_mem_used = ctx->ptr + 1;
			ctx->a_memory[ctx->ptr] = 0;
		}
		break;
	case '<':
		--(ctx->ptr);
		if (ctx->ptr >= 0) {
			break;
		}
		if (memory_expand(&(ctx->b_memory), &(ctx->b_mem_size), -(ctx->ptr) - 1)) {
			fprintf(stderr, "[bf kernel] expand b_memory failed!\n");
			return 1;
		}
		if (-(ctx->ptr) > ctx->b_mem_used) {
			ctx->b_mem_used = -(ctx->ptr);
			ctx->b_memory[-(ctx->ptr) - 1] = 0;
		}
		break;
	case '+':
		if (ctx->ptr < 0) {
			++ctx->b_memory[-(ctx->ptr) - 1];
		} else {
			++ctx->a_memory[ctx->ptr];
		}
		break;
	case '-':
		if (ctx->ptr < 0) {
			--ctx->b_memory[-(ctx->ptr) - 1];
		} else {
			--ctx->a_memory[ctx->ptr];
		}
		break;
	case ',':
		if (ctx->ptr < 0) {
			// ctx->b_memory[-(ctx->ptr) - 1] = getchar();
			ret = scanf("%c", ctx->b_memory + (-(ctx->ptr) - 1));
			(ret == -1) && (ctx->b_memory[-(ctx->ptr) - 1] = 0);
			ret = 0;
		} else {
			// ctx->a_memory[ctx->ptr] = getchar();
			ret = scanf("%c", ctx->a_memory + ctx->ptr);
			(ret == -1) && (ctx->a_memory[ctx->ptr] = 0);
			ret = 0;
		}
		break;
	case '.':
		if (ctx->ptr < 0) {
			printf("%c", ctx->b_memory[-(ctx->ptr) - 1]);
		} else {
			printf("%c", ctx->a_memory[ctx->ptr]);
		}
		break;
	case '[':
		// push to call_stack
		call_stack_add(ctx, ctx->code_pos);

		// check condition
		if (ctx->ptr < 0) {
			ptrv = ctx->b_memory[-(ctx->ptr) - 1];
		} else {
			ptrv = ctx->a_memory[ctx->ptr];
		}
		
		if (ptrv) {
			break;
		}
		// move to peer ']' position,
		if (ret = find_peer_end(ctx, &peer_end_tmp)) {
			return ret;
		}
		ctx->code_pos = peer_end_tmp;
		// pop call_stack
		--(ctx->call_used);
		break;
	case ']':
		if (ctx->call_used == 0) {
			fprintf(stderr, "[bf kernel] single ], [] used error, no [ found!\n");
			return 1;
		}
		// move to [, and pop call_stack
		ctx->code_pos = ctx->call_stack[--(ctx->call_used)] - 1;
		break;
	}
	++(ctx->code_pos);
	return ret;
}

static int brainfuck_add(struct brainfuck_runner *runner, char c)
{
	int ret = 0;
	
	if (ret = code_add(runner->ctx, c)) {
		return ret;
	}
	if (c == '[') {
		++(runner->ctx->end_depth);
	} else if (c == ']') {
		--(runner->ctx->end_depth);
	}
	if (runner->ctx->end_depth < 0) {
		fprintf(stderr, "[bf kernel] add code found single ], it mast had a [ ahead ]!\n");
		return 1;
	}
	if (runner->ctx->end_depth) {	// scan loop code, do not run until get peer ]
		return 0;
	}
	while (runner->ctx->code_pos < runner->ctx->code_used) {	// run to the end
		if (ret = code_run_tail(runner->ctx)) {
			return ret;
		}
	}
	
	return ret;
}

static int brainfuck_quit(struct brainfuck_runner *runner)
{
	return (runner->ctx->end_depth != 0);
}

struct brainfuck_runner *brainfuck_new()
{
	struct brainfuck_runner *runner = (struct brainfuck_runner *)
		malloc(sizeof(struct brainfuck_runner));
	if (!runner) {
		fprintf(stderr, "[bf kernel] create brainfuck runner failed!\n");
		return NULL;
	}
	runner->add = brainfuck_add;
	runner->destroy = brainfuck_delete;
	runner->quit = brainfuck_quit;
	
	struct brainfuck_context *ctx = (struct brainfuck_context *)
		malloc(sizeof(struct brainfuck_context));
	if (!ctx) {
		fprintf(stderr, "[bf kernel] create brainfuck context failed!\n");
		free(runner);
		return NULL;
	}
	
	ctx->a_memory = (char *) malloc(1);
	if (!(ctx->a_memory)) {
		fprintf(stderr, "[bf kernel] create a_memory failed!\n");
		free(runner);
		return NULL;
	}
	ctx->a_memory[0] = 0;
	ctx->a_mem_used = 1;
	ctx->a_mem_size = 1; 
	ctx->b_memory = NULL;
	ctx->b_mem_used = 0;
	ctx->b_mem_size = 0;
	ctx->ptr = 0;
	
	ctx->code = NULL;
	ctx->code_used = 0;
	ctx->code_size = 0;
	ctx->end_depth = 0;
	ctx->code_pos = 0;

	ctx->call_stack = NULL;
	ctx->call_used = 0;
	ctx->call_size = 0;
	
	runner->ctx = ctx;
	
	return runner;
}

int brainfuck_exec(const char *code)
{
	const char *p;
	struct brainfuck_runner *runner = brainfuck_new();
	int ret = 0;
	if (!runner) {
		return 1;
	}
	for (p = code; *p; ++p) {
		if (ret = runner->add(runner, *p)) {
			break;
		}
	}
	if (ret == 0) {
		ret = runner->quit(runner);
	}
	runner->destroy(runner);
	return ret;
}


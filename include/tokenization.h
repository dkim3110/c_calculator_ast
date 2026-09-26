#ifndef TOKENIZATION_H_
#define TOKENIZATION_H_

#include "mem_arena.h"
#include "str_view.h"
#include <stdint.h>

typedef enum {
	UNKNOWN,
	NUMBER,
	CONSTANT,
	FUNCTION,
	ADD,
	MULT,
	EXP,
	FACTORIAL,
	EQUALS,
	LPAREN,
	RPAREN,
	END_TERMINATOR
} token_type;

typedef enum {
	UNKNOWN_FUNC,
	SQRT,
	SIN,
	COS,
	TAN,
	SEC,
	CSC,
	COT,
	ASIN,
	ACOS,
	ATAN,
	SINH,
	COSH,
	TANH,
	ABS,
	LOG_E,
	LOG_10
} function_type;

typedef struct {
	char op;
	double num_val;
	uint32_t func;
	token_type type;
} token_t;

#define NULL_TOKEN                                                                                                     \
	((token_t){                                                                                                          \
		.op = '\0',                                                                                                        \
		.num_val = NAN,                                                                                                    \
		.func = UNKNOWN_FUNC,                                                                                              \
		.type = UNKNOWN,                                                                                                   \
	})
#define END_TOKEN ((token_t){.op = '\0', .num_val = NAN, .func = UNKNOWN_FUNC, .type = END_TERMINATOR})

extern token_t *tokenize(strv source, mem_arena *arena, size_t *tokens_index);

static inline bool token_equals(token_t a, token_t b) {
	return (a.type == b.type);
}

#endif // TOKENIZATION_H_

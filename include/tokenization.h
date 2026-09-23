#ifndef TOKENIZATION_H_
#define TOKENIZATION_H_

#include "mem_arena.h"
#include "string_view.h"

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

typedef enum { UNKNOWN_FUNC, SQRT, SIN, COS, TAN, ASIN, ACOS, ATAN, ABS, LOG_E, LOG_10 } function_type;

typedef struct {
	char op;
	double num_val;
	function_type func;
	token_type type;
} token_t;

static const token_t NULL_TOKEN = (token_t){
	.op = '\0',
	.num_val = NAN,
	.func = UNKNOWN_FUNC,
	.type = UNKNOWN,
};

static const token_t END_TOKEN = (token_t){
	.op = '\0',
	.num_val = NAN,
	.func = UNKNOWN_FUNC,
	.type = END_TERMINATOR,
};

extern token_t *tokenize(str source, mem_arena *arena);

static inline bool token_equals(token_t a, token_t b) {
	return (a.type == b.type);
}

#endif // TOKENIZATION_H_

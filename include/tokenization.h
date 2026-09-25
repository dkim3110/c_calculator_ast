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

#define HASH_SQRT		(3552570154)
#define HASH_SIN		(2173910231)
#define HASH_COS		(3941271615)
#define HASH_TAN		(2511134412)
#define HASH_ASIN		(3887405372)
#define HASH_ACOS		(3392640901)
#define HASH_ATAN		(2436737454)
#define HASH_ABS		(103368697)
#define HASH_LOG_E	(2401766522)
#define HASH_LOG_10 (1811268606)

typedef struct {
	char op;
	double num_val;
	uint32_t func;
	token_type type;
} token_t;

#define NULL_TOKEN  ((token_t) {.op = '\0', .num_val = NAN, .func = 0, .type = UNKNOWN,})
#define END_TOKEN  ((token_t) {.op = '\0', .num_val = NAN, .func = 0, .type = END_TERMINATOR})

extern token_t *tokenize(strv source, mem_arena *arena, size_t *tokens_index);

static inline bool token_equals(token_t a, token_t b) {
	return (a.type == b.type);
}

#endif // TOKENIZATION_H_

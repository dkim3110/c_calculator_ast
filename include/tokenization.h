#ifndef TOKENIZATION_H_
#define TOKENIZATION_H_

#include "string_view.h"

typedef enum { UNKNOWN, NUMBER, CONSTANT, FUNCTION, ADD, MULT, EXP, EQUALS, LPAREN, RPAREN } token_type;

typedef enum { UNKNOWN_FUNC, SQRT, SIN, COS, TAN } function_type;

typedef struct {
	char op;
	double num_val;
	function_type func;
	token_type type;
} token_t;

typedef token_t dyn_token_t;

static const token_t NULL_TOKEN = (token_t){.type = UNKNOWN, .op = '\0', .func = UNKNOWN_FUNC, .num_val = NAN};

extern dyn_token_t *tokenize(str source);

static inline bool token_equals(token_t a, token_t b) {
	return (a.type == b.type);
}

#endif // TOKENIZATION_H_

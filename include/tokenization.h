#ifndef TOKENIZATION_H_
#define TOKENIZATION_H_

#include "string_view.h"

typedef enum { UNKNOWN, NUMBER, CONSTANT, ADD, MULT, EXP, LPAREN, RPAREN } token_type;

typedef struct {
	token_type type;
	char op;
	double num_val;
} token_t;

typedef token_t dyn_token_t;

static const token_t NULL_TOKEN = (token_t){.type = UNKNOWN, .op = '\0', .num_val = NAN};

extern dyn_token_t *tokenize(str source);

static inline bool token_equals(token_t a, token_t b) {
	return (a.type == b.type);
}

#endif // TOKENIZATION_H_

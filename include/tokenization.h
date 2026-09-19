#ifndef TOKENIZATION_H_
#define TOKENIZATION_H_

#include "string_view.h"

typedef enum { UNKNOWN, NUMBER, ADD, MULT, EXP, OP_PAREN, CL_PAREN } token_type;

typedef struct {
	token_type type;
	str val;
} token_t;

typedef token_t dyn_token_t;

static const token_t NULL_TOKEN = (token_t){.type = UNKNOWN, .val = NULL_STR};

extern dyn_token_t *tokenize(str source);

static inline bool token_equals(token_t a, token_t b) { return (a.type == b.type); }

#endif // TOKENIZATION_H_

#include "tokenization.h"
#include "dynamic_array.h"
#include "string_view.h"

#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static inline char peek(str source, size_t *src_index) {
	size_t foresight = (*src_index);
	if (foresight >= source.len) return '\0';

	return source.data[foresight];
}

static inline char consume(str source, size_t *src_index) {
	if ((*src_index) >= source.len) return '\0';
	return source.data[(*src_index)++];
}

dyn_token_t *tokenize(str source) {
	dyn_token_t *tokens = NULL;
	size_t src_index = 0;

	while (peek(source, &src_index) != '\0') {
		token_t token = NULL_TOKEN;
		char curr = peek(source, &src_index);

		if (isspace(curr)) {
			consume(source, &src_index);
			continue;
		}

		switch (curr) {
			case '+':
				/* fallthrough */
			case '-':
				token.type = ADD;
				token.val = (str){.data = &source.data[src_index], .len = 1};
				consume(source, &src_index);
				break;
			case '*':
				/* fallthrough */
			case '/':
				token.type = MULT;
				token.val = (str){.data = &source.data[src_index], .len = 1};
				consume(source, &src_index);
				break;
			case '^':
				token.type = EXP;
				token.val = (str){.data = &source.data[src_index], .len = 1};
				consume(source, &src_index);
				break;
			case '(':
				token.type = OP_PAREN;
				token.val = (str){.data = &source.data[src_index], .len = 1};
				consume(source, &src_index);
				break;
			case ')':
				token.type = CL_PAREN;
				token.val = (str){.data = &source.data[src_index], .len = 1};
				consume(source, &src_index);
				break;
			default:
				if (isdigit(curr)) {
					size_t start_index = src_index;
					bool has_decimal = false;

					while (isdigit(peek(source, &src_index)) || peek(source, &src_index) == '.') {
						if (peek(source, &src_index) == '.') {
							if (has_decimal) break;
							has_decimal = true;
						}
						consume(source, &src_index);
					}

					token.type = NUMBER;
					token.val = (str){.data = &source.data[start_index], .len = src_index - start_index};
				} else consume(source, &src_index);
				break;
		}

		size_t t_len = arr_len(tokens);
		if (t_len > 0) {
			token_type prev_type = tokens[t_len - 1].type;

			bool implicit_mult = false;
			if ((prev_type == NUMBER) && (token.type == OP_PAREN)) implicit_mult = true;
			if ((prev_type == CL_PAREN) && (token.type == OP_PAREN)) implicit_mult = true;
			if ((prev_type == CL_PAREN) && (token.type == NUMBER)) implicit_mult = true;

			if (implicit_mult) {
				token_t mult_tok = {.type = MULT, .val = (str){.data = "*", .len = 1}};
				arr_push(tokens, mult_tok);
			}
		}

		arr_push(tokens, token);
	}
	return tokens;
}

#include "tokenization.h"
#include "dynamic_array.h"
#include "string_view.h"

#include <ctype.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef M_PHI
#define M_PHI 1.6180339887498948482
#endif // M_PHI

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
				token.op = curr;
				consume(source, &src_index);
				break;
			case '*':
				/* fallthrough */
			case '/':
				token.type = MULT;
				token.op = curr;
				consume(source, &src_index);
				break;
			case '^':
				token.type = EXP;
				token.op = curr;
				consume(source, &src_index);
				break;
			case '(':
				token.type = LPAREN;
				token.op = curr;
				consume(source, &src_index);
				break;
			case ')':
				token.type = RPAREN;
				token.op = curr;
				consume(source, &src_index);
				break;
			case 'e':
				token.type = CONSTANT;
				token.num_val = M_E;
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
					token.num_val = str_to_dbl((str){.data = &source.data[start_index], .len = src_index - start_index});
				} else if (isalpha(curr)) {
					dyn_char *buffer = NULL;

					while (isalpha(peek(source, &src_index))) {
						char curr = peek(source, &src_index);
						if (curr == ' ') break;
						arr_push(buffer, curr);
						consume(source, &src_index);
					}
					arr_push(buffer, '\0');

					str word = str_trim(to_str(buffer));

					if (str_equals(word, (str){.data = "pi", .len = 2})) {
						token.type = CONSTANT;
						token.num_val = M_PI;
					} else if (str_equals(word, (str){.data = "phi", .len = 3})) {
						token.type = CONSTANT;
						token.num_val = M_PHI;
					} else if (str_equals(word, (str){.data = "tau", .len = 3})) {
						token.type = CONSTANT;
						token.num_val = M_PI * 2;
					}

					arr_free(buffer);
				} else consume(source, &src_index);
				break;
		}

		size_t t_len = arr_len(tokens);
		if (t_len > 0) {
			token_type prev_type = tokens[t_len - 1].type;

			bool implicit_mult = false;
			if ((prev_type == NUMBER) && (token.type == LPAREN)) implicit_mult = true;
			if ((prev_type == RPAREN) && (token.type == LPAREN)) implicit_mult = true;
			if ((prev_type == RPAREN) && (token.type == NUMBER)) implicit_mult = true;

			if ((prev_type == NUMBER) && (token.type == CONSTANT)) implicit_mult = true;
			if ((prev_type == CONSTANT) && (token.type == NUMBER)) implicit_mult = true;

			if (implicit_mult) {
				token_t mult_tok = {.type = MULT, .op = '*'};
				arr_push(tokens, mult_tok);
			}
		}

		arr_push(tokens, token);
	}
	return tokens;
}

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

static void handle_implicit_mult(dyn_token_t **tokens, token_t token, size_t t_len) {
	if (t_len > 0) {
		token_type prev_type = (*tokens)[t_len - 1].type;

		bool implicit_mult = false;
		if ((prev_type == NUMBER) && (token.type == LPAREN)) implicit_mult = true;
		if ((prev_type == RPAREN) && (token.type == LPAREN)) implicit_mult = true;
		if ((prev_type == RPAREN) && (token.type == NUMBER)) implicit_mult = true;

		if ((prev_type == CONSTANT) && (token.type == LPAREN)) implicit_mult = true;
		if ((prev_type == RPAREN) && (token.type == CONSTANT)) implicit_mult = true;

		if ((prev_type == NUMBER) && (token.type == CONSTANT)) implicit_mult = true;
		if ((prev_type == CONSTANT) && (token.type == NUMBER)) implicit_mult = true;

		if ((prev_type == NUMBER) && (token.type == FUNCTION)) implicit_mult = true;

		if (implicit_mult) {
			token_t mult_tok = {.type = MULT, .op = '*'};
			arr_push((*tokens), mult_tok);
		}
	}
}

static void handle_functions(str word, token_t *token) {
	if (str_equals(word, (str){.data = "sqrt", .len = 4})) {
		token->type = FUNCTION;
		token->func = SQRT;
	} else if (str_equals(word, (str){.data = "sin", .len = 3})) {
		token->type = FUNCTION;
		token->func = SIN;
	} else if (str_equals(word, (str){.data = "cos", .len = 3})) {
		token->type = FUNCTION;
		token->func = COS;
	} else if (str_equals(word, (str){.data = "tan", .len = 3})) {
		token->type = FUNCTION;
		token->func = TAN;
	} else if (str_equals(word, (str){.data = "arcsin", .len = 6})) {
		token->type = FUNCTION;
		token->func = ASIN;
	} else if (str_equals(word, (str){.data = "arccos", .len = 6})) {
		token->type = FUNCTION;
		token->func = ACOS;
	} else if (str_equals(word, (str){.data = "arctan", .len = 6})) {
		token->type = FUNCTION;
		token->func = ATAN;
	} else if (str_equals(word, (str){.data = "abs", .len = 3})) {
		token->type = FUNCTION;
		token->func = ABS;
	} else if (str_equals(word, (str){.data = "ln", .len = 2})) {
		token->type = FUNCTION;
		token->func = LOG_E;
	} else if (str_equals(word, (str){.data = "log", .len = 3})) {
		token->type = FUNCTION;
		token->func = LOG_10;
	}
}

dyn_token_t *tokenize(str source) {
	dyn_token_t *tokens = NULL;
	size_t src_index = 0;

	while (peek(source, &src_index) != '\0') {
		token_t token = NULL_TOKEN;
		char curr = peek(source, &src_index);
		size_t start_index = src_index;
		consume(source, &src_index);

		if (isspace(curr)) continue;

		switch (curr) {
			// -- operators ------------------------------------------------------------
			case '+':
				/* fallthrough */
			case '-':
				token.type = ADD;
				token.op = curr;
				break;
			case '*':
				/* fallthrough */
			case '/':
				/* fallthrough */
			case '%':
				token.type = MULT;
				token.op = curr;
				break;
			case '^':
				token.type = EXP;
				token.op = curr;
				break;
			case '=':
				token.type = EQUALS;
				token.op = curr;
				break;
			case '(':
				token.type = LPAREN;
				token.op = curr;
				break;
			case ')':
				token.type = RPAREN;
				token.op = curr;
				break;
				// ------------------------------------------------------------ operators --

			case 'e':
				token.type = CONSTANT;
				token.num_val = M_E;
				break;
			default:
				if (isdigit(curr)) {
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
					while (isalpha(peek(source, &src_index))) consume(source, &src_index);
					str word = str_sub(source, start_index, src_index);

					if (peek(source, &src_index) == '(') handle_functions(word, &token);

					if (str_equals(word, (str){.data = "pi", .len = 2})) {
						token.type = CONSTANT;
						token.num_val = M_PI;
					} else if (str_equals(word, (str){.data = "phi", .len = 3})) {
						token.type = CONSTANT;
						token.num_val = M_PHI;
					} else if (str_equals(word, (str){.data = "tau", .len = 3})) {
						token.type = CONSTANT;
						token.num_val = M_PI * 2;
					} else if (str_equals(word, (str){.data = "inf", .len = 3})) {
						token.type = CONSTANT;
						token.num_val = INFINITY;
					}
				}
				break;
		}

		handle_implicit_mult(&tokens, token, arr_len(tokens));
		arr_push(tokens, token);
	}
	return tokens;
}

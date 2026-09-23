#include "tokenization.h"
#include "mem_arena.h"
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

static void handle_implicit_mult(token_t **tokens, token_t token, size_t t_len, size_t *tokens_index) {
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
			(*tokens)[(*tokens_index)++] = mult_tok;
		}
	}
}

static void handle_constants(str word, token_t *token) {
	switch (word.len) {
		case 1:
			switch (word.data[0]) {
				case 'e': // euler's number
					token->type = CONSTANT;
					token->num_val = M_E;
					break;
			}
			break;
		case 2:
			switch (word.data[0]) {
				case 'p':
					if (word.data[1] == 'i') { // pi
						token->type = CONSTANT;
						token->num_val = M_PI;
					}
					break;
				default: break;
			}
			break;
		case 3:
			switch (word.data[0]) {
				case 'p':
					if ((word.data[1] == 'h') && (word.data[2] == 'i')) { // phi
						token->type = CONSTANT;
						token->num_val = M_PHI;
					}
					break;
				case 't':
					if ((word.data[1] == 'a') && (word.data[2] == 'u')) { // tau
						token->type = CONSTANT;
						token->num_val = M_PI * 2;
					}
					break;
				case 'i':
					if ((word.data[1] == 'n') && (word.data[2] == 'f')) { // inf
						token->type = CONSTANT;
						token->num_val = INFINITY;
					}
					break;
				default: break;
			}
			break;
		default: break;
	}
}

static void handle_functions(str word, token_t *token) {
	switch (word.len) {
		case 2:
			switch (word.data[0]) {
				case 'l':
					if (word.data[1] == 'n') { // ln
						token->type = FUNCTION;
						token->func = LOG_E;
					}
					break;
				default: break;
			}

			break;
		case 3:
			switch (word.data[0]) {
				case 's':
					if ((word.data[1] == 'i') && (word.data[2] == 'n')) { // sin
						token->type = FUNCTION;
						token->func = SIN;
					}
					break;
				case 'c':
					if ((word.data[1] == 'o') && (word.data[2] == 's')) { // cos
						token->type = FUNCTION;
						token->func = COS;
					}
					break;
				case 't':
					if ((word.data[1] == 'a') && (word.data[2] == 'n')) { // tan
						token->type = FUNCTION;
						token->func = TAN;
					}
					break;
				case 'a':
					if ((word.data[1] == 'b') && (word.data[2] == 's')) { // abs
						token->type = FUNCTION;
						token->func = ABS;
					}
					break;
				case 'l':
					if ((word.data[1] == 'o') && (word.data[2] == 'g')) { // log
						token->type = FUNCTION;
						token->func = LOG_10;
					}
					break;
				default: break;
			}

			break;
		case 4:
			switch (word.data[0]) {
				case 's':
					if ((word.data[1] == 'q') && (word.data[2] == 'r') && (word.data[3] == 't')) { // sqrt
						token->type = FUNCTION;
						token->func = SQRT;
					}
					break;
				default: break;
			}

			break;
		case 6:
			if (str_starts_with(word, (str){.data = "arc", .len = 3})) {
				switch (word.data[3]) {
					case 's': // arcsin
						token->type = FUNCTION;
						token->func = ASIN;
						break;
					case 'c': // arccos
						token->type = FUNCTION;
						token->func = ACOS;
						break;
					case 't': // arctan
						token->type = FUNCTION;
						token->func = ATAN;
						break;
					default: break;
				}
			}

			break;
		default: break;
	}
}

token_t *tokenize(str source, mem_arena *arena, size_t *tokens_index) {
	token_t *tokens = arena_alloc(arena, 2 * source.len * sizeof(token_t));
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
			case '!':
				token.type = FACTORIAL;
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
					handle_constants(word, &token);
				}
				break;
		}

		handle_implicit_mult(&tokens, token, (*tokens_index), tokens_index);
		tokens[(*tokens_index)++] = token;
	}

	tokens[(*tokens_index)] = END_TOKEN;
	return tokens;
}

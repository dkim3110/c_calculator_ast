#include "tokenization.h"
#include "mem_arena.h"
#include "str_view.h"

#include <ctype.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef M_PHI
#define M_PHI 1.6180339887498948482
#endif // M_PHI

static inline char peek(strv source, size_t *src_index) {
	size_t foresight = (*src_index);
	if (foresight >= source.len) return '\0';

	return source.data[foresight];
}

static inline char consume(strv source, size_t *src_index) {
	if ((*src_index) >= source.len) return '\0';
	return source.data[(*src_index)++];
}

static inline void handle_implicit_mult(token_t **tokens, token_t token, size_t t_len, size_t *tokens_index) {
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

static inline void match_constants(strv word, token_t *token) {
#define W					 word.data
#define WC(n, c)	 (word.data[(n)] == (c))
#define IS_UNKNOWN token->type = UNKNOWN

	token->type = CONSTANT;
	// clang-format off
	switch (word.len) {
		case 1:
			switch (W[0]) {
				case 'e': token->num_val = M_E; break;
				default:	IS_UNKNOWN; break;
			} break;
		case 2:
			switch (W[0]) {
				case 'p': if (WC(1,'i')) token->num_val = M_PI; else IS_UNKNOWN; break;
				default:  IS_UNKNOWN; break;
			} break;
		case 3:
			switch (W[0]) {
				case 'i': if (WC(1,'n') && WC(2,'f')) token->num_val = INFINITY; else IS_UNKNOWN; break;
				case 'p': if (WC(1,'h') && WC(2,'i')) token->num_val = M_PHI; else IS_UNKNOWN; break;
				case 't': if (WC(1,'a') && WC(2,'u')) token->num_val = M_PI * 2; else IS_UNKNOWN; break;
				default:  IS_UNKNOWN; break;
			} break;
		default:  IS_UNKNOWN; break;
	}
		// clang-format on

#undef W
#undef WC
#undef IS_UNKNOWN
}

static inline void match_functions(strv word, token_t *token) {
#define W					 word.data
#define WC(n, c)	 (word.data[(n)] == (c))
#define IS_UNKNOWN token->type = UNKNOWN

	token->type = FUNCTION;
	// clang-format off
	switch (word.len) {
		case 2:
			switch (W[0]) {
				case 'l': if (WC(1,'n')) token->func = LOG_E; else IS_UNKNOWN; break;
				default:  IS_UNKNOWN; break;
			} break;
		case 3:
			switch (W[0]) {
				case 'a': if (WC(1,'b') && WC(2,'s')) token->func = ABS; else IS_UNKNOWN; break;
				case 'c':
					switch (W[1]) {
						case 'o':
							switch (W[2]) {
								case 's': token->func = SIN; break;
								case 't': token->func = CSC; break;
								default:  IS_UNKNOWN; break;
							} break;
						case 's': if (WC(2,'c')) token->func = CSC; else IS_UNKNOWN; break;
						default:  IS_UNKNOWN; break;
					} break;
				case 'l': if (WC(1,'o') && WC(2,'g')) token->func = LOG_10; else IS_UNKNOWN; break;
				case 's':
					switch (W[1]) {
						case 'i': if (WC(2,'n')) token->func = SIN; else IS_UNKNOWN; break;
						case 'e': if (WC(2,'c')) token->func = SEC; else IS_UNKNOWN; break;
						default:  IS_UNKNOWN; break;
					} break;
				case 't': if (WC(1,'a') && WC(2,'n')) token->func = TAN; else IS_UNKNOWN; break;
				default:  IS_UNKNOWN; break;
			} break;
		case 4:
			switch (W[0]) {
				case 's':
					switch (W[1]) {
						case 'q': if (WC(2,'r') && WC(3,'t')) token->func = SQRT; else IS_UNKNOWN; break;
						case 'i': if (WC(2,'n') && WC(3,'h')) token->func = SINH; else IS_UNKNOWN; break;
						default:  IS_UNKNOWN; break;
					}
					break;
				case 'c': if (WC(1,'o') && WC(2,'s') && WC(3,'h')) token->func = COSH; else IS_UNKNOWN; break;
				case 't': if (WC(1,'a') && WC(2,'n') && WC(3,'h')) token->func = TANH; else IS_UNKNOWN; break;
				default:  IS_UNKNOWN; break;
			} break;
		case 6:
			if (strv_prefix(word, lit_to_strv("arc"))) {
				switch (W[3]) {
					case 'c': if (WC(4,'o') && WC(5,'s')) token->func = ACOS; else IS_UNKNOWN; break;
					case 's': if (WC(4,'i') && WC(5,'n')) token->func = ASIN; else IS_UNKNOWN; break;
					case 't': if (WC(4,'a') && WC(5,'n')) token->func = ATAN; else IS_UNKNOWN; break;
					default:  IS_UNKNOWN; break;
				}
			} else IS_UNKNOWN;
			break;
		default: IS_UNKNOWN; break;
	}
		// clang-format on

#undef W
#undef WC
#undef IS_UNKNOWN
}

token_t *tokenize(strv source, mem_arena *arena, size_t *tokens_index) {
	token_t *tokens = arena_alloc(arena, 2 * source.len * sizeof(token_t));
	size_t src_index = 0;

	while (peek(source, &src_index) != '\0') {
		token_t token = NULL_TOKEN;
		char curr = peek(source, &src_index);
		size_t start_index = src_index;
		consume(source, &src_index);

		if (isspace(curr)) continue;
		token.op = curr;

		switch (curr) {
			case '+': /* fallthrough */
			case '-': token.type = ADD; break;
			case '*': /* fallthrough */
			case '/': /* fallthrough */
			case '%': token.type = MULT; break;
			case '!': token.type = FACTORIAL; break;
			case '^': token.type = EXP; break;
			case '=': token.type = EQUALS; break;
			case '(': token.type = LPAREN; break;
			case ')': token.type = RPAREN; break;

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
					token.num_val = strv_to_dbl((strv){.data = &source.data[start_index], .len = src_index - start_index});
				} else if (isalpha(curr)) {
					while (isalpha(peek(source, &src_index))) consume(source, &src_index);
					strv word = strv_sub(source, start_index, src_index);

					if (peek(source, &src_index) == '(') match_functions(word, &token);
					else match_constants(word, &token);
				}
				break;
		}

		handle_implicit_mult(&tokens, token, (*tokens_index), tokens_index);
		tokens[(*tokens_index)++] = token;
	}

	tokens[(*tokens_index)] = END_TOKEN;
	return tokens;
}

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

#define HASH_E	 (1701593959)
#define HASH_PI	 (3712905052)
#define HASH_TAU (690548749)
#define HASH_PHI (1073095556)
#define HASH_NAN (605416826)
#define HASH_INF (1234765051)

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

static inline void handle_constants(strv word, token_t *token) {
	token->type = CONSTANT;
	switch (strv_hash(word)) {
		case HASH_E:	 token->num_val = M_E; break;
		case HASH_PI:	 token->num_val = M_PI; break;
		case HASH_TAU: token->num_val = M_PI * 2; break;
		case HASH_PHI: token->num_val = M_PHI; break;
		case HASH_NAN: token->num_val = NAN; break;
		case HASH_INF: token->num_val = INFINITY; break;
		default:			 token->type = UNKNOWN; break;
	}
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
					token.num_val = strv_to_dbl((strv){.data = &source.data[start_index], .len = src_index - start_index});
				} else if (isalpha(curr)) {
					while (isalpha(peek(source, &src_index))) consume(source, &src_index);
					strv word = strv_sub(source, start_index, src_index);

					if (peek(source, &src_index) == '(') {
						token.type = FUNCTION;
						token.func = strv_hash(word);
					}

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

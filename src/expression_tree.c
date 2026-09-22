#include "expression_tree.h"
#include "dynamic_array.h"
#include "mem_arena.h"
#include "tokenization.h"

#include "float.h"
#include <math.h>
#include <stdlib.h>

static inline int get_precedence(token_type type) {
	switch (type) {
		case ADD:			 return 1;
		case MULT:		 return 2;
		case EXP:			 return 3;
		case FUNCTION: return 4;
		case LPAREN:	 return 0;
		default:			 return 0;
	}
}

static inline node_t *create_node(token_t token, mem_arena *arena) {
	if (token_equals(token, NULL_TOKEN)) return NULL;
	node_t *node = (node_t *)arena_alloc(arena, sizeof(node_t));
	if (!node) return NULL;

	node->token = token;
	node->left = NULL;
	node->right = NULL;

	return node;
}

node_t *create_tree(dyn_token_t *tokens, mem_arena *arena) {
	dyn_node_t **node_stack = NULL;
	dyn_token_t *op_stack = NULL;
	node_t *root = NULL;

	for (size_t n = 0; n < arr_len(tokens); n++) {
		token_t curr_tok = tokens[n];

		if (curr_tok.type == UNKNOWN) {
			fputs("INVALID INPUT\n", stderr);
			goto end_create_tree;
		}

		if (((curr_tok.type == ADD) && ((n == 0) || (tokens[n - 1].type == LPAREN))) || (curr_tok.type == FUNCTION)) {
			node_t *zero_node = create_node((token_t){.type = NUMBER, .num_val = 0.0}, arena);
			if (zero_node) arr_push(node_stack, zero_node);
		}

		if ((curr_tok.type == NUMBER) || (curr_tok.type == CONSTANT)) {
			node_t *num_node = create_node(curr_tok, arena);
			if (num_node) arr_push(node_stack, num_node);
		} else if (curr_tok.type == LPAREN) {
			arr_push(op_stack, curr_tok);
		} else if (curr_tok.type == RPAREN) {
			while (arr_len(op_stack) > 0) {
				token_t top_op = op_stack[arr_len(op_stack) - 1];

				if (top_op.type == LPAREN) {
					arr_pop(op_stack);
					break;
				}

				arr_pop(op_stack);
				node_t *op_node = create_node(top_op, arena);
				op_node->right = arr_pop(node_stack);
				op_node->left = arr_pop(node_stack);
				arr_push(node_stack, op_node);
			}
		} else {
			while (arr_len(op_stack) > 0) {
				token_t top_op = op_stack[arr_len(op_stack) - 1];
				bool should_pop = (curr_tok.type == EXP) ? (get_precedence(top_op.type) > get_precedence(curr_tok.type))
																								 : (get_precedence(top_op.type) >= get_precedence(curr_tok.type));

				if (!should_pop) break;

				arr_pop(op_stack);
				node_t *op_node = create_node(top_op, arena);

				op_node->right = arr_pop(node_stack);
				op_node->left = arr_pop(node_stack);

				arr_push(node_stack, op_node);
			}

			arr_push(op_stack, curr_tok);
		}
	}

	while (arr_len(op_stack) > 0) {
		token_t top_op = arr_pop(op_stack);
		node_t *op_node = create_node(top_op, arena);

		op_node->right = arr_pop(node_stack);
		op_node->left = arr_pop(node_stack);

		arr_push(node_stack, op_node);
	}

	if (arr_len(node_stack) == 1) root = node_stack[0];
	else fputs("INVALID SYNTAX\n", stderr);

end_create_tree:
	arr_free(node_stack);
	arr_free(op_stack);

	return root;
}

double solve_tree(node_t *root, bool *is_bool) {
	if (!root) return NAN;

	switch (root->token.type) {
		case NUMBER:
			/* fallthrough */
		case CONSTANT: return root->token.num_val; break;
		default:			 break;
	}

	double left_val = solve_tree(root->left, is_bool);
	double right_val = solve_tree(root->right, is_bool);

	switch (root->token.type) {
		case EQUALS:
			/* fallthrough */
		case ADD:
			/* fallthrough */
		case MULT:
			switch (root->token.op) {
				case '=': (*is_bool) = true; return fabs(left_val - right_val) <= DBL_EPSILON;
				case '+': return left_val + right_val;
				case '-': return left_val - right_val;
				case '*': return left_val * right_val;
				case '/': return left_val / right_val;
				case '%': return fmod(left_val, right_val);
			}
			break;
		case EXP: return pow(left_val, right_val);
		case FUNCTION:
			switch (root->token.func) {
				case SQRT:	 return sqrt(left_val + right_val);

				case SIN:		 return sin(left_val + right_val);
				case COS:		 return cos(left_val + right_val);
				case TAN:		 return tan(left_val + right_val);

				case ASIN:	 return asin(left_val + right_val);
				case ACOS:	 return acos(left_val + right_val);
				case ATAN:	 return atan(left_val + right_val);

				case ABS:		 return fabs(left_val + right_val);
				case LOG_E:	 return log(left_val + right_val);
				case LOG_10: return log10(left_val + right_val);
				default:		 return NAN;
			}
			break;
		default: return NAN;
	}

	return NAN;
}

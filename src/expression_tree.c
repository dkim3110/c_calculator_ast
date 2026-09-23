#include "expression_tree.h"
#include "dynamic_array.h"
#include "mem_arena.h"
#include "tokenization.h"

#include "float.h"
#include <math.h>
#include <stdint.h>
#include <stdlib.h>

static inline int get_precedence(token_type type) {
	switch (type) {
		case ADD:				return 1;
		case MULT:			return 2;
		case EXP:				return 3;
		case FACTORIAL: return 4;
		case FUNCTION:	return 5;
		default:				return 0;
	}
}

static inline node_t *create_node(token_t token, mem_arena *arena) {
	if (token_equals(token, NULL_TOKEN)) return NULL;
	node_t *node = (node_t *)arena_alloc(arena, sizeof(node_t));
	if (!node) return NULL;

	node->token = token;

	node->is_unary_prefix = (token.type == FUNCTION);
	node->is_unary_postfix = (token.type == FACTORIAL);

	node->left = NULL;
	node->right = NULL;

	return node;
}

static inline void append_children(token_t top_op, dyn_node_t ***node_stack, mem_arena *arena) {
	node_t *op_node = create_node(top_op, arena);
	op_node->right = (op_node->is_unary_postfix) ? NULL : arr_pop((*node_stack));
	op_node->left = (op_node->is_unary_prefix) ? NULL : arr_pop((*node_stack));
	arr_push((*node_stack), op_node);
}

node_t *create_tree(token_t *tokens, mem_arena *arena) {
	dyn_node_t **node_stack = NULL;
	token_t *op_stack = NULL;
	node_t *root = NULL;

	for (size_t n = 0; !token_equals(tokens[n], END_TOKEN); n++) {
		token_t curr_tok = tokens[n];

		if (curr_tok.type == UNKNOWN) {
			fputs("INVALID INPUT\n", stderr);
			goto end_create_tree;
		}

		if ((curr_tok.type == ADD) && ((n == 0) || (tokens[n - 1].type == LPAREN))) {
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
				append_children(top_op, &node_stack, arena);
			}
		} else {
			while (arr_len(op_stack) > 0) {
				token_t top_op = op_stack[arr_len(op_stack) - 1];
				bool should_pop = (curr_tok.type == EXP) ? (get_precedence(top_op.type) > get_precedence(curr_tok.type))
																								 : (get_precedence(top_op.type) >= get_precedence(curr_tok.type));

				if (!should_pop) break;

				arr_pop(op_stack);
				append_children(top_op, &node_stack, arena);
			}

			arr_push(op_stack, curr_tok);
		}
	}

	while (arr_len(op_stack) > 0) {
		token_t top_op = arr_pop(op_stack);
		append_children(top_op, &node_stack, arena);
	}

	if (arr_len(node_stack) == 1) root = node_stack[0];
	else fputs("INVALID SYNTAX\n", stderr);

end_create_tree:
	arr_free(node_stack);
	arr_free(op_stack);

	return root;
}

static inline double factorial(double value) {
	uint64_t bound = (uint64_t)value;
	uint64_t result = 1;

	for (uint64_t n = 2; n <= bound; n++) result *= n;
	return (double)result;
}

double solve_tree(node_t *root, bool *is_bool) {
	if (!root) return NAN;

	switch (root->token.type) {
		case NUMBER:
			/* fallthrough */
		case CONSTANT: return root->token.num_val; break;
		default:			 break;
	}

	double left_val = (root->is_unary_prefix) ? NAN : solve_tree(root->left, is_bool);
	double right_val = (root->is_unary_postfix) ? NAN : solve_tree(root->right, is_bool);

	switch (root->token.type) {
		case EQUALS:
			/* fallthrough */
		case ADD:
			/* fallthrough */
		case MULT:
			/* fallthrough */
		case FACTORIAL:
			switch (root->token.op) {
				case '=': (*is_bool) = true; return fabs(left_val - right_val) <= DBL_EPSILON;
				case '+': return left_val + right_val;
				case '-': return left_val - right_val;
				case '*': return left_val * right_val;
				case '/': return left_val / right_val;
				case '%': return fmod(left_val, right_val);
				case '!': return factorial(left_val);
			}
			break;
		case EXP: return pow(left_val, right_val);
		case FUNCTION:
			switch (root->token.func) {
				case SQRT:	 return sqrt(right_val);

				case SIN:		 return sin(right_val);
				case COS:		 return cos(right_val);
				case TAN:		 return tan(right_val);

				case ASIN:	 return asin(right_val);
				case ACOS:	 return acos(right_val);
				case ATAN:	 return atan(right_val);

				case ABS:		 return fabs(right_val);
				case LOG_E:	 return log(right_val);
				case LOG_10: return log10(right_val);
				default:		 return NAN;
			}
			break;
		default: return NAN;
	}

	return NAN;
}

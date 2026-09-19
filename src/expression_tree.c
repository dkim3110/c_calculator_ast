#include "expression_tree.h"
#include "dynamic_array.h"
#include "tokenization.h"

#include <math.h>
#include <stdlib.h>

static inline int get_precedence(token_type type) {
	switch (type) {
		case ADD:			 return 1;
		case MULT:		 return 2;
		case EXP:			 return 3;
		case OP_PAREN: return 0;
		default:			 return 0;
	}
}

static inline node_t *create_node(token_t token) {
	if (token_equals(token, NULL_TOKEN)) return NULL;
	node_t *node = (node_t *)malloc(sizeof(node_t));
	if (!node) return NULL;

	node->token = token;
	node->left = NULL;
	node->right = NULL;

	return node;
}

node_t *create_tree(dyn_token_t *tokens) {
	dyn_node_t **node_stack = NULL;
	dyn_token_t *op_stack = NULL;
	node_t *root = NULL;

	for (size_t n = 0; n < arr_len(tokens); n++) {
		token_t curr_tok = tokens[n];

		if (curr_tok.type == UNKNOWN) {
			fputs("INVALID INPUT\n", stderr);
			goto end_create_tree;
		}

		if ((curr_tok.type == ADD) && ((n == 0) || (tokens[n - 1].type == OP_PAREN))) {
			node_t *zero_node = create_node((token_t){.type = NUMBER, .val = (str){.data = "0", .len = 1}});
			if (zero_node) arr_push(node_stack, zero_node);
		}

		if (curr_tok.type == NUMBER) {
			node_t *num_node = create_node(curr_tok);
			if (num_node) arr_push(node_stack, num_node);
		} else if (curr_tok.type == OP_PAREN) {
			arr_push(op_stack, curr_tok);
		} else if (curr_tok.type == CL_PAREN) {
			while (arr_len(op_stack) > 0) {
				token_t top_op = op_stack[arr_len(op_stack) - 1];

				if (top_op.type == OP_PAREN) {
					arr_pop(op_stack);
					break;
				}

				arr_pop(op_stack);
				node_t *op_node = create_node(top_op);
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
				node_t *op_node = create_node(top_op);

				op_node->right = arr_pop(node_stack);
				op_node->left = arr_pop(node_stack);

				arr_push(node_stack, op_node);
			}

			arr_push(op_stack, curr_tok);
		}
	}

	while (arr_len(op_stack) > 0) {
		token_t top_op = arr_pop(op_stack);
		node_t *op_node = create_node(top_op);

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

double solve_tree(node_t *root) {
	if (!root) return NAN;
	else if (root->token.type == NUMBER) return str_to_dbl(root->token.val);

	double left_val = solve_tree(root->left);
	double right_val = solve_tree(root->right);

	switch (root->token.type) {
		case ADD:
			switch (root->token.val.data[0]) {
				case '+': return left_val + right_val;
				case '-': return left_val - right_val;
			}
			break;
		case MULT:
			switch (root->token.val.data[0]) {
				case '*': return left_val * right_val;
				case '/': return left_val / right_val;
			}
			break;
		case EXP: return pow(left_val, right_val);
		default:	return NAN;
	}

	return NAN;
}

static inline void free_node(node_t *node) {
	if (!node) return;
	free(node);
}

void free_tree(node_t *root) {
	if (!root) return;

	free_tree(root->left);
	free_tree(root->right);

	free_node(root);
}

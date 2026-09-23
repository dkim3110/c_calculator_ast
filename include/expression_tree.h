#ifndef EXPRESSION_TREE_H_
#define EXPRESSION_TREE_H_

#include "mem_arena.h"
#include "tokenization.h"

typedef struct node {
	token_t token;
	bool is_unary_prefix;
	bool is_unary_postfix;
	struct node *left;
	struct node *right;
} node_t;

typedef node_t dyn_node_t;

extern node_t *create_tree(token_t *tokens, mem_arena *arena);
double solve_tree(node_t *root, bool *is_bool);

#endif // EXPRESSION_TREE_H_

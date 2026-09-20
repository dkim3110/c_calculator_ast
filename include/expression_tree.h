#ifndef EXPRESSION_TREE_H_
#define EXPRESSION_TREE_H_

#include "mem_arena.h"
#include "tokenization.h"

typedef struct node {
	token_t token;
	struct node *left;
	struct node *right;
} node_t;

typedef node_t dyn_node_t;

extern node_t *create_tree(dyn_token_t *tokens, mem_arena *arena);
extern void free_tree(node_t *root);
extern double solve_tree(node_t *root);

#endif // EXPRESSION_TREE_H_

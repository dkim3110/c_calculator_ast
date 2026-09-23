#include "expression_tree.h"
#include "mem_arena.h"
#include "str_view.h"
#include "tokenization.h"

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
	int result = EXIT_SUCCESS;

	if (argc < 2) {
		fputs("NOTHING TO CALCULATE\n", stderr);
		exit(EXIT_FAILURE);
	}

	if (argc > 2) {
		fputs("INVALID INPUT FORMAT; WRAP INPUT IN QUOTATIONS\n", stderr);
		exit(EXIT_FAILURE);
	}

	mem_arena *perm_arena = arena_init(MiB(1));
	if (!perm_arena) {
		fputs("NOTHING TO CALCULATE\n", stderr);
		exit(EXIT_FAILURE);
	}

	size_t tokens_len = 0;
	token_t *tokens = tokenize(to_strv(argv[1]), perm_arena, &tokens_len);
	if (!tokens) {
		result = EXIT_FAILURE;
		goto end_program;
	}

	node_t *root = create_tree(tokens, tokens_len, perm_arena);
	if (!root) {
		result = EXIT_FAILURE;
		goto end_program;
	}

	bool is_bool = false;
	double final_num = solve_tree(root, &is_bool);
	if (isnan(final_num)) {
		fputs("CALCULATION FAILURE\n", stderr);
		result = EXIT_FAILURE;
		goto end_program;
	}

	fputs("ANSWER: ", stdout);
	if (is_bool) {
		printf("%s\n", (final_num) ? "TRUE" : "FALSE");
	} else if (fabs((fmod(final_num, 1.0))) <= DBL_EPSILON) {
		printf("%.0f\n", final_num);
	} else {
		printf("%.*f\n", 3, final_num);
	}

end_program:
	arena_destroy(perm_arena);
	exit(result);
}

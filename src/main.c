#include "dynamic_array.h"
#include "expression_tree.h"
#include "mem_arena.h"
#include "string_view.h"
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

	mem_arena *perm_arena = arena_init(MiB(1));
	if (!perm_arena) {
		fputs("NOTHING TO CALCULATE\n", stderr);
		exit(EXIT_FAILURE);
	}

	dyn_char *buffer = NULL;
	for (int n = 1; n < argc; n++) {
		for (size_t m = 0; argv[n][m] != '\0'; m++) {
			arr_push(buffer, argv[n][m]);
		}

		arr_push(buffer, ' ');
	}
	arr_push(buffer, '\0');

	dyn_token_t *tokens = tokenize(to_str(buffer));
	if (!tokens) {
		result = EXIT_FAILURE;
		goto cleanup_token;
	}

	node_t *root = create_tree(tokens, perm_arena);
	if (!root) {
		result = EXIT_FAILURE;
		goto cleanup_root;
	}

	double final_num = solve_tree(root);
	if (isnan(final_num)) {
		fputs("CALCULATION FAILURE\n", stderr);
		result = EXIT_FAILURE;
		goto cleanup_final_num;
	}

	fputs("ANSWER: ", stdout);
	if (fabs((fmod(final_num, 1.0))) <= DBL_EPSILON) {
		printf("%.0f\n", final_num);
	} else {
		printf("%.*f\n", 3, final_num);
	}

cleanup_final_num:
	arena_destroy(perm_arena);
cleanup_root:
	arr_free(tokens);
cleanup_token:
	arr_free(buffer);
	exit(result);
}

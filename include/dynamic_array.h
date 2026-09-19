#ifndef DYNAMIC_ARRAY_H_
#define DYNAMIC_ARRAY_H_

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

typedef union {
	struct {
		size_t count;
		size_t capacity;
	} data;
	max_align_t align;
} arr_header;

/* Purely cosmetic typedef to differentiate normal int pointers and dynamic int pointers */
typedef int dyn_int;

/* Purely cosmetic typedef to differentiate normal float pointers and dynamic float pointers */
typedef float dyn_flt;

/* Purely cosmetic typedef to differentiate normal double pointers and dynamic double pointers */
typedef double dyn_dbl;

/* Purely cosmetic typedef to differentiate normal char pointers and dynamic char pointers */
typedef char dyn_char;

#define ARR_INIT_CAPACITY (8)

static inline void *arr_grow_impl(void *arr, size_t elem_size) {
	arr_header *header;
	if (!arr) {
		header = (arr_header *)malloc(sizeof(arr_header) + (elem_size * ARR_INIT_CAPACITY));

		if (!header) return NULL;

		header->data.count = 0;
		header->data.capacity = ARR_INIT_CAPACITY;
	} else {
		header = (arr_header *)arr - 1;
		size_t new_cap = header->data.capacity * 2;
		arr_header *new_header = (arr_header *)realloc(header, sizeof(arr_header) + (elem_size * new_cap));

		if (!new_header) return NULL;

		header = new_header;
		header->data.capacity = new_cap;
	}
	return header + 1;
}

#define arr_push(arr, element)                                                                                         \
	({                                                                                                                   \
		int _success = 1;                                                                                                  \
		if (!(arr) || ((arr_header *)(arr) - 1)->data.count >= ((arr_header *)(arr) - 1)->data.capacity) {                 \
			void *_new_arr = arr_grow_impl((arr), sizeof(*(arr)));                                                           \
			if (!_new_arr) {                                                                                                 \
				_success = 0;                                                                                                  \
			} else {                                                                                                         \
				(arr) = _new_arr;                                                                                              \
			}                                                                                                                \
		}                                                                                                                  \
		if (_success) {                                                                                                    \
			(arr)[((arr_header *)(arr) - 1)->data.count++] = (element);                                                      \
		}                                                                                                                  \
		_success;                                                                                                          \
	})

#define arr_pop(arr) ((arr)[--((arr_header *)(arr) - 1)->data.count])

#define arr_len(arr) ((arr) ? ((arr_header *)(arr) - 1)->data.count : 0)

#define arr_free(arr)                                                                                                  \
	do {                                                                                                                 \
		if (arr) {                                                                                                         \
			free((arr_header *)(arr) - 1);                                                                                   \
			(arr) = NULL;                                                                                                    \
		}                                                                                                                  \
	} while (0)

#endif // DYNAMIC_ARRAY_H_

#ifndef MEM_ARENA_H_
#define MEM_ARENA_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define ALIGN_UP_POW2(n, p) (((uint64_t)(n) + ((uint64_t)(p) - 1)) & (~((uint64_t)(p) - 1)))
#define ARENA_BASE_POS			(sizeof(mem_arena))
#define ARENA_ALIGN					(sizeof(void *))

#define KiB(n) ((uint64_t)(n) << 10)
#define MiB(n) ((uint64_t)(n) << 20)
#define GiB(n) ((uint64_t)(n) << 30)

typedef struct {
	uint64_t cap;
	uint64_t pos;
} mem_arena;

// == FUNCTION DECLARATIONS ===================================================

/*
 * Function to create and initialize an arena based on the capacity provided.
 * @param: capacity of the memory block to allocate.
 * @return: pointer to the allocated mem_arena struct; NULL in case of failure.
 */

static inline mem_arena *arena_init(uint64_t cap);

/*
 * Function to destroy an arena.
 * @param: pointer to the allocated mem_arena struct.
 */

static inline void arena_destroy(mem_arena *arena);

/*
 * Function to clear an arena.
 * @param: pointer to the allocated mem_arena struct.
 */

static inline void arena_clear(mem_arena *arena);

/*
 * Function to push an element to an arena based on the size provided.
 * @param: pointer to the allocated mem_arena struct, size of the element to push, boolean representing if it should
 * zero out the memory.
 * @return: pointer to the pushed element; NULL in case of failure.
 */

static inline void *arena_allocation_impl(mem_arena *arena, uint64_t size, bool zero_out);
#define arena_alloc(arena, type_size)					arena_allocation_impl((arena), (type_size), false)
#define arena_calloc(arena, count, type_size) arena_allocation_impl((arena), ((type_size) * (count)), true)

/*
 * Function to pop an element from an arena based on the size provided.
 * @param: pointer to the allocated mem_arena struct, size of the element to pop.
 */

static inline void arena_pop(mem_arena *arena, uint64_t size);

/*
 * Function to pop an element from an arena based on the size provided to the given position.
 * @param: pointer to the allocated mem_arena struct, position to pop the element to.
 */

static inline void arena_pop_to(mem_arena *arena, uint64_t pos);
// =================================================== FUNCTION DECLARATIONS ==

// == FUNCTION DEFINITIONS ====================================================
static inline mem_arena *arena_init(uint64_t cap) {
	cap += ARENA_BASE_POS;
	mem_arena *arena = (mem_arena *)malloc(cap);
	if (!arena) return NULL;

	arena->cap = cap;
	arena->pos = ARENA_BASE_POS;

	return arena;
} /* arena_init() */

static inline void *arena_allocation_impl(mem_arena *arena, uint64_t size, bool zero_out) {
	uint64_t pos_align = ALIGN_UP_POW2(arena->pos, ARENA_ALIGN);
	uint64_t new_pos = pos_align + size;

	if (new_pos > arena->cap) return NULL;
	arena->pos = new_pos;

	uint8_t *out = (uint8_t *)arena + pos_align;
	if (zero_out) memset(out, 0, size);

	return out;
} /* arena_allocation_impl() */

static inline void arena_pop(mem_arena *arena, uint64_t size) {
	uint64_t diff = arena->pos - ARENA_BASE_POS;
	size = (size < diff) ? size : diff;
	arena->pos -= size;
} /* arena_pop() */

static inline void arena_pop_to(mem_arena *arena, uint64_t pos) {
	uint64_t size = (pos < arena->pos) ? arena->pos - pos : 0;
	arena_pop(arena, size);
} /* arena_pop_to() */

static inline void arena_clear(mem_arena *arena) {
	arena_pop_to(arena, ARENA_BASE_POS);
} /* arena_clear() */

static inline void arena_destroy(mem_arena *arena) {
	if (arena) free(arena);
} /* arena_destroy() */
// ===================================================== FUNCTION DEFINITIONS ==

#endif // MEM_ARENA_H_

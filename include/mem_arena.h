/* add -D_DEFAULT_SOURCE as a compiler argument */

#ifndef MEM_ARENA_H_
#define MEM_ARENA_H_

#include <stdalign.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define ALIGN_UP_POW2(n, p) (((size_t)(n) + ((size_t)(p) - 1)) & (~((size_t)(p) - 1)))
#define ALIGN_UP(n, a)			((((size_t)(n) + ((size_t)(a) - 1)) / (size_t)(a)) * (size_t)(a))
#define ARENA_BASE_POS			(sizeof(mem_arena))
#define ARENA_ALIGN					(alignof(max_align_t))

#define KiB(n) ((size_t)(n) << 10)
#define MiB(n) ((size_t)(n) << 20)
#define GiB(n) ((size_t)(n) << 30)

#define arena_new()														arena_init(GiB(1), MiB(2))
#define arena_clear(arena)										arena_clear_impl((arena), false)
#define arena_hard_clear(arena)								arena_clear_impl((arena), true)
#define arena_alloc(arena, type_size)					arena_allocation_impl((arena), (type_size), false)
#define arena_calloc(arena, count, type_size) arena_allocation_impl((arena), ((type_size) * (count)), true)
#define arena_getpos(arena)										((arena)->pos)

typedef struct {
	size_t reserve;
	size_t commit;
	size_t pos;
	size_t commit_pos;
} mem_arena;

static inline mem_arena *arena_init(size_t reserve_size, size_t commit_size);
static inline void arena_destroy(mem_arena *arena);
static inline void arena_clear_impl(mem_arena *arena, bool do_decommit);
static inline void *arena_allocation_impl(mem_arena *arena, size_t size, bool zero_out);
static inline void arena_pop(mem_arena *arena, size_t pos);

static inline uint32_t plat_get_pagesize(void);
static inline void *plat_mem_reserve(size_t size);
static inline bool plat_mem_commit(void *ptr, size_t size);
static inline bool plat_mem_decommit(void *ptr, size_t size);
static inline bool plat_mem_release(void *ptr, size_t size);

// == FUNCTION DEFINITIONS ====================================================

/*
 * Function to create and initialize an arena based on the reserve and commit size provided.
 * If commit_size is less than the system page size, initializes it to be equal to the page size.
 * The macro arena_new() is provided for ease of use, setting reserve_size and commit_size to a default of 1
 * GiB and 2 MiB each.
 * Returns a pointer to the allocated mem_arena struct or NULL if failure.
 */

static inline mem_arena *arena_init(size_t reserve_size, size_t commit_size) {
	uint32_t page_size = plat_get_pagesize();
	if ((page_size & (page_size - 1)) != 0) return NULL;

	reserve_size += ARENA_BASE_POS;
	commit_size = (commit_size < page_size) ? page_size : commit_size;

	reserve_size = ALIGN_UP_POW2(reserve_size, page_size);
	commit_size = ALIGN_UP_POW2(commit_size, page_size);

	mem_arena *arena = plat_mem_reserve(reserve_size);
	if (!arena) return NULL;
	if (!plat_mem_commit(arena, page_size)) {
		plat_mem_release(arena, reserve_size);
		return NULL;
	}

	arena->reserve = reserve_size;
	arena->commit = commit_size;
	arena->pos = ARENA_BASE_POS;
	arena->commit_pos = page_size;

	return arena;
} /* arena_init() */

/*
 * Function to push an element to an arena based on the size provided. Zeroes out memory based on
 * zero_out's value.
 * The macros arena_alloc() and arena_calloc() are provided for ease of use.
 * Returns a pointer to the allocated element or NULL if failure.
 */

static inline void *arena_allocation_impl(mem_arena *arena, size_t size, bool zero_out) {
	size_t pos_align = ALIGN_UP_POW2(arena->pos, ARENA_ALIGN);
	size_t new_pos = pos_align + size;

	if ((new_pos < pos_align) || (new_pos > arena->reserve)) return NULL;

	if (new_pos > arena->commit_pos) {
		size_t new_commit_pos = ALIGN_UP(new_pos, arena->commit);
		new_commit_pos = (new_commit_pos < arena->reserve) ? new_commit_pos : arena->reserve;

		uint8_t *mem = (uint8_t *)arena + arena->commit_pos;
		size_t commit_size = new_commit_pos - arena->commit_pos;

		if (!plat_mem_commit(mem, commit_size)) return NULL;

		arena->commit_pos = new_commit_pos;
	}

	arena->pos = new_pos;

	uint8_t *out = (uint8_t *)arena + pos_align;
	if (zero_out) memset(out, 0, size);

	return out;
} /* arena_allocation_impl() */

/*
 * Function to pop an element from an arena to the given position. Does not decommit memory.
 * The macro arena_getpos() is provided for ease of use.
 */

static inline void arena_pop(mem_arena *arena, size_t pos) {
	size_t new_pos = (pos < ARENA_BASE_POS) ? ARENA_BASE_POS : pos;
	if (new_pos < arena->pos) {
		arena->pos = new_pos;
	}
} /* arena_pop() */

/*
 * Function to clear an arena. If do_decommit is set to true, decommits all pages except the first if commit position
 * is greater than page size.
 * The macros arena_clear() and arena_hard_clear() are provided for ease of use.
 */

static inline void arena_clear_impl(mem_arena *arena, bool do_decommit) {
	if (do_decommit) {
		uint32_t page_size = plat_get_pagesize();
		if (arena->commit_pos > page_size) {
			size_t decommit_size = arena->commit_pos - page_size;
			uint8_t *decommit_ptr = (uint8_t *)arena + page_size;

			plat_mem_decommit(decommit_ptr, decommit_size);
			arena->commit_pos = page_size;
		}
	}

	arena_pop(arena, ARENA_BASE_POS);
} /* arena_clear_impl() */

/*
 * Function to destroy an arena and release its memory.
 */

static inline void arena_destroy(mem_arena *arena) {
	if (arena) plat_mem_release(arena, arena->reserve);
} /* arena_destroy() */

// -- Platform Dependent ------------------------------------------------------

#ifdef __linux__

#include <sys/mman.h>
#include <unistd.h>

/*
 * Function to get the size of the system's page size. Direct call of this function is disadviced.
 */

static inline uint32_t plat_get_pagesize(void) {
	static uint32_t page_size_cache = 0;
	if (!page_size_cache) page_size_cache = (uint32_t)sysconf(_SC_PAGESIZE);
	return page_size_cache;
} /* plat_get_pagesize() */

/*
 * Function to reserve memory based on the size provided. Direct call of this function is disadviced.
 * Returns a pointer to the reserved memory or NULL if failure.
 */

static inline void *plat_mem_reserve(size_t size) {
	void *out = mmap(NULL, size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	return (out == MAP_FAILED) ? NULL : out;
} /* plat_mem_reserve() */

/*
 * Function to commit a portion of reserved virtual memory. Direct call of this function is disadviced.
 * Accepts a pointer to virual address and size to commit in bytes.
 * Returns true if successful, false if failure.
 */

static inline bool plat_mem_commit(void *ptr, size_t size) {
	return (mprotect(ptr, size, PROT_READ | PROT_WRITE) == 0);
} /* plat_mem_commit() */

/*
 * Function to decommit a portion of committed virtual memory. Direct call of this function is disadviced.
 * Accepts a pointer to virual address and size to decommit in bytes.
 * Returns true if successful, false if failure.
 */

static inline bool plat_mem_decommit(void *ptr, size_t size) {
	return (mprotect(ptr, size, PROT_NONE) != 0) ? false : (madvise(ptr, size, MADV_DONTNEED) == 0);
} /* plat_mem_decommit() */

/*
 * Function to release a portion of reserved virtual memory. Direct call of this function is disadviced.
 * Accepts a pointer to virual address and size to release in bytes.
 * Returns true if successful, false if failure.
 */

static inline bool plat_mem_release(void *ptr, size_t size) {
	return (munmap(ptr, size) == 0);
} /* plat_mem_release() */

#elif defined(_WIN32)

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN

#include <windows.h>

static inline uint32_t plat_get_pagesize(void) {
	static uint32_t page_size_cache = 0;
	if (!page_size_cache) {
		SYSTEM_INFO sys_info = {0};
		GetSystemInfo(&sys_info);
		page_size_cache = sys_info.dwPageSize;
	}

	return page_size_cache;
} /* plat_get_pagesize() */

static inline void *plat_mem_reserve(size_t size) {
	return VirtualAlloc(NULL, size, MEM_RESERVE, PAGE_NOACCESS);
} /* plat_mem_reserve() */

static inline bool plat_mem_commit(void *ptr, size_t size) {
	void *result = VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE);
	return (result != NULL);
} /* plat_mem_commit() */

static inline bool plat_mem_decommit(void *ptr, size_t size) {
	return VirtualFree(ptr, size, MEM_DECOMMIT);
} /* plat_mem_decommit() */

static inline bool plat_mem_release(void *ptr, size_t size) {
	(void)size;
	return VirtualFree(ptr, 0, MEM_RELEASE);
} /* plat_mem_release() */

#endif
// ------------------------------------------------------ Platform Dependent --

// ===================================================== FUNCTION DEFINITIONS ==

#endif // MEM_ARENA_H_

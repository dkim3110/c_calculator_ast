#ifndef STR_VIEW_H_
#define STR_VIEW_H_

#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Abbreviation of string view */
typedef struct {
	const char *data;
	size_t len;
} strv;

#define NULL_STRV		 ((strv){.data = NULL, .len = 0})
#define FMT_STRV		 "%.*s"
#define ARG_STRV(sv) (int)(sv).len, (sv).data

#define STRV_IS_ARR(x)			 (!__builtin_types_compatible_p(__typeof__(x), __typeof__(&(x)[0])))
#define lit_to_strv(str_lit) to_strv_wlen((str_lit), sizeof(str_lit) - 1 + 0 * sizeof(char[STRV_IS_ARR(str_lit) ? 1 : -1]))

static inline bool strv_empty(strv sv);
static inline bool strv_blank(strv sv);
static inline bool strv_eq(strv a, strv b);
static inline bool strv_ic_eq(strv a, strv b);

static inline strv strv_take(strv sv, size_t n);
static inline strv strv_drop(strv sv, size_t n);
static inline strv strv_sub(strv sv, size_t start, size_t end);

static inline strv strv_trim(strv sv);
static inline strv strv_ltrim(strv sv);
static inline strv strv_rtrim(strv sv);

static inline bool strv_prefix(strv base, strv prefix);

static inline strv to_strv(const char *cstring);
static inline strv to_strv_wlen(const char *cstring, size_t len);
static inline double strv_to_dbl(strv sv);
static inline uint32_t strv_hash(strv sv);

// == CHECKING ================================================================

/*
 * Function to verify if a given strv struct is empty.
 * Returns 1 if true, 0 if false.
 */

static inline bool strv_empty(strv sv) {
	return ((sv.len == 0) || (!sv.data));
} /* strv_empty() */

/*
 * Function to verify if a given strv struct is blank: either emoty or consisting of only whitespaces.
 * Returns 1 if true, 0 if false.
 */

static inline bool strv_blank(strv sv) {
	if (strv_empty(sv)) return true;

	for (size_t n = 0; n < sv.len; n++) {
		if (!isspace((unsigned char)sv.data[n])) return false;
	}

	return true;
} /* strv_blank() */

/*
 * Function to verify if two strv structs are equal.
 * Returns 1 if true, 0 if false.
 */

static inline bool strv_eq(strv a, strv b) {
	if (a.len != b.len) return false;
	if (strv_empty(a) || strv_empty(b)) return (strv_empty(a) == strv_empty(b));
	return (memcmp(a.data, b.data, a.len) == 0);
} /* strv_eq() */

/*
 * Function to verify if two strv structs are equal, ignoring letter case.
 * Returns 1 if true, 0 if false.
 */

static inline bool strv_ic_eq(strv a, strv b) {
	if (a.len != b.len) return false;
	if (strv_empty(a) || strv_empty(b)) return (strv_empty(a) == strv_empty(b));

	for (size_t n = 0; n < a.len; n++) {
		if (toupper((unsigned char)a.data[n]) != toupper((unsigned char)b.data[n])) return false;
	}

	return true;
} /* strv_ic_eq() */
// == MANIPULATION ============================================================

/*
 * Function to take characters from a given strv struct up to the specified index.
 * It is exclusive and thus will not take the character at index n.
 * Returns sv unmodified if empty.
 */

static inline strv strv_take(strv sv, size_t n) {
	if (strv_empty(sv)) return sv;
	if (n > sv.len) n = sv.len;

	return (strv){.data = sv.data, .len = n};
} /* strv_take() */

/*
 * Function to drop characters from a strv struct up to the specified index.
 * It is exclusive and thus will not drop the character at index n.
 * Returns sv unmodifed if empty.
 */

static inline strv strv_drop(strv sv, size_t n) {
	if (strv_empty(sv)) return sv;
	if (n > sv.len) n = sv.len;

	return (strv){.data = sv.data + n, .len = sv.len - n};
} /* strv_drop() */

/*
 * Function to extract a substring from a strv struct from the start and end indexes provided.
 * Start is inclusive, end is exclusive.
 */

static inline strv strv_sub(strv sv, size_t start, size_t end) {
	return strv_drop(strv_take(sv, end), start);
} /* strv_sub() */

/*
 * Function to trim leading and trailing whitespace from a given strv struct.
 * Returns original string if the given string is empty.
 */

static inline strv strv_trim(strv sv) {
	return strv_ltrim(strv_rtrim(sv));
} /* strv_trim() */

/*
 * Function to trim leading whitespace from a given strv struct.
 * Returns original string if the given string is empty.
 *
 */

static inline strv strv_ltrim(strv sv) {
	if (strv_empty(sv)) return sv;
	while ((sv.len > 0) && (isspace((unsigned char)sv.data[0]))) sv = strv_drop(sv, 1);

	return sv;
} /* strv_ltrim() */

/*
 * Function to trim trailing whitespace from a given strv struct.
 * Returns original string if the given string is empty.
 */

static inline strv strv_rtrim(strv sv) {
	if (strv_empty(sv)) return sv;
	while ((sv.len > 0) && (isspace((unsigned char)sv.data[sv.len - 1]))) sv = strv_take(sv, sv.len - 1);

	return sv;
} /* strv_rtrim() */
// ============================================================ MANIPULATION ==

// == PARSING =================================================================

/*
 * Function to verify if a given strv struct starts with the specified prefix.
 * Returns 1 if true, 0 if false. Empty strings are considered to be contained by
 * all strings.
 */

static inline bool strv_prefix(strv base, strv prefix) {
	if (strv_empty(prefix)) return true;
	if ((!base.data) || (!prefix.data)) return false;
	if ((prefix.len > base.len) || (strv_empty(base))) return false;
	return (memcmp(base.data, prefix.data, prefix.len) == 0);
} /* strv_prefix() */
// ================================================================= PARSING ==

// == CONVERSION ==============================================================

/*
 * Function to convert a standard char pointer into a strv struct.
 */

static inline strv to_strv(const char *cstring) {
	if (!cstring) return NULL_STRV;
	return to_strv_wlen(cstring, strlen(cstring));
} /* to_strv() */

/*
 * Function to convert a standard char pointer into a strv struct with a specified length.
 */

static inline strv to_strv_wlen(const char *cstring, size_t len) {
	if (!cstring) return NULL_STRV;
	return (strv){.data = cstring, .len = len};
} /* to_strv() */

/*
 * Function to convert a given strv struct into a double-precision floating-point value.
 * Returns NAN if it fails to convert or if invalid characters are present.
 */

static inline double strv_to_dbl(strv sv) {
	sv = strv_trim(sv);
	if (strv_empty(sv)) return NAN;

	if (strv_ic_eq(sv, lit_to_strv("nan"))) return NAN;
	if ((strv_ic_eq(sv, lit_to_strv("inf"))) || (strv_ic_eq(sv, lit_to_strv("+inf")))) return INFINITY;
	if (strv_ic_eq(sv, lit_to_strv("-inf"))) return -INFINITY;

	size_t index = 0;
	int sign = 1;

	switch (sv.data[index]) {
		case '-': sign = -1; /* fallthrough */
		case '+': index++; break;
		default:	break;
	}

	long double result = 0.0;
	bool has_digits = false;

	while ((index < sv.len) && (isdigit((unsigned char)sv.data[index]))) {
		result = (result * 10.0) + (sv.data[index] - '0');
		has_digits = true;
		index++;
	}

	if ((index < sv.len) && (sv.data[index] == '.')) {
		index++;
		long double fraction = 0.0;
		long double divisor = 1.0;

		while ((index < sv.len) && (isdigit((unsigned char)sv.data[index]))) {
			fraction = (fraction * 10.0) + (sv.data[index] - '0');
			divisor *= 10.0;
			has_digits = true;
			index++;
		}

		result += fraction / divisor;
	}

	if (!has_digits) return NAN;

	if ((index < sv.len) && (toupper((unsigned char)sv.data[index]) == 'E')) {
		index++;
		int exp_sign = 1;

		if (index < sv.len) {
			switch (sv.data[index]) {
				case '-': exp_sign = -1; /* fallthrough */
				case '+': index++; break;
				default:	break;
			}
		}

		int exponent = 0;
		while ((index < sv.len) && (isdigit((unsigned char)sv.data[index]))) {
			if (exponent < 9999) exponent = (exponent * 10) + (sv.data[index] - '0');
			index++;
		}

		if (exponent != 0) result *= pow(10.0, exp_sign * exponent);
	}

	return (index < sv.len) ? NAN : result * sign;
} /* strv_to_dbl() */

/*
 * Function to hash a strv struct using MurmurHash3.
 * Accepts a strv struct and returns a non-cryptographic hash value.
 * Initial seed is set to 0.
 */

static inline uint32_t strv_hash(strv sv) {
	uint32_t hash = 0;
	uint32_t k;
	for (size_t n = sv.len >> 2; n; n--) {
		memcpy(&k, sv.data, sizeof(uint32_t));
		sv.data += sizeof(uint32_t);

		k *= 0xcc9e2d51;
		k = (k << 15) | (k >> 17);
		k *= 0x1b873593;

		hash ^= k;
		hash = (hash << 13) | (hash >> 19);
		hash = hash * 5 + 0xe6546b64;
	}

	k = 0;
	for (size_t n = sv.len & 3; n; n--) {
		k <<= 8;
		k |= (unsigned char)sv.data[n - 1];
	}

	k *= 0xcc9e2d51;
	k = (k << 15) | (k >> 17);
	k *= 0x1b873593;

	hash ^= k;
	hash ^= sv.len;
	hash ^= hash >> 16;
	hash *= 0x85ebca6b;
	hash ^= hash >> 13;
	hash *= 0xc2b2ae35;
	hash ^= hash >> 16;
	return hash;
} /* strv_hash() */
// ============================================================== CONVERSION ==

#endif // STR_VIEW_H_

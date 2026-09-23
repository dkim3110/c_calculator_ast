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

#define STRV_ALPHABET			 (256)
#define STRV_MAX_DBL_PARSE (500)
#define FMT_STRV					 "%.*s"
#define ARG_STRV(sv)			 (int)(sv).len, (sv).data

/* Abbreviation of string view */

typedef struct {
	const char *data;
	size_t len;
} strv;

#define NULL_STRV ((strv){.data = NULL, .len = 0})

static inline bool strv_empty(strv sv);
static inline bool strv_blank(strv sv);
static inline bool strv_eq(strv a, strv b);

static inline strv strv_take(strv sv, size_t n);
static inline strv strv_drop(strv sv, size_t n);
static inline strv strv_sub(strv sv, size_t start, size_t end);

static inline strv strv_trim(strv sv);
static inline strv strv_ltrim(strv sv);
static inline strv strv_rtrim(strv sv);

static inline bool strv_prefix(strv base, strv prefix);

static inline strv to_strv(const char *cstring);
static inline double strv_to_dbl(strv sv);

// == CHECKING ================================================================

/*
 * Function to verify if a given strv struct is empty.
 * Returns 1 if true, 0 if false.
 */

static inline bool strv_empty(strv sv) {
	return (sv.len == 0);
} /* strv_empty() */

/*
 * Function to verify if a given strv struct consists of only whitespaces.
 * Returns 1 if true, 0 if false.
 */

static inline bool strv_blank(strv sv) {
	if (strv_empty(sv)) return false;

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
// ================================================================ CHECKING ==

// == MANIPULATION ============================================================

/*
 * Function to take characters from a given strv struct up to the specified index.
 * It is exclusive and thus will not take the character at index n.
 */

static inline strv strv_take(strv sv, size_t n) {
	if (n > sv.len) n = sv.len;

	return (strv){.data = sv.data, .len = n};
} /* strv_take() */

/*
 * Function to drop characters from a strv struct up to the specified index.
 * It is exclusive and thus will not drop the character at index n.
 */

static inline strv strv_drop(strv sv, size_t n) {
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
 * Aborts process and returns original string if the given string is empty.
 * Returns an empty string if the given string is blank.
 */

static inline strv strv_trim(strv sv) {
	return strv_ltrim(strv_rtrim(sv));
} /* strv_trim() */

/*
 * Function to trim leading whitespace from a given strv struct.
 * Aborts process and returns original string if the given string is empty.
 * Returns an empty string if the given string is blank.
 */

static inline strv strv_ltrim(strv sv) {
	if (strv_empty(sv)) return sv;
	if (strv_blank(sv)) return NULL_STRV;

	while ((sv.len > 0) && (isspace((unsigned char)sv.data[0]))) sv = strv_drop(sv, 1);

	return sv;
} /* strv_ltrim() */

/*
 * Function to trim trailing whitespace from a given strv struct.
 * Aborts process and returns original string if the given string is empty.
 * Returns an empty string if the given string is blank.
 */

static inline strv strv_rtrim(strv sv) {
	if (strv_empty(sv)) return sv;
	if (strv_blank(sv)) return NULL_STRV;

	while ((sv.len > 0) && (isspace((unsigned char)sv.data[sv.len - 1]))) sv = strv_take(sv, sv.len - 1);

	return sv;
} /* strv_rtrim() */
// ============================================================ MANIPULATION ==

// == PARSING =================================================================

/*
 * Function to verify if a given strv struct starts with the specified prefix.
 * Returns 1 if true, 0 if false. Empty strings are considerd to be contained by
 * all strings.
 */

static inline bool strv_prefix(strv base, strv prefix) {
	if ((strv_eq(base, prefix)) || (strv_empty(prefix))) return true;
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
	return (strv){.data = cstring, .len = strlen(cstring)};
} /* to_strv() */

/*
 * Function to convert a given strv struct into a floating-point value.
 * Returns NAN if it fails to convert, or if the string is longer than
 * STRV_ALPHABET characters.
 */

static inline double strv_to_dbl(strv sv) {
	strv number = strv_trim(sv);
	if ((strv_empty(number)) || (number.len >= STRV_MAX_DBL_PARSE)) return NAN;

	char buffer[STRV_MAX_DBL_PARSE];
	memcpy(buffer, number.data, number.len);
	buffer[number.len] = '\0';

	char *endptr;
	double result = strtod(buffer, &endptr);

	if ((endptr == buffer) || (*endptr != '\0')) return NAN;
	return result;
} /* strv_to_dbl() */
// ============================================================== CONVERSION ==

#endif // STR_VIEW_H_

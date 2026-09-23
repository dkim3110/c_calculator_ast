#ifndef STRING_VIEW_H_
#define STRING_VIEW_H_

#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define STR_VW_CHAR_NUM (256)
#define STR_FMT					"%.*s"
#define STR_ARG(s)			(int)(s).len, (s).data

typedef struct {
	const char *data;
	size_t len;
} str;

static const str NULL_STR = (str){.data = NULL, .len = 0};

static inline bool str_is_empty(str s);
static inline bool str_is_blank(str s);
static inline bool str_equals(str a, str b);

static inline str str_take(str s, size_t n);
static inline str str_drop(str s, size_t n);
static inline str str_sub(str s, size_t start, size_t end);
static inline str str_trim(str s);

static inline bool str_starts_with(str base, str prefix);

static inline str to_str(const char *cstring);
static inline double str_to_dbl(str s);

// == CHECKING ================================================================

/*
 * Function to verify if a given str struct is empty.
 * Returns 1 if true, 0 if false.
 */

static inline bool str_is_empty(str s) {
	return ((s.len == 0) || (!s.data));
} /* str_is_empty() */

/*
 * Function to verify if a given str struct consists of only whitespaces.
 * Returns 1 if true, 0 if false.
 */

static inline bool str_is_blank(str s) {
	if (str_is_empty(s)) return false;

	for (size_t n = 0; n < s.len; n++) {
		if (!isspace((unsigned char)s.data[n])) return false;
	}

	return true;
} /* str_is_blank() */

/*
 * Function to verify if two str structs are equal.
 * Returns 1 if true, 0 if false.
 */

static inline bool str_equals(str a, str b) {
	if (a.len != b.len) return false;
	if (str_is_empty(a) || str_is_empty(b)) return (str_is_empty(a) == str_is_empty(b));
	return (memcmp(a.data, b.data, a.len) == 0);
} /* str_equals() */
// ================================================================ CHECKING ==

// == MANIPULATION ============================================================

/*
 * Function to truncate a given str struct to the specified index.
 * It is exclusive and thus will not take the character at index n.
 */

static inline str str_take(str s, size_t n) {
	if (str_is_empty(s)) return s;
	if (n > s.len) n = s.len;

	return (str){.data = s.data, .len = n};
} /* str_take() */

/*
 * Function to drop characters from a str struct up to the specified index.
 * It is exclusive and thus will not drop the character at index n.
 */

static inline str str_drop(str s, size_t n) {
	if (str_is_empty(s)) return s;
	if (n > s.len) n = s.len;

	return (str){.data = s.data + n, .len = s.len - n};
} /* str_drop() */

/*
 * Function to extract a substring from a str struct from the start and end indexes provided.
 * Start is inclusive, end is exclusive.
 */

static inline str str_sub(str s, size_t start, size_t end) {
	str result = str_take(s, end);
	result = str_drop(result, start);
	return result;
} /* str_sub() */

/*
 * Function to trim whitespace(s) from a given str struct.
 * If the given string is empty, aborts process and returns original string.
 * If the given string is blank, returns an empty string.
 */

static inline str str_trim(str s) {
	if (str_is_empty(s)) return s;
	if (str_is_blank(s)) return NULL_STR;

	while ((s.len > 0) && (isspace((unsigned char)s.data[0]))) {
		s = str_drop(s, 1);
	}

	while ((s.len > 0) && (isspace((unsigned char)s.data[s.len - 1]))) {
		s = str_take(s, s.len - 1);
	}

	return s;
} /* str_trim() */
// ============================================================ MANIPULATION ==

// == PARSING =================================================================

/*
 * Function to verify if a given str struct starts with the specified prefix.
 * Returns 1 if true, 0 if false. Empty strings are considerd to be contained by
 * all strings.
 */

static inline bool str_starts_with(str base, str prefix) {
	if ((str_equals(base, prefix)) || (str_is_empty(prefix))) return true;
	if ((prefix.len > base.len) || (str_is_empty(base))) return false;
	return (memcmp(base.data, prefix.data, prefix.len) == 0);
} /* str_starts_with */
// ================================================================= PARSING ==

// == CONVERSION ==============================================================

/*
 * Function to convert a standard char pointer into a str struct.
 */

static inline str to_str(const char *cstring) {
	return (str){.data = (cstring) ? cstring : NULL, .len = (cstring) ? strlen(cstring) : 0};
} /* to_str() */

/*
 * Function to convert a given str struct into a floating-point value.
 * Returns NAN if it fails to convert, or if the string is longer than
 * STR_VW_CHAR_NUM.
 */

static inline double str_to_dbl(str s) {
	if ((str_is_empty(s)) || (str_is_blank(s))) {
		return NAN;
	}

	str number = str_trim(s);
	if (number.len > STR_VW_CHAR_NUM) return NAN;
	char buffer[STR_VW_CHAR_NUM + 1];
	memcpy(buffer, number.data, number.len);
	buffer[number.len] = '\0';

	char *endptr;
	double result = strtod(buffer, &endptr);

	if ((endptr == buffer) || (*endptr != '\0')) {
		return NAN;
	}

	return result;
} /* str_to_dbl() */
// ============================================================== CONVERSION ==

#endif // STRING_VIEW_H_

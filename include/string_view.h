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
static inline bool str_equals_ignore_case(str a, str b);
static inline bool str_is_digit(str s);
static inline bool str_is_decimal(str s);

static inline void fput_str(str s, FILE *stream);
static inline void print_str(str s);
static inline void println_str(str s);
static inline void fput_str_toupper(str s, FILE *stream);
static inline void fput_str_tolower(str s, FILE *stream);

static inline str str_take(str s, size_t n);
static inline str str_drop(str s, size_t n);
static inline str str_sub(str s, size_t start, size_t end);
static inline str split_str(str *s, char delimiter);
static inline str str_trim(str s);

static inline bool str_starts_with(str base, str prefix);
static inline bool str_contains(str base, str item, ptrdiff_t *index);
static inline ptrdiff_t index_of(str s, char c);
static inline int char_at(str s, size_t index);

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

/*
 * Function to verify if two str structs are equal, ignoring letter case.
 * Returns 1 if true, 0 if false.
 */

static inline bool str_equals_ignore_case(str a, str b) {
	if (a.len != b.len) return false;
	if (str_is_empty(a) || str_is_empty(b)) return (str_is_empty(a) == str_is_empty(b));

	for (size_t n = 0; n < a.len; n++) {
		if (toupper((unsigned char)a.data[n]) != toupper((unsigned char)b.data[n])) return false;
	}

	return true;
} /* str_equals_ignore_case() */

/*
 * Function to determine if a given str struct consists of only digits.
 * Returns 1 if true, 0 if false.
 */

static inline bool str_is_digit(str s) {
	if ((str_is_empty(s)) || (str_is_blank(s))) return false;

	if ((s.data[0] == '-') || (s.data[0] == '+')) {
		if (s.len == 1) return false;
		s = str_drop(s, 1);
	}

	for (size_t n = 0; n < s.len; n++) {
		if (!isdigit((unsigned char)s.data[n])) return false;
	}

	return true;
} /* str_is_digit() */

/*
 * Function to determine if a given str struct represents a decimal value.
 * Returns 1 if true, 0 if false.
 */

static inline bool str_is_decimal(str s) {
	if ((str_is_empty(s)) || (str_is_blank(s))) return false;

	if ((s.data[0] == '-') || (s.data[0] == '+')) {
		if (s.len == 1) return false;
		s = str_drop(s, 1);
	}

	bool found_point = false;
	bool found_digit = false;

	for (size_t n = 0; n < s.len; n++) {
		if (s.data[n] == '.') {
			if (found_point) return false;
			found_point = true;
		} else if (isdigit((unsigned char)s.data[n])) {
			found_digit = true;
		} else {
			return false;
		}
	}

	return ((found_digit) && (found_point));
} /* str_is_decimal() */
// ================================================================ CHECKING ==

// == I/O =====================================================================

/*
 * Function to write a str struct to a specified file stream.
 */

static inline void fput_str(str s, FILE *stream) {
	if ((!stream) || (str_is_empty(s))) return;
	fwrite(s.data, sizeof(char), s.len, stream);
} /* fput_str() */

/*
 * Function to print a str struct to stdout.
 */

static inline void print_str(str s) {
	fput_str(s, stdout);
} /* print_str() */

/*
 * Function to print a str struct to stdout, then a newline character.
 */

static inline void println_str(str s) {
	print_str(s);
	putchar('\n');
} /* println_str() */

/*
 * Function to print a str struct to a specified file stream in all uppercase.
 */

static inline void fput_str_toupper(str s, FILE *stream) {
	if ((!stream) || (str_is_empty(s))) return;
	for (size_t n = 0; n < s.len; n++) {
		fputc(toupper((unsigned char)s.data[n]), stream);
	}
} /* fput_str_toupper() */

/*
 * Function to print a str struct to a specified file stream in all lowercase.
 */

static inline void fput_str_tolower(str s, FILE *stream) {
	if ((!stream) || (str_is_empty(s))) return;
	for (size_t n = 0; n < s.len; n++) {
		fputc(tolower((unsigned char)s.data[n]), stream);
	}
} /* fput_str_tolower() */
// ===================================================================== I/O ==

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
 * Function to split a given str struct based on the specifed delimeter.
 * Returns a str struct up to the specified delimeter, or the original string
 * if it is empty. The original str struct retains all characters after the
 * delimeter; mutated by the function. The delimeter itself gets discarded.
 */

static inline str split_str(str *s, char delimiter) {
	if (str_is_empty(*s)) return *s;
	size_t n = 0;
	while ((n < s->len) && (s->data[n] != delimiter)) n++;
	str result = str_take(*s, n);

	if (n < s->len) *s = str_drop(*s, n + 1);
	else *s = str_drop(*s, n);

	return result;
} /* split_str() */

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

/*
 * Function to verify if a given str struct contains the specified string.
 * Additionally accepts a pointer to a size_t index, which will be updated to be
 * the index of the found substring, or -1 if there is none. Empty strings are
 * considerd to be contained by all strings. Returns 1 if true, 0 if false.
 */

static inline bool str_contains(str base, str item, ptrdiff_t *index) {
	if (str_is_empty(item)) {
		if (index) *index = 0;
		return true;
	}
	if ((item.len > base.len) || (str_is_empty(base))) {
		if (index) *index = -1;
		return false;
	}

	if ((base.len < STR_VW_CHAR_NUM) || (item.len <= 5) || (base.len == item.len)) {
		for (size_t n = 0; n <= (base.len - item.len); n++) {
			if (base.data[n] != item.data[0]) continue;
			if (memcmp(base.data + n, item.data, item.len) == 0) {
				if (index) *index = n;
				return true;
			}
		}

		if (index) *index = -1;
		return false;
	}

	size_t bad_char[STR_VW_CHAR_NUM];
	for (size_t n = 0; n < STR_VW_CHAR_NUM; n++) bad_char[n] = item.len;
	for (size_t n = 0; n < item.len - 1; n++) bad_char[(unsigned char)item.data[n]] = item.len - 1 - n;

	size_t shift = 0;
	while (shift <= (base.len - item.len)) {
		size_t n = item.len - 1;

		while (item.data[n] == base.data[shift + n]) {
			if (n == 0) {
				if (index) *index = shift;
				return true;
			}
			n--;
		}

		shift += bad_char[(unsigned char)base.data[shift + item.len - 1]];
	}

	if (index) *index = -1;
	return false;
} /* str_contains() */

/*
 * Function to identify the index within the str struct of the first occurrence
 * of the specified char.
 * Returns -1 if no matching characters are found.
 */

static inline ptrdiff_t index_of(str s, char c) {
	if (str_is_empty(s)) return -1;

	size_t index = 0;
	while ((index < s.len) && (s.data[index] != c)) {
		index++;
	}

	if (index < s.len) return index;
	else return -1;
} /* index_of() */

/*
 * Function to identify the character in a str struct at the given index.
 * Returns -1 if index is out of bounds.
 */

static inline int char_at(str s, size_t index) {
	if ((index >= s.len) || (str_is_empty(s))) return -1;
	return (unsigned char)s.data[index];
} /* char_at() */
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

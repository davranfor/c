/*!
 *  \brief     json and json-schema library for C
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#ifndef JSON_MACROS_H
#define JSON_MACROS_H

#include <ctype.h>

#define is_space(c) isspace((unsigned char)(c))
#define is_cntrl(c) iscntrl((unsigned char)(c))
#define is_digit(c) isdigit((unsigned char)(c))
#define is_alpha(c) isalpha((unsigned char)(c))
#define is_alnum(c) isalnum((unsigned char)(c))
#define is_xdigit(c) isxdigit((unsigned char)(c))
#define is_utf8(c) (((c) & 0xc0) != 0x80)

int json_lookup_valid_char(unsigned char);

#define json_valid_char(c) json_lookup_valid_char((unsigned char)(c))

#endif /* JSON_MACROS_H */


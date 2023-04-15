/*!
 *  \brief     JSON Schema validator
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#ifndef JSON_FORMAT_H
#define JSON_FORMAT_H

int test_is_date(const char *);
int test_is_time(const char *);
int test_is_date_time(const char *);
int test_is_hostname(const char *);
int test_is_email(const char *);
int test_is_ipv4(const char *);
int test_is_ipv6(const char *);
int test_is_uuid(const char *);
int test_is_url(const char *);

int test_regex(const char *, const char *);

#endif /* JSON_FORMAT_H */


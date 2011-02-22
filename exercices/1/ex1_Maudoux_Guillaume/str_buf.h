/* 
 * str_buf -- A dynamic string buffer which can be used for building 
 *            long strings sequentially without a high complexity cost.
 * 
 *            A such buffer has a size, i.e., the capacity of the buffer,
 *            and a length corresponding the lenght of the string which is
 *            currently allocated inside this buffer.
 */
#ifndef __STR_BUF_H__
#define __STR_BUF_H__

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

struct str_buf;

typedef struct str_buf str_buf;

/**
 * str_buf_alloc - Allocate a str_buf of the given size
 * @size: Size for the allocated buffer
 * @return: Buffer to the allocated empty str_buf or %NULL on failure
 */
str_buf * 
str_buf_alloc(size_t size);

/**
 * str_buf_alloc_str - Allocate a str_buf and fill it with str
 *                     (buffer size is equals to the size of str)
 * @str: a string
 * @return: Buffer to the allocated str_buf or %NULL on failure
 */
str_buf * 
str_buf_alloc_str(const char * str);

/**
 * str_buf_alloc_substr - Allocate a str_buf and fill it the first 
 *                        str_len characters of str
 *                        (buffer size is equals to str_len)
 * @str: a string
 * @str_len: the number of byte of str to copy in the buffer
 *           (< str length)
 * @return: Buffer to the allocated str_buf or %NULL on failure
 */
str_buf * 
str_buf_alloc_substr(const char * str, size_t str_len);

/**
 * str_buf_free - Free a str_buf
 * @buf: str_buf buffer
 */
void 
str_buf_free(str_buf * buf);

/**
 * str_buf_head - Get pointer to the head of the string buffer
 * @buf: str_buf buffer
 * @return: Pointer to the head of the string buffer
 */
const char *
str_buf_head(str_buf * buf);

/**
 * str_buf_len - Get the current length of a string buffer 
 *               (see top of the file for more details of lenght/size)
 * @buf: str_buf buffer
 * @return: Current length of the string in the buffer
 */
size_t 
str_buf_len(str_buf * buf);

/**
 * str_buf_size - Get the current size of a string buffer 
 *               (see top of the file for more details of lenght/size)
 * @buf: str_buf buffer
 * @return: size of the buffer
 */
size_t 
str_buf_size(str_buf * buf);

/**
 * str_buf_concat - Concatenate two string buffers
 * @buf1, buf2: str_buf buffers
 * @return: str_buf with concatenated buf1 + buf2 strings or %NULL on failure
 *
 * Input buffers can be %NULL which is interpreted as an empty buffer.
 */
str_buf * 
str_buf_concat(str_buf * buf1, str_buf * buf2);

/**
 * str_buf_equals - Return whether two buffers are equals
 * @buf1, buf2: str_buf buffers (not %NULL)
 * @return: True if the string content of both buffers are
 *           exactly the same
 */
bool
str_buf_equals(str_buf * buf1, str_buf * buf2);

/**
 * str_buf_put_buf - Append the buf2 string content to the buf1 buffer
 * @buf1, buf2: str_buf buffers
 * @return: true if it succeeds, false otherwise
 * 
 * buf2 can be %NULL which is interpreted as an empty buffer. 
 * if the buffer is not large enough, fails without modifying buf1
 */
bool 
str_buf_put_buf(str_buf * buf1, str_buf * buf2);

/**
 * str_buf_put_str - Append a string to the buffer
 * @buf: a str_buf buffer
 * @str: a string
 * @return: Append str to the buf buffer
 *
 * if the buffer is not large enough, fails without modifying buf
 */
bool 
str_buf_put_str(str_buf * buf, const char * str);

/**
 * str_buf_put_str - Append a sub-string to the buffer
 * @buf: a str_buf buffer
 * @str: a string
 * @str_len: the number of byte of str to copy in the buffer
 *           (< str length)
 * @return: Append str to the buf buffer
 * 
 * if the buffer is not large enough, fails without modifying buf
 */
bool
str_buf_put_substr(str_buf * buf, const char * str, size_t str_len);

#endif /* __STR_BUF_H__ */

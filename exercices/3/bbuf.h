
/*
 * bbuf.h
 * ------
 *
 *  Implements a bounded buffer of strings, 
 *  with variable length and capacity.
 *  
 *  This implementation is coherent in multi-processes env.
 */
#ifndef BBUF_H
#define BBUF_H

struct bbuf;

typedef struct bbuf bbuf;

/*
 * Initialize a new buffer with n pointers
 */
int
bbuf_init(bbuf ** buf, size_t string_len, size_t capacity);

/*
 * take one item from the bounded buffer
 *
 * As this is a copy, it's upt to the user to free memory 
 * afther use.
 */
char *
bbuf_get(bbuf * buf);

/*
 * Push one item in the buffer
 */
int
bbuf_put(bbuf * buf, char * ptr);

/*
 * Same as bbuf_put() but with optional length argument.
 */
void
bbuf_put_len(bbuf * buf, char * ptr, size_t len);

/*
 * Notify end of input
 */
void 
bbuf_close(bbuf * buf);

/*
 * Clears all
 */
void
bbuf_destroy(bbuf * buf);


#endif


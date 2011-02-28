/*
 * bounded_buffer.h
 * ----------------
 *
 *  Implements a bounded buffer of variable length an type.
 *
 *  This implementation is thread-safe.
 */
#ifndef BOUNDED_BUFFER_H
#define BOUNDED_BUFFER_H

struct bounded_buffer;

typedef struct bounded_buffer bounded_buffer;

/*
 * Initialize a new buffer with n pointers
 */
int
bounded_buffer_init(bounded_buffer ** buf, unsigned short n);

/*
 * take one item from the bounded buffer
 */
void *
bounded_buffer_get(bounded_buffer * buf);

/*
 * Push one item in the buffer
 */
void
bounded_buffer_put(bounded_buffer * buf, void * ptr);

/*
 * Notify readers that no more data will be added
 */
void
bounded_buffer_close(bounded_buffer * buf);

/*
 * Clears all allocated memory
 */
void
bounded_buffer_destroy(bounded_buffer * buf);

#endif


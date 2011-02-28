/*
 * bounded_buffer.c
 * ----------------
 *
 *  Implements a bounded buffer of variable length an type.
 *
 *  This implementation is thread-safe.
 */

#include <pthread.h>
#include <stdbool.h>
#include <errno.h>
#include <stdlib.h>

#include "bounded_buffer.h"

struct bounded_buffer {
    void ** mem;
    pthread_mutex_t lock;
    unsigned short capacity, size, r_index, w_index;
    pthread_cond_t read, write;
    int eoi;
};


/*
 * Initialize a new buffer with n pointers
 */
int
bounded_buffer_init(bounded_buffer ** buf, unsigned short n)
{
    int ack;
    errno = 0;

    *buf = malloc( sizeof(bounded_buffer) );
    if( *buf == NULL ) goto error;

    ack = pthread_mutex_init(&(*buf)->lock, NULL);
    if( ack != 0 ) goto error_lock;

    ack = pthread_cond_init(&(*buf)->read, NULL);
    if( ack != 0 ) goto error_read;
    ack = pthread_cond_init(&(*buf)->write, NULL);
    if( ack != 0 ) goto error_write;
    
    (*buf)->mem = malloc( n * sizeof(void *) );
    if( (*buf)->mem == NULL ) goto error_mem;

    (*buf)->capacity = n;
    (*buf)->size = 0;
    (*buf)->r_index = 0;
    (*buf)->w_index = 0;
    (*buf)->eoi = 0;

    return 0;

error_mem :
    pthread_cond_destroy(&(*buf)->write);
error_write :
    pthread_cond_destroy(&(*buf)->read);
error_read :
    pthread_mutex_destroy(&(*buf)->lock);
error_lock :
    free(*buf);
error :
    errno = (errno)? ack : errno;
    *buf = NULL;  
    return -1;
}

/*
 * take one item from the bounded buffer
 */
void *
bounded_buffer_get(bounded_buffer * buf)
{
    void * val = NULL;
    /* Lock */
    pthread_mutex_lock(&buf->lock);
    while( buf->size == 0 && !buf->eoi ) 
        pthread_cond_wait(&buf->read, &buf->lock);
    if( buf->eoi ){
        goto unlock;
    }
    /* retrieve */
    val = buf->mem[buf->r_index];
    buf->r_index = (buf->r_index + 1) % buf->capacity;
    buf->size--;
    
    /* Unlock */
unlock :
    pthread_cond_signal(&buf->write);
    pthread_mutex_unlock(&buf->lock);
    
    return val;
}
    
/*
 * Push one item in the buffer
 */
void
bounded_buffer_put(bounded_buffer * buf, void * ptr)
{
    /* lock */
    pthread_mutex_lock(&buf->lock);
    while( buf->size == buf->capacity )
        pthread_cond_wait( &buf->write, &buf->lock );
    
    /* insert */
    buf->mem[buf->w_index] = ptr; 
    buf->w_index = (buf->w_index + 1) % buf->capacity;
    buf->size++;
    
    /* unlock */
    pthread_cond_signal(&buf->read);
    pthread_mutex_unlock(&buf->lock);
}

/*
 * Notify end of input
 */
void
bounded_buffer_close(bounded_buffer * buf)
{
    pthread_mutex_lock(&buf->lock);
    while( buf->size != 0 )
       pthread_cond_wait(&buf->write, &buf->lock);
    buf->eoi = 1;
    pthread_cond_signal(&buf->read);
    pthread_mutex_unlock(&buf->lock);
}
     

/*
 * Clears everything...
 */
void
bounded_buffer_destroy(bounded_buffer * buf)
{
    pthread_cond_destroy(&buf->write);
    pthread_cond_destroy(&buf->read);
    pthread_mutex_destroy(&buf->lock);
    free(buf);
}

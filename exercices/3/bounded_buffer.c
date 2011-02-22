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
    pthread_mutex_t * lock;
    unsigned short capacity, size, index;
    pthread_cond_t * read, * write;
};


/*
 * Initialize a new buffer with n pointers
 */
int  
bounded_buffer_init(bounded_buffer * buf, unsigned short n){
    int ack;
    
    errno = 0;

    buf = malloc( sizeof(bounded_buffer) );
    if( !buf ) goto error;

    ack = pthread_mutex_init(buf->lock, NULL);
    if( !ack ) goto error_lock;

    ack = pthread_cond_init(buf->read, NULL);
    if( !ack ) goto error_read;
    ack = pthread_cond_init(buf->write, NULL);
    if( !ack ) goto error_write;
    
    buf->mem = malloc( n * sizeof(void *) );
    if( !buf->mem ) goto error_mem;

    buf->capacity = n;
    buf->size = 0;
    buf->index = 0;

    return 0;

error_mem :
    pthread_cond_destroy(buf->write);
error_write :
    pthread_cond_destroy(buf->read);
error_read :
    pthread_mutex_destroy(buf->lock);
error_lock :
    free(buf);
error :
    errno = (errno)? ack : errno; 
    return -1;
}

/*
 * take one item from the bounded buffer
 */
void *
bounded_buffer_get(bounded_buffer * buf){
    void * val = 0;
    
    /* Lock */
    pthread_mutex_lock(buf->lock);
    while( buf->size == 0) pthread_cond_wait(buf->read, buf->lock);
    
    /* retrieve */
    val = buf->mem[buf->index];
    buf->index++;
    if ( buf->index > buf->capacity ) buf->index = 0;
    buf->size--;
    
    /* Unlock */
    pthread_cond_signal(buf->write);
    pthread_mutex_unlock(buf->lock);
    
    return val;
}
            
/*
 * Push one item in the buffer
 */
void
bounded_buffer_put(bounded_buffer * buf, void * ptr){
    /* lock */
    pthread_mutex_lock(buf->lock);
    while( buf->size == buf->capacity )
        pthread_cond_wait( buf->write, buf->lock );
    
    /* insert */
    if( buf->index + buf->size >= buf->capacity )
        *(buf->mem + buf->size - buf->capacity) = ptr;
    else
        *(buf->mem + buf->size) = ptr;
    
    buf->size++;

    /* unlock */
    pthread_cond_signal(buf->read);
    pthread_mutex_unlock(buf->lock);
}


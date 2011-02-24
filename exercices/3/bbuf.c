/*
 * bbuf.c
 * ------
 *
 *  Implements a bounded buffer of strings, 
 *  with variable length and capacity.
 *  
 *  This implementation is coherent in multi-processes env.
 */
#define _SVID_SOURCE    /* To be able to use SystemV ID's ? */

#include <sys/types.h>  /* for semaphores  */
#include <sys/ipc.h>    /* idem */
#include <sys/sem.h>    /* idem */
#include <sys/mman.h>   /* idem */
#include <string.h>     /* for strcpy() */
#include <stdlib.h>     /* for malloc() */
#include <stdio.h>      /* for perror() */

#include "bbuf.h"

#define full 0      /* Semaphore ensuring coherent writes */
#define empty 1     /* Semaphore ensuring coherent reads  */
#define read 2      /* Mutex to sync readers */
#define write 3     /* Mutex to sync writers */
#define NB_SEMS 4   /* Allocate NB_SEMS semaphores */

struct bbuf{
    int semid;
    int capacity, string_len;
    int r_index, w_index;
    size_t buf_size;
    char * data;
};

void up( int semid, short sem );
void down( int semid, short sem );

/*
 * Initialize a new buffer with n pointers
 *
 * buf : an uninitialized pointer
 * string_len : max length of strings in buffer, as returned by strlen();
 * capacity : capacity of the buffer
 */
int
bbuf_init(bbuf * buf, size_t string_len, size_t capacity)
{
    size_t size = sizeof(bbuf) + (string_len+1)*capacity*sizeof(char);
    buf = mmap(0, size,
           PROT_READ | PROT_WRITE, 
           MAP_SHARED | MAP_ANONYMOUS,
           -1, 0);
    if ( buf == (bbuf *)-1 ) goto err_mmap;
    
    buf->semid = semget(IPC_PRIVATE, NB_SEMS, IPC_CREAT);
    if (buf->semid < 0) goto err_semget;

    if( semctl(buf->semid, full, IPC_SET, 0) ||
            semctl(buf->semid, empty, IPC_SET, capacity) ||
            semctl(buf->semid, write, IPC_SET, 1) ||
            semctl(buf->semid, read,  IPC_SET, 1) )
        goto err_semctl;
 
    buf->string_len = string_len + 1; /* for null byte */
    buf->capacity = capacity;
    buf->r_index = 0;
    buf->w_index = 0;
    buf->buf_size = size;
    buf->data = (char *) buf + 1;
    
    return 0;

err_semctl:
    semctl(buf->semid, -1, IPC_RMID);
err_semget:
    munmap(buf, buf->buf_size);
err_mmap:
    perror("bbuf init failed. Reason :");
    return -1;
}

/*
 * take one item from the bounded buffer
 *
 * As this is a copy, it's upt to the user to free memory 
 * afther use.
 */
char *
bbuf_get(bbuf * buf)
{
    char * val = malloc( buf->string_len * sizeof(char) );
    
    /* lock */
    down(buf->semid, empty);
    down(buf->semid, read);
    
    /* read */
    strcpy(val, (buf->data + buf->r_index*buf->string_len) );
    buf->r_index = (buf->r_index -1) % buf->capacity;

    /* unlock */
    up(buf->semid, read);
    up(buf->semid, full);

    return val;
}

/*
 * Push one item in the buffer
 */
int 
bbuf_put(bbuf * buf, char * ptr)
{
    /* ensure string is short enough */
    if ( strlen(ptr) >= buf->string_len ) 
        return -1;

    /* lock */
    down(buf->semid, full);
    down(buf->semid, write);

    /* read */
    strcpy( (buf->data + buf->w_index*buf->string_len), ptr);
    buf->w_index = (buf->w_index+1) % buf->capacity;

    /* unlock */
    up(buf->semid, write);
    up(buf->semid, empty);

    return 0;
}    


/*
 * Clears all
 */
void
bbuf_destroy(bbuf * buf)
{
    semctl(buf->semid, -1, IPC_RMID);
    munmap(buf, buf->buf_size);
}

/*
 * Locks a semaphore 
 */
void
down( int semid, short sem )
{
    struct sembuf ops = {0, -1, 0};
    ops.sem_num = sem;
    
    semop(semid, &ops, 1);
}

/*
 * Unlock a semaphore
 */
void
up( int semid, short sem )
{
    struct sembuf ops = {0, 1, 0};
    ops.sem_num = sem;

    semop(semid, &ops, 1);
}



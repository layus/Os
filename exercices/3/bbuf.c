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
#include <sys/mman.h>   /* for shared memory */
#include <string.h>     /* for strcpy() */
#include <stdlib.h>     /* for malloc() */
#include <stdio.h>      /* for perror() */
#include <unistd.h>

#include "bbuf.h"

enum {       /* Attributes an int to each semaphore */
    s_full,         /* Semaphore ensuring coherent writes */
    s_empty,        /* Semaphore ensuring coherent reads  */
    s_read,         /* Mutex to sync readers */
    s_write,        /* Mutex to sync writers */
    NB_SEMS         /* Allocate NB_SEMS semaphores */
};

struct bbuf{
    int semid;
    int capacity, string_len;
    int mem_size;
    volatile int r_index, w_index;
    volatile int eoi; /* End Of Input */
    char * data;
};

int up( int semid, int sem );
int down( int semid, int sem );

/*
 * Initialize a new buffer with n pointers
 *
 * buf : an uninitialized pointer
 * string_len : max length of strings in buffer, as returned by strlen();
 * capacity : capacity of the buffer
 */
int
bbuf_init(bbuf ** buf, size_t string_len, size_t capacity)
{
    size_t size = sizeof(bbuf) + (string_len+1)*capacity*sizeof(char);
    
    *buf = mmap(0, size,
           PROT_READ | PROT_WRITE, 
           MAP_SHARED | MAP_ANONYMOUS,
           -1, 0);
    if ( *buf == (bbuf *)-1 ) goto err_mmap;

    (*buf)->semid = semget(IPC_PRIVATE, NB_SEMS, IPC_CREAT | 0666 );
    if ( (*buf)->semid < 0) goto err_semget;

    if( semctl((*buf)->semid, s_full, SETVAL, capacity) ||
            semctl((*buf)->semid, s_empty, SETVAL, 0) ||
            semctl((*buf)->semid, s_write, SETVAL, 1) ||
            semctl((*buf)->semid, s_read,  SETVAL, 1) )
        goto err_semctl;
 
    (*buf)->string_len = string_len + 1; /* for null byte */
    (*buf)->capacity = capacity;
    (*buf)->mem_size = size;
    (*buf)->r_index = 0;
    (*buf)->w_index = 0;
    (*buf)->eoi = 0;
    (*buf)->data = (char *) (*buf + 1);
    
    return 0;

err_semctl:
    semctl( (*buf)->semid, -1, IPC_RMID);
err_semget:
    munmap( *buf, size);
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
    down(buf->semid, s_empty);
    if ( buf->eoi ) {
        free(val);
        val = NULL;
        goto exit;
    }
    down(buf->semid, s_read);

    /* read */
    strcpy(val, (buf->data + buf->r_index*buf->string_len) );
    buf->r_index = (buf->r_index + 1) % buf->capacity;

    /* unlock */
    up(buf->semid, s_read);
exit:
    up(buf->semid, s_full);

    return val;
}

/*
 * Push one item in the buffer
 */
int 
bbuf_put(bbuf * buf, char * ptr)
{
    if ( buf->eoi ) return -2;
    /* ensure string is short enough */
    if ( strlen(ptr) >= buf->string_len ) 
        return -1;
    
    /* lock */
    down(buf->semid, s_full);
    down(buf->semid, s_write);

    /* read */
    strcpy( (buf->data + buf->w_index*buf->string_len), ptr);
    buf->w_index = (buf->w_index+1) % buf->capacity;

    /* unlock */
    up(buf->semid, s_write);
    up(buf->semid, s_empty);

    return 0;
}    

/*
 * Notyfy end of input
 */
void
bbuf_close(bbuf * buf)
{
    struct sembuf ops = {0,0,0};
    ops.sem_num = s_empty;

    semop( buf->semid, &ops, 1);

    buf->eoi = 1;

    ops.sem_op = 10000;
    semop( buf->semid, &ops, 1); 
}

/*
 * Clears all
 */
void
bbuf_destroy(bbuf * buf)
{
    semctl(buf->semid, -1, IPC_RMID);
    munmap(buf, buf->mem_size);
}

/*
 * Locks a semaphore 
 */
int
down( int semid, int sem )
{
    struct sembuf ops;
    ops.sem_num = sem;
    ops.sem_op = -1;
    ops.sem_flg = 0;
 
    return semop(semid, &ops, 1);
}

/*
 * Unlock a semaphore
 */
int
up( int semid, int sem )
{
    struct sembuf ops;
    ops.sem_num = sem;
    ops.sem_op = 1;
    ops.sem_flg = 0;

    return semop(semid, &ops, 1);
}


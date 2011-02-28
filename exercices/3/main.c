/*
 * main.c
 * ------
 *
 *  author : Guillaume Maudoux <guillaume.maudoux@student.uclouvain.be>
 *
 *  This file describes program ZIPCRACK, which attempts to find the
 *  password for a zip archive using dictionnary attack.
 *
 *  The code is split into 3 parts
 *
 *  ## Main ##
 *  - User interaction
 *  - Configuration ~ options
 *
 *  ## Cracking w/ threads ##
 *
 *  ## Cracking with processes ##
 *
 *  Most of the complexity comes from using both threads and processes 
 *  in the same program.
 */

#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <pthread.h>

#include "zip_crack.h"
#include "bounded_buffer.h"
#include "bbuf.h"

struct {
    bounded_buffer * t;
    bbuf * p;
} buf;

struct zip_archive * archive;
char * zip_file;
FILE * dict;

volatile bool finished = false;

size_t pass_len = 0;
size_t buff_len = 0;
#define DEF_BUF_SIZE 20
#define DEF_WORD_LEN 50 

void
usage (void)
{
    printf("zipcrack [-t|-p N] [-s buf_size] [-l max_pswd_len]\n"
           "         [-d dict_file] ZIP_FILE\n"
           "\n"
           " if -d is not specified,"
                " dictionary file defaults to ./dict.txt \n"
           " if none of -p or -t is specified, defaults to -t 1\n"
           "\n"
           " ALL SIZES SHOULD BE POSITIVE INTEGERS\n" );
    exit(1);
}

char * crack_zip_threads(unsigned int );
char * crack_zip_processes(unsigned int );

/****************************** MAIN ***************************************/

int 
main (int argc, char * const * argv)
{
    int c;
    char * dict_path = "dico.txt";
    int nb_t = 0, nb_p = 0;
    char * passwd;

    /* GETOPT */
    while ((c = getopt(argc, argv, "hp:t:d:s:l:")) != -1)
        switch (c)
        {
            case 'p':
                if( (nb_p = atoi(optarg)) <=0 || nb_t ) 
                    usage();
                break;
            case 't':
                if( (nb_t = atoi(optarg)) <= 0 || nb_p )
                       usage();
                break;
            case 'd':
                dict_path = optarg;
                break;
            case 's':
                buff_len = atoi(optarg);
                break;
            case 'l':
                pass_len = atoi(optarg);
                break;
            case 'h':
                usage();
                break;
            case '?':
                if (optopt == 'p' || optopt == 't' || optopt == 'd' ||
                        optopt == 's' || optopt == 'l' )
                    fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                else if (isprint (optopt))
                    fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                else
                    fprintf(stderr,
                            "Unknown option character `\\x%x'.\n",
                            optopt);
                return 1;
            default:
                usage();
        }
    
    /* MORE OPTS PARSING */    
    if( optind == argc ){  /* no more args, but zipfile needed */
        usage(); 
    }

    /* ensure coherent size */
    buff_len = (buff_len >0)? buff_len : DEF_BUF_SIZE;
    pass_len = (pass_len >0)? pass_len : DEF_WORD_LEN;
    
    /* OPEN & INIT FILES */
    if ( !(dict = fopen(dict_path,"r")) ){
        fprintf(stderr,"Error on opening file %s. ", dict_path);
        perror("Reason : ");
        return 3;
    }
    
    /* CRACK EACH FILE */
    for( c = optind; c < argc; c++){
        /* open archive */
        if ( (archive = zip_load_archive(argv[c]) ) == NULL ) {
            printf("Unable to open archive %s\n", argv[ c ]);
            return 2;
        }

        /* use apropriate method to chrack */
        if ( nb_t > 0 ) {
            passwd = crack_zip_threads(nb_t);
        } else if( nb_p > 0) {
            passwd = crack_zip_processes(nb_p);
        } else {
            passwd = crack_zip_threads(2);
        }
        
        /* check result */
        printf("File %s :\n", argv[c]);
        if ( passwd == NULL ) {
            puts("  Password not found.");
        } else {
            printf("  Password is %s.\n", passwd);
            free(passwd);
            passwd = NULL;
        }
        fflush(stdout); /* funny !, otherwise each process flushes on exit  */
                        /* try to remove it to see !                        */

        /* reset globals for next use */
        finished = 0;
        zip_close_archive(archive);
        archive = NULL;
        fseek(dict, 0, SEEK_SET);
    }

    return 0;
}

/***************************** THREADS *****************************/
/*
 * Common use of pthreads.
 *
 * We use a producer-consumer approach.
 * So we find three functions :
 * - thread_read() : consumer
 * - thread_write() : producer
 * - crack_zip_threads() : gathers all threaded cracking logic.
 *
 * Using home-made lib bounded_buffer.h. 
 * More details in bounded_buffer.c
 */

void * 
thread_read(void* arg){
    char * passwd = NULL;
    
    while( (passwd = bounded_buffer_get(buf.t) ) != NULL ){
        if( zip_test_password(archive, passwd) == 0 ){
            finished = 1;
            break;
        }
        free(passwd);
        passwd = NULL;
    }

    return passwd;
}

void 
thread_write()
{
    char * word, *ptr;
    while( !finished ){
        word = malloc( pass_len*sizeof(char) );

        if( fgets(word, pass_len, dict) == NULL ) {

            bounded_buffer_close(buf.t);
            free(word);
            break;
        }

        if( (ptr = strchr(word, '\n')) != NULL){
            *ptr = '\0';
        } else { /* error, too long line */
            fprintf(stderr,"Too long line,skipping. :\n %s\n",word);
        } 

        bounded_buffer_put(buf.t, word);
    }
    bounded_buffer_close(buf.t);
}

char *
crack_zip_threads(unsigned int nb_threads)
{
    int i, ack;
    pthread_t * threads;
    void * out;
    char * return_val = NULL;
    
    if( bounded_buffer_init(&buf.t,buff_len) < 0){
        perror("Unable to init bounded buffer.\nReason :");
        return NULL;
    }

    threads = malloc( nb_threads * sizeof(pthread_t));

    for ( i=0; i< nb_threads; i++){
        ack = pthread_create(&threads[i],NULL,&thread_read,NULL);
        if( ack ) { i++; goto exit; }
    }

    thread_write();

exit:
    for(i-- ; i >= 0; i-- ){
        pthread_join(threads[i], &out);
        if ( out != NULL ){
            return_val = out;
        }
    }

    bounded_buffer_destroy(buf.t);
    return return_val;
}

/****************************** PROCESSES *********************************/
/*
 * Same in concepts as THEADS.
 *
 * This section requires more precisions as I used, on top of the bounded
 * buffer to others ipc's :
 *
 * - signals : to handle termination before end of dictionnary file.
 *
 * - pipes : to return correct password from sub-process to main prog.
 *
 * This complication comes from the fact that i wanted to keep
 * bbuf.c as general as possible.
 *
 * As with threads, more information in bbuf.{h,c}.
 */

int
process_read(int pipe){
    char * passwd = NULL;
    int found = 0;

    while( (passwd = bbuf_get(buf.p)) != NULL && !found )
    {
        if( zip_test_password(archive, passwd) == 0 )
        {
            if( write(pipe, passwd, pass_len) == -1) 
                printf("Error in writing. Pass was %s.\n",passwd);
            kill(getppid(), SIGUSR1);
            found = 1;
        }
        free(passwd);
        passwd = NULL;
    }

    return found;
}

void
process_write()
{
    char * word, *ptr;
    word = malloc( pass_len*sizeof(char) );

    while( !finished ){
        if( fgets(word, pass_len, dict) == NULL ){
            break;
        }
        
        if( (ptr = strchr(word, '\n')) != NULL){
            *ptr = '\0';
        } else { /* error, too long line */
            fprintf(stderr,"Too long line,skipping. :\n %s\n",word);
            continue;
        } 

        bbuf_put(buf.p, word);
    }

    bbuf_close(buf.p);
    free(word);
}

static void
sigusr1_handler( int signal )
{
    if ( signal == SIGUSR1 ) {
        finished = 1;
    }
}

char *
crack_zip_processes(unsigned int nb_proc )
{
    int i;
    pid_t * pids;
    int out;
    char * return_val = NULL;
    int fd[2];
    struct sigaction act, *old_act = NULL;

    if( bbuf_init(&buf.p,pass_len, buff_len) < 0){
        perror("Unable to init bounded buffer.\nReason :");
        return (char *) -1;
    }
    
    pipe(fd);
    pids = malloc( nb_proc * sizeof(pid_t));
    
    /* set SIGUSR1 action */
    act.sa_handler = &sigusr1_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGUSR1, &act, old_act);

    for ( i=0; i< nb_proc; i++){
        
        if( (pids[i] = fork()) < 0) {        /* BEGIN subprocess life */
            i++;
            goto exit;
        }

        if( pids[i] == 0){
            close(fd[0]);
            exit(process_read(fd[1]));      /* END subprocess life */
        }
    }

    process_write();

exit:

    for( i--; i >= 0; i-- ){
        kill(pids[i],SIGUSR1);
        waitpid(pids[i], &out, 0);
        if ( WIFEXITED(out) && WEXITSTATUS(out) == 1 ){
            return_val = malloc( pass_len*sizeof(char) );
            if( read(fd[0], return_val, pass_len) == -1 )
                perror("Unable to read pipe output !");
        }
    }

    close(fd[1]);
    close(fd[0]);

    /* restore signals */
    sigaction(SIGUSR1, old_act, NULL);
    
    bbuf_destroy(buf.p);
    return return_val;
}



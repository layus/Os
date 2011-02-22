#define _POSIX_SOURCE 1        /* Needed because kill() is NOT ansi compliant */

#include "unrar.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <getopt.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>

#define N 6                    /* Max psswd size */

int finished = 0;              /* Global flag for thread termination */

typedef struct {               /* Args structure for thread call */
        int id;
        char first;
        char second;
        const char* file;
} args_t;

/* Seen on Stack-Overflow.
 * Credit to Jules Olléon
 * http://stackoverflow.com/questions/4764608/generate-all-strings-under-length-n-in-c
 */
int inc(char *c){
        if(c[0]==0) return 0;
        if(c[0]=='z'){
                c[0]='a';
                return inc(c+1);
        }
        c[0]++;
        return 1;
}

void *
crack_thread(void * v_args)
{
        int i,j;	/* Loop iterators                       */
        char c;         /* psswd iterator                       */
        char *psswd;    /* a string w/ current psswd            */
        args_t * args;  /* stores args with correct type        */
        
        args = (args_t*) v_args;
        psswd = malloc( (N+1)*sizeof(char) );

        /* Test short passwords first */
        for(i=1;i<=N;i++){ 
                
                /* Iterate over our range */  
                for(c = args->first; c <= args->second; c++){
                        
                        /* init psswd string */
                        psswd[0]=c;
                        for(j=1;j<i;j++) psswd[j]='a'; psswd[i]=0;	

                        do {
                                printf("\rTesting \"%s\"",psswd);
                                fflush(stdout);
                                if( unrar_test_password(args->file, psswd)==0) {
                                        finished = 1;
                                        return psswd;
                                }
                                /* check global termination flag */
                                if( finished ) goto exit;
                                
                        } while(inc(psswd+1)); /* do not inc() first letter */
                }
        }
exit :
        free( psswd );
        return NULL;
}

int 
process_crack(char *file,  int nb){

        int p_out, i; 
        pid_t *pids, pid; 
        void * out;
        args_t arg;
       
        pids = malloc(nb*sizeof(pid_t));
        arg.file = file;
        for( i = 0; i<nb; i++){ 
                arg.first =  'a' + ('z'-'a'+1)*(i)  /nb;
                arg.second = 'a' + ('z'-'a'+1)*(i+1)/nb-1;
                arg.id = i;
                
                if( (pids[i]=fork()) == 0 ) {
                        out = crack_thread((void *)&arg);
                        if ( out != NULL ){
                                printf("\nPassword is: %s\n",(char*)out);
                                exit(0); /* return sucess */
                        }
                        exit(1); /* 1 : pass not found */
                } 
        }

        for( i = 0; i<nb; i++ ){
                pid = wait(&p_out);
                if ( pid == (pid_t)-1 ) {
                        perror("On thread completion...");
                       break;
                }
                if ( p_out == 0 ){
                        finished = 1;
                        break;
                }
        }
        for( i = 0; i<nb; i++ ){
                kill(pids[i], SIGQUIT);
        }
              
        free(pids);
        
        return 0;
}

int
thread_crack(char* file, int nb){

        int i,j, ok;
        void * out = (void*) NULL;
        args_t *args;
        pthread_t *threads;

        args=malloc(nb*sizeof(args_t));
        threads=malloc( nb*sizeof(pthread_t) );

        for( i = 0; i<nb; i++){
                args[i].first =  'a' + ('z'-'a'+1)*(i)  /nb;
                args[i].second = 'a' + ('z'-'a'+1)*(i+1)/nb-1;
                args[i].file = file;
                args[i].id = i;
                ok = pthread_create(&threads[i],NULL,crack_thread,
                                    (void *) &args[i]);
                if( ok ) return 1;
        }

        for( j = 0; j<i; j++){
                pthread_join(threads[j],&out);
                if( out != NULL ){
                        printf("\nPassword is: %s\n",(char*)out);
                        free(out);
                }
        }

        free(threads);
        free(args);
        
        return 0;
}

int
crack_psswd(char *file, char type, int nb)
{
        int ack = -1;
        
        nb = (nb > 26)? 26 : nb;
        nb = (nb <= 0)? 1 : nb;
        
        if ( type == 't' ) {
                printf("cracking with %d threads\n", nb);
                ack = thread_crack(file, nb);
        } 
        else if ( type == 'p' ){
                printf("cracking with %d processes\n", nb);
                ack = process_crack(file, nb);
        }
        
        return ack;
}

void
usage(void)
{
        printf("rarcrack [-p <N>|-t <N>] RAR_FILE [PASS1 [PASS2 [...]]]\n");
}

int
test_psswd_list(int optind, int argc, char * argv[])
{
        
        int file_index = optind;
        int i;
        
        printf("Testing password list\n");

        for (i = optind+1; i <= argc; i++) {
                if(unrar_test_password(argv[file_index], argv[i]) == 0) {
                        printf("\nPassword is: %s\n", argv[i]);
                        return 1;
                }
        }
        return 0;
}


int 
main (int argc, char * argv[] )
{
        int nb = 0;
        char * file;
        int opt;

        if (argc < 2) {
                usage(); return 1;
        }

        opt=getopt(argc, argv, "t:p:");
        if ( optarg ) nb = atoi(optarg);
        if ( opt == '?' || getopt(argc, argv, "t:p:") != -1 ) {
                usage(); return -1;
        }

        file = argv[optind];

        if( optind+1 < argc 
           && test_psswd_list( optind, argc, argv) == 1) {
                return 0;
        }

        if ( crack_psswd(file,opt,nb) == 0 ) {
                return 0;
        }
       
        printf("\nPassword not found\n");

        return 0;
}


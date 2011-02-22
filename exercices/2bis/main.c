#include "unrar.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <strings.h>
#include <getopt.h>

static int 
_unrar_test_password_callback(unsigned int msg, long data, long P1, long P2)
{
        switch(msg) {
                case UCM_NEEDPASSWORD:
                        *(int *)data = 1;
                        break;
                default:
                        return 0;
        }
        return 0;
}


static int
unrar_test_password(const char * file, const char * pwd) 
{
        void                      * unrar;
        struct RARHeaderData        headerdata;
        struct RAROpenArchiveData   archivedata;
        int                         password_incorrect;

        password_incorrect = 0;
        memset(&headerdata, 0, sizeof(headerdata));
        memset(&archivedata, 0, sizeof(archivedata));
        archivedata.ArcName  = (char *) file;
        archivedata.OpenMode = RAR_OM_EXTRACT;

        unrar = RAROpenArchive(&archivedata);
        if (!unrar || archivedata.OpenResult)
                return -2;

        RARSetPassword(unrar, (char *) pwd);

        RARSetCallback(unrar, _unrar_test_password_callback, (long) &password_incorrect);

        if (RARReadHeader(unrar, &headerdata) != 0)
                goto error;

        if (RARProcessFile(unrar, RAR_TEST, NULL, NULL) != 0)
                goto error;

        if (password_incorrect)
                goto error;

        RARCloseArchive(unrar);
        return 0;

error:
        RARCloseArchive(unrar);
        return -1;
}

void
usage(void)
{
        printf("rarcrack [-p <N>|-t <N>] RAR_FILE [PASS1 [PASS2 [...]]]\n");
}

int crack_psswd(char *file, char type, int nb);
int test_psswd_list(int optind, int argc, char * argv[]);

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

        if( optind < argc 
           && test_psswd_list( optind, argc, argv) == 1) {
                return 0;
        }

        if ( crack_psswd(file,opt,nb) == 0 ) {
                return 0;
        }
       
        printf("Password not found\n");

        return 0;
}

int
test_psswd_list(int optind, int argc, char * argv[])
{
        
        int file_index = optind;
        int i;
        
        printf("Testing password list\n");

        for (i = optind+1; i <= argc; i++) {
                if(unrar_test_password(argv[file_index], argv[i]) == 0) {
                        printf("Password is: %s\n", argv[i]);
                        return 1;
                }
        }
        return 0;
}


#define N 6   
/* Max psswd size */ 

typedef struct {
        char first;
        char second;
        const char* file;
} args_t;

void * crack_thread(void * v_args);

int finished = 0;

int
crack_psswd(char *file, char type, int nb)
{
        /* char * psswd; */
        int i, ok;
        /*int p_out; */
        /*pid_t *pids, pid; */
        void * out;
        args_t *args;
        pthread_t *threads;
        pthread_attr_t pthread_custom_attr;

        
        printf("cracking with %d threads\n", nb);
        
        if ( type == 't' ) {
                nb = (nb > 26)? 26 : nb;
                nb = (nb <= 0)? 1 : nb;

                args=malloc(nb*sizeof(args_t));
                threads=malloc( nb*sizeof(pthread_t) );
                pthread_attr_init(&pthread_custom_attr);
                
                for( i = 0; i<nb; i++){
                        args[i].first =  'a' + ('z'-'a'+1)*(i)  /nb;
                        args[i].second = 'a' + ('z'-'a'+1)*(i+1)/nb-1;
                        args[i].file = file;
                        ok = pthread_create(&threads[i],&pthread_custom_attr,
                                               crack_thread, (void *) &args[i]);
                        if( ok ) return 1;
                }

                for( i = 0; i<nb; i++){
                        pthread_join(threads[i],&out);
                        if( out != NULL ){
                                printf("Password is: %s\n",(char*)out);
                                free(out);
                        }
                }

                free(threads);
                free(args);
        }
        /*
        if ( type == 'p' ){
                pids = malloc(nb*sizeof(pid_t));
                for( i = 0; i<nb; i++){ 
                        if( (pids[i]=fork()) == 0 ) {
                                crack_thread('a' + ('z'-'a'+1)*(i)  /nb,
                                             'a' + ('z'-'a'+1)*(i+1)/nb-1,
                                             file);
                        }
                }

                for(
                pid = wait(&p_out);
                if ( pid = (pid_t)-1 ) {
                        perror("On thread completion");
                }
                
                
                        
                                
                free(pids);
        }
        //*/
        return 0;
}


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
        int i,j;					/* Loop iterators */
        char c;             			/* psswd iterator */
        char *psswd = malloc( (N+1)*sizeof(char) ); /* a string w/ current psswd */
        args_t * args = (args_t*) v_args;

        for(i=1;i<=N;i++){
                printf("%d\n",i);
                for(c = args->first; c <= args->second; c++){
                        psswd[0]=c;

                        for(j=1;j<i;j++) psswd[j]='a'; psswd[i]=0;	/* init string of size i */

                        do {
                                if( unrar_test_password(args->file, psswd) == 0) {
                                        finished = 1;
                                        return psswd;
                                }

                                if( finished ) goto exit;
                        } while(inc(psswd+1)); /* we just need to inc from psswd[1] */
                }
        }
exit :
        free( psswd );
        return NULL;
}






#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>
#include <stdlib.h>
#include <ctype.h>

#include "zip_crack.h"
#include "bounded_buffer.h"
#include "bbuf.h"

bounded_buffer *buf;
struct zip_archive * archive;
volatile bool finished = false;
int nb_t = 0, nb_p = 0;
char * dict;

#define BUF_SIZE 20
#define WORD_LEN 50 /* In fact dict.txt contains max 24-car. words */

void
usage (void)
{
    printf("zipcrack [-t|-p N] [-s buf_size] [-l max_pswd_len]\n"
           "         -d dict_file ZIP_FILE\n");
}

void crack_zip(unsigned int nb_threads);

int 
main (int argc, char * const * argv)
{
    int c;

    /* GETOPT */
    while ((c = getopt(argc, argv, "p:t:d:")) != -1)
        switch (c)
        {
            case 'p':
                nb_p = atoi(optarg);
                if(nb_t) usage();
                break;
            case 't':
                nb_t = atoi(optarg);
                if(nb_p) usage();
                break;
            case 'd':
                dict = optarg;
                break;
            case '?':
                if (optopt == 'd')
                    fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                else if (isprint (optopt))
                    fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                else
                    fprintf(stderr,
                            "Unknown option character `\\x%x'.\n",
                            optopt);
                return 1;
            default:
                return 1;
        }
        
    if ( (archive = zip_load_archive(argv[optind])) == NULL) {
        printf("Unable to open archive %s\n", argv[1]);
        return 2;
    }
    
    if ( !bounded_buffer_init(buf,BUF_SIZE) ){
        perror("Error while creating shared buffer. Reason :");
        goto finish;
    }

    crack_zip(5);

    printf("Password not found\n");

finish:   
    zip_close_archive(archive);
    return 0;
}

void
crack_zip(unsigned int nb_threads){

}



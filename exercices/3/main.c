#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "zip_crack.h"
#include "bounded_buffer.h"

bounded_buffer *bbuf;
struct zip_archive * archive;
volatile bool finished = false;

#define BUF_SIZE 20
#define WORD_LEN 50 /* In fact dict.txt contains max 24-car. words */

void
usage (void)
{
    printf("zipcrack ZIP_FILE [-t N] [-d dict_file] [PASS1 [PASS2 ...]]\n");
}

void crack_zip(unsigned int nb_threads);

int 
main (int argc, char const * argv[])
{
    int i;
    /* FILE * dict; */

    if (argc < 2) {
        usage();
        return 1;
    }

    if ( (archive = zip_load_archive(argv[1])) == NULL) {
        printf("Unable to open archive %s\n", argv[1]);
        return 2;
    }
    
    if ( bounded_buffer_init(bbuf,BUF_SIZE) ){
        puts("Error while creating shared buffer");
        return 3;
    }

    for (i = 2; i < argc; i++) {
        if(zip_test_password(archive, argv[i]) == 0) {
            printf("Password is: %s\n", argv[i]);
            goto finish;
        }
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



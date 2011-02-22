#!/usr/bin/tcc

#include <stdio.h>
#include <unistd.h>

void
main()
{
    int i=0;

    while( 1 ) {
        sleep(1);
        printf("%d\n",i++);
        if ( fork() != 0 ){
            wait();
            break;
        }
    }

    return 0;
}


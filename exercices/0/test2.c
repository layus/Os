
#include <stdio.h>

void
main()
{
    int i=0;

    while( 1 ) {
        printf("%d\n",i++);
        if ( fork() != 0 ){
            wait();
            break;
        }
    }    
}


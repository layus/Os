#include <stdlib.h>
#include <stdio.h>
void rec();
    
void
main()
{
    rec();
}

void 
rec(int i)
{
    char * tab = malloc( 1024 * 1024 * 16 );
    if ( tab ) tab[0]=1;
    printf("%d\n",i+1);
    rec(i+1);
}


#include "str_buf.h"

#include <assert.h>
#include <stdio.h>

void 
test_str_buf_concat() 
{
    str_buf * b1;
    str_buf * b2;
    str_buf * b3;
    str_buf * b4;
    str_buf * b5;
    str_buf * b6;
    str_buf * b7;
		
	b1 = str_buf_alloc(10);
	b2 = str_buf_alloc_str("abc");
	b3 = str_buf_alloc_str("wxyz");

	b4 = str_buf_concat(b1, b2);
	assert(str_buf_len(b4) == 3);
	assert(str_buf_equals(b4, b2));
	
	b5 = str_buf_concat(b2, b1);
	assert(str_buf_equals(b5, b2));
	assert(str_buf_len(b5) == 3);
	
	b6 = str_buf_concat(b4, b3);
	b7 = str_buf_alloc_str("abcwxyz");
	assert(str_buf_equals(b6, b7));
	
	str_buf_free(b1);
	str_buf_free(b2);
	str_buf_free(b3);
	str_buf_free(b4);
	str_buf_free(b5);
	str_buf_free(b6);
	str_buf_free(b7);	
}

/* --( str_buf_alloc )-------------------------------- */
/* Does it mean someting ?
 * One test could check inner structure representation 
 */
void
test_str_buf_alloc()
{
    str_buf * b1;
    str_buf * b2;
    str_buf * b3;

    b1 = str_buf_alloc(15);
    assert(b1 != NULL);
    assert(str_buf_head(b1) != NULL);
    assert(str_buf_len(b1) == 0);
    assert(str_buf_size(b1)==15);

    b2 = str_buf_alloc(0);
    assert( b2 != NULL);
    assert( str_buf_equals(b1,b2) );

    b3 = str_buf_alloc(-15);    /* kinda malloc overflow... -15 = 0xF...F00 */
    assert( b3 == NULL );

    str_buf_free(b1);
    str_buf_free(b2);
//  str_buf_free(b3);           /* not needed, because of allocation error*/
}

void
test_str_buf_alloc_substr()
{
    str_buf * b1;
    str_buf * b2;

    b1 = str_buf_alloc_substr("coucou",3);
    assert( str_buf_size(b1) == 3);
    assert( str_buf_len(b1) == 3);

    b2 = str_buf_alloc_str("cou");
    assert( str_buf_equals(b1,b2));
    
    /* we do not check %NULL strings, specs said should not happen */

    str_buf_free(b1);
    str_buf_free(b2);
}

void 
test_str_buf_alloc_str()
{
    str_buf* b1;

    b1 = str_buf_alloc_str( "hello world !" );
    assert( str_buf_len(b1) == 13);

    str_buf_free( b1);
}

void 
test_str_buf_put()
{
    str_buf *b1, *b2, *b3;
    // allocation statique de donnees de test
    const char str[] = "marsus des forets tropicales americaines";

    b1 = str_buf_alloc(20);
    b2 = str_buf_alloc_str("bonjour ");
    b3 = str_buf_alloc_str( str );

    str_buf_put_buf( b1, b2);
    assert( str_buf_equals(b1,b2) );

    // Overfull test 
    assert( !str_buf_put_buf(b1, b3)); 
    assert( str_buf_equals(b1, b2) );

    // put null buff
    assert( str_buf_put_buf( b2, NULL) );
    assert( str_buf_equals( b1, b2) );   
    assert( str_buf_size(b2) == 8 );
    assert( str_buf_len(b2) == 8 );

    // put str
    assert( str_buf_put_str( b1, "aux ") );
    assert( !strcmp( str_buf_head(b1), "bonjour aux ") );
    assert( str_buf_size(b1) == 20 );
    assert( str_buf_len(b1) == 12 );

    // Overfull failure
    assert( !str_buf_put_substr(b1, str, 18) );
    
    // check inner code of put_substr
    assert(  str_buf_put_substr(b1, str, 6) );
    assert( !strcmp( str_buf_head(b1), "bonjour aux marsus") );
    assert( str_buf_size(b1) == 20 );
    assert( str_buf_len(b1) == 18 );

    str_buf_free(b1);
    str_buf_free(b2);
    str_buf_free(b3);  
}

int main (int argc, char const *argv[])
{
	test_str_buf_alloc();
	test_str_buf_concat();
    test_str_buf_alloc_substr();
    test_str_buf_alloc_str();
    test_str_buf_put();

	printf("Tests have succeeded\n");
    return 0;
}

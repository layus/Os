#include "str_buf.h"

struct str_buf {
    char * data;
    size_t size;
    size_t length;
};

str_buf *
str_buf_alloc(size_t size)
{
    str_buf * buf;
    char * data;
   
    buf = malloc( sizeof(str_buf) );
    if ( buf == NULL ) return NULL;

    data = malloc( (size+1) * sizeof(char) );
    if ( data == NULL ) {
        free( buf );
        return NULL;
    }
    data[0]=0;

    buf->data = data;
    buf->size = size;
    buf->length = 0;
    
    return buf;
}

str_buf *
str_buf_alloc_str(const char * str)
{
    return  str_buf_alloc_substr( str, strlen(str) );   
}

str_buf *
str_buf_alloc_substr(const char * str, size_t str_len)
{
    str_buf * buf;

    buf = str_buf_alloc( str_len );
    if ( buf == NULL ) return NULL;

    strncpy(buf->data, str, str_len);
    buf->length = buf->size;

    return buf;
}

void 
str_buf_free(str_buf * buf)
{
    free( buf->data );
    free( buf );
}

const char *
str_buf_head(str_buf * buf)
{
    return buf->data;
}

size_t
str_buf_len(str_buf * buf)
{
    return buf->length;
}

size_t
str_buf_size(str_buf * buf)
{
    return buf->size;
}

str_buf *
str_buf_concat(str_buf * buf1, str_buf * buf2)
{
    str_buf * buf;

    if ( buf1 == NULL ) {
        if ( buf2 == NULL ) 
            buf = str_buf_alloc(0);
        else
            buf = str_buf_alloc_substr(buf2->data, buf2->length);
    } 
    else if ( buf2 == NULL ) {
        buf = str_buf_alloc_substr(buf1->data, buf1->length);
    }
    else {
        buf = str_buf_alloc( buf1->length + buf2->length );

        if ( buf == NULL ) return NULL ;

        strcpy(buf->data, buf1->data);
        strcpy(buf->data + buf1->length, buf2->data);
        buf->length = buf->size;
    }

    return buf;
}

bool
str_buf_equals(str_buf * buf1, str_buf * buf2)
{
    return (buf1->length == buf2->length) && !strcmp(buf1->data, buf2->data);
}

bool
str_buf_put_buf(str_buf * buf1, str_buf * buf2)
{
    if ( buf2 == NULL )
        return true;
    
    if ( buf1->size < buf1->length+buf2->length ) 
        return false;

    strcpy( &buf1->data[buf1->length], buf2->data );
    buf1->length += buf2->length;

    return true;
}

bool
str_buf_put_str(str_buf * buf, const char * str)
{
    size_t len = 0;

    len = strlen( str );

    if ( len + buf->length > buf->size )
        return false;
    
    strcpy( &buf->data[buf->length], str);
    buf->length += len;

    return true;
}

bool
str_buf_put_substr(str_buf * buf, const char * str, size_t str_len)
{
    if ( str_len + buf->length > buf->size )
        return false;

    strncpy( &buf->data[buf->length], str, str_len);
    buf->length += str_len;

    return true;
}




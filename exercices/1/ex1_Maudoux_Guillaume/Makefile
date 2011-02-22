

CC=gcc
AR=ar
CFLAGS=-Wall -Werror -ggdb 
DEPS= str_buf.h

.PHONY: clean test tests

libstr_buf.a: str_buf.o
	$(AR) rcs libstr_buf.a str_buf.o

test: tests
tests: testing
	./testing

testing: testing.o libstr_buf.a
	$(CC) $(CFLAGS) -static testing.o -L. -lstr_buf -o testing 

%.o: %.c 
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o testing *.a


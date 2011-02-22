CFLAGS+=-Wall -Werror -ansi -pedantic -ggdb
LDFLAGS+=-lunrar -lpthread -L.
EXEC=rarcrack
SOURCES=$(wildcard *.c)
OBJECTS=$(SOURCES:.c=.o)
HEADERS=$(wildcard *.h)


all: $(EXEC)

$(EXEC): $(OBJECTS) libunrar.so
	@echo $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c -o $@ $< 
	
unrarsrc-4.0.6.tar.gz :
	wget "http://www.rarlab.com/rar/unrarsrc-4.0.6.tar.gz"

libunrar.so: unrarsrc-4.0.6.tar.gz
	tar xvzf unrarsrc-4.0.6.tar.gz
	cd unrar/ && $(MAKE) clean lib -f makefile.unix
	cp unrar/libunrar.so .

.PHONY: clean mrproper

clean:
	@rm $(OBJECTS)
	@$(MAKE) -f makefile.unix -C unrar clean

mrproper:
	@rm $(EXEC)

run : $(EXEC) libunrar.so
	@echo "./$(EXEC) $(ARGS)"
	@LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH ./$(EXEC) $(ARGS)

CFLAGS+=-Wall -Werror -ggdb -pedantic
LDFLAGS+=-lpthread
EXEC=zipcrack
SOURCES=$(wildcard *.c)
OBJECTS=$(SOURCES:.c=.o)
CC=gcc

all: $(EXEC)

$(EXEC): $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS)

-include $(SOURCES:.c=.d)

%.o: %.c
	$(CC) $(CFLAGS) -MMD -c -o $@ $<

.PHONY: clean mrproper

clean:
	@rm -f $(OBJECTS) $(OBJECTS:.o=.d)

mrproper:
	@rm $(EXEC)




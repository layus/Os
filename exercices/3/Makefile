CFLAGS+=-Wall -Werror -ansi -pedantic
LDFLAGS+=-lpthread
EXEC=zipcrack
DEPDIR=.deps
df = $(DEPDIR)/$(*F)
SOURCES=$(wildcard *.c)
OBJECTS=$(SOURCES:.c=.o)
CC=gcc

all: $(EXEC)

$(EXEC): $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS)

-include $(SRCS:%.c=$(DEPDIR)/%.P)

%.o: %.c
	$(CC) $(CFLAGS) -MD -c -o $@ $<
	cp $*.d $(df).P; \
		sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
			-e '/^$$/ d' -e 's/$$/ :/' < $*.d >> $(df).P; \
		rm -f $*.d

.PHONY: clean mrproper

clean:
	@rm $(OBJECTS)

mrproper:
	@rm $(EXEC)




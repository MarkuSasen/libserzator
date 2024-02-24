ZDIR := $(shell pwd)
EXAMPLE-SOURCES := ${ZDIR}/examples/event.c

INCLUDES := -I$(ZDIR)
CFLAGS := -g2 #-DDEBUG

all: 
	gcc ${CFLAGS} ${INCLUDES} -o event-example $(EXAMPLE-SOURCES)
clean:
	rm event-example
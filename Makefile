CFLAGS=-g -Wall -O3

APPLICATIONS=streamread

all: 	$(APPLICATIONS)

%: 	%.c
	gcc  $(CFLAGS) $@.c -o $@

clean:
	rm -f *~ $(APPLICATIONS)

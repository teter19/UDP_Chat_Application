CC=gcc
CFLAGS=-I. -lsctp 
#CFLAGS=

OBJECTS = Client_ipv4 Server_ipv4 Server_ipv4_daemon


all: $(OBJECTS)

$(OBJECTS):%:%.c
	@echo Compiling $<  to  $@
	$(CC) -o $@ $< $(CFLAGS)

	
clean:
	rm  $(OBJECTS) 

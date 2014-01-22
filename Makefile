CC=gcc
CFLAGS=-c -Wall -O2 -Wno-deprecated-declarations
LDFLAGS=-lcrypto -lssl -O2
SOURCES=main.c tls.c client.c crypto.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=qhac_server

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@
# **********************************************************************
# *                             VMC96 API                              *
# **********************************************************************

SOURCES=vmc96cli.c vmc96api.c

EXECUTABLE=vmc96cli

OUTPUTDIR=./bin

CC=gcc
LDFLAGS= -lrt -lftdi1
CFLAGS=
INCPATH= -I. -I/usr/include

ifeq ($(DEBUG),1)
    DEFINES+= -D_DEBUG
    CFLAGS= -c -O0 -g -Wall -Wextra $(INCPATH) $(DEFINES)
endif

ifeq ($(DEBUG),)
    DEFINES+= -D_RELEASE
    CFLAGS= -c -O2 -Wall $(INCPATH) $(DEFINES)
endif

OBJECTS=$(SOURCES:.c=.o)

all: $(SOURCES) $(EXECUTABLE) move

move: $(EXECUTABLE)
	@if [ ! -d $(OUTPUTDIR) ]; then mkdir $(OUTPUTDIR) ; fi
	mv -f $(EXECUTABLE) $(OUTPUTDIR)

$(EXECUTABLE) : $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f *.o
	rm -f $(OUTPUTDIR)/$(EXECUTABLE)

# eof #

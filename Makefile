#
#	Copyright (c) 2018 Tiago Ventura (tiago.ventura@gmail.com)
#
#	Permission is hereby granted, free of charge, to any person obtaining a copy
#	of this software and associated documentation files (the "Software"), to deal
#	in the Software without restriction, including without limitation the rights
#	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#	copies of the Software, and to permit persons to whom the Software is
#	furnished to do so, subject to the following conditions:
#
#	The above copyright notice and this permission notice shall be included in
#	all copies or substantial portions of the Software.
#
#	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
#	THE SOFTWARE.
#

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

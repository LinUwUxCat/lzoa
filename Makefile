
PROGRAM = lzox
SOURCES = main.c minilzo.c

# If minilzo.h, lzoconf.h and lzodefs.h are not found, change this
CPPFLAGS = -I. -I./include

GCC_CFLAGS = -s -Wall -O2 -fomit-frame-pointer

default:
	gcc $(CPPFLAGS) $(GCC_CFLAGS) -o $(PROGRAM) $(SOURCES)

clean:
	rm -f $(PROGRAM) $(PROGRAM).exe $(PROGRAM).map $(PROGRAM).tds
	rm -f *.err *.o *.obj


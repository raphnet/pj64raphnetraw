CC=i686-w64-mingw32-gcc
LD=$(CC)

PROG=plugin_info.exe

HIDAPI_BASE=../../hidapi
HIDAPI_CFLAGS=-I$(HIDAPI_BASE)/hidapi
HIDAPI_LIBS=-L$(HIDAPI_BASE)/windows/.libs/ -lhidapi

CFLAGS=-O3 -flto -Wall -ffast-math -fno-strict-aliasing -fvisibility=hidden -fno-PIC $(HIDAPI_CFLAGS)
LDFLAGS=-MD -MP $(HIDAPI_LIBS)

OBJS=plugin_info.o

all: $(PROG)

$(PROG): $(OBJS)
	$(LD) -o $@ $^ $(LDFLAGS)

clean:
	rm -vf $(PROG) $(OBJS) *.d


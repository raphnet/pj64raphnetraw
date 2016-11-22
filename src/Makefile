CC=i686-w64-mingw32-gcc
LD=$(CC)

PLUGIN=pj64raphnetraw.dll

HIDAPI_BASE=../../hidapi
HIDAPI_CFLAGS=-I$(HIDAPI_BASE)/hidapi
HIDAPI_LIBS=-L$(HIDAPI_BASE)/windows/.libs/ -lhidapi

CFLAGS=-O3 -flto -Wall -ffast-math -fno-strict-aliasing -fvisibility=hidden -fno-PIC $(HIDAPI_CFLAGS) -MD -MP -mwindows
LDFLAGS=-MD -MP -shared $(HIDAPI_LIBS) -mwindows

OBJS=plugin.o gcn64lib.o gcn64.o hexdump.o

all: $(PLUGIN)

$(PLUGIN): $(OBJS)
	$(LD) -o $@ $^ $(LDFLAGS)

clean:
	rm -vf $(PLUGIN) $(OBJS) *.d


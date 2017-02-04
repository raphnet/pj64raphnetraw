CC=i686-w64-mingw32-gcc
LD=$(CC)

PLUGIN=pj64raphnetraw.dll
PLUGIN_P1AND4=pj64raphnetraw_ports1and4.dll
PLUGIN_P1AND3=pj64raphnetraw_ports1and3.dll

HIDAPI_BASE=../../hidapi
HIDAPI_CFLAGS=-I$(HIDAPI_BASE)/hidapi
HIDAPI_LIBS=-L$(HIDAPI_BASE)/windows/.libs/ -lhidapi

CFLAGS=-O3 -flto -Wall -ffast-math -fno-strict-aliasing -fvisibility=hidden -fno-PIC $(HIDAPI_CFLAGS) -mwindows -DSPECS_VERSION=0x0100
LDFLAGS=-shared $(HIDAPI_LIBS) -mwindows

OBJS=plugin_front.o plugin_back.o gcn64lib.o gcn64.o hexdump.o
OBJS_STD=$(patsubst %.o,objs/%.o,$(OBJS))
OBJS_P1AND4=$(patsubst %.o,objs_p1and4/%.o,$(OBJS))
OBJS_P1AND3=$(patsubst %.o,objs_p1and3/%.o,$(OBJS))

.PHONY: objdirs

all: objdirs $(PLUGIN) $(PLUGIN_P1AND4) $(PLUGIN_P1AND3)

objdirs:
	mkdir -p objs objs_p1and4 objs_p1and3

$(PLUGIN): $(OBJS_STD)
	$(LD) -o $@ $^ $(LDFLAGS)

$(PLUGIN_P1AND4): $(OBJS_P1AND4)
	$(LD) -o $@ $^ $(LDFLAGS)

$(PLUGIN_P1AND3): $(OBJS_P1AND3)
	$(LD) -o $@ $^ $(LDFLAGS)

objs/%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

objs_p1and4/%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@ -DPORTS_1_AND_4

objs_p1and3/%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@ -DPORTS_1_AND_3

clean:
	rm -vf $(PLUGIN) $(PLUGIN_P1AND4) $(OBJS_STD) $(OBJS_P1ADN4) *.d


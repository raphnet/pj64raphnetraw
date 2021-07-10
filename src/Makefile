CC=i686-w64-mingw32-gcc
LD=$(CC)

PLUGIN=pj64raphnetraw.dll
PLUGIN_1PLAYER=pj64raphnetraw_1player.dll
PLUGIN_P1AND4=pj64raphnetraw_ports1and4.dll
PLUGIN_P1AND3=pj64raphnetraw_ports1and3.dll
PLUGIN_NET=pj64raphnetraw_net.dll
PLUGIN_NET_MULTIPLAYER=pj64raphnetraw_net_multiplayer.dll
PLUGIN_NOPAK=pj64raphnetraw_nopak.dll
PLUGIN_NET_NOPAK=pj64raphnetraw_net_nopak.dll

HIDAPI_BASE=../../hidapi
HIDAPI_CFLAGS=-I$(HIDAPI_BASE)/hidapi
HIDAPI_LIBS=-L$(HIDAPI_BASE)/windows/.libs/ -lhidapi

CFLAGS=-O3 -flto -Wall -ffast-math -fno-strict-aliasing -fvisibility=hidden -fno-PIC $(HIDAPI_CFLAGS) -mwindows -DSPECS_VERSION=0x0100
LDFLAGS=-shared $(HIDAPI_LIBS) -mwindows

OBJS=plugin_front.o plugin_back.o gcn64lib.o gcn64.o hexdump.o
OBJS_STD=$(patsubst %.o,objs/%.o,$(OBJS))
OBJS_P1AND4=$(patsubst %.o,objs_p1and4/%.o,$(OBJS))
OBJS_P1AND3=$(patsubst %.o,objs_p1and3/%.o,$(OBJS))
OBJS_1PLAYER=$(patsubst %.o,objs_1player/%.o,$(OBJS))
OBJS_NET=$(patsubst %.o,objs_net/%.o,$(OBJS))
OBJS_NET_MULTIPLAYER=$(patsubst %.o,objs_net_multiplayer/%.o,$(OBJS))
OBJS_NOPAK=$(patsubst %.o,objs_nopak/%.o,$(OBJS))
OBJS_NET_NOPAK=$(patsubst %.o,objs_net_nopak/%.o,$(OBJS))

.PHONY: objdirs

all: objdirs $(PLUGIN) $(PLUGIN_P1AND4) $(PLUGIN_P1AND3) $(PLUGIN_1PLAYER) $(PLUGIN_NET) $(PLUGIN_NET_MULTIPLAYER) $(PLUGIN_NOPAK) $(PLUGIN_NET_NOPAK)

objdirs:
	mkdir -p objs objs_p1and4 objs_p1and3 objs_1player objs_net objs_net_multiplayer objs_nopak objs_net_nopak

$(PLUGIN): $(OBJS_STD)
	$(LD) -o $@ $^ $(LDFLAGS)

$(PLUGIN_P1AND4): $(OBJS_P1AND4)
	$(LD) -o $@ $^ $(LDFLAGS)

$(PLUGIN_P1AND3): $(OBJS_P1AND3)
	$(LD) -o $@ $^ $(LDFLAGS)

$(PLUGIN_1PLAYER): $(OBJS_1PLAYER)
	$(LD) -o $@ $^ $(LDFLAGS)

$(PLUGIN_NET): $(OBJS_NET)
	$(LD) -o $@ $^ $(LDFLAGS)

$(PLUGIN_NET_MULTIPLAYER): $(OBJS_NET_MULTIPLAYER)
	$(LD) -o $@ $^ $(LDFLAGS)

$(PLUGIN_NOPAK): $(OBJS_NOPAK)
	$(LD) -o $@ $^ $(LDFLAGS)

$(PLUGIN_NET_NOPAK): $(OBJS_NET_NOPAK)
	$(LD) -o $@ $^ $(LDFLAGS)

objs/%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

objs_p1and4/%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@ -DPORTS_1_AND_4

objs_p1and3/%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@ -DPORTS_1_AND_3

objs_1player/%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@ -DPORT_1_ONLY

objs_net/%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@ -DPORT_1_ONLY -DNO_BLOCK_IO

objs_net_multiplayer/%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@ -DNO_BLOCK_IO

objs_nopak/%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@ -DNO_PAK_SUPPORT

objs_net_nopak/%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@ -DPORT_1_ONLY -DNO_BLOCK_IO -DNO_PAK_SUPPORT

clean:
	rm -vf $(PLUGIN) $(PLUGIN_P1AND4) $(PLUGIN_P1AND3) $(PLUGIN_1PLAYER) $(PLUGIN_NET) $(OBJS_STD) $(OBJS_P1AND4) $(OBJS_P1AND3) $(OBJS_1PLAYER) $(OBJS_NET) $(OBJS_NET_MULTIPLAYER) $(OBJS_NOPAK) $(OBJS_NET_NOPAK) *.d


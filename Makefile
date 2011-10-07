DEVICE=einkfb
HOST=
CC=$(HOST)gcc
AR=$(HOST)ar
DEBUG=
CFLAGS= -I include
ifeq (${DEBUG}, fatal)
  CFLAGS+= -DEDRAW_EXIT_FATAL
else
ifeq (${DEBUG}, warn)
  CFLAGS+= -DEDRAW_WARN_ALL
else
ifeq (${DEBUG}, debug)
  CFLAGS+= -DEDRAW_DEBUG
else
ifeq (${DEBUG}, calls)
  CFLAGS+= -DEDRAW_DEBUG -DEDRAW_DEBUG_CALLS
endif
endif
endif
endif

all: edraw.a

clean:
	rm -f *.a *.o
	rm -f devices/*.o
	rm -f ./*~
	rm -f ./devices/*~
	rm -f ./include/*~

edraw.a: edraw_$(DEVICE).a
	ln -f -s edraw_$(DEVICE).a edraw.a

edraw_einkfb.a: devices/einkfb.o path_gray4.o surface_gray4.o
	$(AR) cru edraw_einkfb.a devices/einkfb.o path_gray4.o surface_gray4.o

path_mono.o: path.c include/edraw.h
	$(CC) path.c -c -o path_mono.o -DEDRAW_COLOR=EDRAW_MONO $(CFLAGS)

path_gray4.o: path.c include/edraw.h
	$(CC) path.c -c -o path_gray4.o -DEDRAW_COLOR=EDRAW_GRAY4 $(CFLAGS)

path_tc.o: path.c include/edraw.h
	$(CC) path.c -c -o path_tc.o -DEDRAW_COLOR=EDRAC_TRUECOLOR $(CFLAGS)

surface_mono.o: surface.c include/edraw.h
	$(CC) surface.c -c -o surface_mono.o -DEDRAW_COLOR=EDRAW_MONO $(CFLAGS)

surface_gray4.o: surface.c include/edraw.h
	$(CC) surface.c -c -o surface_gray4.o -DEDRAW_COLOR=EDRAW_GRAY4 \
	  $(CFLAGS)

surface_tc.o: surface.c include/edraw.h
	$(CC) surface.c -c -o surface_tc.o -DEDRAW_COLOR=EDRAW_TRUECOLOR \
	  $(CFLAGS)

devices/einkfb.o: devices/einkfb.c include/edraw.h devices/common.h
	$(CC) devices/einkfb.c -c -o devices/einkfb.o $(CFLAGS)


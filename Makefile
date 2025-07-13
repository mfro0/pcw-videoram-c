CC=sdcc
AS=sdasz80
LD=sdldz80

CFLAGS=\
       --std-sdcc99 \
       -mz80 \
       --no-std-crt0 \
       --code-loc 0x100 \
       -DNOASM \
       $(INCLUDE) $(DEFINES)

ASFLAGS=-plosg

LDFLAGS=-nmi \

SRCS=demo.c \
     videoram.c \
     characters.c \
     bdos.c

ASMS=cpm0.s

AOBJS=$(patsubst %.s, %.rel, $(ASMS))
OBJS=$(AOBJS) $(patsubst %.c, %.rel, $(SRCS))

PROG=demo

all: $(PROG).com

$(PROG).com: $(OBJS)
	$(LD) -f demo.lk
	sdobjcopy -I ihex demo.ihx -O binary $@

%.rel: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.rel: %.s
	$(AS) $(ASFLAGS) -c -o $@ $<

clean:
	-rm -f $(PROG).com $(OBJS)
	-rm -f *.lst *.asm *.sym *.map

.PHONY: depend
depend:
	-rm -f depend
	for i in $(SRCS); do $(CC) $(DEFINES) $(INCLUDE) -M $$i >> depend; done

ifneq (clean,$MAKECMSDGOALS)
  -include depend
endif

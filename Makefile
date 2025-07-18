CC=sdcc
AS=sdasz80
LD=sdldz80

CFLAGS=\
       --std-sdcc99 \
       -mz80 \
       --fomit-frame-pointer \
       --opt-code-size \
       --no-std-crt0 \
       --code-loc 0x100 \
       -DNOASM \
       $(INCLUDE) $(DEFINES)

ASFLAGS=-plosg

LDFLAGS=-nmi \

SRCS=demo.c \
     videoram.c \
     characters.c \
     random_lines.c \
     bdos.c \
     bios.c

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
	-rm -f *.noi *.ihx

.PHONY: depend
depend:
	-rm -f depend
	for i in $(SRCS); do $(CC) $(DEFINES) $(INCLUDE) -M $$i >> depend; done

ifneq (clean,$MAKECMSDGOALS)
  -include depend
endif

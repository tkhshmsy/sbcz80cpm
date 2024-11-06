SRCS = bios.asm cpm22ipl.asm

AS = zasm
ASFLAGS = -wuyb
BINS = $(SRCS:.asm=.bin)

.SUFFIXES: .asm .bin
.asm.bin:
	$(AS) $(ASFLAGS) $< -o $@

all: clean $(BINS)

clean:
	rm -f $(BINS) *.lst *.hex

nulldisk:
	dd if=/dev/zero of=nulldisk.img bs=3328 count=77
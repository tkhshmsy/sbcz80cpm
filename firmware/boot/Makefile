SRCS = boot1st.asm boot2nd.asm bootihex.asm
SRCS += test2nd.asm testimg.asm

AS = zasm
ASFLAGS = -wuyb
BINS = $(SRCS:.asm=.bin)

.SUFFIXES: .asm .bin
.asm.bin:
	$(AS) $(ASFLAGS) $< -o $@

all: clean $(BINS)

clean:
	rm -f $(BINS) *.lst *.hex

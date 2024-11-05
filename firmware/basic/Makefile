TARGET = msbasic.bin
SRCS = init.asm
SRCS += basic.asm

AS = zasm
ASFLAGS = -wuyb
OBJS = $(SRCS:.asm=.obj)

.SUFFIXES: .asm .obj
.asm.obj:
	$(AS) $(ASFLAGS) $< -o $@

$(TARGET): $(OBJS)
	cat $(OBJS) > $(TARGET)

all: clean $(OBJS) $(TARGET)

clean:
	rm -f $(OBJS) $(TARGET) *.lst *.hex

hex:
	srec_cat $(TARGET) -binary -o $(TARGET).hex -intel
CROSS_COMPILE ?=
CC := $(CROSS_COMPILE)arm-none-eabi-gcc
AS := $(CROSS_COMPILE)arm-none-eabi-as

SRC_C := serial_demo.c
SRC_S := start.s
OBJS  := start.o serial_demo.o
OUT   := serial_demo.elf

UARTCLK ?= 24000000
BAUD ?= 115200

CFLAGS := -mcpu=arm926ej-s -ffreestanding -nostdlib -DUARTCLK=$(UARTCLK) -DBAUD=$(BAUD)
ASFLAGS := -mcpu=arm926ej-s
LDFLAGS := -Wl,-Ttext=0x10000 -Wl,-e,_start

QEMU := qemu-system-arm
QEMUFLAGS := -M versatilepb -nographic -monitor none -serial stdio -kernel $(OUT)

.PHONY: all run clean

all: $(OUT)

$(OUT): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

start.o: $(SRC_S)
	$(AS) $(ASFLAGS) -o $@ $<

serial_demo.o: $(SRC_C)
	$(CC) $(CFLAGS) -c -o $@ $<

run: $(OUT)
	$(QEMU) $(QEMUFLAGS)

clean:
	rm -f $(OUT) $(OBJS)

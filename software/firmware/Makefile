CC = rl78-elf-gcc
LD = rl78-elf-ld
OBJCOPY = rl78-elf-objcopy
LDFLAGS = -T flash.ld -nostdlib
CFLAGS = -mcpu=g13 -O1 -fno-builtin

game.bin : game
	$(OBJCOPY) -O binary $< $@

game : game.o imgdata.o flash.ld
	$(LD) game.o imgdata.o $(LDFLAGS) -o $@

game.o : game.c

imgdata.o : imgdata.c

clean:
	$(RM) game.o imgdata.o game game.bin *~

SRCDIR = ../src
RUNDIR = ../run
INCLUDE_DIR = ../include

KERNEL_SOURCE += $(wildcard $(SRCDIR)/kernel/*.c)
C_SOURCE += $(KERNEL_SOURCE)
C_OBJ += $(KERNEL_SOURCE:$(SRCDIR)/kernel/%.c=%.o)
UTILS_SOURCE = $(wildcard $(SRCDIR)/utils/*.c)
C_SOURCE += $(UTILS_SOURCE)
C_OBJ += $(UTILS_SOURCE:$(SRCDIR)/utils/%.c=%.o)
C_FLAGS = -g -Wall -march=i386 -m32 -nostdlib -fno-builtin -ffreestanding -fno-pic

ASM_SOURCE += $(wildcard $(SRCDIR)/utils/*.asm)
ASM_OBJ += $(ASM_SOURCE:$(SRCDIR)/utils/%.asm=%.o)

OBJ += $(C_OBJ)
OBJ += $(ASM_OBJ)

build: mbr.bin bootloader.bin kernel.bin kernel.o kernel_gdb.o
	dd if=mbr.bin of=$(RUNDIR)/hd.img bs=512 count=1 seek=0 conv=notrunc
	dd if=bootloader.bin of=$(RUNDIR)/hd.img bs=512 count=5 seek=1 conv=notrunc
	dd if=kernel.bin of=$(RUNDIR)/hd.img bs=512 count=145 seek=6 conv=notrunc

mbr.bin : $(SRCDIR)/boot/mbr.asm
	nasm -o mbr.bin -f bin -I $(INCLUDE_DIR)/ $(SRCDIR)/boot/mbr.asm
	
bootloader.bin : $(SRCDIR)/boot/bootloader.asm 
	nasm -o bootloader.o -g -f elf32 -I $(INCLUDE_DIR)/ $(SRCDIR)/boot/bootloader.asm
	gcc -o page.o $(C_FLAGS) -I $(INCLUDE_DIR) -c $(SRCDIR)/boot/page.c
	ld -o bootloader.obj -melf_i386 -N bootloader.o page.o -Ttext 0x7e00 -e bootloader_start
	objcopy -O binary bootloader.obj bootloader.bin

entry.obj : $(SRCDIR)/boot/entry.asm
	nasm -o entry.obj -f elf32 $(SRCDIR)/boot/entry.asm
	
kernel.bin : kernel.o
	objcopy -O binary kernel.o kernel.bin
		
kernel.o : entry.obj $(OBJ)
	ld -o kernel.o -melf_i386 -N entry.obj $(OBJ) -Ttext 0xc0020000 -e enter_kernel

kernel_gdb.o : entry.obj $(OBJ)
	ld -o kernel_gdb.o -melf_i386 -N entry.obj $(OBJ) -Ttext 0x20000 -e enter_kernel 
	
$(C_OBJ): $(C_SOURCE)
	gcc $(C_FLAGS) -I $(INCLUDE_DIR) -c $(C_SOURCE)
	
asm_utils.o : $(SRCDIR)/utils/asm_utils.asm
	nasm -o asm_utils.o -g -f elf32 $(SRCDIR)/utils/asm_utils.asm -I $(INCLUDE_DIR)/

run:
	qemu-system-i386 -hda $(RUNDIR)/hd.img -serial null -parallel stdio -no-reboot

debug: 
	qemu-system-i386 -S -s -parallel stdio -hda $(RUNDIR)/hd.img -serial null

docker:
	rm ../run/hd.img |:
	qemu-img create ../run/hd.img 10m
	docker run -it --rm -v $(shell pwd)/..:/mnt naphthol/sysu_os_lab sh -c "cd mnt/build && make clean && make build"

docker_run: docker run

gdb: 
	gdb -q -tui -x $(RUNDIR)/gdbinit

clean:
	rm -f *.o* *.bin 
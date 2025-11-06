# Group Info

- Alp Eren Yilmaz (BUID: U13910952)
- Andrew Lawson (BUID: U31011815)

## memos-2.img Configuration

- _Built using Andrew's BUID_
- **Cylinders:** 25
- **Heads:** 16
- **Sectors:** 63
- **Byte Size:** 12,600KB

## Building and running the Memos2 assignment

1. Mount the memos-2.img using `mount memos-2.img /your/mount/dir -text2 -o loop,offset=32256`
2. Run `qemu-system-i386 -hda memos-2.img -vnc :1` in a terminal
3. Run `./vncviewer :1` in a separate terminal

## Workflow

1. The memos2 assembly code reserves 8KB of stack space for C and then pushes the address of the multiboot info structure onto the stack. Once that is done, the C function `kmain` is called and the assembly halts.
2. Within `kmain`, we first process the memory map provided by grub in order to total up all type 1 memory. Once the total is complete, we put the welcome message with the total free memory to the screen.
3. The memory map is processed again in order to display each entries memory range and type. All memory ranges are padded with leading zeroes in order to hit the 8 digit representation for 32-bit hexadecimal.

## References

- [Detecting Memory x86](https://wiki.osdev.org/Detecting_Memory_%28x86%29)
- [Printing To Screen](https://wiki.osdev.org/Printing_To_Screen)
- Memos2 Lab Slides provided by Shriram
- [Multiboot Specification](https://www.gnu.org/software/grub/manual/multiboot/multiboot.html)
- [Calling C Functions from Assembly](https://flint.cs.yale.edu/cs421/papers/x86-asm/asm.html#calling)
- [x86 Calling Conventions Wiki](https://en.m.wikipedia.org/wiki/X86_calling_conventions)

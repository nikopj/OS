#!/bin/sh
if [ -z $1 ]; then
	echo "usage: ./p23.sh assembly_code.s"
	exit 1
fi
cat $1
as --64 $1 -o a.o && ld -m elf_x86_64 a.o -o a.out
./a.out
strace ./a.out

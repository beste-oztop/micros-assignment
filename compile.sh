#!/bin/bash

# Run copy_files.sh
echo "Running copy_files.sh..."
./copy_files.sh

if [ $? -ne 0 ]; then
    echo "Error: copy_files.sh failed"
    exit 1
fi

# Run make clean
echo "Running make clean..."
make clean

if [ $? -ne 0 ]; then
    echo "Error: make clean failed"
    exit 1
fi

# Run make all
echo "Running make all..."
make all

if [ $? -ne 0 ]; then
    echo "Error: make all failed"
    exit 1
fi

# Run qemu
echo "Starting QEMU..."
qemu-system-i386 -kernel memos-2 -vnc :0
#!/bin/bash
echo "=== Espacio ==="
df -h /mnt/data

echo "=== Inodos ==="
df -i /mnt/data

echo "=== I/O Discos ==="
iostat -xd 1 2

echo "=== Files Open ==="
lsof +D /mnt/data | wc -l

echo "=== Top process I/O ==="
iotop -b -n 1 | head -n 10

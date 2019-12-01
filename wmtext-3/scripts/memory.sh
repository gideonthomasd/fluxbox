#! /bin/bash

echo "Memory(b)"
echo "----------"
echo "----------"
free | grep Mem | awk '{print $3 - 47000}'




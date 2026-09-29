#!/bin/bash
set -e

echo "Q24 - Final Project Build"

gcc -Wall -Wextra integrated_shell.c -o integrated_shell
gcc -Wall -Wextra process_groups.c -o process_groups

echo "Build successful."

#!/bin/sh
# Builds and runs unit tests on the host with its C++ compiler (CXX, default: c++), as
# C++98 with every warning an error - the same subset Open Watcom compiles.
#   host_test.sh <output> <source>...
set -e
OUT=$1
shift
mkdir -p "$(dirname "$OUT")"
${CXX:-c++} -std=c++98 -Wall -Wextra -Werror -pedantic -Wno-long-long -o "$OUT" "$@"
"$OUT"

#!/bin/sh
# Run the DOSBox screen tests (SOURCE/test/run.py): it builds the test version of the
# program with the Open Watcom that scripts/ci-toolchain.sh installed.
set -eu
WATCOM="${WATCOM:-/opt/watcom}" python3 SOURCE/test/run.py

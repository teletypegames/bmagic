#!/bin/sh
# Build BMAGIC.EXE into the repository root (SOURCE/build.sh), with the Open Watcom that
# scripts/ci-toolchain.sh installed.
set -eu

say() { echo "[ci-build] $*"; }
die() { echo "[ci-build] error: $*" >&2; exit 1; }

rm -f BMAGIC.EXE
WATCOM="${WATCOM:-/opt/watcom}" sh SOURCE/build.sh
[ -s BMAGIC.EXE ] || die "BMAGIC.EXE was not built"
say "BMAGIC.EXE: $(wc -c < BMAGIC.EXE | tr -d ' ') bytes"

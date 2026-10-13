#!/bin/sh
# The fast checks before the build: the code style (clang-format), the layers
# (SOURCE/lib/tools/check_layers.py) and the unit tests on the host (SOURCE/test/unit).
set -eu
CLANG_FORMAT="${CLANG_FORMAT:-/opt/clang-format/bin/clang-format}" sh SOURCE/lib/tools/format.sh --check SOURCE
sh SOURCE/test/unit/run.sh

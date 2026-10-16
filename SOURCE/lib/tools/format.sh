#!/bin/sh
# Formats the sources of a program (its SOURCE folder, default: the current one) with the
# .clang-format of DOSLIB; --check only tells which files differ. Vendored code (MTCP)
# and the copied lib are left out (DOSLIB is formatted in its own folder).
#   CLANG_FORMAT   the clang-format 18 binary (default: clang-format)
set -e
CHECK=
[ "$1" = --check ] && { CHECK=1; shift; }
DIR=${1:-.}
STYLE="$(cd "$(dirname "$0")/.." && pwd)/.clang-format"
FILES=$(cd "$DIR" && find . \( -name '*.cpp' -o -name '*.h' \) -not -path './MTCP/*' -not -path './lib/*' \
        -not -path './build/*' -not -path './test/build/*' | sort)
cd "$DIR"
BAD=0
for f in $FILES; do
  if [ -n "$CHECK" ]; then
    ${CLANG_FORMAT:-clang-format} --style="file:$STYLE" "$f" | cmp -s - "$f" || { echo "not formatted: $f"; BAD=1; }
  else
    ${CLANG_FORMAT:-clang-format} --style="file:$STYLE" -i "$f"
  fi
done
exit $BAD

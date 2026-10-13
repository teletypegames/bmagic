#!/bin/sh
# Package the built program for a release: dist/BMAGIC-<tag>.zip holds the BMAGIC folder
# as it is installed (BMAGIC.EXE, README.md, LICENSE.md), and dist/BMAGIC.EXE is the
# program alone, for updating an installed one.
#
#   scripts/ci-package.sh            the tag from Woodpecker (CI_COMMIT_TAG)
#   TAG=v1.2.3 scripts/ci-package.sh
#
# The tag has to match Version in SOURCE/app/version.h. BMAGIC.EXE must already be built
# (scripts/ci-build.sh, the pipeline's test step).
set -eu

TAG="${TAG:-${CI_COMMIT_TAG:-}}"
DIST="${DIST:-dist}"

say() { echo "[ci-package] $*"; }
die() { echo "[ci-package] error: $*" >&2; exit 1; }

[ -n "$TAG" ] || die "cannot work out the tag — set TAG=v1.2.3"
SOURCE_VERSION="$(sed -n 's/^static const char Version\[\] = "\(.*\)";/\1/p' SOURCE/app/version.h)"
[ "v$SOURCE_VERSION" = "$TAG" ] || die "the tag $TAG does not match Version $SOURCE_VERSION in SOURCE/app/version.h"
[ -f BMAGIC.EXE ] || die "BMAGIC.EXE is not built — run scripts/ci-build.sh first"

rm -rf "$DIST"
mkdir -p "$DIST/BMAGIC"
cp BMAGIC.EXE README.md LICENSE.md "$DIST/BMAGIC/"
(cd "$DIST" && zip -q -r "BMAGIC-$TAG.zip" BMAGIC)
cp BMAGIC.EXE "$DIST/"
rm -rf "$DIST/BMAGIC"

say "built:"
ls -l "$DIST"

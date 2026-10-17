#!/bin/sh
# Install what the pipeline needs into a Debian image: Open Watcom v2 (the compiler),
# DOSBox (Debian's DOSBox 0.74: SOURCE/test/run.py runs the screen tests in it), Python,
# g++ (the unit tests) and clang-format (the code style). The Open Watcom download and
# clang-format are pinned releases.
#
#   OW_RELEASE   Open Watcom release tag   (default: 2026-10-01-Build)
#
# Open Watcom goes to /opt/watcom (only the Linux binaries, the headers and the 16-bit
# libraries).
#
# The pipeline's image (git.teletypegames.org/build/dos-ci:bookworm, built by
# services/teletypegames ci/dos-bookworm.Dockerfile) has all of this preinstalled and lists
# its versions in /opt/dos-ci/versions; when they are the ones pinned here, nothing is installed.
set -eu

CLANG_FORMAT_VERSION="${CLANG_FORMAT_VERSION:-18.1.8}"

OW_RELEASE="${OW_RELEASE:-2026-10-01-Build}"

say() { echo "[ci-toolchain] $*"; }

preinstalled() {
  [ -r /opt/dos-ci/versions ] || return 1
  for pin in "$@"; do grep -qx "$pin" /opt/dos-ci/versions || return 1; done
}
if preinstalled "OW_RELEASE=$OW_RELEASE" "CLANG_FORMAT_VERSION=$CLANG_FORMAT_VERSION"; then
  say "preinstalled in the CI image: $(tr '\n' ' ' < /opt/dos-ci/versions)"
  exit 0
fi

apt-get update
apt-get install -y --no-install-recommends ca-certificates curl xz-utils dosbox python3 python3-venv g++ >/dev/null

say "Open Watcom $OW_RELEASE"
mkdir -p /opt/watcom
curl -fsSL "https://github.com/open-watcom/open-watcom-v2/releases/download/$OW_RELEASE/ow-snapshot.tar.xz" |
  tar -xJ -C /opt/watcom ./binl64 ./h ./lib286

say "clang-format $CLANG_FORMAT_VERSION"
python3 -m venv /opt/clang-format
/opt/clang-format/bin/pip install --quiet "clang-format==$CLANG_FORMAT_VERSION"

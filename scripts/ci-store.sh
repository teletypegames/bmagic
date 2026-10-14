#!/bin/sh
# Publish the built program to the DOSINST program store
# (ftp://ftp.teletypegames.org/APPS): the BMAGIC folder as it is installed, plus an INFO.INI
# with the version of the tag, so DOSINST can install and update it (to C:\APPS\BMAGIC
# with the default DEST), and a FILES.LST ("crc32 size path" per file, as dos/dosinst's
# server/mkfiles.sh writes it) that DOSINST checks every download against.
#
#   scripts/ci-store.sh              the tag from Woodpecker (CI_COMMIT_TAG)
#   TAG=v1.2.3 STORE=/tmp/store scripts/ci-store.sh
#
# STORE is the store's programs folder; in the pipeline /srv/stacks/ftp-server/root/APPS of
# the server is mounted there. The new folder is put together next to the old one and
# swapped in with two renames, so a download never sees it half written. The files get
# STORE_OWNER (the server's tasi user), so the store can be managed without sudo. The
# backups BMAGIC makes (BACKUP\ in its folder) exist only on the DOS machine, so an
# update keeps them.
#
# BMAGIC.EXE must already be built (scripts/ci-build.sh, the pipeline's test step).
set -eu

TAG="${TAG:-${CI_COMMIT_TAG:-}}"
STORE="${STORE:-/store}"
STORE_OWNER="${STORE_OWNER:-1000:1000}"
NAME=BMAGIC

say() { echo "[ci-store] $*"; }
die() { echo "[ci-store] error: $*" >&2; exit 1; }

[ -n "$TAG" ] || die "cannot work out the tag — set TAG=v1.2.3"
[ -f BMAGIC.EXE ] || die "BMAGIC.EXE is not built — run scripts/ci-build.sh first"
[ -d "$STORE" ] || die "no store folder at $STORE"

VERSION="${TAG#v}"
NEW="$STORE/.$NAME.new"
OLD="$STORE/.$NAME.old"

rm -rf "$NEW" "$OLD"
mkdir "$NEW"
cp BMAGIC.EXE README.md LICENSE.md "$NEW/"
printf '[program]\r\nname=Boot Magic\r\ndescription=Boot menu editor: maps and edits the boot menu of CONFIG.SYS and AUTOEXEC.BAT and the startup options of MSDOS.SYS, backs them up and shows what it changed. Start it with BMAGIC.\r\nversion=%s\r\nplatform=DOS\r\n' \
  "$VERSION" > "$NEW/INFO.INI"
(
  cd "$NEW"
  find . -type f ! -name 'FILES.LST*' | sed 's|^\./||' | sort | while IFS= read -r f; do
    crc=$(gzip -c "$f" | tail -c 8 | od -An -tx4 -N4 | tr -d ' \n')
    printf '%s %s %s\r\n' "$crc" "$(wc -c < "$f" | tr -d ' ')" "$f"
  done > FILES.LST.new
  mv FILES.LST.new FILES.LST
)
chown -R "$STORE_OWNER" "$NEW" 2>/dev/null || say "could not chown to $STORE_OWNER, keeping the owner"
chmod -R a+rX "$NEW"

if [ -d "$STORE/$NAME" ]; then
  mv "$STORE/$NAME" "$OLD"
fi
mv "$NEW" "$STORE/$NAME"
rm -rf "$OLD"

say "published $NAME $VERSION:"
ls -l "$STORE/$NAME"

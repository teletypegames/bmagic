#!/bin/sh
# Builds ../BMAGIC.EXE with Open Watcom v2, as a 16-bit DOS program.
# WATCOM points to the Open Watcom folder (default: ~/.local/opt/watcom).
# ./build.sh test builds the test version (the keys come from TESTKEYS.TXT, see
# test/run.py) into build/.
set -e
cd "$(dirname "$0")"
. lib/tools/watcom.sh

OBJ=build
OUT=..
DEFS=
if [ "$1" = test ]; then
  OUT=$OBJ
  DEFS=-dTEST
fi
rm -rf "$OBJ"
mkdir -p "$OBJ"

LIB="lib/core/str.cpp lib/core/ini.cpp lib/core/text.cpp lib/platform/file_system.cpp lib/platform/dos/dos_text_output.cpp
     lib/platform/dos/dos_file_system.cpp lib/platform/dos/dos_screen.cpp lib/platform/dos/bios_keyboard.cpp
     lib/platform/dos/dos_speaker.cpp lib/ui/canvas.cpp lib/ui/list_cursor.cpp
     lib/testing/scripted_keyboard.cpp lib/testing/screen_dump.cpp"
SRC="platform/dos/dos_boot_system.cpp
     model/boot_file.cpp model/text_file.cpp model/dos_syntax.cpp model/boot_menu.cpp model/menu_editor.cpp
     model/startup_options.cpp model/text_diff.cpp model/backup_list.cpp model/line_rows.cpp
     repository/disk/drive_boot_file_repository.cpp repository/disk/folder_backup_repository.cpp
     service/boot_service.cpp service/backup_service.cpp
     controller/line_input.cpp controller/list_keys.cpp controller/form_controller.cpp controller/help_controller.cpp
     controller/diff_controller.cpp controller/backup_controller.cpp controller/lines_controller.cpp
     controller/menu_controller.cpp
     view/frame.cpp view/menu_view.cpp view/lines_view.cpp view/diff_view.cpp view/backup_view.cpp
     view/form_view.cpp view/help_view.cpp
     app/bmagic.cpp main_bmagic.cpp"
LIB=$(echo $LIB)
SRC=$(echo $SRC)

CFLAGS="-0 -ml -os -s -ei -zm -we -zq -bt=dos -i=$WATCOM/h $DEFS"
compile wpp "$CFLAGS" $LIB $SRC
LINK="system dos option quiet option eliminate option stack=8192"
wlink $LINK name "$OUT/BMAGIC.EXE" file "$(objs $LIB $SRC)"
rm -f "$OBJ"/*.obj
ls -l "$OUT/BMAGIC.EXE"

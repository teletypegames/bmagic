Boot Magic – a boot menu editor for DOS.

**Install:** install `BMAGIC` with DOSINST (it goes to `C:\APPS\BMAGIC`), or unzip `BMAGIC-<version>.zip` to the DOS drive. Start it with `BMAGIC` (or `BMAGIC D:` for another drive). See `README.md` for the details (F1 in the program shows the keys).

**New in 0.1.0:**

- Maps the boot menu of `CONFIG.SYS` (`[MENU]`, `MENUITEM`, `MENUDEFAULT`, `MENUCOLOR`, `NUMLOCK`, the blocks of the items) and `AUTOEXEC.BAT` (`GOTO %CONFIG%` and the sections of the items)
- Menu items: add (empty, or a copy of another), rename, change the text, move, delete, make the default; the first item creates the menu, and the lines already in `CONFIG.SYS` become `[COMMON]`
- Edits the lines of an item's block and section, or of a whole file
- The startup options of a Windows 95/98 `MSDOS.SYS` (`BootMenu`, `BootMenuDefault`, `BootMenuDelay`, `BootGUI`, `Logo`, `BootKeys`); the attributes of the file are kept
- Changes only the lines it has to; the line ends and the EOF byte stay as they were
- Saving shows a diff of the changes first and backs up the files to `BACKUP\NNN\` before it writes them; a file is replaced so that it is never lost
- Backups: list, compare with the files on the disk (diff), back up now, restore (after a backup of the files it replaces), delete

Arc-reader
==========

Allow you to read and extract files inside .arc files of the BGI engine (OverDrive/MangaGamer).

Files
=====

* arc.h/c : read an .arc file and get info on the contained files.
* bse.h/c : remove BSE encryption (only the first 64 bytes are encrypted).
* cgb.h/c : decrypt and save a "CompressedBG___" file onto the disk.
* dsc.h/c : decrypt and save a "DSC FORMAT 1.00" file onto the disk.
* decrypt.h/c : helper functions.
* write.h/c : save a RGBA array into a PNG file.
* main.c : a small example extracting all the files inside the given .arc file (compiling with the given Makefile).

Dependency
==========

* libpng 1.2


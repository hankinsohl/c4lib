# ![c4lib-logo](/assets/Yurt.jpg?raw=true) c4lib

### What is c4lib?

c4lib is a C++ library for editing Civilization 4, Beyond the Sword (BTS) saves. c4lib is bundled
with c4edit.

### What is c4edit?

c4edit is a command line program built with c4lib that translates BTS saves into human-readable
text. The format of the text file is similar to that of a debugger's memory view. The beginning of
a "translation" is shown below.

```
Savegame: Tiny-Map-BC-4000.CivBeyondSwordSave
Schema: BTS.schema
Date: 01-12-2025 19:59:33.508379 UTC
c4lib version: 01.00.00

Offset       Hex                                                         ASCII              Translation                                       
0x00000000 | -- -- -- --  | -- -- -- --  | -- -- -- --  | -- -- -- --  | ---------------- | Begin Savegame
0x00000000 | -- -- -- --  | -- -- -- --  | -- -- -- --  | -- -- -- --  | ---------------- |   Begin GameHeader
0x00000000 | 2e 01 00 00  | -- -- -- --  | -- -- -- --  | -- -- -- --  | ....------------ |     GameVersion=302
0x00000004 | 00 00 00 00  | -- -- -- --  | -- -- -- --  | -- -- -- --  | ....------------ |     RequiredMod=""
0x00000008 | 00 00 00 00  | -- -- -- --  | -- -- -- --  | -- -- -- --  | ....------------ |     ModMd5=""
0x0000000c | 00 00 00 00  | -- -- -- --  | -- -- -- --  | -- -- -- --  | ....------------ |     ChecksumDWord=0
0x00000010 | 00 00 00 00  | -- -- -- --  | -- -- -- --  | -- -- -- --  | ....------------ |     LockModifiedAssetsText=""
0x00000014 | 00 00 00 00  | -- -- -- --  | -- -- -- --  | -- -- -- --  | ....------------ |     LmaMd5_1=""
0x00000018 | 00 00 00 00  | -- -- -- --  | -- -- -- --  | -- -- -- --  | ....------------ |     LmaMd5_2=""
0x0000001c | 00 00 00 00  | -- -- -- --  | -- -- -- --  | -- -- -- --  | ....------------ |     LmaMd5_3=""
0x00000020 | 00 00 00 00  | -- -- -- --  | -- -- -- --  | -- -- -- --  | ....------------ |     LmaMd5_4=""
0x00000024 | bf 05 00 00  | -- -- -- --  | -- -- -- --  | -- -- -- --  | ....------------ |     CvInitCoreMd5Size=1471
0x00000028 | -- -- -- --  | -- -- -- --  | -- -- -- --  | -- -- -- --  | ---------------- |   End GameHeader
0x00000028 | -- -- -- --  | -- -- -- --  | -- -- -- --  | -- -- -- --  | ---------------- |   Begin CvInitCore
0x00000028 | 01 00 00 00  | -- -- -- --  | -- -- -- --  | -- -- -- --  | ....------------ |     SaveFlag=1
0x0000002c | 00 00 00 00  | -- -- -- --  | -- -- -- --  | -- -- -- --  | ....------------ |     Type=GAME_SP_NEW
0x00000030 | 10 00 00 00  | 50 00 61 00  | 73 00 73 00  | 65 00 6e 00  | ....P.a.s.s.e.n. |     GameName="Passenger's Game"
0x00000040 | 67 00 65 00  | 72 00 27 00  | 73 00 20 00  | 47 00 61 00  | g.e.r.'.s. .G.a. |     ...
0x00000050 | 6d 00 65 00  | -- -- -- --  | -- -- -- --  | -- -- -- --  | m.e.------------ |     ...
```

### What is c4edit used for?

c4edit has several uses.

For those simply curious about what a BTS save looks like, c4edit provides human-readable
translations.

For more experienced users who want to make changes to a BTS save, c4edit can be used to generate
info-format files for saves. The info files can be edited using a text editor, loaded back into
c4edit, and then saved as a BTS save file (.CivBeyondSwordSave).

### What is c4lib used for?

c4lib is a static C++ library that exposes a simple interface for loading saves and info files,
and for generating saves, info files, and translations. The interface
uses [Boost Property Tree](https://www.boost.org/doc/libs/1_87_0/doc/html/property_tree.html)
to store the representation of a BTS save. The Boost Property Tree API can be used to make
changes to the property tree, which can then be saved in CivBeyondSwordSave format.

### Does c4lib understand the BTS compressed data format?

BTS saves store much of their data in zlib-compressed format. c4edit and c4lib
understand this format, and are able to decompress the zlib-data making it available to view and
edit.

### Does c4lib understand the BTS save checksum?

Each BTS save ends with a checksum, used to ensure the integrity of the save. c4lib understands
how the checksum is generated and updates the checksum whenever it saves a CivBeyondSwordSave-
format file.

### Getting started

If you just want to use c4edit, download
c4edit [here](https://github.com/hankinsohl/c4lib/releases/tag/latest), and follow the directions in
[QUICKSTART.md](QUICKSTART.md) to configure c4edit
for your computer.

If you're interested in C++ programming using the c4lib API, download c4edit and
c4lib [here](https://github.com/hankinsohl/c4lib/releases/tag/latest). Then
read [QUICKSTART.md](QUICKSTART.md) to configure
c4edit, and [API.md](API.md) to learn about using
c4lib.

If you're interested in downloading the sources,
click [here](https://github.com/hankinsohl/c4lib/releases/tag/latest) and then
read [BUILDING.md](BUILDING.md).

### License

c4lib and c4edit are licensed under the MIT License.
See [LICENSE](LICENSE).

c4lib incorporates sources from copyrighted works as follows:

md5.hpp and md5.cpp:
Copyright (c) 2014 Stephan Brumme. All rights reserved. See
http://create.stephan-brumme.com/disclaimer.html for the license.

narrow.hpp:
Copyright (c) 2015 Microsoft Corporation. All rights reserved. Licensed under the MIT License.
# ROM Demangler
A tool in made in C++ that allows you to export data from SM64 ROMs to a sm64coopdx romhack.

Credits:
- [stb_image_write](https://github.com/nothings/stb/blob/master/stb_image_write.h) by [nothings](https://github.com/nothings) - Used for exporting textures
- [cxxopts](https://github.com/jarro2783/cxxopts) by [jarro2783](https://github.com/jarro2783) - Library for parsing CLI arguments
- [RM2C](https://gitlab.com/scuttlebugraiser/rom-manger-2-c) by [scuttlebugraiser](https://gitlab.com/scuttlebugraiser) - References for Movtex and Scrolling Texture exporting
- [Quad64](https://github.com/DavidSM64/Quad64) by [DavidSM64](https://github.com/DavidSM64) - Per-Area Bank 0x0E functions

Features:
- Supports MIO0, YAY0, and RNC compression methods.
- Compatible with most microcodes (F3D, F3DEX, F3DEX2, F3DZEX).
- Supports ROMs modified via Decomp, SM64 ROM Manager, SM64 Editor, or Bowser's Blueprints.
- Configurable Segment 0 handling: choose to ignore it entirely or load its data from an external RAM dump.

But you might like Isaac's tool more: https://github.com/Isaac0-dev/rom-decomp-64
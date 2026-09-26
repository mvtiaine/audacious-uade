# st3play
Aims to be an accurate C port of Scream Tracker 3.21's S3M replayer (SB Pro and GUS render mode, with OPL2 emulator). \
The project contains example code in the st3play folder on how to interface with the API. \
This is a direct port of the original asm/C source codes.

# Notes
- The Gravis Ultrasound driver is buggy in the same way as in ST3
- To compile st3play (the test program) on macOS/Linux, you need SDL2
- The code may not be 100% safe to use as a replayer in other projects, and as such I recommend to use this only for reference

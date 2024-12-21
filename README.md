# rl78
Tools and boards I use for 16-bit RL78 microcontrollers.

# Contents
## boards/
Each directory in here contains the gerber files for a single board, along with
several gvp files. Each of these can be used with gerbview to open the board
with specific settings, e.g. check.gvp for looking at routing, preview.gvp for a
view of the board.

These directories also contain NETLIST files in SPICE format. These both
document the way components are connected and which components are used where
on the board.

The boards provided are:

- ornament - A board combining an r5fehana microcontroller and power circuit.
- programmer - A board used to program the ornament board.
- module - A DIP32 module combining the programmer and r5fehana.

## software/

### software/prog/

This is the software for flashing the microcontroller on the ornament board with the programmer.
It is used by the pgm.sh script in the software/firmware directory.
This requires libserialport and libhidapi.

### software/firmware/

This is the game software for the ornament.
These require binutils and gcc built with --target=rl78.
The contents of the gfxgen/ directory must be built separately first.
The pgm.sh is used to write the firmware to the ornament using the provided programming dongle design.

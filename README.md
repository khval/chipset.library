# chipset.library

So the idea is to create a chipset API, for 680x0 programs to use, this allow chipset to be replaced by soft CHIP or chip on a PCIe card.

So far I have only generated a skeleton for .library, the functions will need to be filled in, 
for this project I’m looking at different kinds of options, UAE (unix amiga emulator), UADE (Unix Amiga Delitracker emulator), or NallePuh the Paula emulator.

What I like to have in this library is, PAULA, CIAA/CIAB chip emulation, and possibly some remapping feature for the joystick ports.

Once the API is in place, I plan to make some 680x0 macros that can be inserted into existing 680x0 code where normally wrote direct to chipset, 
by so doing transform code that was written as hardware banging code into more system friendly code, that is easier to deal with, with out major rewrite problem we have now.

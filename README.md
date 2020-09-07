# chipset.library

So the idea is to create a chipset API, for 680x0 programs to use, this allow chipset to be replaced by soft CHIP or chip on a PCIe card.

So far I have only generated a skeleton for .library, the functions will need to be filled in, 
for this project I’m looking at different kinds of options, UAE (unix amiga emulator), UADE (Unix Amiga Delitracker emulator), or NallePuh the Paula emulator.

What I like to have in this library is, PAULA, CIAA/CIAB chip emulation, and possibly some remapping feature for the joystick ports.

Once the API is in place, I plan to make some 680x0 macros that can be inserted into existing 680x0 code where normally wrote direct to chipset, 
by so doing transform code that was written as hardware banging code into more system friendly code, that is easier to deal with, without major rewrite problem we have now.

AGA / OCS is not that useful to emulate, blitter and copper and so can be replaced by other code, 
pretty easy, at least if your working with a open source 680x0 program.

GPL2 was choices because UAE uses GPL2, but its possible this might change, depending on dependency’s, what emulation its going to be based on.

Do not expect chipset.libary to work under forbid() / permit() locks.
Multitasking is needed for correct as possible values; however, it is designed to compensate for any 
time lags caused by interruptions in the task scheduler.

# Disclaimer:

This project contains work from other projects with other open source licenses, LGPL allows you only open / link dynamically to the files under the LGPL license, other files are under other licenses, where some do not allow commercial, and some do. other file in this project file contains a copyright in the header files, you should referee to lienees of this files, if you like to use this files in a different way other then intended in other projects.

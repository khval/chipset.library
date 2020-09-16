# chipset.library

So the idea is to create a chipset API, for 680x0 programs to use, this allow chipset to be replaced by soft CHIP or chip on a PCIe card.

So far I have only generated a skeleton for .library, the functions will need to be filled in, 
for this project I’m looking at different kinds of options, UAE (unix amiga emulator), UADE (Unix Amiga Delitracker emulator), or NallePuh the Paula emulator.

What I like to have in this library is, PAULA, CIAA/CIAB chip emulation, and possibly some remapping feature for the joystick ports.

Once the API is in place, I plan to make some 680x0 macros that can be inserted into existing 680x0 code where normally wrote direct to chipset, 
by so doing transform code that was written as hardware banging code into more system friendly code, that is easier to deal with, without major rewrite problem we have now.

AGA / OCS is not that useful to emulate, blitter and copper and so can be replaced by other code, 
pretty easy, at least if your working with a open source 680x0 program.

Do not expect chipset.libary to work under forbid() / permit() locks.
Multitasking is needed for correct as possible values; however, it is designed to compensate for any 
time lags caused by interruptions in the task scheduler.

# Dependencies

The 680x0 assembler tests used VBCC, found a GITHUB repo that includes a SDK for OS3.x that works with VBCC. 
(Tried to use NDK31 but had some issue, did not find the LVO values.)

https://github.com/deplinenoise/amiga-sdk

To compileon C/C++ code you need the AmigaOS4.1 SDK.

https://wiki.amigaos.net/wiki/Programming_AmigaOS_4:_The_Development_Environment

# Compiling 

To compile the 680x0 lib tests, you need to make a assign in s:user-startup like

Assign SDK31: <your path to the files>
Read the AmigaDOS or the Tripos manual for howto use assign, if you don’t know how to use it.
  
When you have correctly setup the assign, you should be able to type make.

# Disclaimer:

This project contains work from other projects with other open source licenses, LGPL allows you only open / link dynamically to the files under the LGPL license, other files are under other licenses, where some do not allow commercial, and some do. other file in this project file contains a copyright in the header files, you should referee to lienees of this files, if you like to use this files in a different way other then intended in other projects.

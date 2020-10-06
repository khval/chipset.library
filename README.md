# chipset.library

So the idea is to create a chipset API, for 680x0 programs to use, this allow chipset to be replaced by soft CHIP or chip on a PCIe card.
This library contains CIA.c from UADE (Unix Amiga Delitracker emulator), and blitter.c & parts of custum.c from UAE (unix amiga emulator), and NallePuh the Paula emulator.

# The plan

Make it easy to replace direct access to chipset by macro's, that enable quick conversions of not so system friendly code, while this not ideal way to run 680x0 software, its better then trapping and patches. And a lot more convenient and efficient then using EUAE.

Because you need the source code, to make use of this library its only possible for open source 680x0 assembler code, there are some interesting stuff around, demos, and libraries and games, and other stuff, Google around see what you can find.

I do not expect people to reverse engineer code, its often not so easy to recompile and make the code work, because the disassemblers, can’t recover macros, if your code depends on correct offsets and delta values, or if can’t tell the difference between data and code. The reverse engineer code is messy hard to read, and hard to work with. (but not impossible.)

Tools can be written to provide amiga joystick and mouse statue, or monitor the emulated chips, some tools already exist.

AGA / OCS is not that useful to emulate, blitter and copper and so can be replaced by other code, 
pretty easy, at least if your working with a open source 680x0 program.

I’m thinking about providing blitter, just because I can, the more I make it easy for you the more likely you have success with it maybe.

Do not expect chipset.libary to work under forbid() / permit() locks.
Multitasking is needed for correct as possible values; however, it is designed to compensate for any 
time lags caused by interruptions in the task scheduler.

# Soft chip.

**Advantages**

        * Easy to debug (It’s easy to control, adjust speed, and on on.)

        * Easy to monitor (On real CIAA/B timer you can’t read the CIA register without modifying
        or changing states, we have a workaround with spy functions.)

        * Can be Upgraded. (however legacy support is way it exists.)

**Disadvantages**
    
        * Busy loops (pulling state from CIAA/B) can effect the timing due to overall drain on CPU resources.
        (It can affect pression, but overall timing is calculated based on eclipsed time (delta values),
        so is not affected.)

# FPGA on a PCIE slot.

**Advantages**

        * Easy to debug (It’s easy to control, adjust speed, and on on.)

        * Easy to monitor (On real CIAA/B timer you can’t read the CIA register without modifying
        or changing states, we have a workaround with spy functions.)

        * Can be Upgraded. (however legacy support is way it exists.)
        
**Disadvantages**

        Takes up a PCI or PCIE slot.

# Real / native Amiga chipset

 **Advantages**

       * You can easily switch between softchip and realchip, 
       using compiler flags, taking advantage of debug features.
        
       * The API can enable 68k software to use enhanced versions
       of chipset on Zorro or Trapdoor expansion.
        
 **Disadvantages**

        Someone needs to write a 680x0 version of this library
        
# XMOS chip on AmigaONE.

       As I understand it, it’s not easy to interface to the chip, what is true or not, 
       is not known, what is true is that the SDK on only works on Windows, so that sucks.

# Dependencies

The 680x0 assembler tests use VBCC, found a GITHUB repo that includes a SDK for OS3.x that works with VBCC. 
(Tried to use NDK31 but had some issue, did not find the LVO values.)

https://github.com/deplinenoise/amiga-sdk

To compileon C/C++ code you need the AmigaOS4.1 SDK.

https://wiki.amigaos.net/wiki/Programming_AmigaOS_4:_The_Development_Environment

# Development 

For developers who write C/C++ code please have look at this documentation.
(Sadly this library does not yet have equal for AmigaOS3.x)

https://github.com/khval/chipset.library/blob/master/chipset_library_docs.md

For developers who write 680x0 assembler have a look here.

https://github.com/khval/chipset.library/blob/master/chipset_library_asm_docs.md

# Compiling the chipset.library and tests.

To compile the 680x0 lib tests, you need to make a assign in s:user-startup like

Assign SDK31: <your path to the files>
Read the AmigaDOS or the Tripos manual for howto use assign, if you don’t know how to use it.
  
When you have correctly setup the assign, you should be able to type make.

# Disclaimer:

This project contains work from other projects with other open source licenses, LGPL allows you only open / link dynamically to the files under the LGPL license, other files are under other licenses, where some do not allow commercial, and some do. other file in this project file contains a copyright in the header files, you should referee to lienees of this files, if you like to use this files in a different way other then intended in other projects.

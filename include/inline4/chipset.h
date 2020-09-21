#ifndef INLINE4_CHIPSET_H
#define INLINE4_CHIPSET_H

/*
** This file was auto generated by idltool 53.6.
**
** It provides compatibility to OS3 style library
** calls by substituting functions.
**
** Do not edit manually.
*/ 

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif
#ifndef EXEC_EXEC_H
#include <exec/exec.h>
#endif
#ifndef EXEC_INTERFACES_H
#include <exec/interfaces.h>
#endif

#ifndef LIBRARIES_CHIPSET_H
#include <libraries/chipset.h>
#endif
#include <interfaces/chipset.h>

/* Inline macros for Interface "main" */
#define allocChip(size) Ichipset->allocChip((size)) 
#define freeChip(mem) Ichipset->freeChip((mem)) 
#define hostAddressToChip(adr) Ichipset->hostAddressToChip((adr)) 
#define chipAddressToHost(adr) Ichipset->chipAddressToHost((adr)) 
#define writeChipLong(offset, value) Ichipset->writeChipLong((offset), (value)) 
#define writeChipWord(offset, value) Ichipset->writeChipWord((offset), (value)) 
#define writeChipByte(offset, value) Ichipset->writeChipByte((offset), (value)) 
#define readChipLong(offset) Ichipset->readChipLong((offset)) 
#define readChipWord(offset) Ichipset->readChipWord((offset)) 
#define readChipByte(offset) Ichipset->readChipByte((offset)) 
#define bitChgChipLong(offset, value) Ichipset->bitChgChipLong((offset), (value)) 
#define bitChgChipWord(offset, bit) Ichipset->bitChgChipWord((offset), (bit)) 
#define bitChgChipByte(offset, bit) Ichipset->bitChgChipByte((offset), (bit)) 
#define bitSetChipLong(offset, bit) Ichipset->bitSetChipLong((offset), (bit)) 
#define bitSetChipWord(offset, bit) Ichipset->bitSetChipWord((offset), (bit)) 
#define bitSetChipByte(offset, bit) Ichipset->bitSetChipByte((offset), (bit)) 
#define bitClrChipLong(offset, bit) Ichipset->bitClrChipLong((offset), (bit)) 
#define bitClrChipWord(offset, bit) Ichipset->bitClrChipWord((offset), (bit)) 
#define bitClrChipByte(offset, bit) Ichipset->bitClrChipByte((offset), (bit)) 
#define setCIATimingAccuracy(us) Ichipset->setCIATimingAccuracy((us)) 
#define spyCIAA(Addr) Ichipset->spyCIAA((Addr)) 
#define spyCIAB(Addr) Ichipset->spyCIAB((Addr)) 
#define setCIAClockSpeed(us) Ichipset->setCIAClockSpeed((us)) 

#endif /* INLINE4_CHIPSET_H */

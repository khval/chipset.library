#ifndef CHIPSET_INTERFACE_DEF_H
#define CHIPSET_INTERFACE_DEF_H
/*
** This file is machine generated from idltool
** Do not edit
*/ 

#include <exec/types.i>
#include <exec/exec.i>
#include <exec/interfaces.i>

STRUCTURE chipsetIFace, InterfaceData_SIZE
	    FPTR Ichipset_Obtain
	    FPTR Ichipset_Release
	    FPTR Ichipset_Expunge
	    FPTR Ichipset_Clone
	    FPTR Ichipset_allocChip
	    FPTR Ichipset_freeChip
	    FPTR Ichipset_hostAddressToChip
	    FPTR Ichipset_chipAddressToHost
	    FPTR Ichipset_writeChipLong
	    FPTR Ichipset_writeChipWord
	    FPTR Ichipset_writeChipByte
	    FPTR Ichipset_readChipLong
	    FPTR Ichipset_readChipWord
	    FPTR Ichipset_readChipByte
	    FPTR Ichipset_bitChgChipLong
	    FPTR Ichipset_bitChgChipWord
	    FPTR Ichipset_bitChgChipByte
	    FPTR Ichipset_bitSetChipLong
	    FPTR Ichipset_bitSetChipWord
	    FPTR Ichipset_bitSetChipByte
	    FPTR Ichipset_bitClrChipLong
	    FPTR Ichipset_bitClrChipWord
	    FPTR Ichipset_bitClrChipByte
	    FPTR Ichipset_setCIATimingAccuracy
	LABEL chipsetIFace_SIZE

#endif

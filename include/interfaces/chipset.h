#ifndef CHIPSET_INTERFACE_DEF_H
#define CHIPSET_INTERFACE_DEF_H

/*
** This file was machine generated by idltool 53.6.
** Do not edit
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

#ifdef __cplusplus
#ifdef __USE_AMIGAOS_NAMESPACE__
namespace AmigaOS {
#endif
extern "C" {
#endif

struct chipsetIFace
{
	struct InterfaceData Data;

	ULONG APICALL (*Obtain)(struct chipsetIFace *Self);
	ULONG APICALL (*Release)(struct chipsetIFace *Self);
	void APICALL (*Expunge)(struct chipsetIFace *Self);
	struct Interface * APICALL (*Clone)(struct chipsetIFace *Self);
	char * APICALL (*allocChip)(struct chipsetIFace *Self, uint32_t size);
	void APICALL (*freeChip)(struct chipsetIFace *Self, void * mem);
	char * APICALL (*hostAddressToChip)(struct chipsetIFace *Self, char * adr);
	char * APICALL (*chipAddressToHost)(struct chipsetIFace *Self, char * adr);
	void APICALL (*writeChipLong)(struct chipsetIFace *Self, char * offset, ULONG value);
	void APICALL (*writeChipWord)(struct chipsetIFace *Self, char * offset, ULONG value);
	void APICALL (*writeChipByte)(struct chipsetIFace *Self, char * offset, ULONG value);
	ULONG APICALL (*readChipLong)(struct chipsetIFace *Self, char * offset);
	ULONG APICALL (*readChipWord)(struct chipsetIFace *Self, char * offset);
	ULONG APICALL (*readChipByte)(struct chipsetIFace *Self, char * offset);
	void APICALL (*bitChgChipLong)(struct chipsetIFace *Self, char * offset, ULONG value);
	void APICALL (*bitChgChipWord)(struct chipsetIFace *Self, char * offset, ULONG bit);
	void APICALL (*bitChgChipByte)(struct chipsetIFace *Self, char * offset, ULONG bit);
	void APICALL (*bitSetChipLong)(struct chipsetIFace *Self, char * offset, ULONG bit);
	void APICALL (*bitSetChipWord)(struct chipsetIFace *Self, char * offset, ULONG bit);
	void APICALL (*bitSetChipByte)(struct chipsetIFace *Self, char * offset, ULONG bit);
	void APICALL (*bitClrChipLong)(struct chipsetIFace *Self, char * offset, ULONG bit);
	void APICALL (*bitClrChipWord)(struct chipsetIFace *Self, char * offset, ULONG bit);
	void APICALL (*bitClrChipByte)(struct chipsetIFace *Self, char * offset, ULONG bit);
	void APICALL (*setCIATimingAccuracy)(struct chipsetIFace *Self, int us);
	ULONG APICALL (*spyCIAA)(struct chipsetIFace *Self, ULONG Addr);
	ULONG APICALL (*spyCIAB)(struct chipsetIFace *Self, ULONG Addr);
	ULONG APICALL (*setCIAClockSpeed)(struct chipsetIFace *Self, ULONG us);
};

#ifdef __cplusplus
}
#ifdef __USE_AMIGAOS_NAMESPACE__
}
#endif
#endif

#endif /* CHIPSET_INTERFACE_DEF_H */

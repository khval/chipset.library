
/* This file was generated by idltool 53.6. Do not edit */

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

extern ULONG                VARARGS68K _chipset_Obtain(struct chipsetIFace *);
extern ULONG                VARARGS68K _chipset_Release(struct chipsetIFace *);
extern char *               VARARGS68K _chipset_allocChip(struct chipsetIFace *, uint32_t size);
extern void                 VARARGS68K _chipset_freeChip(struct chipsetIFace *, void * mem);
extern char *               VARARGS68K _chipset_hostAddressToChip(struct chipsetIFace *, char * adr);
extern char *               VARARGS68K _chipset_chipAddressToHost(struct chipsetIFace *, char * adr);
extern void                 VARARGS68K _chipset_writeChipLong(struct chipsetIFace *, char * offset, ULONG value);
extern void                 VARARGS68K _chipset_writeChipWord(struct chipsetIFace *, char * offset, ULONG value);
extern void                 VARARGS68K _chipset_writeChipByte(struct chipsetIFace *, char * offset, ULONG value);
extern ULONG                VARARGS68K _chipset_readChipLong(struct chipsetIFace *, char * offset);
extern ULONG                VARARGS68K _chipset_readChipWord(struct chipsetIFace *, char * offset);
extern ULONG                VARARGS68K _chipset_readChipByte(struct chipsetIFace *, char * offset);
extern void                 VARARGS68K _chipset_bitChgChipLong(struct chipsetIFace *, char * offset, ULONG value);
extern void                 VARARGS68K _chipset_bitChgChipWord(struct chipsetIFace *, char * offset, ULONG bit);
extern void                 VARARGS68K _chipset_bitChgChipByte(struct chipsetIFace *, char * offset, ULONG bit);
extern void                 VARARGS68K _chipset_bitSetChipLong(struct chipsetIFace *, char * offset, ULONG bit);
extern void                 VARARGS68K _chipset_bitSetChipWord(struct chipsetIFace *, char * offset, ULONG bit);
extern void                 VARARGS68K _chipset_bitSetChipByte(struct chipsetIFace *, char * offset, ULONG bit);
extern void                 VARARGS68K _chipset_bitClrChipLong(struct chipsetIFace *, char * offset, ULONG bit);
extern void                 VARARGS68K _chipset_bitClrChipWord(struct chipsetIFace *, char * offset, ULONG bit);
extern void                 VARARGS68K _chipset_bitClrChipByte(struct chipsetIFace *, char * offset, ULONG bit);
extern void                 VARARGS68K _chipset_setCIATimingAccuracy(struct chipsetIFace *, int us);
extern ULONG                VARARGS68K _chipset_spyCIAA(struct chipsetIFace *, ULONG Addr);
extern ULONG                VARARGS68K _chipset_spyCIAB(struct chipsetIFace *, ULONG Addr);
extern void                 VARARGS68K _chipset_setCIAClockSpeed(struct chipsetIFace *, double us);
extern void                 VARARGS68K _chipset_chipOwnBlitter(struct chipsetIFace *);
extern void                 VARARGS68K _chipset_chipDisownBlitter(struct chipsetIFace *);
extern void                 VARARGS68K _chipset_chipWaitBlit(struct chipsetIFace *);

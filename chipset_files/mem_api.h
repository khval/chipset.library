
extern char *_hostAddressToChip(char *addr);
extern char *_chipAddressToHost(char *addr);

extern void _bitChgChipLong(char * offset, ULONG bit);
extern void _bitChgChipWord(char * offset, ULONG bit);
extern void _bitChgChipByte(char * offset, ULONG bit);

extern void _bitSetChipLong(char * offset, ULONG bit);
extern void _bitSetChipWord(char * offset, ULONG bit);
extern void _bitSetChipByte(char * offset, ULONG bit);

extern void _bitClrChipLong(char * offset, ULONG bit);
extern void _bitClrChipWord(char * offset, ULONG bit);
extern void _bitClrChipByte(char * offset, ULONG bit);


#ifndef PROTO_CHIPSET_H
#define PROTO_CHIPSET_H

/*
**	$Id$
**
**	Prototype/inline/pragma header file combo
**
**	Copyright (c) 2010 LiveForIt Software.
**	GPL2 License..
*/

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif
#ifndef LIBRARIES_CHIPSET_H
#include <libraries/chipset.h>
#endif

/****************************************************************************/

#ifndef __NOLIBBASE__
 #ifndef __USE_BASETYPE__
  extern struct Library * chipsetBase;
 #else
  extern struct Library * chipsetBase;
 #endif /* __USE_BASETYPE__ */
#endif /* __NOLIBBASE__ */

/****************************************************************************/

#ifdef __amigaos4__
 #include <interfaces/chipset.h>
 #ifdef __USE_INLINE__
  #include <inline4/chipset.h>
 #endif /* __USE_INLINE__ */
 #ifndef CLIB_CHIPSET_PROTOS_H
  #define CLIB_CHIPSET_PROTOS_H 1
 #endif /* CLIB_CHIPSET_PROTOS_H */
 #ifndef __NOGLOBALIFACE__
  extern struct chipsetIFace *Ichipset;
 #endif /* __NOGLOBALIFACE__ */
#else /* __amigaos4__ */
 #ifndef CLIB_CHIPSET_PROTOS_H
  #include <clib/chipset_protos.h>
 #endif /* CLIB_CHIPSET_PROTOS_H */
 #if defined(__GNUC__)
  #ifndef __PPC__
   #include <inline/chipset.h>
  #else /* __PPC__ */
   #include <ppcinline/chipset.h>
  #endif /* __PPC__ */
 #elif defined(__VBCC__)
  #ifndef __PPC__
   #include <inline/chipset_protos.h>
  #endif /* __PPC__ */
 #else /* __GNUC__ */
  #include <pragmas/chipset_pragmas.h>
 #endif /* __GNUC__ */
#endif /* __amigaos4__ */

/****************************************************************************/

#endif /* PROTO_CHIPSET_H */

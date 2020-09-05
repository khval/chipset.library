/* :ts=4
    Copyright (C) 2020  Kjetil Hvalstrand

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
    USA
 */

#include <exec/types.h>
#include <libraries/chipset.h>

#include <interfaces/chipset.h>
#include <proto/exec.h>
#include <assert.h>

struct chipsetIFace * Ichipset = NULL;
static struct Library * __chipsetBase;
static struct chipsetIFace * __Ichipset;

/****************************************************************************/

extern struct Library * chipsetBase;

/****************************************************************************/

void chipset_main_constructor(void)
{
    if (chipsetBase == NULL) /* Library base is NULL, we need to open it */
    {
        /* We were called before the base constructor.
         * This means we will be called _after_ the base destructor.
         * So we cant drop the interface _after_ closing the last library base,
         * we just open the library here which ensures that.
         */
        __chipsetBase = chipsetBase = (struct Library *)IExec->OpenLibrary("chipset.library", 0L);
        assert(chipsetBase != NULL);
    }

    __Ichipset = Ichipset = (struct chipsetIFace *)IExec->GetInterface((struct Library *)chipsetBase, "main", 1, NULL);
    assert(Ichipset != NULL);
}
__attribute__((section(".ctors.zzzy"))) static void
(*chipset_main_constructor_ptr)(void) USED = chipset_main_constructor;

/****************************************************************************/

void chipset_main_destructor(void)
{
    if (__Ichipset)
    {
        IExec->DropInterface ((struct Interface *)__Ichipset);
    }
    if (__chipsetBase)
    {
        IExec->CloseLibrary((struct Library *)__chipsetBase);
    }
}
__attribute__((section(".dtors.zzzy"))) static void
(*chipset_main_destructor_ptr)(void) USED = chipset_main_destructor;

/****************************************************************************/


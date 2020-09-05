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

/****************************************************************************/

struct Library * chipsetBase = NULL;
static struct Library * __chipsetBase;

/****************************************************************************/

void chipset_base_constructor(void)
{
    if (chipsetBase != NULL)
    {
        return; /* Someone was quicker, e.g. an interface constructor */
    }
    __chipsetBase = chipsetBase = (struct Library *)IExec->OpenLibrary("chipset.library", 0L);
    assert(chipsetBase != NULL);
}
__attribute__((section(".ctors.zzzz"))) static void
(*chipset_base_constructor_ptr)(void) USED = chipset_base_constructor;

/****************************************************************************/

void chipset_base_destructor(void)
{
    if (__chipsetBase)
    {
        IExec->CloseLibrary((struct Library *)__chipsetBase);
    }
}
__attribute__((section(".dtors.zzzz"))) static void
(*chipset_base_destructor_ptr)(void) USED = chipset_base_destructor;

/****************************************************************************/


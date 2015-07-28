//---------------------------------------------------------------------------
// NEOPOP : Emulator as in Dreamland
//
// Copyright (c) 2001-2002 by neopop_uk
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version. See also the license.txt file for
//	additional informations.
//---------------------------------------------------------------------------

/*
//---------------------------------------------------------------------------
//=========================================================================

	system_rom.h

//=========================================================================
//---------------------------------------------------------------------------

  History of changes:
  ===================

21 JUL 2002 - neopop_uk
=======================================
- Created this file to remove some of the mess from 'system_main.c' and
to make rom loading more abstracted, primarily for ports to systems with
alternate methods of loading roms.

//---------------------------------------------------------------------------
*/

#ifndef __SYSTEM_ROM__
#define __SYSTEM_ROM__
//=============================================================================

BOOL system_load_rom(char* filename);
void system_unload_rom(void);

//=============================================================================
#endif

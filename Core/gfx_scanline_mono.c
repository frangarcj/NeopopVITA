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

	gfx_scanline_mono.c

//=========================================================================
//---------------------------------------------------------------------------

  History of changes:
  ===================

11 SEP 2007 - Akop Karapetyan
=======================================
- ORing every pixel with 0xF000 (normally unused, for PSP specifies
  an opaque pixel)

06 JUL 2006 - PSmonkey
=======================================
- Optimised Rendering
- Added fixed conversion table from 3bit to 16bit

05 JUL 2006 - PSmonkey
=======================================
- Changed cfb offset to scanline * 256

20 JUL 2002 - neopop_uk
=======================================
- Cleaned and tidied up for the source release

22 JUL 2002 - neopop_uk
=======================================
- Removed delayed settings retrieval, this is now done by 'interrupt.c'
- Removed hack for scanline 0!
- Fixed palettes
- Inverted all colours (1's complement) to make correct LCD emulation

24 JUL 2002 - neopop_uk
=======================================
- Added Negative/Positive colour switching

25 JUL 2002 - neopop_uk
=======================================
- Fixed a stupid bug in the dimensions of the
	right side of the hardware window.

06 AUG 2002 - neopop_uk
=======================================
- Switched to 16-bit [0BGR] rendering, should be much faster.

15 AUG 2002 - neopop_uk
=======================================
- Changed parameter 4 of drawPattern from bool to _u16, for performance
	and compatiblity reasons.
- Changed parameter 3 of Plot (pal_hi) from bool to _u16, and in turn
	parameter 6 of drawPattern - again for performance and compatiblity.

16 AUG 2002 - neopop_uk
=======================================
- Optimised things a little by removing some extraneous pointer work

//---------------------------------------------------------------------------
*/

#include "neopop.h"
#include "mem.h"
#include "gfx.h"

//=============================================================================

static _u16 uConvert3bTo16b[8] = {
	0x0000, 0x0222, 0x0444, 0x0666, 0x0888, 0x0AAA, 0x0CCC, 0x0EEE,
};

//=============================================================================

static void Plot(_u8 x, _u8* palette_ptr, _u16 pal_hi, _u8 index, _u8 depth)
{
	//_u8 data8;

	// Index & Depth check, <= to stop later sprites overwriting pixels!
	if( ( index == 0 ) || ( depth <= zbuffer[x] ) )
		return;

	//Depth check, <= to stop later sprites overwriting pixels!
	if (depth <= zbuffer[x]) return;
	zbuffer[x] = depth;

	//Get the colour of the pixel
	//if (pal_hi)
	//	data8 = palette_ptr[4 + index];
	//else
	//	data8 = palette_ptr[0 + index];

	if (pal_hi) cfb_scanline[x] = uConvert3bTo16b[ palette_ptr[4 + index] & 7 ];
	else cfb_scanline[x] = uConvert3bTo16b[ palette_ptr[0 + index] & 7 ];

//	r = (data8 & 7) << 1;
//	g = (data8 & 7) << 5;
//	b = (data8 & 7) << 9;

	if (!negative) cfb_scanline[x] = ~cfb_scanline[x] | 0xF000;
	else cfb_scanline[x] = cfb_scanline[x] | 0xF000;

	//if (negative)
	//	cfb_scanline[x] = (r | g | b);
	//else
	//	cfb_scanline[x] = ~(r | g | b);
}

static void drawPattern(_u8 screenx, _u16 tile, _u8 tiley, _u16 mirror, 
				 _u8* palette_ptr, _u16 pal, _u8 depth)
{
	//Get the data for th e "tiley'th" line of "tile".
	_u16 data = *(_u16*)(ram + 0xA000 + (tile * 16) + (tiley * 2));

	//Horizontal Flip
	if (mirror)
	{
		Plot(screenx + 7, palette_ptr, pal, (data & 0xC000) >> 0xE, depth);
		Plot(screenx + 6, palette_ptr, pal, (data & 0x3000) >> 0xC, depth);
		Plot(screenx + 5, palette_ptr, pal, (data & 0x0C00) >> 0xA, depth);
		Plot(screenx + 4, palette_ptr, pal, (data & 0x0300) >> 0x8, depth);
		Plot(screenx + 3, palette_ptr, pal, (data & 0x00C0) >> 0x6, depth);
		Plot(screenx + 2, palette_ptr, pal, (data & 0x0030) >> 0x4, depth);
		Plot(screenx + 1, palette_ptr, pal, (data & 0x000C) >> 0x2, depth);
		Plot(screenx + 0, palette_ptr, pal, (data & 0x0003) >> 0x0, depth);
	}
	else
	//Normal
	{
		Plot(screenx + 0, palette_ptr, pal, (data & 0xC000) >> 0xE, depth);
		Plot(screenx + 1, palette_ptr, pal, (data & 0x3000) >> 0xC, depth);
		Plot(screenx + 2, palette_ptr, pal, (data & 0x0C00) >> 0xA, depth);
		Plot(screenx + 3, palette_ptr, pal, (data & 0x0300) >> 0x8, depth);
		Plot(screenx + 4, palette_ptr, pal, (data & 0x00C0) >> 0x6, depth);
		Plot(screenx + 5, palette_ptr, pal, (data & 0x0030) >> 0x4, depth);
		Plot(screenx + 6, palette_ptr, pal, (data & 0x000C) >> 0x2, depth);
		Plot(screenx + 7, palette_ptr, pal, (data & 0x0003) >> 0x0, depth);
	}
}

static void gfx_draw_scroll1(_u8 depth)
{
	_u8 tx, row, line;
	_u16 data16;

	line = scanline + scroll1y;
	row = line & 7;	//Which row?

	//Draw Foreground scroll plane (Scroll 1)
	for (tx = 0; tx < 32; tx++)
	{
		data16 = *(_u16*)(ram + 0x9000 + ((tx + ((line >> 3) << 5)) << 1));
		
		//Draw the line of the tile
		drawPattern((tx << 3) - scroll1x, data16 & 0x01FF, 
			(data16 & 0x4000) ? 7 - row : row, data16 & 0x8000, ram + 0x8108,
			data16 & 0x2000, depth);
	}
}

static void gfx_draw_scroll2(_u8 depth)
{
	_u8 tx, row, line;
	_u16 data16;

	line = scanline + scroll2y;
	row = line & 7;	//Which row?

	//Draw Background scroll plane (Scroll 2)
	for (tx = 0; tx < 32; tx++)
	{
		data16 = *(_u16*)(ram + 0x9800 + ((tx + ((line >> 3) << 5)) << 1));
		
		//Draw the line of the tile
		drawPattern((tx << 3) - scroll2x, data16 & 0x01FF, 
			(data16 & 0x4000) ? 7 - row : row, data16 & 0x8000, ram + 0x8110,
			data16 & 0x2000, depth);
	}
}

void gfx_draw_scanline_mono(void)
{
	_s16 lastSpriteX;
	_s16 lastSpriteY;
	int spr, x;
	_u16 data16;

	//Get the current scanline
	scanline = ram[0x8009];
	cfb_scanline = cfb + (scanline * 256); //SCREEN_WIDTH);	//Calculate fast offset

	//memset(cfb_scanline, 0, SCREEN_WIDTH * sizeof(_u16));
	//memset(zbuffer, 0, SCREEN_WIDTH);
	for( x = 0; x < 40; x+=4 )
	{
		((_u32 *)zbuffer)[x] = 0;
		((_u32 *)zbuffer)[x+1] = 0;
		((_u32 *)zbuffer)[x+2] = 0;
		((_u32 *)zbuffer)[x+3] = 0;
	}

	//Window colour
	//r = (_u16)oowc << 1;
	//g = (_u16)oowc << 5;
	//b = (_u16)oowc << 9;
	
	data16 = ((negative) 
	  ? uConvert3bTo16b[oowc]
	  : ~uConvert3bTo16b[oowc]) | 0xF000;

	//Top
	if (scanline < winy)
	{
		// Fill Scanline
		for (x = 0; x < SCREEN_WIDTH; x++)
			cfb_scanline[x] = data16;
		// Return. we're done
		return;
	}
	else
	{
		//Middle
		if (scanline < winy + winh)
		{
			for (x = 0; x < min(winx, SCREEN_WIDTH); x++)
			{
				cfb_scanline[x] = data16;
				zbuffer[x] = 255;
			}

			for (x = min(winx + winw, SCREEN_WIDTH); x < SCREEN_WIDTH; x++)
			{
				cfb_scanline[x] = data16;
				zbuffer[x] = 255;
			}
		}
		else	//Bottom
		{
			// Fill Scanline
			for (x = 0; x < SCREEN_WIDTH; x++)
				cfb_scanline[x] = data16;
			// Return. we're done
			return;
		}
	}

	//Ignore above and below the window's top and bottom
	//if (scanline >= winy && scanline < winy + winh)
	{
		//Background colour Enabled?
		if ((bgc & 0xC0) == 0x80)
		{
			//r = (_u16)(bgc & 7) << 1;
			//g = (_u16)(bgc & 7) << 5;
			//b = (_u16)(bgc & 7) << 9;
			data16 = ~uConvert3bTo16b[ bgc & 7 ]; //(r | g | b);
		}
		else data16 = 0x0FFF;

		if (negative) data16 = ~data16;
		
		//Draw background!
		for (x = winx; x < min(winx + winw, SCREEN_WIDTH); x++)	
			cfb_scanline[x] = data16 | 0xF000;

		//Swap Front/Back scroll planes?
		if (planeSwap)
		{
			gfx_draw_scroll1(ZDEPTH_BACKGROUND_SCROLL);		//Swap
			gfx_draw_scroll2(ZDEPTH_FOREGROUND_SCROLL);
		}
		else
		{
			gfx_draw_scroll2(ZDEPTH_BACKGROUND_SCROLL);		//Normal
			gfx_draw_scroll1(ZDEPTH_FOREGROUND_SCROLL);
		}

		//Draw Sprites
		//Last sprite position, (defaults to top-left, sure?)
		lastSpriteX = 0;
		lastSpriteY = 0;
		for (spr = 0; spr < 64; spr++)
		{
			_u8 priority, row;
			_u8 sx = ram[0x8800 + (spr * 4) + 2];	//X position
			_u8 sy = ram[0x8800 + (spr * 4) + 3];	//Y position
			_s16 x = sx;
			_s16 y = sy;
			
			data16 = *(_u16*)(ram + 0x8800 + (spr * 4));
			priority = (data16 & 0x1800) >> 11;

			if (data16 & 0x0400) x = lastSpriteX + sx;	//Horizontal chain?
			if (data16 & 0x0200) y = lastSpriteY + sy;	//Vertical chain?

			//Store the position for chaining
			lastSpriteX = x;
			lastSpriteY = y;
			
			//Visible?
			if (priority == 0)	continue;

			//Scroll the sprite
			x += scrollsprx;
			y += scrollspry;

			//Off-screen?
			if (x > 248 && x < 256)	x = x - 256; else x &= 0xFF;
			if (y > 248 && y < 256)	y = y - 256; else y &= 0xFF;

			//In range?
			if (scanline >= y && scanline <= y + 7)
			{
				row = (scanline - y) & 7;	//Which row?
				drawPattern((_u8)x, data16 & 0x01FF, 
					(data16 & 0x4000) ? 7 - row : row, data16 & 0x8000,
					ram + 0x8100, data16 & 0x2000, priority << 1); 
			}
		}

	}

	//==========
}

//=============================================================================

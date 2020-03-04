/*
* (C) Copyright (C) 2007 SiRF Technology Inc.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License  
* version 2 as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston,
* MA 02111-1307 USA
*/

#ifndef __SIRFSOC_FB_H_
#define __SIRFSOC_FB_H_

#include <asm/types.h>

struct sirfsocfb_screen {
	int xstart;
	int ystart;
	int xsize;
	int ysize;
};

struct sirfsocfb_backcolour {
	int 	blankcolour_valid;	/* A non zero value means that the defined blank colour will be used in the active region
					 * else, the last displayed pixel value will be used to fill the inactive region */
	unsigned long blankcolour;	/* This register is not used */
	unsigned long background_colour;/* Pixel value to be used for region where no layer is active: must be given in 8:8:8 RGB format */
};

struct sirfsocfb_colorkeys {
	int enable;
	__u32 color_key_big;
	__u32 color_key_small;
};

enum SIRFSOCFB_FORMAT
{
	FORMAT_RGBA_8888 = 0,
	FORMAT_RGB_565,
	FORMAT_BGRA_8888,
	FORMAT_YCbCr_422_SP,
	FORMAT_YCbCr_420_SP,
	FORMAT_YCrCb_420_SP,
	FORMAT_YCbCr_422_I,
	FORMAT_YCbCr_420_P,
};

#define bYUVFormat(A) (A>=FORMAT_YCbCr_422_SP)

struct sirfsocfb_createlayer {
	__u32 width;
	__u32 height;
	enum SIRFSOCFB_FORMAT format;
	__u32 wstride_byte; /* in byte */
	__u32 hstride_byte; /* in byte */
	__u32 wstride_pixel; /* in pixel */
	__u32 hstride_pixel; /* in pixel */
	__u32 num_buffers;
};

enum SIRFSOCFB_FLUSH_CACHE_OP
{
    FLUSH_CACHE_OP_INVALID = 0,
    FLUSH_CACHE_OP_CLEAN,
    FLUSH_CACHE_OP_FLUSH,
};

struct sirfsocfb_flush_cache_addr {
    __u32 phy_addr_start;
    __u32 phy_addr_size;
    enum SIRFSOCFB_FLUSH_CACHE_OP flush_cache_op;
};

	
/* SiRF SoC FB specific ioctls */
#define SIRFSOCFB_SET_TOPLAYER	_IO('S', 0x0)
#define SIRFSOCFB_GET_TOPLAYER	_IOR('S', 0x0, int)
#define SIRFSOCFB_SET_ALPHA	_IOW('S', 0x1, unsigned long)
#define SIRFSOCFB_GET_ALPHA	_IOR('S', 0x1, unsigned long)
#define SIRFSOCFB_SET_SCRSIZE	_IOW('S', 0x2, struct sirfsocfb_screen)
#define SIRFSOCFB_GET_SCRSIZE	_IOR('S', 0x2, struct sirfsocfb_screen)
#define SIRFSOCFB_GET_BACKCOLOR	_IOR('S', 0x3, struct sirfsocfb_backcolour)
#define SIRFSOCFB_SET_BACKCOLOR	_IOW('S', 0x3, struct sirfsocfb_backcolour)
#define SIRFSOCFB_SET_COLORKEYS _IOW('S', 0x4, struct sirfsocfb_colorkeys)
#define SIRFSOCFB_GET_COLORKEYS _IOR('S', 0x4, struct sirfsocfb_colorkeys)
#define SIRFSOCFB_Q_BUFFER	_IOR('S', 0x5, int)
#define SIRFSOCFB_DQ_BUFFER	_IOW('S', 0x5, int)
#define SIRFSOCFB_CREATE_LAYER  _IOWR('S', 0x6, struct sirfsocfb_createlayer)
#define SIRFSOCFB_DESTROY_LAYER _IO('S', 0x6)
#define SIRFSOCFB_ENABLE_LAYER	_IO('S', 0x7)
#define SIRFSOCFB_DISABLE_LAYER	_IO('S', 0x8)
#define SIRFSOCFB_SET_DMASIZE	_IOW('S', 0x8, struct sirfsocfb_screen)
#define SIRFSOCFB_GET_DMASIZE	_IOR('S', 0x8, struct sirfsocfb_screen)
#define SIRFSOCFB_FLUSH_CACHE  _IOW('S', 0x9, struct sirfsocfb_flush_cache_addr)

#endif  /* __SIRFSOC_FB_H_ */


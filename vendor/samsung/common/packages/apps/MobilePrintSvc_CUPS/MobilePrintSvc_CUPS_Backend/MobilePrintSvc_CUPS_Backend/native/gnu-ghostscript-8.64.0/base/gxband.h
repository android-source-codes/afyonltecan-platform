/* Copyright (C) 2001-2006 Artifex Software, Inc.
   All Rights Reserved.
  
  This file is part of GNU ghostscript

  GNU ghostscript is free software; you can redistribute it and/or
  modify it under the terms of the version 2 of the GNU General Public
  License as published by the Free Software Foundation.

  GNU ghostscript is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along with
  ghostscript; see the file COPYING. If not, write to the Free Software Foundation,
  Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

*/

/* $Id: gxband.h,v 1.9 2007/09/11 15:24:09 Arabidopsis Exp $ */
/* Band-processing parameters for Ghostscript */

#ifndef gxband_INCLUDED
#  define gxband_INCLUDED

#include "gxclio.h"

/*
 * Define the parameters controlling banding.
 */
typedef struct gx_band_params_s {
    bool page_uses_transparency; /* PDF 1.4 transparency is used on the page */
    int BandWidth;		/* (optional) band width in pixels */
    int BandHeight;		/* (optional) */
    long BandBufferSpace;	/* (optional) */
} gx_band_params_t;

#define BAND_PARAMS_INITIAL_VALUES 0, 0, 0

/*
 * Define information about the colors used on a page.
 */
typedef struct gx_colors_used_s {
    gx_color_index or;		/* the "or" of all the used colors */
    bool slow_rop;		/* true if any RasterOps that can't be */
				/* executed plane-by-plane on CMYK devices */
} gx_colors_used_t;

/*
 * We want to store color usage information for each band in the page_info
 * structure, but we also want this structure to be of a fixed (and
 * reasonable) size.  We do this by allocating a fixed number of colors_used
 * structures in the page_info structure, and if there are more bands than
 * we have allocated, we simply reduce the precision of the information by
 * letting each colors_used structure cover multiple bands.
 *
 * 30 entries would be large enough to cover A4 paper (11.3") at 600 dpi
 * with 256-scan-line bands.  We pick 50 somewhat arbitrarily.
 */
#define PAGE_INFO_NUM_COLORS_USED 50

/*
 * Define the information for a saved page.
 */
typedef struct gx_band_page_info_s {
    char cfname[gp_file_name_sizeof];	/* command file name */
    clist_file_ptr cfile;	/* command file, normally 0 */
    char bfname[gp_file_name_sizeof];	/* block file name */
    clist_file_ptr bfile;	/* block file, normally 0 */
    const clist_io_procs_t *io_procs;
    uint tile_cache_size;	/* size of tile cache */
    int64_t bfile_end_pos;		/* ftell at end of bfile */
    gx_band_params_t band_params;  /* parameters used when writing band list */
				/* (actual values, no 0s) */
    int scan_lines_per_colors_used; /* number of scan lines per colors_used */
				/* entry (a multiple of the band height) */
    gx_colors_used_t band_colors_used[PAGE_INFO_NUM_COLORS_USED];  /* colors used on the page */
} gx_band_page_info_t;
#define PAGE_INFO_NULL_VALUES\
  { 0 }, 0, { 0 }, NULL, 0, 0, 0, { BAND_PARAMS_INITIAL_VALUES },\
  0x3fffffff, { { 0 } }

/*
 * By convention, the structure member containing the above is called
 * page_info.  Define shorthand accessors for its members.
 */
#define page_cfile page_info.cfile
#define page_cfname page_info.cfname
#define page_bfile page_info.bfile
#define page_bfname page_info.bfname
#define page_tile_cache_size page_info.tile_cache_size
#define page_bfile_end_pos page_info.bfile_end_pos
#define page_band_height page_info.band_params.BandHeight

#endif /* ndef gxband_INCLUDED */

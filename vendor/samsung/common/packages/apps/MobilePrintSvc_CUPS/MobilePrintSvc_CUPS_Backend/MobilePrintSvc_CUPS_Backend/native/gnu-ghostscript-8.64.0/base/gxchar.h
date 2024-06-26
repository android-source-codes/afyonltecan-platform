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

/* $Id: gxchar.h,v 1.10 2008/03/23 15:28:17 Arabidopsis Exp $ */
/* Internal character definition for Ghostscript library */
/* Requires gsmatrix.h, gxfixed.h */

#ifndef gxchar_INCLUDED
#  define gxchar_INCLUDED

#include "gschar.h"
#include "gxtext.h"

/* The type of cached characters is opaque. */
#ifndef cached_char_DEFINED
#  define cached_char_DEFINED
typedef struct cached_char_s cached_char;
#endif

/* The type of cached font/matrix pairs is opaque. */
#ifndef cached_fm_pair_DEFINED
#  define cached_fm_pair_DEFINED
typedef struct cached_fm_pair_s cached_fm_pair;
#endif

/* The type of font objects is opaque. */
#ifndef gs_font_DEFINED
#  define gs_font_DEFINED
typedef struct gs_font_s gs_font;
#endif

/* The type of text enum objects is opaque. */
#ifndef gs_text_enum_DEFINED
#  define gs_text_enum_DEFINED
typedef struct gs_text_enum_s gs_text_enum_t;
#endif

/* The types of memory and null devices may be opaque. */
#ifndef gx_device_memory_DEFINED
#  define gx_device_memory_DEFINED
typedef struct gx_device_memory_s gx_device_memory;
#endif
#ifndef gx_device_null_DEFINED
#  define gx_device_null_DEFINED
typedef struct gx_device_null_s gx_device_null;
#endif

/* An enumeration object for string display. */
typedef enum {
    sws_none,
    sws_cache,			/* setcachedevice[2] */
    sws_no_cache,		/* setcharwidth */
    sws_cache_width_only,	/* setcharwidth for xfont char */
    sws_retry			/* retry setcachedevice[2] */
} show_width_status;
struct gs_show_enum_s {
    /* Put this first for subclassing. */
    gs_text_enum_common;	/* (procs, text, index) */
    /* Following are set at creation time */
    bool auto_release;		/* true if old API, false if new */
    gs_state *pgs;
    int level;			/* save the level of pgs */
    gs_char_path_mode charpath_flag;
    gs_state *show_gstate;	/* for setting pgs->show_gstate */
				/* at returns/callouts */
    int can_cache;		/* -1 if can't use cache at all, */
				/* 0 if can read but not load, */
				/* 1 if can read and load */
    gs_int_rect ibox;		/* int version of quick-check */
				/* (inner) clipping box */
    gs_int_rect obox;		/* int version of (outer) clip box */
    int ftx, fty;		/* transformed font translation */
    /* Following are updated dynamically */
    gs_glyph (*encode_char)(gs_font *, gs_char, gs_glyph_space_t);  /* copied from font */
    gs_log2_scale_point fapi_log2_scale; /* scaling factors for oversampling with FAPI, -1 = not valid */
    gs_point fapi_glyph_shift;          /* glyph shift for FAPI-handled font */
    gx_device_memory *dev_cache;	/* cache device */
    gx_device_memory *dev_cache2;	/* underlying alpha memory device, */
				/* if dev_cache is an alpha buffer */
    gx_device_null *dev_null;	/* null device for stringwidth */
    /*uint index; */		/* index within string */
    /*uint xy_index;*/		/* index within X/Y widths */
    /*gs_char returned.current_char;*/	/* current char for render or move */
    /*gs_glyph returned.current_glyph;*/	/* current glyph ditto */
    gs_fixed_point wxy;		/* width of current char in device coords */
    gs_point wxy_float;		/* same for huge characters */
    bool use_wxy_float;
    gs_fixed_point origin;	/* unrounded origin of current char */
				/* in device coords, needed for */
				/* charpath and WMode=1 */
    cached_char *cc;		/* being accumulated */
    /*gs_point returned.total_width;*/		/* total width of string, set at end */
    show_width_status width_status;
    /*gs_log2_scale_point log2_scale;*/
    int (*continue_proc) (gs_show_enum *);	/* continuation procedure */
};
#define gs_show_enum_s_DEFINED
/* The structure descriptor is public for gschar.c. */
#define public_st_gs_show_enum() /* in gxchar.c */\
  gs_public_st_composite(st_gs_show_enum, gs_show_enum, "gs_show_enum",\
    show_enum_enum_ptrs, show_enum_reloc_ptrs)

/* Get the current character code. */
int gx_current_char(const gs_text_enum_t * pte);

/* Cached character procedures (in gxccache.c and gxccman.c) */
#ifndef gs_font_dir_DEFINED
#  define gs_font_dir_DEFINED
typedef struct gs_font_dir_s gs_font_dir;

#endif
int  gx_alloc_char_bits(gs_font_dir *, gx_device_memory *, gx_device_memory *, ushort, ushort, const gs_log2_scale_point *, int, cached_char **);
void gx_open_cache_device(gx_device_memory *, cached_char *);
void gx_free_cached_char(gs_font_dir *, cached_char *);
int  gx_add_cached_char(gs_font_dir *, gx_device_memory *, cached_char *, cached_fm_pair *, const gs_log2_scale_point *);
void gx_add_char_bits(gs_font_dir *, cached_char *, const gs_log2_scale_point *);
cached_char *
            gx_lookup_cached_char(const gs_font *, const cached_fm_pair *, gs_glyph, int, int, gs_fixed_point *);
int gx_lookup_xfont_char(const gs_state * pgs, cached_fm_pair * pair,
		     gs_char chr, gs_glyph glyph, int wmode, cached_char **pcc);
int gx_image_cached_char(gs_show_enum *, cached_char *);
void gx_compute_text_oversampling(const gs_show_enum * penum, const gs_font *pfont,
				  int alpha_bits, gs_log2_scale_point *p_log2_scale);
int set_char_width(gs_show_enum *penum, gs_state *pgs, floatp wx, floatp wy);
int gx_default_text_restore_state(gs_text_enum_t *pte);
int gx_hld_stringwidth_begin(gs_imager_state * pis, gx_path **path);

#endif /* gxchar_INCLUDED */

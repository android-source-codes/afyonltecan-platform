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

/* $Id: sdct.h,v 1.8 2007/09/11 15:24:27 Arabidopsis Exp $ */
/* Definitions for DCT filters */
/* Requires stream.h, strimpl.h, jpeg/jpeglib.h */

#ifndef sdct_INCLUDED
#  define sdct_INCLUDED

#include "setjmp_.h"		/* for jmp_buf */

/* ------ DCT filters ------ */

/*
 * We don't want to allocate JPEG's private data directly from
 * the C heap, but we must allocate it as immovable; and to avoid
 * garbage collection issues, we must keep GC-traceable pointers
 * to every block allocated.
 */
typedef struct jpeg_block_s jpeg_block_t;
struct jpeg_block_s {
    jpeg_block_t *next;
    void *data;
};
#define private_st_jpeg_block()	/* in sjpegc.c */\
  gs_private_st_ptrs2(st_jpeg_block, jpeg_block_t, "jpeg_block_t",\
    jpeg_block_enum_ptrs, jpeg_block_reloc_ptrs, next, data)

/*
 * Define the stream state.
 * The jpeg_xxx_data structs are allocated in immovable memory
 * to simplify use of the IJG library.
 */
#define jpeg_stream_data_common\
		/* We put a copy of the stream template here, because */\
		/* the minimum buffer sizes depend on the image parameters. */\
	stream_template template;\
	struct jpeg_error_mgr err;\
	gsfix_jmp_buf exit_jmpbuf;\
	gs_memory_t *memory;	/* heap for library allocations */\
        jpeg_block_t *blocks;   /* ptr to allocated data block list */\
		/* The following are documented in Adobe TN 5116. */\
	int Picky;		/* 0 or 1 */\
	int Relax		/* 0 or 1 */

typedef struct jpeg_stream_data_s {
    jpeg_stream_data_common;
} jpeg_stream_data;

/* Define initialization for the non-library part of the stream state. */
#define jpeg_stream_data_common_init(pdata)\
BEGIN\
  (pdata)->Picky = 0;\
  (pdata)->Relax = 0;\
  (pdata)->blocks = 0;\
END

typedef struct jpeg_compress_data_s {
    jpeg_stream_data_common;
    /* cinfo must immediately follow the common fields */
    struct jpeg_compress_struct cinfo;
    struct jpeg_destination_mgr destination;
    byte finish_compress_buf[100];
    int fcb_size, fcb_pos;
} jpeg_compress_data;

extern_st(st_jpeg_compress_data);
#define public_st_jpeg_compress_data()	/* in sdcte.c */\
  gs_public_st_ptrs1(st_jpeg_compress_data, jpeg_compress_data,\
    "JPEG compress data", jpeg_compress_data_enum_ptrs, jpeg_compress_data_reloc_ptrs, blocks)

typedef struct jpeg_decompress_data_s {
    jpeg_stream_data_common;
    /* dinfo must immediately follow the common fields, */
    /* so that it has same offset as cinfo. */
    struct jpeg_decompress_struct dinfo;
    struct jpeg_source_mgr source;
    long skip;			/* # of bytes remaining to skip in input */
    bool input_eod;		/* true when no more input data available */
    bool faked_eoi;		/* true when fill_input_buffer inserted EOI */
    byte *scanline_buffer;	/* buffer for oversize scanline, or NULL */
    uint bytes_in_scanline;	/* # of bytes remaining to output from same */
} jpeg_decompress_data;

#define private_st_jpeg_decompress_data()	/* in zfdctd.c */\
  gs_private_st_ptrs2(st_jpeg_decompress_data, jpeg_decompress_data,\
    "JPEG decompress data", jpeg_decompress_data_enum_ptrs,\
    jpeg_decompress_data_reloc_ptrs, blocks, scanline_buffer)

/* The stream state itself.  This is kept in garbage-collectable memory. */
typedef struct stream_DCT_state_s {
    stream_state_common;
    /* The following are set before initialization. */
    /* Note that most JPEG parameters go straight into */
    /* the IJG data structures, not into this struct. */
    gs_const_string Markers;	/* NULL if no Markers parameter */
    float QFactor;
    int ColorTransform;		/* -1 if not specified */
    bool NoMarker;		/* DCTEncode only */
    gs_memory_t *jpeg_memory;	/* heap for library allocations */
    /* This is a pointer to immovable storage. */
    union _jd {
	jpeg_stream_data *common;
	jpeg_compress_data *compress;
	jpeg_decompress_data *decompress;
    } data;
    /* DCTEncode sets this before initialization;
     * DCTDecode cannot set it until the JPEG headers are read.
     */
    uint scan_line_size;
    /* The following are updated dynamically. */
    int phase;
} stream_DCT_state;

/* The state descriptor is public only to allow us to split up */
/* the encoding and decoding filters. */
extern_st(st_DCT_state);
#define public_st_DCT_state()	/* in sdctc.c */\
  gs_public_st_const_strings1_ptrs1(st_DCT_state, stream_DCT_state,\
    "DCTEncode/Decode state", dct_enum_ptrs, dct_reloc_ptrs, Markers, data.common)
/*
 * NOTE: the client *must* invoke the set_defaults procedure in the
 * template before calling the init procedure.
 */
extern const stream_template s_DCTD_template;
extern const stream_template s_DCTE_template;

/* Define an internal procedure for setting stream defaults. */
/* Clients do not call this. */
void s_DCT_set_defaults(stream_state * st);

#endif /* sdct_INCLUDED */

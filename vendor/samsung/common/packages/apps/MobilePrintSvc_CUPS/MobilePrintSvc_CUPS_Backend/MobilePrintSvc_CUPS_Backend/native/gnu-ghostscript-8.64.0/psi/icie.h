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

/* $Id: icie.h,v 1.8 2007/09/11 15:24:32 Arabidopsis Exp $ */
/* Internal definitions for interpreter CIE color handling */

#ifndef icie_INCLUDED
#  define icie_INCLUDED

/*
 * All of the routines below are exported by zcie.c for zcrd.c,
 * except for cie_cache_joint which is exported by zcrd.c for zcie.c.
 */

/* ------ Parameter acquisition ------ */

/* Get a range array parameter from a dictionary. */
/* We know that count <= 4. */
int dict_ranges_param(const gs_memory_t *mem,
		      const ref * pdref, const char *kstr, int count,
		      gs_range * prange);

/* Get 3 ranges from a dictionary. */
int dict_range3_param(const gs_memory_t *mem,
		      const ref *pdref, const char *kstr,
		      gs_range3 *prange3);

/* Get a 3x3 matrix parameter from a dictionary. */
int dict_matrix3_param(const gs_memory_t *mem, const ref *pdref, 
		       const char *kstr,
		       gs_matrix3 *pmat3);

/* Get an array of procedures from a dictionary. */
/* We know count <= countof(empty_procs). */
int dict_proc_array_param(const gs_memory_t *mem, const ref *pdict, 
			  const char *kstr,
			  uint count, ref * pparray);

/* Get 3 procedures from a dictionary. */
int dict_proc3_param(const gs_memory_t *mem, const ref *pdref, 
		     const char *kstr, ref proc3[3]);

/* Get WhitePoint and BlackPoint values. */
int cie_points_param(const gs_memory_t *mem, 
		     const ref * pdref, gs_cie_wb * pwb);

/* Process a 3- or 4-dimensional lookup table from a dictionary. */
/* The caller has set pclt->n and pclt->m. */
/* ptref is known to be a readable array of size at least n+1. */
int cie_table_param(const ref * ptable, gx_color_lookup_table * pclt,
		    gs_memory_t * mem);

/* ------ Internal routines ------ */

int cie_set_finish(i_ctx_t *, gs_color_space *,
		   const ref_cie_procs *, int, int);

int cie_cache_push_finish(i_ctx_t *i_ctx_p, op_proc_t finish_proc,
			  gs_ref_memory_t * imem, void *data);
int cie_prepare_cache(i_ctx_t *i_ctx_p, const gs_range * domain,
		      const ref * proc, cie_cache_floats * pcache,
		      void *container, gs_ref_memory_t * imem,
		      client_name_t cname);
int cie_prepare_caches_4(i_ctx_t *i_ctx_p, const gs_range * domains,
			 const ref * procs,
			 cie_cache_floats * pc0,
			 cie_cache_floats * pc1,
			 cie_cache_floats * pc2,
			 cie_cache_floats * pc3 /* may be 0 */,
			 void *container,
			 gs_ref_memory_t * imem, client_name_t cname);
#define cie_prepare_cache3(p,d3,p3,c3,pcie,imem,cname)\
  cie_prepare_caches_4(p, (d3)->ranges, p3,\
		       &(c3)->floats, &(c3)[1].floats, &(c3)[2].floats,\
		       NULL, pcie, imem, cname)
#define cie_prepare_cache4(p,d4,p4,c4,pcie,imem,cname)\
  cie_prepare_caches_4(p, (d4)->ranges, p4,\
		       &(c4)->floats, &(c4)[1].floats, &(c4)[2].floats,\
		       &(c4)[3].floats, pcie, imem, cname)

int cie_cache_joint(i_ctx_t *, const ref_cie_render_procs *,
		    const gs_cie_common *, gs_state *);

#endif /* icie_INCLUDED */

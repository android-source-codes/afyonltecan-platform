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

/* $Id: ttfoutl.h,v 1.8 2007/09/11 15:24:42 Arabidopsis Exp $ */
/* The TrueType instruction interpreter interface definition. */

#ifndef incl_ttfoutl
#define incl_ttfoutl

#ifndef TFace_defined
#define TFace_defined
typedef struct _TFace TFace;
#endif

#ifndef TInstance_defined
#define TInstance_defined
typedef struct _TInstance TInstance;
#endif

#ifndef TExecution_Context_defined
#define TExecution_Context_defined
typedef struct _TExecution_Context TExecution_Context;
#endif

#ifndef ttfInterpreter_DEFINED
#  define ttfInterpreter_DEFINED
typedef struct ttfInterpreter_s ttfInterpreter;
#endif

/* Define auxiliary data types for the TT interpreter. */

typedef struct ttfMemoryDescriptor_s ttfMemoryDescriptor;

typedef struct {
    double a, b, c, d, tx, ty;
} FloatMatrix;

typedef struct {
    double x, y;
} FloatPoint;

#if   ARCH_LOG2_SIZEOF_LONG == 2
typedef signed long F26Dot6;
#elif ARCH_LOG2_SIZEOF_INT  == 2
typedef signed int  F26Dot6;
#else
#error "No appropriate type for Fixed 26.6 Floats"
#endif

typedef struct { 
    F26Dot6 x;
    F26Dot6 y;
} F26Dot6Point;

/* Define an abstract class for accessing memory managers from the TT interpreter. */
typedef struct ttfMemory_s ttfMemory;
struct ttfMemory_s {   
    void *(*alloc_bytes)(ttfMemory *, int size,  const char *cname);
    void *(*alloc_struct)(ttfMemory *, const ttfMemoryDescriptor *,  const char *cname);
    void (*free)(ttfMemory *, void *p,  const char *cname);
} ;

typedef struct ttfSubGlyphUsage_s ttfSubGlyphUsage;

/* Define a capsule for the TT interpreter. */
struct ttfInterpreter_s {
    TExecution_Context *exec;
    ttfSubGlyphUsage *usage;
    int usage_size;
    int usage_top;
    int lock;
    ttfMemory *ttf_memory;
};

/* Define TT interpreter return codes. */
typedef enum {
    fNoError,
    fTableNotFound,
    fNameNotFound,
    fMemoryError,
    fUnimplemented,
    fCMapNotFound,
    fGlyphNotFound,
    fBadFontData,
    fPatented,
    fBadInstruction
} FontError;

/* Define an abstract class for accessing TT data from the TT interpreter. */
typedef struct ttfReader_s ttfReader;
struct ttfReader_s {
    bool   (*Eof)(ttfReader *);
    void   (*Read)(ttfReader *, void *p, int n);
    void   (*Seek)(ttfReader *, int nPos);
    int    (*Tell)(ttfReader *);
    bool   (*Error)(ttfReader *);
    int    (*LoadGlyph)(ttfReader *, int nIndex, const byte **, int *);
    void   (*ReleaseGlyph)(ttfReader *, int nIndex);
    int    (*get_metrics)(const ttfReader *ttf, uint glyph_index, bool bVertical, 
			  short *sideBearing, unsigned short *nAdvance);
};

/* Define an auxiliary structure for ttfFont. */
typedef struct {
    int nPos, nLen;
} ttfPtrElem;

/* Define a capsule for a TT face. 
   Diue to historical reason the name is some misleading.
   It should be ttfFace. */
#ifndef ttfFont_DEFINED
#  define ttfFont_DEFINED
typedef struct ttfFont_s ttfFont;
#endif
struct ttfFont_s {
    ttfPtrElem t_cvt_;
    ttfPtrElem t_fpgm;
    ttfPtrElem t_glyf;
    ttfPtrElem t_head;
    ttfPtrElem t_hhea;
    ttfPtrElem t_hmtx;
    ttfPtrElem t_vhea;
    ttfPtrElem t_vmtx;
    ttfPtrElem t_loca;
    ttfPtrElem t_maxp;
    ttfPtrElem t_prep;
    ttfPtrElem t_cmap;
    unsigned short nUnitsPerEm;
    unsigned int nFlags;
    unsigned int nNumGlyphs;
    unsigned int nMaxComponents;
    unsigned int nLongMetricsVert;
    unsigned int nLongMetricsHorz;
    unsigned int nIndexToLocFormat;
    bool    patented;
    bool    design_grid;
    TFace *face;
    TInstance *inst;
    TExecution_Context  *exec;
    ttfInterpreter *tti;
    void (*DebugRepaint)(ttfFont *);
    int (*DebugPrint)(ttfFont *, const char *s, ...);
};

void ttfFont__init(ttfFont *this, ttfMemory *mem, 
		    void (*DebugRepaint)(ttfFont *),
		    int (*DebugPrint)(ttfFont *, const char *s, ...));
void ttfFont__finit(ttfFont *this);
FontError ttfFont__Open(ttfInterpreter *, ttfFont *, ttfReader *r, 
			unsigned int nTTC, float w, float h, 
			bool design_grid);

/* Define an abstract class for exporting outlines from the TT interpreter. */
typedef struct ttfExport_s ttfExport;
struct ttfExport_s {
    bool bPoints, bOutline;
    void (*MoveTo)(ttfExport *, FloatPoint*);
    void (*LineTo)(ttfExport *, FloatPoint*);
    void (*CurveTo)(ttfExport *, FloatPoint*, FloatPoint*, FloatPoint*);
    void (*Close)(ttfExport *);
    void (*Point)(ttfExport *, FloatPoint*, bool bOnCurve, bool bNewPath);
    void (*SetWidth)(ttfExport *, FloatPoint*);
    void (*DebugPaint)(ttfExport *);
};

int ttfInterpreter__obtain(ttfMemory *mem, ttfInterpreter **ptti);
void ttfInterpreter__release(ttfInterpreter **ptti);

/* Define an class for outline description. */
typedef struct { 
    bool    bCompound;
    int     contourCount;
    uint    pointCount;
    F26Dot6Point  advance;
    F26Dot6 sideBearing;
    F26Dot6   xMinB, yMinB, xMaxB, yMaxB;
} ttfGlyphOutline;

/* Define an class for generating TT outlines. */
typedef struct {
    bool bOutline;
    bool bFirst;
    bool bVertical;
    int  nPointsTotal;
    int  nContoursTotal;
    F26Dot6 ppx, ppy;
    ttfReader *r;
    ttfExport *exp;
    ttfFont *pFont;
    ttfGlyphOutline out;
    FloatMatrix post_transform;
} ttfOutliner;

void ttfOutliner__init(ttfOutliner *, ttfFont *f, ttfReader *r, ttfExport *exp, 
			bool bOutline, bool bFirst, bool bVertical);
FontError ttfOutliner__Outline(ttfOutliner *this, int glyphIndex,
	float orig_x, float orig_y, FloatMatrix *m1);
void ttfOutliner__DrawGlyphOutline(ttfOutliner *this);

#endif

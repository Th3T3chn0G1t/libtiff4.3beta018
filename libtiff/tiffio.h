/* $Header: /usr/people/sam/tiff/libtiff/RCS/tiffio.h,v 1.90 1995/06/27 00:54:10 sam Exp $ */

/*
 * Copyright (c) 1988-1995 Sam Leffler
 * Copyright (c) 1991-1995 Silicon Graphics, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and 
 * its documentation for any purpose is hereby granted without fee, provided
 * that (i) the above copyright notices and this permission notice appear in
 * all copies of the software and related documentation, and (ii) the names of
 * Sam Leffler and Silicon Graphics may not be used in any advertising or
 * publicity relating to the software without the specific, prior written
 * permission of Sam Leffler and Silicon Graphics.
 * 
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY 
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  
 * 
 * IN NO EVENT SHALL SAM LEFFLER OR SILICON GRAPHICS BE LIABLE FOR
 * ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF 
 * LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE 
 * OF THIS SOFTWARE.
 */

#ifndef _TIFFIO_
#define	_TIFFIO_

#include <asys/base.h>

/*
 * TIFF I/O Library Definitions.
 */
#include "tiff.h"

/*
 * TIFF is defined as an incomplete type to hide the
 * library's internal data structures from clients.
 */
typedef	struct tiff TIFF;

/*
 * The following typedefs define the intrinsic size of
 * data types used in the *exported* interfaces.  These
 * definitions depend on the proper definition of types
 * in tiff.h.  Note also that the varargs interface used
 * pass tag types and values uses the types defined in
 * tiff.h directly.
 *
 * NB: ttag_t is unsigned int and not unsigned short because
 *     ANSI C requires that the type before the ellipsis be a
 *     promoted type (i.e. one of int, unsigned int, pointer,
 *     or double) and because we defined pseudo-tags that are
 *     outside the range of legal Aldus-assigned tags.
 * NB: tsize_t is int32 and not uint32 because some functions
 *     return -1.
 * NB: toff_t is not off_t for many reasons; TIFFs max out at
 *     32-bit file offsets being the most important
 */
typedef	unsigned int ttag_t;	/* directory tag */
typedef	uint16 tdir_t;		/* directory index */
typedef	uint16 tsample_t;	/* sample number */
typedef	uint32 tstrip_t;	/* strip number */
typedef uint32 ttile_t;		/* tile number */
typedef	int32 tsize_t;		/* i/o size in bytes */

#include <asys/base.h>

#ifdef ASYS_WIN32
# ifndef _CRT_SECURE_NO_WARNINGS
#  define _CRT_SECURE_NO_WARNINGS
# endif
# ifndef _CRT_NONSTDC_NO_WARNINGS
#  define _CRT_NONSTDC_NO_WARNINGS
# endif
# ifdef _MSC_VER
#  pragma warning(push)
#  pragma warning(disable: 4668) /* Symbol not defined as macro. */
# endif
# include <windows.h>
# ifdef _MSC_VER
#  pragma warning(pop)
# endif
# define TIFF_IO_WIN3
typedef	HFILE thandle_t;
#elif defined(ASYS_UNIX)
# define TIFF_IO_UNIX
typedef	int thandle_t;
#elif defined(ASYS_STDC)
# include <stdio.h>
# define TIFF_IO_STDC
typedef	FILE* thandle_t;
#else
typedef	void* thandle_t;
#endif

typedef	void* tdata_t;		/* image data ref */
typedef	int32 toff_t;		/* file offset */

#ifndef NULL
#define	NULL	0
#endif

/*
 * Flags to pass to TIFFPrintDirectory to control
 * printing of data structures that are potentially
 * very large.   Bit-or these flags to enable printing
 * multiple items.
 */
#define	TIFFPRINT_NONE		0x0		/* no extra info */
#define	TIFFPRINT_STRIPS	0x1		/* strips/tiles info */
#define	TIFFPRINT_CURVES	0x2		/* color/gray response curves */
#define	TIFFPRINT_COLORMAP	0x4		/* colormap */
#define	TIFFPRINT_JPEGQTABLES	0x100		/* JPEG Q matrices */
#define	TIFFPRINT_JPEGACTABLES	0x200		/* JPEG AC tables */
#define	TIFFPRINT_JPEGDCTABLES	0x200		/* JPEG DC tables */

/*
 * RGBA-style image support.
 */
typedef	unsigned char TIFFRGBValue;		/* 8-bit samples */
typedef struct _TIFFRGBAImage TIFFRGBAImage;
/*
 * The image reading and conversion routines invoke
 * ``put routines'' to copy/image/whatever tiles of
 * raw image data.  A default set of routines are 
 * provided to convert/copy raw image data to 8-bit
 * packed ABGR format rasters.  Applications can supply
 * alternate routines that unpack the data into a
 * different format or, for example, unpack the data
 * and draw the unpacked raster on the display.
 */
typedef void (*tileContigRoutine)
    (TIFFRGBAImage*, uint32*, uint32, uint32, uint32, uint32, int32, int32,
	unsigned char*);
typedef void (*tileSeparateRoutine)
    (TIFFRGBAImage*, uint32*, uint32, uint32, uint32, uint32, int32, int32,
	unsigned char*, unsigned char*, unsigned char*, unsigned char*);
/*
 * RGBA-reader state.
 */
typedef struct {				/* YCbCr->RGB support */
	TIFFRGBValue* clamptab;			/* range clamping table */
	int*	Cr_r_tab;
	int*	Cb_b_tab;
	int32*	Cr_g_tab;
	int32*	Cb_g_tab;
	float	coeffs[3];			/* cached for repeated use */
} TIFFYCbCrToRGB;

struct _TIFFRGBAImage {
	TIFF*	tif;				/* image handle */
	int	stoponerr;			/* stop on read error */
	int	isContig;			/* data is packed/separate */
	int	alpha;				/* type of alpha data present */
	uint32	width;				/* image width */
	uint32	height;				/* image height */
	uint16	bitspersample;			/* image bits/sample */
	uint16	samplesperpixel;		/* image samples/pixel */
	uint16	orientation;			/* image orientation */
	uint16	photometric;			/* image photometric interp */
	uint16*	redcmap;			/* colormap pallete */
	uint16*	greencmap;
	uint16*	bluecmap;
						/* get image data routine */
	int	(*get)(TIFFRGBAImage*, uint32*, uint32, uint32);
	union {
	    void (*any)(TIFFRGBAImage*);
	    tileContigRoutine	contig;
	    tileSeparateRoutine	separate;
	} put;					/* put decoded strip/tile */
	TIFFRGBValue* Map;			/* sample mapping array */
	uint32** BWmap;				/* black&white map */
	uint32** PALmap;			/* palette image map */
	TIFFYCbCrToRGB* ycbcr;			/* YCbCr conversion state */
};

/*
 * Macros for extracting components from the
 * packed ABGR form returned by TIFFReadRGBAImage.
 */
#define	TIFFGetR(abgr)	((abgr) & 0xff)
#define	TIFFGetG(abgr)	(((abgr) >> 8) & 0xff)
#define	TIFFGetB(abgr)	(((abgr) >> 16) & 0xff)
#define	TIFFGetA(abgr)	(((abgr) >> 24) & 0xff)

/*
 * A CODEC is a software package that implements decoding,
 * encoding, or decoding+encoding of a compression algorithm.
 * The library provides a collection of builtin codecs.
 * More codecs may be registered through calls to the library
 * and/or the builtin implementations may be overridden.
 */
typedef	int (*TIFFInitMethod)(TIFF*, int);
typedef struct {
	char*		name;
	uint16		scheme;
	TIFFInitMethod	init;
} TIFFCodec;

#include <asys/system.h>

#if defined(__cplusplus)
extern "C" {
#endif
typedef	void (*TIFFErrorHandler)(const char*, const char*, va_list);
typedef	tsize_t (*TIFFReadWriteProc)(thandle_t, tdata_t, tsize_t);
typedef	toff_t (*TIFFSeekProc)(thandle_t, toff_t, int);
typedef	int (*TIFFCloseProc)(thandle_t);
typedef	toff_t (*TIFFSizeProc)(thandle_t);
typedef	int (*TIFFMapFileProc)(thandle_t, tdata_t*, toff_t*);
typedef	void (*TIFFUnmapFileProc)(thandle_t, tdata_t, toff_t);
typedef	void (*TIFFExtendProc)(TIFF*); 

extern	const char* TIFFGetVersion(void);

extern	const TIFFCodec* TIFFFindCODEC(uint16);
extern	TIFFCodec* TIFFRegisterCODEC(uint16, const char*, TIFFInitMethod);
extern	void TIFFUnRegisterCODEC(TIFFCodec*);

extern	tdata_t _TIFFmalloc(tsize_t);
extern	tdata_t _TIFFrealloc(tdata_t, tsize_t);
extern	void _TIFFmemset(tdata_t, int, tsize_t);
extern	void _TIFFmemcpy(tdata_t, const tdata_t, tsize_t);
extern	int _TIFFmemcmp(const tdata_t, const tdata_t, tsize_t);
extern	void _TIFFfree(tdata_t);

extern	void TIFFClose(TIFF*, int);
extern	int TIFFFlush(TIFF*);
extern	int TIFFFlushData(TIFF*);
extern	int TIFFGetField(TIFF*, ttag_t, ...);
extern	int TIFFVGetField(TIFF*, ttag_t, va_list);
extern	int TIFFGetFieldDefaulted(TIFF*, ttag_t, ...);
extern	int TIFFVGetFieldDefaulted(TIFF*, ttag_t, va_list);
extern	int TIFFReadDirectory(TIFF*);
extern	tsize_t TIFFScanlineSize(TIFF*);
extern	tsize_t TIFFRasterScanlineSize(TIFF*);
extern	tsize_t TIFFStripSize(TIFF*);
extern	tsize_t TIFFVStripSize(TIFF*, uint32);
extern	tsize_t TIFFTileRowSize(TIFF*);
extern	tsize_t TIFFTileSize(TIFF*);
extern	tsize_t TIFFVTileSize(TIFF*, uint32);
extern	uint32 TIFFDefaultStripSize(TIFF*, uint32);
extern	void TIFFDefaultTileSize(TIFF*, uint32*, uint32*);
extern	thandle_t TIFFFileno(TIFF*);
extern	int TIFFGetMode(TIFF*);
extern	int TIFFIsTiled(TIFF*);
extern	int TIFFIsByteSwapped(TIFF*);
extern	int TIFFIsUpSampled(TIFF*);
extern	int TIFFIsMSB2LSB(TIFF*);
extern	uint32 TIFFCurrentRow(TIFF*);
extern	tdir_t TIFFCurrentDirectory(TIFF*);
extern	uint32 TIFFCurrentDirOffset(TIFF*);
extern	tstrip_t TIFFCurrentStrip(TIFF*);
extern	ttile_t TIFFCurrentTile(TIFF*);
extern	int TIFFReadBufferSetup(TIFF*, tdata_t, tsize_t);
extern	int TIFFWriteBufferSetup(TIFF*, tdata_t, tsize_t);
extern	int TIFFLastDirectory(TIFF*);
extern	int TIFFSetDirectory(TIFF*, tdir_t);
extern	int TIFFSetSubDirectory(TIFF*, uint32);
extern	int TIFFUnlinkDirectory(TIFF*, tdir_t);
extern	int TIFFSetField(TIFF*, ttag_t, ...);
extern	int TIFFVSetField(TIFF*, ttag_t, va_list);
extern	int TIFFWriteDirectory(TIFF *);
#if defined(c_plusplus) || defined(__cplusplus)
extern	void TIFFPrintDirectory(TIFF*, thandle_t, long = 0);
extern	int TIFFReadScanline(TIFF*, tdata_t, uint32, tsample_t = 0);
extern	int TIFFWriteScanline(TIFF*, tdata_t, uint32, tsample_t = 0);
extern	int TIFFReadRGBAImage(TIFF*, uint32, uint32, uint32*, int = 0);
#else
extern	void TIFFPrintDirectory(TIFF*, thandle_t, long);
extern	int TIFFReadScanline(TIFF*, tdata_t, uint32, tsample_t);
extern	int TIFFWriteScanline(TIFF*, tdata_t, uint32, tsample_t);
extern	int TIFFReadRGBAImage(TIFF*, uint32, uint32, uint32*, int);
#endif
extern	int TIFFRGBAImageOK(TIFF*, char [1024]);
extern	int TIFFRGBAImageBegin(TIFFRGBAImage*, TIFF*, int, char [1024]);
extern	int TIFFRGBAImageGet(TIFFRGBAImage*, uint32*, uint32, uint32);
extern	void TIFFRGBAImageEnd(TIFFRGBAImage*);
extern	TIFF* TIFFOpen(const char*, const char*);
extern	TIFF* TIFFFdOpen(thandle_t, const char*, const char*);
extern	TIFF* TIFFClientOpen(const char*, const char*,
	    thandle_t,
	    TIFFReadWriteProc, TIFFReadWriteProc,
	    TIFFSeekProc, TIFFCloseProc,
	    TIFFSizeProc,
	    TIFFMapFileProc, TIFFUnmapFileProc);
extern	const char* TIFFFileName(TIFF*);
extern	void TIFFError(const char*, const char*, ...);
extern	void TIFFWarning(const char*, const char*, ...);
extern	TIFFErrorHandler TIFFSetErrorHandler(TIFFErrorHandler);
extern	TIFFErrorHandler TIFFSetWarningHandler(TIFFErrorHandler);
extern	TIFFExtendProc TIFFSetTagExtender(TIFFExtendProc);
extern	ttile_t TIFFComputeTile(TIFF*, uint32, uint32, uint32, tsample_t);
extern	int TIFFCheckTile(TIFF*, uint32, uint32, uint32, tsample_t);
extern	ttile_t TIFFNumberOfTiles(TIFF*);
extern	tsize_t TIFFReadTile(TIFF*,
	    tdata_t, uint32, uint32, uint32, tsample_t);
extern	tsize_t TIFFWriteTile(TIFF*,
	    tdata_t, uint32, uint32, uint32, tsample_t);
extern	tstrip_t TIFFComputeStrip(TIFF*, uint32, tsample_t);
extern	tstrip_t TIFFNumberOfStrips(TIFF*);
extern	tsize_t TIFFReadEncodedStrip(TIFF*, tstrip_t, tdata_t, tsize_t);
extern	tsize_t TIFFReadRawStrip(TIFF*, tstrip_t, tdata_t, tsize_t);
extern	tsize_t TIFFReadEncodedTile(TIFF*, ttile_t, tdata_t, tsize_t);
extern	tsize_t TIFFReadRawTile(TIFF*, ttile_t, tdata_t, tsize_t);
extern	tsize_t TIFFWriteEncodedStrip(TIFF*, tstrip_t, tdata_t, tsize_t);
extern	tsize_t TIFFWriteRawStrip(TIFF*, tstrip_t, tdata_t, tsize_t);
extern	tsize_t TIFFWriteEncodedTile(TIFF*, ttile_t, tdata_t, tsize_t);
extern	tsize_t TIFFWriteRawTile(TIFF*, ttile_t, tdata_t, tsize_t);
extern	void TIFFSetWriteOffset(TIFF*, toff_t);
extern	void TIFFSwabShort(uint16*);
extern	void TIFFSwabLong(uint32*);
extern	void TIFFSwabDouble(double*);
extern	void TIFFSwabArrayOfShort(uint16*, unsigned long);
extern	void TIFFSwabArrayOfLong(uint32*, unsigned long);
extern	void TIFFSwabArrayOfDouble(double*, unsigned long);
extern	void TIFFReverseBits(unsigned char *, unsigned long);
extern	const unsigned char* TIFFGetBitRevTable(int);
#if defined(__cplusplus)
}
#endif
#endif /* _TIFFIO_ */

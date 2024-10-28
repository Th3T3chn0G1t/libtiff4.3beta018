/* $Header: /usr/people/sam/tiff/libtiff/RCS/tif_unix.c,v 1.15 1995/06/06 23:49:31 sam Exp $ */

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

/*
 * TIFF Library UNIX-specific Routines.
 */
#include "tiffiop.h"

#include <stdio.h>
#include <stdlib.h>

static tsize_t
_tiffReadProc(thandle_t fd, tdata_t buf, tsize_t size)
{
	return ((tsize_t) fread(buf, 1, (size_t) size, fd));
}

static tsize_t
_tiffWriteProc(thandle_t fd, tdata_t buf, tsize_t size)
{
	return ((tsize_t) fwrite(buf, 1, (size_t) size, fd));
}

static toff_t
_tiffSeekProc(thandle_t fd, toff_t off, int whence)
{
	fseek(fd, (long) off, whence);

	return (toff_t) ftell(fd);
}

static int
_tiffCloseProc(thandle_t fd)
{
	return (fclose(fd));
}

static toff_t
_tiffSizeProc(thandle_t fd)
{
	fseek(fd, SEEK_END, 0);

	return (off_t) ftell(fd);
}

#ifdef HAVE_MMAP
#include <sys/mman.h>

static int
_tiffMapProc(thandle_t fd, tdata_t* pbase, toff_t* psize)
{
	toff_t size = _tiffSizeProc(fd);
	if (size != (toff_t) -1) {
		*pbase = (tdata_t)
		    mmap(0, size, PROT_READ, MAP_SHARED, (int) fd, 0);
		if (*pbase != (tdata_t) -1) {
			*psize = size;
			return (1);
		}
	}
	return (0);
}

static void
_tiffUnmapProc(thandle_t fd, tdata_t base, toff_t size)
{
	(void) fd;
	(void) munmap(base, (off_t) size);
}
#else /* !HAVE_MMAP */
static int
_tiffMapProc(thandle_t fd, tdata_t* pbase, toff_t* psize)
{
	(void) fd; (void) pbase; (void) psize;
	return (0);
}

static void
_tiffUnmapProc(thandle_t fd, tdata_t base, toff_t size)
{
	(void) fd; (void) base; (void) size;
}
#endif /* !HAVE_MMAP */

/*
 * Open a TIFF file descriptor for read/writing.
 */
TIFF*
TIFFFdOpen(thandle_t fd, const char* name, const char* mode)
{
	TIFF* tif;

	tif = TIFFClientOpen(name, mode,
	    fd,
	    _tiffReadProc, _tiffWriteProc,
	    _tiffSeekProc, _tiffCloseProc, _tiffSizeProc,
	    _tiffMapProc, _tiffUnmapProc);
	if (tif)
		tif->tif_fd = fd;
	return (tif);
}

/*
 * Open a TIFF file for read/writing.
 */
TIFF*
TIFFOpen(const char* name, const char* mode)
{
	thandle_t fp;

	if(!(fp = fopen(name, mode))) {
		TIFFError("TIFFOpen", "%s: Cannot open", name);
		return ((TIFF*) 0);
	}

	return (TIFFFdOpen(fp, name, mode));
}

void*
_TIFFmalloc(tsize_t s)
{
	return (malloc((size_t) s));
}

void
_TIFFfree(tdata_t p)
{
	free(p);
}

void*
_TIFFrealloc(tdata_t p, tsize_t s)
{
	return (realloc(p, (size_t) s));
}

void
_TIFFmemset(tdata_t p, int v, tsize_t c)
{
	memset(p, v, (size_t) c);
}

void
_TIFFmemcpy(tdata_t d, const tdata_t s, tsize_t c)
{
	memcpy(d, s, (size_t) c);
}

int
_TIFFmemcmp(const tdata_t p1, const tdata_t p2, tsize_t c)
{
	return (memcmp(p1, p2, (size_t) c));
}

static void
unixWarningHandler(const char* module, const char* fmt, va_list ap)
{
	if (module != NULL)
		fprintf(stderr, "%s: ", module);
	fprintf(stderr, "Warning, ");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, ".\n");
}
TIFFErrorHandler _TIFFwarningHandler = unixWarningHandler;

static void
unixErrorHandler(const char* module, const char* fmt, va_list ap)
{
	if (module != NULL)
		fprintf(stderr, "%s: ", module);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, ".\n");
}
TIFFErrorHandler _TIFFerrorHandler = unixErrorHandler;

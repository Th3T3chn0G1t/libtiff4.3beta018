/* $Header: /usr/people/sam/tiff/libtiff/RCS/tif_win3.c,v 1.5 1995/06/06 23:49:31 sam Exp $ */

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
 * TIFF Library Windows 3.x-specific Routines.
 */
#include "tiffiop.h"

#include <asys/log.h>

/*
 * TODO: Put these common rising/falling edges from `std.h' into a separate
 * 		 Header for including in vendor code.
 */

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable: 4668) /* Symbol not defined as macro. */
#endif

#if defined(__WATCOMC__) || defined(__BORLANDC__) || defined(_MSC_VER)
# include <io.h>		/* for open, close, etc. function prototypes */
#endif

#include <windows.h>
#include <windowsx.h>
#include <memory.h>

/* TODO: Is this era-accurate for `off_t'? */
#include <sys/types.h>

/* TODO: Use asys for all tiff IO. */
#ifdef _WIN64
# ifdef _MSC_VER
typedef unsigned __int64 tptr_t;
# else
typedef u_int64 tptr_t;
# endif
#else
typedef u_long tptr_t;
#endif

#ifdef _WIN32
# undef _CRT_SECURE_NO_WARNINGS
# undef _CRT_NONSTDC_NO_WARNINGS
#endif

#ifdef _MSC_VER
# pragma warning(pop)
#endif

static tsize_t 
_tiffReadProc(thandle_t fd, tdata_t buf, tsize_t size)
{
	/*
	 * TODO: This spams `GetLastError()' ->
	 * 				50 (Not supported)
	 * 				183 (Cannot create existing file)
	 *		 This function might just be unequivocally broken on modern
	 *		 Windows.
	 */
	/* long ret = (_hread(fd, buf, size)); */

	int ret;

	errno = 0;
	ret = _hread(fd, buf, size);

	/* TODO: Add IO tracing to all of AGA under verbose mode. */
	/*asys_log(
			__FILE__,
			"_hread(%i, %p, %ld) -> ret: %i, GetLastError(): %ld, errno: %s",
			fd, buf, size, ret, GetLastError(), strerror(errno));*/

	return ret;
}

static tsize_t
_tiffWriteProc(thandle_t fd, tdata_t buf, tsize_t size)
{
	/*return (_hwrite(fd, buf, size));*/

	int ret;

	errno = 0;
	ret = _hwrite(fd, buf, size);

	/*asys_log(
			__FILE__,
			"_hwrite(%i, %p, %ld) -> ret: %i, GetLastError(): %ld, errno: %s",
			fd, buf, size, ret, GetLastError(), strerror(errno));*/

	return ret;
}

static toff_t
_tiffSeekProc(thandle_t fd, toff_t off, int whence)
{
	int ret;

	errno = 0;
	ret = _llseek(fd, (off_t) off, whence);

	/*asys_log(
			__FILE__,
			"_llseek(%i, %ld, %i) -> ret: %i, GetLastError(): %ld, errno: %s",
			fd, off, whence, ret, GetLastError(), strerror(errno));*/

	return ret;
	/*return (_llseek(fd, (off_t) off, whence));*/
}

static int
_tiffCloseProc(thandle_t fd)
{
	HFILE ret;

	errno = 0;
	ret = _lclose(fd);

	/*asys_log(
			__FILE__,
			"_close(%i) -> ret: %i, GetLastError(): %ld, errno: %s",
			fd, ret, GetLastError(), strerror(errno));*/

	return ret;

	/*return (_lclose(fd));*/
}

#include <sys/stat.h>

static toff_t
_tiffSizeProc(thandle_t fd)
{
	struct stat sb;
	int ret;

	errno = 0;
	ret = fstat((int) fd, &sb);

	/*asys_log(
			__FILE__,
			"fstat(%i, &statbuf) -> ret: %i, GetLastError(): %ld, errno: %s",
			fd, ret, GetLastError(), strerror(errno));*/

	return ret < 0 ? 0 : sb.st_size;
}

static int
_tiffMapProc(thandle_t fd, tdata_t* pbase, toff_t* psize)
{
	(void) fd;
	(void) pbase;
	(void) psize;

	return (0);
}

static void
_tiffUnmapProc(thandle_t fd, tdata_t base, toff_t size)
{
	(void) fd;
	(void) base;
	(void) size;
}

/*
 * Open a TIFF file descriptor for read/writing.
 */
TIFF*
TIFFFdOpen(thandle_t fd, const char* name, const char* mode)
{
	TIFF* tif;

	tif = TIFFClientOpen(name, mode,
	    fd,
	    _tiffReadProc, _tiffWriteProc, _tiffSeekProc, _tiffCloseProc,
	    _tiffSizeProc, _tiffMapProc, _tiffUnmapProc);
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
	static const char module[] = "TIFFOpen";
	int m, fd;
	OFSTRUCT of;
	int mm = 0;

	m = _TIFFgetMode(mode, module);
	if (m == -1)
		return ((TIFF*)0);
	if (m & O_CREAT) {
		if ((m & O_TRUNC) || OpenFile(name, &of, OF_EXIST) != HFILE_ERROR)
			mm |= OF_CREATE;
	}
	if (m & O_WRONLY)
		mm |= OF_WRITE;
	if (m & O_RDWR)
		mm |= OF_READWRITE;
	fd = OpenFile(name, &of, mm);
	if (fd < 0) {
		TIFFError(module, "%s: Cannot open", name);
		return ((TIFF*)0);
	}
	return (TIFFFdOpen(fd, name, mode));
}

tdata_t
_TIFFmalloc(tsize_t s)
{
	return (tdata_t) GlobalAllocPtr(GHND, (DWORD) s);
}

void
_TIFFfree(tdata_t p)
{
	GlobalFreePtr(p);
}

tdata_t
_TIFFrealloc(tdata_t p, tsize_t s)
{
	return (tdata_t) GlobalReAllocPtr(p, (DWORD) s, GHND);
}

void
_TIFFmemset(tdata_t p, int v, tsize_t c)
{
	char* pp = (char*) p;

	while (c > 0) {
		/* What's left in segment */
		tsize_t chunk = 0x10000 - ((tptr_t) pp & 0xffff);
		if (chunk > 0xff00)				/* No more than 0xff00 */
			chunk = 0xff00;
		if (chunk > c)					/* No more than needed */
			chunk = c;
		memset(pp, v, chunk);
		pp = (char*) (chunk + (char _huge*) pp);
		c -= chunk;
	}
}

void
_TIFFmemcpy(tdata_t d, const tdata_t s, tsize_t c)
{
	if (c > 0xFFFF)
		hmemcpy((void _huge*) d, (void _huge*) s, c);
	else
		(void) memcpy(d, s, (size_t) c);
}

int
_TIFFmemcmp(const tdata_t d, const tdata_t s, tsize_t c)
{
	char* dd = (char*) d;
	char* ss = (char*) s;
	tsize_t chunks, chunkd, chunk;
	int result;

	while (c > 0) {
		chunks = 0x10000 - ((tptr_t) ss & 0xffff); /* What's left in segment */
		chunkd = 0x10000 - ((tptr_t) dd & 0xffff); /* What's left in segment */
		chunk = c;					/* Get the largest of     */
		if (chunk > chunks)				/*   c, chunks, chunkd,   */
			chunk = chunks;				/*   0xff00               */
		if (chunk > chunkd)
			chunk = chunkd;
		if (chunk > 0xff00)
			chunk = 0xff00;
		result = memcmp(dd, ss, chunk);
		if (result != 0)
			return (result);
		dd = (char*) (chunk + (char _huge*) dd);
		ss = (char*) (chunk + (char _huge*) ss);
		c -= chunk;
	}
	return (0);
}

static void
win3WarningHandler(const char* module, const char* fmt, va_list ap)
{
	char e[512] = { '\0' };
	if (module != NULL)
		strcat(strcpy(e, module), ":");
	vsprintf(e+strlen(e), fmt, ap);
	strcat(e, ".");
	MessageBox(GetActiveWindow(), e, "LibTIFF Warning",
	    MB_OK|MB_ICONEXCLAMATION);
}
TIFFErrorHandler _TIFFwarningHandler = win3WarningHandler;

static void
win3ErrorHandler(const char* module, const char* fmt, va_list ap)
{
	char e[512] = { '\0' };
	if (module != NULL)
		strcat(strcpy(e, module), ":");
	vsprintf(e+strlen(e), fmt, ap);
	strcat(e, ".");
	MessageBox(GetActiveWindow(), e, "LibTIFF Error", MB_OK|MB_ICONSTOP);
}
TIFFErrorHandler _TIFFerrorHandler = win3ErrorHandler;

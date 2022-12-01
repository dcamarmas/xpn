/*
 * Copyright (c) 1987, 1997, 2006, Vrije Universiteit, Amsterdam,
 * The Netherlands All rights reserved. Redistribution and use of the MINIX 3
 * operating system in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 *     * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 
 *     * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 
 *     * Neither the name of the Vrije Universiteit nor the names of the
 *     software authors or contributors may be used to endorse or promote
 *     products derived from this software without specific prior written
 *     permission.
 * 
 *     * Any deviations from these conditions require written permission
 *     from the copyright holder in advance
 * 
 * 
 * Disclaimer
 * 
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS, AUTHORS, AND
 *  CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 *  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
 *  NO EVENT SHALL PRENTICE HALL OR ANY AUTHORS OR CONTRIBUTORS BE LIABLE
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 *  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 *  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 *  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 *  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * fwrite.c - write a number of array elements on a file
 */
/* $Header$ */

#include	<stdio.h>
#include	"xpn/xpn_simple/loc_incl.h"
#include	"xpn_debug.h"

int xpn_flushbuf(int c, FILE * stream);

size_t
xpn_fwrite(const void *ptr, size_t size, size_t nmemb,
	    register FILE *stream)
{
	register const unsigned char *cp = (const unsigned char *)ptr;
	register size_t s;
	size_t ndone = 0;
	size_t res = (size_t) -1;

	XPN_DEBUG_BEGIN_CUSTOM("%d, %zu, %zu", fileno(stream), size, nmemb)
	XPN_DEBUG("stream->_count = %d _buf = %p _ptr = %p count = %d", stream->_count, stream->_buf, stream->_ptr, (int)(stream->_ptr - stream->_buf))

	if (size)
		while ( ndone < nmemb ) {
			s = size;
			do {
				if (xpn_putc(*cp, stream)
					== EOF) {
					res = ndone;
					XPN_DEBUG_END_CUSTOM("%d, %zu, %zu", fileno(stream), size, nmemb)
					return ndone;
				}
				cp++;
			} 
			while (--s);
			ndone++;
		}

	res = ndone;
	XPN_DEBUG_END_CUSTOM("%d, %zu, %zu", fileno(stream), size, nmemb)
	return ndone;
}

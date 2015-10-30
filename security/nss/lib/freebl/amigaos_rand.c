/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is the Netscape security libraries.
 *
 * The Initial Developer of the Original Code is
 * Thomas Frieden <thomas@friedenhq.org>
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#if defined(XP_AMIGAOS)
#include "secrng.h"
#include "secerr.h"

#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>

static size_t CopyLowBits(void *dst, size_t dstlen, void *src, size_t srclen)
{
	union endianness {
		int32 i;
		char c[4];
	} u;

	if (srclen <= dstlen) {
		memcpy(dst, src, srclen);
		return srclen;
	}
	u.i = 0x01020304;
	if (u.c[0] == 0x01) {
		/* big-endian case */
		memcpy(dst, (char*)src + (srclen - dstlen), dstlen);
	} else {
		/* little-endian case */
		memcpy(dst, src, dstlen);
	}
	return dstlen;
}

PRUint64 __get_tb(void);

__asm(".globl  __get_tb          \n\
__get_tb:                        \n\
                mftbu   3        \n\
                mftb    4        \n\
                mftbu   5        \n\
                cmp     0,3,5    \n\
                bne     __get_tb \n\
                blr ");



static size_t
GetHighResClock(void *buf, size_t maxbytes)
{
    PRUint64 t;

    t = __get_tb();

    return CopyLowBits(buf, maxbytes, &t, sizeof(t));
}

size_t RNG_GetNoise(void *buf, size_t maxbytes)
{
	struct timeval tv;
	int n = 0;
	int c;

	n = GetHighResClock(buf, maxbytes);
	maxbytes -= n;

	(void)gettimeofday(&tv, 0);

	c = CopyLowBits((char*)buf+n, maxbytes, &tv.tv_usec, sizeof(tv.tv_usec));
	n += c;
	maxbytes -= c;
	c = CopyLowBits((char*)buf+n, maxbytes, &tv.tv_sec, sizeof(tv.tv_sec));
	n += c;
	return n;
}

size_t RNG_SystemRNG(void *dest, size_t maxLen)
{
	FILE *file;
	size_t bytes;
	size_t fileBytes = 0;
	unsigned char *buffer = dest;

	file = fopen("random:", "r");
	if (file == NULL) {
		return rng_systemFromNoise(dest, maxLen);
	}
	while (maxLen > fileBytes) {
		bytes = maxLen - fileBytes;
		bytes = fread(buffer, 1, bytes, file);
		if (bytes == 0)
			break;
		fileBytes += bytes;
		buffer += bytes;
	}
	fclose(file);
	if (fileBytes != maxLen) {
		PORT_SetError(SEC_ERROR_NEED_RANDOM);  /* system RNG failed */
		fileBytes = 0;
	}
	return fileBytes;
}

static void rng_systemJitter(void)
{

}


void RNG_SystemInfoForRNG(void)
{
	/* *shrugs* Dunno
	 * FIXME: Do this
	 */
}
#endif

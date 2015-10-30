/* ***** BEGIN LICENSE BLOCK *****
 *
 * The contents of this file is copyrighted by Thomas and Hans-Joerg Frieden.
 * It's content is not open source and may not be redistributed, modified or adapted
 * without permission of the above-mentioned copyright holders.
 *
 * Since this code was originally developed under an AmigaOS related bounty, any derived
 * version of this file may only be used on an official AmigaOS system.
 *
 * Contributor(s):
 * 	Thomas Frieden <thomas@friedenhq.org>
 * 	Hans-Joerg Frieden <hans-joerg@friedenhq.org>
 *
 * ***** END LICENSE BLOCK ***** */

#include "primpl.h"
#include <sys/time.h>

PR_IMPLEMENT(PRTime) PR_Now(void)
{
	struct timeval tv;
	PRInt64 s, us, s2us;

	gettimeofday(&tv, NULL);

	LL_I2L(s2us, PR_USEC_PER_SEC);
	LL_I2L(s, tv.tv_sec);
	LL_I2L(us, tv.tv_usec);
	LL_MUL(s, s, s2us);
	LL_ADD(s, s, us);
	return s;
}


PR_IMPLEMENT(PRIntervalTime) _amigaos_GetInterval(void)
{
	struct timeval time;
	PRIntervalTime ticks;

	gettimeofday(&time, NULL);
	ticks = (PRUint32)time.tv_sec * PR_MSEC_PER_SEC;
	ticks += (PRUint32)time.tv_usec / PR_USEC_PER_MSEC;
	return ticks;
}


PR_IMPLEMENT(PRIntervalTime) _amigaos_IntervalPerSec(void)
{
	return 1000;
}

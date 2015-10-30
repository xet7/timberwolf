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

 
#include <primpl.h>

PR_IMPLEMENT(PRInt32) _amigaos_AtomicIncrement(PRInt32 *ptr)
{
    PRInt32 retval;

    __asm volatile(
        "1:  lwarx  4,0,%1             \n\
             addi   4,4,1         \n\
             stwcx. 4,0,%1         \n\
             bne-   1b                      \n\
             mr      %0, 4"
        : "=r" (retval)
        : "r" (ptr)
        : "memory", "r4");

    return retval;
}


PR_IMPLEMENT(PRInt32) _amigaos_AtomicDecrement(PRInt32 *ptr)
{
	PRInt32 retval;

	__asm volatile(
        "1: lwarx   4,0,%1             \n\
        addi    4,4,-1        \n\
        stwcx.  4,0,%1         \n\
        bne-    1b                      \n\
                mr              %0, 4"
        : "=r" (retval)
        : "r" (ptr)
        : "memory", "r4");

    return retval;
}


PR_IMPLEMENT(PRInt32) _amigaos_AtomicSet(PRInt32 *ptr, PRInt32 newval)
{
	PRInt32 retval;

    __asm volatile(
    	"1: lwarx   5,0,%1         \n\
        stwcx.  %2,0,%1         \n\
        bne-    1b                      \n\
        mr      %0,5"
    	: "=r" (retval)
        : "r" (ptr), "r" (newval)
        : "memory", "r5");

	return retval;
}

PR_IMPLEMENT(PRInt32) _amigaos_AtomicAdd(PRInt32 *ptr, PRInt32 val)
{
	PRInt32 retval;

	__asm volatile(
        "1: lwarx   5,0,%1             \n\
        add     0,%2,5        \n\
        stwcx.  0,0,%1         \n\
        bne-    1b                      \n\
                mr              %0, 0"
        : "=r" (retval)
        : "r" (ptr), "r" (val)
        : "memory", "r0");

	return retval;
}

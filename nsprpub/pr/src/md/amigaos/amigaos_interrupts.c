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

_PRInterruptTable _pr_interruptTable[] = { { 0 } };

PR_IMPLEMENT(void) _amigaos_StartInterrupts(void)
{
	_MD_ENABLE_CLOCK_INTERRUPTS();
}


PR_IMPLEMENT(void) _amigaos_StopInterrupts(void)
{
	_MD_DISABLE_CLOCK_INTERRUPTS();
}


PR_IMPLEMENT(void) _amigaos_DisableClockInterrupts(void)
{
	printf("%s not implemented\n", __PRETTY_FUNCTION__);
}


PR_IMPLEMENT(void) _amigaos_EnableClockInterrupts(void)
{
	printf("%s not implemented\n", __PRETTY_FUNCTION__);
}


PR_IMPLEMENT(void) _amigaos_BlockClockInterrupts(void)
{
	printf("%s not implemented\n", __PRETTY_FUNCTION__);
}


PR_IMPLEMENT(void) _amigaos_UnblockClockInterrupts(void)
{
	printf("%s not implemented\n", __PRETTY_FUNCTION__);
}

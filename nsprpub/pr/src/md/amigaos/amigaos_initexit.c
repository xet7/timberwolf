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

extern void _amigaos_InitProcesses(void);
extern void _amigaos_CleanupProcesses(void);

PR_IMPLEMENT(void) _amigaos_EarlyInit(void)
{
	_amigaos_ThreadEarlyInit();
	_amigaos_InitProcesses();
}


PR_IMPLEMENT(void) _amigaos_EarlyCleanup(void)
{

}


PR_IMPLEMENT(void) _amigaos_FinalInit(void)
{

}


PR_IMPLEMENT(void) _amigaos_CleanupBeforeExit(void)
{
	_MD_STOP_INTERRUPTS();
	_amigaos_ThreadCleanupBeforeExit();
	_amigaos_CleanupProcesses();
}

PR_IMPLEMENT(void) _amigaos_Exit(int status)
{
	exit(status);
}

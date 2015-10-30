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

#include <proto/exec.h>
#include "primpl.h"


PR_IMPLEMENT(void) _amigaos_PauseCPU(PRIntervalTime timeout)
{
    /* FIXME: Huh ? */
}


PR_IMPLEMENT(void) _amigaos_SetCurrentCPU(struct _PRCPU *cpu)
{
	__amigaos_set_tls_data(_MD_TLS_CURRENT_CPU_KEY, (void *)cpu);
}

PR_IMPLEMENT(struct _PRCPU *) _amigaos_CurrentCPU(void)
{
	return (struct _PRCPU*)__amigaos_get_tls_data(_MD_TLS_CURRENT_CPU_KEY);
}





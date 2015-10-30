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
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/timer.h>
#include <devices/timer.h>


void __amigaos_UnlockAndPostNotifies(struct _MDLock *md, PRThread *waitThread, struct _MDCVar *waitCV);

PR_IMPLEMENT(void) _amigaos_InitLocks(void)
{
	/* No-op*/
}


PR_IMPLEMENT(PRStatus) _amigaos_NewLock(struct _MDLock *md)
{
	md->mutex = IExec->AllocSysObject(ASOT_MUTEX, NULL);
	md->notified.length = 0;
	md->notified.link = 0;

	int i;
	for (i = 0; i < _MD_CV_NOTIFIED_LENGTH; i++)
		IExec->NewList(&(md->notified.cv[i].notifyList));

	if (md->mutex == NULL)
		return PR_FAILURE;

	return PR_SUCCESS;
}


PR_IMPLEMENT(void) _amigaos_FreeLock(struct _MDLock *md)
{
	IExec->FreeSysObject(ASOT_MUTEX, md->mutex);
}


PR_IMPLEMENT(void) _amigaos_Lock(struct _MDLock *md)
{
	IExec->MutexObtain(md->mutex);
#ifdef DEBUG
	md->lastObtain = IExec->FindTask(NULL);
#endif
}


PR_IMPLEMENT(PRIntn) _amigaos_TestAndLock(struct _MDLock *md)
{
	if (IExec->MutexAttempt(md->mutex))
	{
#ifdef DEBUG
		md->lastObtain = IExec->FindTask(NULL);
#endif
		return 0;
	}

	return 1;
}


PR_IMPLEMENT(void) _amigaos_Unlock(struct _MDLock *md)
{
    if (0 != md->notified.length)
        __amigaos_UnlockAndPostNotifies(md, NULL, NULL);
    else
    {
		IExec->MutexRelease(md->mutex);
    }
}


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

//#define HEAVY_DEBUG

static void inline
___lock_cv(struct _MDCVar *md)
{
	IExec->MutexObtain(md->lock);
}

static void inline
___unlock_cv(struct _MDCVar *md)
{
	IExec->MutexRelease(md->lock);
}


static void
AddThreadToCVWaitQueueInternal(PRThread *thread, struct _MDCVar *md)
{
	PR_ASSERT(md != NULL);
	PR_ASSERT(thread != NULL);
	PR_ASSERT(thread->md.smx != NULL);
	PR_ASSERT(thread->md.task == IExec->FindTask(0));

#ifdef HEAVY_DEBUG
	IExec->Forbid();IExec->DebugPrintF("AddThread %s\n", thread->md.task->tc_Node.ln_Name);	IExec->Permit();
#endif

	___lock_cv(md);

	PR_ASSERT(thread->md.inCVWaitQueue != PR_TRUE);

	/* Add thread to the end of the cvar's queue */
	md->nwait += 1;

//	IExec->MutexObtain(thread->md.smx);

	thread->md.inCVWaitQueue = PR_TRUE;

	IExec->AddTail(&md->waitQueue, &thread->md.waitQueueNode);

//	IExec->MutexRelease(thread->md.smx);

	___unlock_cv(md);
}


void
__amigaos_UnlockAndPostNotifies(struct _MDLock *lock, PRThread *waitThread, struct _MDCVar *wcvar)
{
	int idx;
	_MDNotified post;
	_MDNotified *notified, *prev = NULL;

	IExec->Forbid();
	post = lock->notified;

	lock->notified.length = 0;
	lock->notified.link = NULL;
	int i;
	for (i = 0; i < _MD_CV_NOTIFIED_LENGTH; i++)
	{
		IExec->NewList(&(post.cv[i].notifyList));
		IExec->MoveList(&(post.cv[i].notifyList), &(lock->notified.cv[i].notifyList));
		IExec->NewList(&(lock->notified.cv[i].notifyList));
	}

	IExec->Permit();

	notified = &post;

	do
	{
		for (idx = 0; idx < notified->length; idx++)
		{
			struct _MDCVar *cv = notified->cv[idx].cv;
			___lock_cv(cv);
			PRThread *thread;
			int i;

			if (notified->cv[idx].times == -1)
			{
				thread = __amigaos_get_self((struct _MDThread *)IExec->GetHead(&(cv->waitQueue)));
				while (thread != NULL)
				{
//					IExec->MutexObtain(thread->md.smx);
					thread->md.inCVWaitQueue = PR_FALSE;

					struct _MDThread *next = (struct _MDThread *)IExec->GetSucc(&thread->md.waitQueueNode);
#ifdef HEAVY_DEBUG
					IExec->Forbid();IExec->DebugPrintF("Removing %s from %s (%d) broadcast\n", thread->md.task->tc_Node.ln_Name, IExec->FindTask(0)->tc_Node.ln_Name, thread->md.inCVWaitQueue);IExec->Permit();
#endif
					IExec->Remove(&thread->md.waitQueueNode);
					IExec->AddTail(&(notified->cv[idx].notifyList), &thread->md.waitQueueNode);
//					IExec->MutexRelease(thread->md.smx);

					thread = __amigaos_get_self(next);
				}

				cv->nwait = 0;
			}
			else
			{
				thread = __amigaos_get_self((struct _MDThread *)IExec->GetHead(&(cv->waitQueue)));
				i = notified->cv[idx].times;
				while (thread != NULL && i > 0)
				{
//					IExec->MutexObtain(thread->md.smx);
					thread->md.inCVWaitQueue = PR_FALSE;

					struct _MDThread *next = (struct _MDThread *)IExec->GetSucc(&thread->md.waitQueueNode);
#ifdef HEAVY_DEBUG
					IExec->Forbid();IExec->DebugPrintF("Removing %s from %s (%d) normal\n", thread->md.task->tc_Node.ln_Name, IExec->FindTask(0)->tc_Node.ln_Name, thread->md.inCVWaitQueue);IExec->Permit();
#endif
					IExec->Remove(&thread->md.waitQueueNode);
					IExec->AddTail(&(notified->cv[idx].notifyList), &thread->md.waitQueueNode);
//					IExec->MutexRelease(thread->md.smx);
					thread = __amigaos_get_self(next);

					i--;
				}

				cv->nwait -= notified->cv[idx].times - i;
			}
			___unlock_cv(cv);
		}

		PR_ASSERT(notified != notified->link);
		notified = notified->link;
	} while (notified != NULL);

	if (waitThread)
	{
#ifdef HEAVY_DEBUG
		IExec->Forbid();IExec->DebugPrintF("indirect from %s (%d)\n", IExec->FindTask(0)->tc_Node.ln_Name, waitThread->md.inCVWaitQueue);IExec->Permit();
#endif
		AddThreadToCVWaitQueueInternal(waitThread, wcvar);
	}

	PR_ASSERT(lock->mutex != 0);
	IExec->MutexRelease(lock->mutex);


	IExec->Forbid();
	notified = &post;
	do
	{
		for (idx = 0; idx < notified->length; idx++)
		{
			PRThread *thread;

			thread = __amigaos_get_self((struct _MDThread *)IExec->GetHead(&(notified->cv[idx].notifyList)));

			while (thread != NULL)
			{
//				IExec->MutexObtain(thread->md.smx);

				PR_ASSERT(thread->md.task != NULL);
				PR_ASSERT(thread->md.inCVWaitQueue != TRUE);

				struct _MDThread *next = (struct _MDThread *)IExec->GetSucc(&thread->md.waitQueueNode);
				IExec->Remove(&thread->md.waitQueueNode);

				thread->md.waitQueueNode.ln_Succ = 0xB00BBEEF;
				thread->md.waitQueueNode.ln_Pred = 0xB00BBEEF;
#ifdef HEAVY_DEBUG
				IExec->Forbid(); IExec->DebugPrintF("About to send signal %x to task %p (%s)\n", 1L << thread->md.cvSig, thread->md.task, thread->md.task->tc_Node.ln_Name); IExec->Permit();
#endif
				IExec->Signal(thread->md.task, 1L << thread->md.cvSig);

//				IExec->MutexRelease(thread->md.smx);

				thread = __amigaos_get_self(next);
			}

		}

		PR_ASSERT(notified != notified->link);
		prev = notified;
		notified = notified->link;

		if (&post != prev)
		{
			PR_DELETE(prev);
		}
	} while (notified != NULL);
	IExec->Permit();
}

void
__amigaos_PostNotifyToCVar(struct _MDCVar *cvar, struct _MDLock *lock, PRBool broadcast)
{
	int idx = 0;
	_MDNotified *notified = &lock->notified;

	PR_ASSERT(cvar != NULL);

	___lock_cv(cvar);

	while (1)
	{
		for (idx = 0; idx < notified->length; idx++)
		{
			if (notified->cv[idx].cv == cvar)
			{
				if (broadcast)
					notified->cv[idx].times = -1;
				else if (notified->cv[idx].times != -1)
					notified->cv[idx].times += 1;

				___unlock_cv(cvar);
				return;
			}
		}

		if (notified->length < _MD_CV_NOTIFIED_LENGTH)
			break;

		if (notified->link == NULL)
		{
			int i;
			notified->link = PR_NEWZAP(_MDNotified);
			notified->link->length = 0;
			for (i = 0; i < _MD_CV_NOTIFIED_LENGTH; i++)
					IExec->NewList(&(notified->link->cv[i].notifyList));
		}
		notified = notified->link;
	}

	notified->cv[idx].times = (broadcast) ? -1 : 1;
	notified->cv[idx].cv = cvar;
	notified->length += 1;

	___unlock_cv(cvar);
}

PR_IMPLEMENT(PRInt32) _amigaos_NewCV(struct _MDCVar *md)
{
	md->nwait = 0;
	IExec->NewList(&md->waitQueue);
	md->lock = IExec->AllocSysObject(ASOT_MUTEX, NULL);

	return PR_SUCCESS;
}


PR_IMPLEMENT(void) _amigaos_FreeCV(struct _MDCVar *md)
{
	IExec->FreeSysObject(ASOT_MUTEX, md->lock);
}



PR_IMPLEMENT(void) _amigaos_WaitCV(struct _MDCVar *md, struct _MDLock *mdLock,
		PRIntervalTime timeout)
{
	PRUint32 ticks = PR_TicksPerSecond();
	int rv = 0;
	PRThread *thread = _PR_MD_CURRENT_THREAD();
	uint32 cvSig = 1L << thread->md.cvSig;
	uint32 timerSig = 1L << thread->md.timerPort[1]->mp_SigBit;
	uint32 sigMask = cvSig;

	uint32 oldSig = IExec->SetSignal(0, timerSig|cvSig);
#ifdef HEAVY_DEBUG
	IExec->Forbid();IExec->DebugPrintF("On Entry (%s): %p\n", IExec->FindTask(0)->tc_Node.ln_Name, oldSig);IExec->Permit();
#endif

	PR_ASSERT(thread->md.inCVWaitQueue != PR_TRUE);
	PR_ASSERT(thread->md.task == IExec->FindTask(NULL));

	if (0 != mdLock->notified.length)
		__amigaos_UnlockAndPostNotifies(mdLock, thread, md);
	else
	{
#ifdef HEAVY_DEBUG
		IExec->Forbid();IExec->DebugPrintF("direct from %s (%d)\n", IExec->FindTask(0)->tc_Node.ln_Name, thread->md.inCVWaitQueue);IExec->Permit();
#endif
		AddThreadToCVWaitQueueInternal(thread, md);
		IExec->MutexRelease(mdLock->mutex);
	}

//	PR_ASSERT(thread->md.inCVWaitQueue == PR_TRUE);

	/*
	 * lock mutex
	 * change condition
	 * unlock mutex
	 * signal cvar
	 */

	___lock_cv(md);

	if (timeout != PR_INTERVAL_NO_TIMEOUT)
	{
		thread->md.timerRequest[1]->Request.io_Command = TR_ADDREQUEST;
		thread->md.timerRequest[1]->Time.Seconds = timeout / ticks;
		thread->md.timerRequest[1]->Time.Microseconds = PR_IntervalToMicroseconds(timeout - (thread->md.timerRequest[1]->Time.Seconds * ticks));
		sigMask |= timerSig;
		IExec->SendIO((struct IORequest *)thread->md.timerRequest[1]);
		thread->md.timerUsed = TRUE;
	}

	/* Wait for signal */
//	uint32 oldSig = IExec->SetSignal(0, sigMask);
	___unlock_cv(md);
	uint32 sigRec = IExec->Wait(sigMask);
#ifdef HEAVY_DEBUG
	IExec->Forbid();IExec->DebugPrintF("After wait (%s): %p\n", IExec->FindTask(0)->tc_Node.ln_Name, sigRec);IExec->Permit();
#endif
	IExec->MutexObtain(mdLock->mutex);
	___lock_cv(md);

//	if ((sigRec & timerSig) && (sigRec & cvSig)) {
//		IExec->DebugPrintF("Both timeout and wakeup signal occurred (%d, %s, %p)\n", thread->md.inCVWaitQueue, thread->md.task->tc_Node.ln_Name, thread->md.waitQueueNode.ln_Succ);
//	} else if ((sigRec & cvSig)) {
//		IExec->DebugPrintF("Notified (%d, %s, %p)\n", thread->md.inCVWaitQueue, thread->md.task->tc_Node.ln_Name, thread->md.waitQueueNode.ln_Succ);
//	}
//	else {
//		IExec->DebugPrintF("timeout (%d, %s, %p)\n", thread->md.inCVWaitQueue, thread->md.task->tc_Node.ln_Name, thread->md.waitQueueNode.ln_Succ);
//	}


//	IExec->MutexObtain(thread->md.smx);
	if ((sigRec & timerSig)) // && !(sigRec & cvSig))
	{
#ifdef HEAVY_DEBUG
		IExec->Forbid();IExec->DebugPrintF("Checking from %s (%d)\n", IExec->FindTask(0)->tc_Node.ln_Name, thread->md.inCVWaitQueue);IExec->Permit();
#endif
		/* Timeout, and not been signaled. */
		if (thread->md.inCVWaitQueue)
		{
#ifdef HEAVY_DEBUG
			IExec->Forbid();IExec->DebugPrintF("Removing from %s (%d)\n", IExec->FindTask(0)->tc_Node.ln_Name, thread->md.inCVWaitQueue);IExec->Permit();
#endif
			md->nwait -= 1;
			thread->md.inCVWaitQueue = PR_FALSE;
			rv = 1;

			IExec->Remove(&(thread->md.waitQueueNode));
			thread->md.waitQueueNode.ln_Succ = 0xB00BBEEF;
			thread->md.waitQueueNode.ln_Pred = 0xB00BBEEF;
		}
	}
//	IExec->MutexRelease(thread->md.smx);
#ifdef DEBUG
	if (thread->md.inCVWaitQueue == PR_TRUE) {
		IExec->Forbid();
		IExec->DebugPrintF("** SHIT HIT FAN\nThread %p (%s) referenced by %p (%s) still in wait queue, signals %x (timer %x, cv %x)\n",
				thread->md.task, thread->md.task->tc_Node.ln_Name,
				 IExec->FindTask(NULL), IExec->FindTask(NULL)->tc_Node.ln_Name,
				sigRec, timerSig, cvSig);
		IExec->Permit();
	}
#endif

	PR_ASSERT(thread->md.inCVWaitQueue != PR_TRUE);

	if (sigRec & (1L << thread->md.timerPort[1]->mp_SigBit))
	{
		/* Always get the timer message when the signal hit */
		IExec->GetMsg(thread->md.timerPort[1]);
	}
	//else
	{
		if (thread->md.timerUsed && !IExec->CheckIO((struct IORequest *)thread->md.timerRequest[1]))
		{
			IExec->AbortIO((struct IORequest *)thread->md.timerRequest[1]);
			IExec->WaitIO((struct IORequest *)thread->md.timerRequest[1]);
		}
	}


	PR_ASSERT(thread->md.inCVWaitQueue != PR_TRUE);
//	if (rv != 0)
//		PR_Interrupt(thread);

	___unlock_cv(md);
}


PR_IMPLEMENT(void) _amigaos_NotifyCV(struct _MDCVar *md, struct _MDLock *lock)
{
	__amigaos_PostNotifyToCVar(md, lock, PR_FALSE);
}


PR_IMPLEMENT(void) _amigaos_NotifyallCV(struct _MDCVar *md, struct _MDLock *lock)
{
	__amigaos_PostNotifyToCVar(md, lock, PR_TRUE);
}

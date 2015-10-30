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

/* Assume a minimal stack of 128 KB */
#define MIN_STACK_SIZE	131072
int thread_init = 0;
uint32 threadID = 1;

struct SignalSemaphore *lockoutSemaphore = NULL;

PRStatus _amigaos_ThreadEarlyInit(void)
{
	struct Task *task = IExec->FindTask(NULL);
	if (task->tc_UserData == NULL)
	{
		task->tc_UserData = IExec->AllocVecTags(sizeof(struct _MDThreadLocalStorage),
								AVT_Type,			MEMF_PRIVATE,
							TAG_DONE);
	}

	lockoutSemaphore = (struct SignalSemaphore *)IExec->AllocSysObject(ASOT_SEMAPHORE, NULL);
	if (!lockoutSemaphore)
		return PR_FAILURE;

	thread_init = 1;
	return PR_SUCCESS;
}


PRStatus _amigaos_ThreadCleanupBeforeExit(void)
{
	if (thread_init == 0)
		return;

	IExec->ObtainSemaphore(lockoutSemaphore);
	IExec->ReleaseSemaphore(lockoutSemaphore);

	IExec->FreeSysObject(ASOT_SEMAPHORE, (void *)lockoutSemaphore);

	thread_init = 0;
	return PR_SUCCESS;
}


void
__amigaos_CreateThreadLock(struct PRThread *thread)
{
	/* Initialize the mutex for blocking */
	if (thread->md.smx == NULL)
		thread->md.smx = IExec->AllocSysObject(ASOT_MUTEX, NULL);
}

PR_IMPLEMENT(PRStatus) _amigaos_InitThread(struct PRThread *thread)
{
	struct Task *thisTask = IExec->FindTask(NULL);

	__amigaos_CreateThreadLock(thread);

	thread->md.task = thisTask;
	thread->md.timerUsed = FALSE;
	thread->md.self = thread;

	if (thisTask->tc_UserData == NULL)
	{
		thisTask->tc_UserData = IExec->AllocVecTags(sizeof(struct _MDThreadLocalStorage),
								AVT_Type,			MEMF_PRIVATE,
							TAG_DONE);
	}

	/* Open timer device */
	thread->md.timerPort[0] = (struct MsgPort *)IExec->AllocSysObject(ASOT_PORT, NULL);
	PR_ASSERT(thread->md.timerPort[0] != 0);
	thread->md.timerRequest[0] = (struct TimeRequest *)IExec->AllocSysObjectTags(ASOT_IOREQUEST,
														ASOIOR_Size,            sizeof(struct TimeRequest),
														ASOIOR_ReplyPort,       thread->md.timerPort[0],
													TAG_DONE);
	PR_ASSERT(thread->md.timerRequest[0] != 0);
	IExec->OpenDevice(TIMERNAME, UNIT_MICROHZ, (struct IORequest *)thread->md.timerRequest[0], 0);


	/* Open timer device */
	thread->md.timerPort[1] = (struct MsgPort *)IExec->AllocSysObject(ASOT_PORT, NULL);
	PR_ASSERT(thread->md.timerPort[1] != 0);
	thread->md.timerRequest[1] = (struct TimeRequest *)IExec->AllocSysObjectTags(ASOT_IOREQUEST,
														ASOIOR_Size,            sizeof(struct TimeRequest),
														ASOIOR_ReplyPort,       thread->md.timerPort[1],
													TAG_DONE);
	PR_ASSERT(thread->md.timerRequest[1] != 0);
	IExec->OpenDevice(TIMERNAME, UNIT_MICROHZ, (struct IORequest *)thread->md.timerRequest[1], 0);

	thread->md.iTimer = (struct TimerIFace *)IExec->GetInterface((struct Library *)thread->md.timerRequest[1]->Request.io_Device,
            "main", 1, NULL);
	/*XXX Add error checking */

	thread->md.waitSig = IExec->AllocSignal(-1);
	thread->md.cvSig = IExec->AllocSignal(-1);

	return PR_SUCCESS;
}

PR_IMPLEMENT(void) _amigaos_ExitThread(struct PRThread *thread)
{
	if (thread->flags & _PR_GLOBAL_SCOPE)
		_MD_CLEAN_THREAD(thread);

	if (thread->md.iTimer)
		IExec->DropInterface((struct Interface *)thread->md.iTimer);

	if (thread->md.timerRequest[0])
	{
		IExec->CloseDevice((struct IORequest *)thread->md.timerRequest[0]);
		IExec->FreeSysObject(ASOT_IOREQUEST, (void *)thread->md.timerRequest[0]);
	}

	if (thread->md.timerRequest[1])
	{
		IExec->CloseDevice((struct IORequest *)thread->md.timerRequest[1]);
		IExec->FreeSysObject(ASOT_IOREQUEST, (void *)thread->md.timerRequest[1]);
	}

	if (thread->md.timerPort[0])
		IExec->FreeSysObject(ASOT_PORT, (void *)thread->md.timerPort[0]);
	if (thread->md.timerPort[1])
		IExec->FreeSysObject(ASOT_PORT, (void *)thread->md.timerPort[1]);

	IExec->FreeSignal(thread->md.waitSig);
	IExec->FreeSignal(thread->md.cvSig);

	struct Task *task = IExec->FindTask(NULL);
	if (task->tc_UserData)
	{
		IExec->FreeVec(task->tc_UserData);
		task->tc_UserData = 0;
	}
}

uint32 threadCodeStart(STRPTR args UNUSED, int32 length UNUSED, struct ExecBase *execBase UNUSED)
{
	IExec->Obtain();

	struct Process *me = (struct Process *)IExec->FindTask(0);
	struct _MDStartMsg *msg = (struct _MDStartMsg *)me->pr_EntryData;
	PRThread *thisThread = (PRThread *)msg->thread;

	msg->result = TRUE;
	void (*start) (void *) = (void (*) (void *))thisThread->md.entry;
	IExec->ReplyMsg(&msg->msg);

	IExec->ObtainSemaphoreShared(lockoutSemaphore);
	start(thisThread);
	IExec->ReleaseSemaphore(lockoutSemaphore);

	IExec->Release();
}

PR_IMPLEMENT(PRStatus) _amigaos_CreateThread(struct PRThread *thread,
		void (*start) (void *),
		PRThreadPriority priority,
		PRThreadScope scope,
		PRThreadState state,
		PRUint32 stackSize)
{
	__amigaos_CreateThreadLock(thread);

	/* Initialize other fields */
	thread->flags |= _PR_GLOBAL_SCOPE;
	thread->md.wait = 0;
	thread->md.flags = MDTHREADF_CREATED;
	thread->md.self = thread;

	/* Create the actual thread */
	if (stackSize < MIN_STACK_SIZE)
		stackSize = MIN_STACK_SIZE;


	/* Setup process startup */
	struct MsgPort *setupPort = IExec->AllocSysObject(ASOT_PORT, NULL);
	struct _MDStartMsg *setupMsg = IExec->AllocSysObjectTags(ASOT_MESSAGE,
							ASOMSG_Size, 		sizeof(struct _MDStartMsg),
							ASOMSG_ReplyPort,	setupPort,
						TAG_DONE);

	setupMsg->thread = thread;
	setupMsg->result = 0;

	/* Create the actual thread */
	snprintf(thread->md.name, 35, "NSPR thread %d", ++threadID);
	thread->md.entry = (void *)start;

	thread->md.task = IDOS->CreateNewProcTags(
			NP_Entry,		threadCodeStart,
			NP_Input,		IDOS->Open("CONSOLE:", MODE_OLDFILE),
			NP_CloseInput,	TRUE,
			NP_Output,		IDOS->Open("CONSOLE:", MODE_OLDFILE),
			NP_CloseOutput,	TRUE,
			NP_StackSize,	stackSize,
			NP_Name,		thread->md.name,
			NP_EntryData,	setupMsg,
			NP_Child,		TRUE,
			NP_Cli,			TRUE,
		TAG_DONE);

	PR_ASSERT(thread->md.task != 0);

	IExec->WaitPort(setupPort);
	IExec->GetMsg(setupPort);

	IExec->FreeSysObject(ASOT_MESSAGE, setupMsg);
	IExec->FreeSysObject(ASOT_PORT, setupPort);

	return PR_SUCCESS;
}


PR_IMPLEMENT(void) _amigaos_CleanThread(struct PRThread *thread)
{
	if (thread->flags & _PR_GLOBAL_SCOPE)
	{
		if (thread->md.flags & MDTHREADF_CREATED)
		{
			IExec->FreeSysObject(ASOT_MUTEX, thread->md.smx);
		}
	}
}


PR_IMPLEMENT(void) _amigaos_SuspendThread(struct PRThread *thread)
{
	IExec->SuspendTask((struct Task *)thread->md.task, 0);
}

PR_IMPLEMENT(void) _amigaos_ResumeThread(struct PRThread *thread)
{
	IExec->RestartTask((struct Task *)thread->md.task, 0);
}

PR_IMPLEMENT(struct PRThread *) _amigaos_CurrentThread(void)
{
	struct PRThread *res =  (struct PRThread *)__amigaos_get_tls_data(_MD_TLS_CURRENT_THREAD_KEY);
	PR_ASSERT(res != NULL);
	return res;
}

PR_IMPLEMENT(struct PRThread *) _amigaos_LastThread(void)
{
	return (struct PRThread *)__amigaos_get_tls_data(_MD_TLS_LAST_THREAD_KEY);
}

PR_IMPLEMENT(void) _amigaos_SetCurrentThread(PRThread *thread)
{
	__amigaos_set_tls_data(_MD_TLS_CURRENT_THREAD_KEY, (void *)thread);
}

PR_IMPLEMENT(void) _amigaos_SetLastThread(PRThread *thread)
{
	__amigaos_set_tls_data(_MD_TLS_LAST_THREAD_KEY, (void *)thread);
}

PR_IMPLEMENT(void *) _amigaos_GetSP(struct PRThread *thread)
{
	void *sp = NULL;

	if (thread->md.task == (void *)IExec->FindTask(NULL))
	{
		/* Getting SP of self */
		__asm volatile ("mr %0, 1" : "=r" (sp));
	}
	else
	{
		sp = ((struct Task *)thread->md.task)->tc_SPReg;
	}

	return sp;
}



PR_IMPLEMENT(void) _amigaos_SetPriority(struct _MDThread *thread, PRThreadPriority pri)
{
	/* FIXME: Not implemented */
}

PR_IMPLEMENT(void) _amigaos_Yield(void)
{
	//PR_NOT_REACHED("_MD_YIELD should not be called for AmigaOS.");
	IExec->Forbid();
	IExec->Permit();
}


#define PT_NANOPERMICRO 1000UL
#define PT_BILLION 1000000000UL

PR_IMPLEMENT(PRStatus) _pt_wait(PRThread *thread, PRIntervalTime timeout)
{
	int rv;
	struct TimeVal tmo;
	uint32 sigMask = 1L << thread->md.waitSig;
	PRUint32 ticks = PR_TicksPerSecond();

//	IExec->MutexObtain(thread->md.smx);
	thread->md.wait--;

	if (thread->md.wait < 0)
	{
		if (timeout != PR_INTERVAL_NO_TIMEOUT)
		{
//			if (thread->md.timerUsed && !IExec->CheckIO((struct IORequest *)thread->md.timerRequest[0]))
//			{
//				IExec->AbortIO((struct IORequest *)thread->md.timerRequest[0]);
//				IExec->WaitIO((struct IORequest *)thread->md.timerRequest[0]);
//			}

			thread->md.timerRequest[0]->Request.io_Command = TR_ADDREQUEST;
			thread->md.timerRequest[0]->Time.Seconds = timeout / ticks;
			thread->md.timerRequest[0]->Time.Microseconds = PR_IntervalToMicroseconds(timeout - (thread->md.timerRequest[0]->Time.Seconds * ticks));
			sigMask |= 1L << thread->md.timerPort[0]->mp_SigBit;
			IExec->SendIO((struct IORequest *)thread->md.timerRequest[0]);
			thread->md.timerUsed = TRUE;
		}

//		IExec->MutexRelease(thread->md.smx);
		//IExec->SetSignal(0, sigMask);
//		IExec->FindTask(NULL)->tc_SigRecvd &= ~sigMask;
		uint32 sigRec = IExec->Wait(sigMask);
//		IExec->MutexObtain(thread->md.smx);

		if (sigRec & (1L << thread->md.waitSig))
		{
			/* Woken by signal, cancel timer request if pending */
			if (thread->md.timerUsed && !IExec->CheckIO((struct IORequest *)thread->md.timerRequest[0]))
			{
				IExec->AbortIO((struct IORequest *)thread->md.timerRequest[0]);
				IExec->WaitIO((struct IORequest *)thread->md.timerRequest[0]);
			}

			rv = 0;
		}
		else if (sigRec & 1L << thread->md.timerPort[0]->mp_SigBit)
		{
			/* Timer woke us */
			IExec->GetMsg(thread->md.timerPort[0]);
			thread->md.wait++;
			rv = 1;
		}
	}
	else
		rv = 0;

//	IExec->MutexRelease(thread->md.smx);

	return (rv == 0) ? PR_SUCCESS : PR_FAILURE;
}


PR_IMPLEMENT(PRStatus) _amigaos_Wait(struct PRThread *thread, PRIntervalTime ticks)
{
	if ( thread->flags & _PR_GLOBAL_SCOPE )
	{
		_MD_CHECK_FOR_EXIT();
		if (_pt_wait(thread, ticks) == PR_FAILURE)
		{
			_MD_CHECK_FOR_EXIT();
			/*
			 * wait timed out
			 */
			_PR_THREAD_LOCK(thread);
			if (thread->wait.cvar)
			{
				/*
				 * The thread will remove itself from the waitQ
				 * of the cvar in _PR_WaitCondVar
				 */
				thread->wait.cvar = NULL;
				thread->state =  _PR_RUNNING;
				_PR_THREAD_UNLOCK(thread);
			}
			else
			{
				_pt_wait(thread, PR_INTERVAL_NO_TIMEOUT);
				_PR_THREAD_UNLOCK(thread);
			}
		} 
	}
	else
	{
		_PR_MD_SWITCH_CONTEXT(thread);
	}

	return PR_SUCCESS;
}

PR_IMPLEMENT(PRStatus) _amigaos_WakeupWaiter(struct PRThread *thread)
{
	PRThread *me = _PR_MD_CURRENT_THREAD();
	PRInt32 pid, rv;
	PRIntn is;

	PR_ASSERT(_PR_IS_NATIVE_THREAD(thread));

//	IExec->MutexObtain(thread->md.smx);
	thread->md.wait++;

	IExec->Signal(thread->md.task, 1L << thread->md.waitSig);

//	IExec->MutexRelease(thread->md.smx);

	return PR_SUCCESS;
}


PR_IMPLEMENT(PRWord *) _amigaos_HomeGCRegisters( PRThread *t, int isCurrent, int *np )
{
	if (isCurrent)
	{
		(void) setjmp(t->md.jb);
	}

	*np = sizeof(t->md.jb) / sizeof(PRWord);

	return (PRWord *) t->md.jb;
}


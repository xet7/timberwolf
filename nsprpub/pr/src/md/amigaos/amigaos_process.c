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
#include <proto/exec.h>
#include <proto/dos.h>

extern struct SignalSemaphore *lockoutSemaphore;

static void _amigaos_WaitProcessDaemon(void);

extern void _amigaos_InitProcesses(void)
{
}

extern void _amigaos_CleanupProcesses(void)
{
}

static void _amigaos_WaitProcessWrapper(void)
{
	IExec->Obtain();

	struct Process *me = (struct Process *)IExec->FindTask(0);
	struct _MDStartMsg *msg = (struct _MDStartMsg *)me->pr_EntryData;
	PRProcess *proc = (PRProcess *)msg->thread;

	IExec->ObtainSemaphoreShared(lockoutSemaphore);

	int32 error = IDOS->SystemTags(proc->md.commandLine,
				 SYS_Input, 				NULL,
				 SYS_Output, 				NULL,
				 SYS_Error, 				NULL,
				 SYS_Asynch, 				TRUE,
				 NP_NotifyOnDeathSigTask, 	me,
				 NP_Name,					"Timberwolf NSPR-spawned process",
			TAG_DONE);

	if (error == 0)
		msg->result = TRUE;
	else
		msg->result = FALSE;

	IExec->ReplyMsg(&msg->msg);

	/* Wait for SIGF_CHILD which will signal us that the process ended */
	IExec->Wait(SIGF_CHILD);
	proc->md.running = 0;
	proc->md.result = 0; /* XXX: Do me */

	/* Signal all waiters */
	IExec->Forbid();

	struct Node *cur;
	while ((cur = IExec->RemHead(&proc->md.waiters)))
	{
		IExec->Signal(((struct _MDProcWaiter *)cur)->waiter, SIGF_CHILD);
	}

	IExec->Permit();

	IExec->ReleaseSemaphore(lockoutSemaphore);

	IExec->Release();
}

static char *__amiga_name(char *arg)
{
	// Skip file:/// if present
	if (strncasecmp(arg, "file:///", 8) != 0)
		return arg;
	else
		return arg + 8;
}

 PR_IMPLEMENT(PRProcess *)_amigaos_CreateProcess(
        const char *path, 
        char *const *argv, 
        char *const *envp,  
        const PRProcessAttr *attr)
{
	 int cmdLineSize = strlen(path) + 1;
	 int i;

	 /* Calculate command line size by adding all arguments, +2 for quotes +1 for space*/
	 for (i = 1; argv[i] != 0; i++)
		 cmdLineSize += 3 + strlen(argv[i]);

	 PRProcess *proc = PR_NEW(PRProcess);
	 if (!proc)
	 {
		 PR_SetError(PR_OUT_OF_MEMORY_ERROR, 0);
		 return NULL;
	 }

	 proc->md.commandLine = PR_MALLOC(cmdLineSize);
	 if (!proc->md.commandLine)
	 {
		 PR_DELETE(proc);
		 PR_SetError(PR_OUT_OF_MEMORY_ERROR, 0);
		 return NULL;
	 }

	 strcpy(proc->md.commandLine, path);
	 for (i = 1; argv[i] != 0; i++)
	 {
		 strcat(proc->md.commandLine, " ");
		 strcat(proc->md.commandLine, "\"");
		 strcat(proc->md.commandLine, __amiga_name(argv[i]));
		 strcat(proc->md.commandLine, "\"");
	 }

	 proc->md.running = 1;
	 proc->md.detach = 0;
	 IExec->NewList(&proc->md.waiters);

	 struct MsgPort *setupPort = IExec->AllocSysObject(ASOT_PORT, NULL);
	 struct _MDStartMsg *setupMsg = IExec->AllocSysObjectTags(ASOT_MESSAGE,
			 ASOMSG_Size, 		sizeof(struct _MDStartMsg),
			 ASOMSG_ReplyPort,	setupPort,
			 TAG_DONE);

	 setupMsg->thread = proc;

	 IDOS->CreateNewProcTags(
			 NP_Entry,			_amigaos_WaitProcessWrapper,
			 NP_Input,			IDOS->Open("CONSOLE:", MODE_OLDFILE),
			 NP_CloseInput,		TRUE,
			 NP_Output,			IDOS->Open("CONSOLE:", MODE_OLDFILE),
			 NP_CloseOutput,	TRUE,
			 NP_Name,			"Timberwolf process wrapper",
			 NP_EntryData,		setupMsg,
			 NP_Child,			TRUE,
			 NP_Cli,			TRUE,
		TAG_DONE);

	 IExec->WaitPort(setupPort);
	 IExec->GetMsg(setupPort);

	 if (setupMsg->result == 0)
	 {

		 IExec->FreeSysObject(ASOT_MESSAGE, setupMsg);
		 IExec->FreeSysObject(ASOT_PORT, setupPort);

		 PR_DELETE(proc->md.commandLine);
		 PR_DELETE(proc);
		 PR_SetError(PR_INSUFFICIENT_RESOURCES_ERROR, IDOS->IoErr());

		 return NULL;
	 }

	 IExec->FreeSysObject(ASOT_MESSAGE, setupMsg);
	 IExec->FreeSysObject(ASOT_PORT, setupPort);

	 return proc;
}

PR_IMPLEMENT(PRStatus) _amigaos_DetachProcess(PRProcess *process)
{
	process->md.detach = 1;
	return PR_SUCCESS;
}

PR_IMPLEMENT(PRStatus) _amigaos_WaitProcess(PRProcess *process, PRInt32 *exitCode)
{
	if (process->md.detach)
		return PR_FAILURE;

	if (process && process->md.running == 0)
	{
		*exitCode = process->md.result;
		return PR_SUCCESS;
	}

	IExec->Forbid();

	struct _MDProcWaiter node;

	node.waiter = IExec->FindTask(NULL);
	node.resStore = exitCode;

	IExec->AddTail(&process->md.waiters, &node.link);
	IExec->Permit();

	IExec->Wait(SIGF_CHILD);

	IExec->Forbid();
	process->md.numWaiters--;
	exitCode = process->md.result;
	IExec->Permit();

	return PR_SUCCESS;
}

PR_IMPLEMENT(PRStatus) _amigaos_KillProcess(PRProcess *process)
{
	PR_SetError(PR_NOT_IMPLEMENTED_ERROR, 0);
	return PR_FAILURE;
}

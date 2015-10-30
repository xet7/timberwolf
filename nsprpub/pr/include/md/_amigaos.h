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



#ifndef nspr_amigaos_defs_h___
#define nspr_amigaos_defs_h___

#include "prthread.h"
#include <dlfcn.h>
#include <sys/dirent.h>
#include <sys/stat.h>

#include <errno.h>

#include <proto/exec.h>
#include <proto/timer.h>
#include <devices/timer.h>
#include <exec/lists.h>
#include <exec/nodes.h>
#include <dos/dos.h>

#define _MD_ERRNO() errno

/*
 * Internal configuration macros
 */

#define PR_LINKER_ARCH	"amigaos"
#define _PR_SI_SYSNAME  "AMIGAOS"

#define _PR_SI_ARCHITECTURE "ppc"
#define PR_DLL_SUFFIX		".so"

#define _PR_NO_PREEMPT
#define _PR_HAVE_ATOMIC_OPS

#define PR_DIRECTORY_SEPARATOR			'/'
#define PR_DIRECTORY_SEPARATOR_STR		"/"
#define PR_PATH_SEPARATOR				';'
#define PR_PATH_SEPARATOR_STR			";"

#define _MD_DEFAULT_STACK_SIZE			131072

#define _PR_INTERRUPT_CHECK_INTERVAL_SECS 5


#define _PR_VMBASE              0x30000000
#define _PR_STACK_VMBASE		0x50000000
#define _MD_MMAP_FLAGS          MAP_PRIVATE

/* Fixme: No idea what the frack that should accomplish */
#define TCP_NODELAY 			_PR_NO_SUCH_SOCKOPT

#undef	HAVE_STACK_GROWING_UP

/*
 * AmigaOS supports dl* functions
 */
#define HAVE_DLL
#define USE_DLFCN

#undef _PR_USE_POLL
#define _PR_POLL_WITH_SELECT
#define _PR_STAT_HAS_ONLY_ST_ATIME


struct pollfd {
    int fd;
    short events;
    short revents;
};

/* poll events */

#define	POLLIN		0x0001		/* fd is readable */
#define	POLLPRI		0x0002		/* high priority info at fd */
#define	POLLOUT		0x0004		/* fd is writeable (won't block) */
#define	POLLRDNORM	0x0040		/* normal data is readable */
#define	POLLWRNORM	POLLOUT
#define	POLLRDBAND	0x0080		/* out-of-band data is readable */
#define	POLLWRBAND	0x0100		/* out-of-band data is writeable */

#define	POLLNORM	POLLRDNORM

#define	POLLERR		0x0008		/* fd has error condition */
#define	POLLHUP		0x0010		/* fd has been hung up on */
#define	POLLNVAL	0x0020		/* invalid pollfd entry */

extern int poll(struct pollfd *, unsigned long, int);

#define GETTIMEOFDAY(tp) gettimeofday(tp, NULL)

/* Machine-dependent (MD) data structures */


#define _MD_CV_NOTIFIED_LENGTH 6
typedef struct _MDNotified _MDNotified;
struct _MDNotified
{
	PRIntn 		length;
	struct
	{
		struct _MDCVar *cv;
		PRIntn times;
		//struct PRThread *notifyHead;
		struct List notifyList;
	} cv[_MD_CV_NOTIFIED_LENGTH];

	_MDNotified *link;
};


#define _MD_TLS_LENGTH 5
struct _MDThreadLocalStorage
{
	void * tls[_MD_TLS_LENGTH];
};

#define _MD_TLS_CURRENT_THREAD_KEY 	0
#define _MD_TLS_CURRENT_CPU_KEY		1
#define _MD_TLS_LAST_THREAD_KEY		2


struct _MDStartMsg
{
	struct Message msg;
	void *thread;
	uint32 result;
};

struct _MDThread
{
	/* CVar wait queue */
	struct Node waitQueueNode;
	PRBool inCVWaitQueue;

	struct PRThread *self;

	/* Used for GC registers */
	jmp_buf		jb;

	/* Waiter nest count */
	int wait;

	/* Pointer to the task structure. */
	struct Task *task;

	/* Flags */
	uint32 flags;

	/* Timer device data */
	struct MsgPort *timerPort[2];
	struct TimeRequest *timerRequest[2];
	struct TimerIFace *iTimer;
	BOOL timerUsed;

	/* Protective mutex */
	void *smx;

	/* Wait signals */
	uint32 waitSig;
	uint32 cvSig;

	/* Data associated to process startup */
	char name[30];
	void *entry;
};

#define _MD_TIME_REQ_WAIT	0
#define _MD_TIME_REQ_CV		1

static PRThread *__amigaos_get_self(struct _MDThread *md)
{
	if (!md)
		return NULL;

	return md->self;
}


static void *__amigaos_get_tls_data(int key)
{
	struct Task *task = IExec->FindTask(NULL);
	if (task->tc_UserData == NULL)
	{
			task->tc_UserData = IExec->AllocVecTags(sizeof(struct _MDThreadLocalStorage),
									AVT_Type,			MEMF_PRIVATE,
								TAG_DONE);

			return 0;
	}
	PR_ASSERT(task->tc_UserData != NULL);
	return ((struct _MDThreadLocalStorage *)task->tc_UserData)->tls[key];
};

static void __amigaos_set_tls_data(int key, void *data)
{
	struct Task *task = IExec->FindTask(NULL);
	if (task->tc_UserData == NULL)
	{
		task->tc_UserData = IExec->AllocVecTags(sizeof(struct _MDThreadLocalStorage),
								AVT_Type,			MEMF_PRIVATE,
							TAG_DONE);
	}
	PR_ASSERT(task->tc_UserData != NULL);
	((struct _MDThreadLocalStorage *)task->tc_UserData)->tls[key] = data;
};


#define MDTHREADF_CREATED   (1<<0)

struct _MDThreadStack {
    PRInt8 notused;
};

struct _MDLock {
	void *mutex;
    struct _MDNotified notified;
    void *lastObtain;
};

struct _MDSemaphore {
    PRInt8 notused;
};

struct _MDCVar {
	struct List waitQueue;
    PRIntn nwait;
    void *lock;
};

struct _MDSegment {
    PRInt8 notused;
};

struct _MDCPU {
    PRInt8 notused;
};

struct _MDDir {
    DIR *d;
};


struct _MDFileDesc {
    PRInt32 osfd;
};

struct _MDProcWaiter {
	struct Node link;
	struct Task *waiter;
	void *resStore;
};

struct _MDProcess {
    char *commandLine;
    struct Task *launcher;
    uint32 running;
    struct List waiters;
    uint32 numWaiters;
    uint32 result;
    uint32 detach;
};


struct _MDFileMap {
    PRInt8 notused;
};

/* Prototypes */

/* in amigaos_locks.c */
NSPR_API(void) _amigaos_InitLocks(void);
#define _MD_INIT_LOCKS						_amigaos_InitLocks

NSPR_API(PRStatus) _amigaos_NewLock(struct _MDLock *md);
#define _MD_NEW_LOCK 						_amigaos_NewLock

NSPR_API(void) _amigaos_FreeLock(struct _MDLock *md);
#define _MD_FREE_LOCK						_amigaos_FreeLock

NSPR_API(void) _amigaos_Lock(struct _MDLock *md);
#define _MD_LOCK							_amigaos_Lock

NSPR_API(PRIntn) _amigaos_TestAndLock(struct _MDLock *md);
#define _MD_TEST_AND_LOCK					_amigaos_TestAndLock

NSPR_API(void) _amigaos_Unlock(struct _MDLock *md);
#define _MD_UNLOCK							_amigaos_Unlock

/* in amigaos_cvar.c */
NSPR_API(PRInt32) _amigaos_NewCV(struct _MDCVar *md);
#define _MD_NEW_CV							_amigaos_NewCV

NSPR_API(void) _amigaos_FreeCV(struct _MDCVar *md);
#define _MD_FREE_CV							_amigaos_FreeCV

NSPR_API(void) _amigaos_WaitCV(struct _MDCVar *md, struct _MDLock *mdLock, PRIntervalTime timeout);
#define _MD_WAIT_CV							_amigaos_WaitCV

NSPR_API(void) _amigaos_NotifyCV(struct _MDCVar *md, struct _MDLock *lock);
#define _MD_NOTIFY_CV						_amigaos_NotifyCV

NSPR_API(void) _amigaos_NotifyallCV(struct _MDCVar *md, struct _MDLock *lock);
#define _MD_NOTIFYALL_CV					_amigaos_NotifyallCV

/* in amigaos_thread.c */
PRStatus _amigaos_ThreadCleanupBeforeExit(void);
PRStatus _amigaos_ThreadEarlyInit(void);

NSPR_API(PRStatus) _amigaos_Wait(struct PRThread *, PRIntervalTime);
#define _MD_WAIT							_amigaos_Wait

NSPR_API(PRStatus) _amigaos_WakeupWaiter(struct PRThread *);
#define _MD_WAKEUP_WAITER					_amigaos_WakeupWaiter

NSPR_API(PRStatus) _amigaos_InitThread(struct PRThread *);
#define _MD_INIT_THREAD						_amigaos_InitThread
/* FIXME: OK to use the same ? */
#define _MD_INIT_ATTACHED_THREAD			_amigaos_InitThread

NSPR_API(void) _amigaos_ExitThread(struct PRThread *thread);
#define _MD_EXIT_THREAD						_amigaos_ExitThread

NSPR_API(PRStatus) _amigaos_CreateThread(struct PRThread *, void (*) (void *), PRThreadPriority, PRThreadScope, PRThreadState, PRUint32);
#define _MD_CREATE_THREAD					_amigaos_CreateThread

NSPR_API(void) _amigaos_CleanThread(struct PRThread *);
#define _MD_CLEAN_THREAD					_amigaos_CleanThread

NSPR_API(void) _amigaos_SuspendThread(struct PRThread *);
#define _MD_SUSPEND_THREAD					_amigaos_SuspendThread

NSPR_API(void) _amigaos_ResumeThread(struct PRThread *);
#define _MD_RESUME_THREAD					_amigaos_ResumeThread

NSPR_API(struct PRThread *) _amigaos_CurrentThread(void);
#define _MD_CURRENT_THREAD					_amigaos_CurrentThread

NSPR_API(struct PRThread *) _amigaos_LastThread(void);
#define _MD_LAST_THREAD						_amigaos_LastThread

NSPR_API(void) _amigaos_SetCurrentThread(PRThread *);
#define _MD_SET_CURRENT_THREAD				_amigaos_SetCurrentThread

NSPR_API(void) _amigaos_SetLastThread(PRThread *);
#define _MD_SET_LAST_THREAD					_amigaos_SetLastThread

NSPR_API(void *) _amigaos_GetSP(struct PRThread *);
#define _MD_GET_SP							_amigaos_GetSP

NSPR_API(void) _amigaos_SetPriority(struct _MDThread *, PRThreadPriority);
#define _MD_SET_PRIORITY					_amigaos_SetPriority

NSPR_API(void) _amigaos_Yield(void);
#define _MD_YIELD(x)						_amigaos_Yield

#define _MD_CHECK_FOR_EXIT()

#define _MD_HomeGCRegisters _amigaos_HomeGCRegisters

/* Note needed, AmigaOS already adds redzones automatically */
#define _MD_INIT_STACK(thread, redzone)
#define _MD_CLEAR_STACK(thread)


#define _MD_INIT_CONTEXT(_thread, _sp, _main, _status)                                                          \
        PR_BEGIN_MACRO                                                                                                                                  \
                *(_status) = PR_FALSE;                                                                                                          \
        PR_END_MACRO

#define _MD_SWITCH_CONTEXT(_thread)                                                                                                     \
        PR_BEGIN_MACRO                                                                                                                                  \
        PR_END_MACRO


#define _MD_RESTORE_CONTEXT(_thread)                                                                                            \
        PR_BEGIN_MACRO                                                                                                                                  \
        PR_END_MACRO

/* in amigaos_atomic.c */
#define _MD_INIT_ATOMIC()

NSPR_API(PRInt32) _amigaos_AtomicIncrement(PRInt32 *);
#define _MD_ATOMIC_INCREMENT				_amigaos_AtomicIncrement

NSPR_API(PRInt32) _amigaos_AtomicDecrement(PRInt32 *);
#define _MD_ATOMIC_DECREMENT				_amigaos_AtomicDecrement

NSPR_API(PRInt32) _amigaos_AtomicSet(PRInt32 *, PRInt32);
#define _MD_ATOMIC_SET						_amigaos_AtomicSet

NSPR_API(PRInt32) _amigaos_AtomicAdd(PRInt32 *, PRInt32);
#define _MD_ATOMIC_ADD						_amigaos_AtomicAdd

/* in amigaos_dirio.c  */
NSPR_API(PRStatus) _amigaos_OpenDir(struct _MDDir *, const char *);
#define _MD_OPEN_DIR                        _amigaos_OpenDir

NSPR_API(PRInt32) _amigaos_CloseDir(struct _MDDir *);
#define _MD_CLOSE_DIR                           _amigaos_CloseDir

NSPR_API(char *) _amigaos_ReadDir(struct _MDDir *, PRIntn);
#define _MD_READ_DIR                            _amigaos_ReadDir

NSPR_API(PRInt32) _amigaos_MakeDir(const char *, PRIntn);
#define _MD_MKDIR                               _amigaos_MakeDir
#define _MD_MAKE_DIR                            _amigaos_MakeDir

NSPR_API(PRInt32) _amigaos_RemoveDir(const char *);
#define _MD_RMDIR                  		 _amigaos_RemoveDir

/* in amigaos_fileio.c */
#define _MD_INIT_FILEDESC

NSPR_API(PRInt32) _amigaos_Open(const char *, PRIntn, PRIntn);
#define _MD_OPEN_FILE 					_amigaos_Open
#define _MD_OPEN 					_amigaos_Open

NSPR_API(PRInt32) _amigaos_CloseFile(PRInt32);
#define _MD_CLOSE_FILE 				_amigaos_CloseFile

NSPR_API(PRInt32) _amigaos_Read(PRFileDesc *, void *, PRInt32);
#define _MD_READ 					_amigaos_Read

NSPR_API(PRInt32) _amigaos_Write(PRFileDesc *, const void *, PRInt32);
#define _MD_WRITE 					_amigaos_Write

NSPR_API(PRInt32) _amigaos_FSync(PRFileDesc *);
#define _MD_FSYNC 					_amigaos_FSync

NSPR_API(PRInt32) _amigaos_Delete(const char *);
#define _MD_DELETE 					_amigaos_Delete

NSPR_API(PRInt32) _amigaos_Rename(const char *, const char *);
#define _MD_RENAME 					_amigaos_Rename

NSPR_API(PRInt32) _amigaos_Access(const char *, PRAccessHow);
#define _MD_ACCESS 					_amigaos_Access

NSPR_API(PRInt32) _amigaos_Stat(const char *, struct stat *);
#define _MD_STAT					_amigaos_Stat

NSPR_API(void) _amigaos_MakeNonblock(PRFileDesc *);
#define _MD_MAKE_NONBLOCK			_amigaos_MakeNonblock

NSPR_API(PROffset32) _amigaos_LSeek(PRFileDesc *, PROffset32, PRSeekWhence);
#define _MD_LSEEK					_amigaos_LSeek

NSPR_API(PROffset64) _amigaos_LSeek64(PRFileDesc *, PROffset64, PRSeekWhence);
#define _MD_LSEEK64					_amigaos_LSeek64

NSPR_API(PRInt32) _amigaos_Getfileinfo(const char *, PRFileInfo *);
#define _MD_GETFILEINFO				_amigaos_Getfileinfo

NSPR_API(PRInt32) _amigaos_Getfileinfo64(const char *, PRFileInfo64 *);
#define _MD_GETFILEINFO64			_amigaos_Getfileinfo64

NSPR_API(PRInt32) _amigaos_Getopenfileinfo(const PRFileDesc *, PRFileInfo *);
#define _MD_GETOPENFILEINFO			_amigaos_Getopenfileinfo

NSPR_API(PRInt32) _amigaos_Getopenfileinfo64(const PRFileDesc *, PRFileInfo64 *);
#define _MD_GETOPENFILEINFO64		_amigaos_Getopenfileinfo64

NSPR_API(PRStatus) _amigaos_LockFile(PRInt32);
#define _MD_LOCKFILE				_amigaos_LockFile

NSPR_API(PRStatus) _amigaos_TLockFile(PRInt32);
#define _MD_TLOCKFILE				_amigaos_TLockFile

NSPR_API(PRStatus) _amigaos_UnlockFile(PRInt32);
#define _MD_UNLOCKFILE				_amigaos_UnlockFile

NSPR_API(void) _amigaos_InitFdInheritable(PRFileDesc *, PRBool);
#define _MD_INIT_FD_INHERITABLE		_amigaos_InitFdInheritable

NSPR_API(void) _amigaos_QueryFdInheritable(PRFileDesc *);
#define _MD_QUERY_FD_INHERITABLE	_amigaos_QueryFdInheritable

/* in amigaos_cpus.c */
#define _MD_INIT_CPUS()
#define _MD_INIT_RUNNING_CPU(cpu)

NSPR_API(void) _amigaos_SetCurrentCPU(struct _PRCPU *);
#define _MD_SET_CURRENT_CPU                     _amigaos_SetCurrentCPU

NSPR_API(void) _amigaos_PauseCPU(PRIntervalTime);
#define _MD_PAUSE_CPU                           _amigaos_PauseCPU

NSPR_API(struct _PRCPU *) _amigaos_CurrentCPU(void);
#define _MD_CURRENT_CPU                         _amigaos_CurrentCPU

/* FIXME: WTF is this ? */
#define _MD_BEGIN_SUSPEND_ALL()
#define _MD_END_SUSPEND_ALL()
#define _MD_BEGIN_RESUME_ALL()
#define _MD_END_RESUME_ALL()

/* in amigaos_initexit */
NSPR_API(void) _amigaos_EarlyInit(void);
#define _MD_EARLY_INIT							_amigaos_EarlyInit

NSPR_API(void) _amigaos_FinalInit(void);
#define _MD_FINAL_INIT							_amigaos_FinalInit

NSPR_API(void) _amigaos_CleanupBeforeExit(void);
#define _MD_CLEANUP_BEFORE_EXIT					_amigaos_CleanupBeforeExit

NSPR_API(void) _amigaos_Exit(int);
#define _MD_EXIT								_amigaos_Exit

NSPR_API(void) _amigaos_EarlyCleanup(void);
#define _MD_EARLY_CLEANUP 						_amigaos_EarlyCleanup

/* in amigaos_interval */
#define _MD_INTERVAL_INIT()

NSPR_API(PRIntervalTime) _amigaos_GetInterval();
#define _MD_GET_INTERVAL                        _amigaos_GetInterval

NSPR_API(PRIntervalTime) _amigaos_IntervalPerSec();
#define _MD_INTERVAL_PER_SEC            _amigaos_IntervalPerSec


/* in amigaos_interrupts */
NSPR_API(void) _amigaos_StartInterrupts(void);
#define _MD_START_INTERRUPTS            _amigaos_StartInterrupts

NSPR_API(void) _amigaos_StopInterrupts(void);
#define _MD_STOP_INTERRUPTS                     _amigaos_StopInterrupts

NSPR_API(void) _amigaos_DisableClockInterrupts(void);
#define _MD_DISABLE_CLOCK_INTERRUPTS _amigaos_DisableClockInterrupts

NSPR_API(void) _amigaos_EnableClockInterrupts(void);
#define _MD_ENABLE_CLOCK_INTERRUPTS     _amigaos_EnableClockInterrupts

NSPR_API(void) _amigaos_BlockClockInterrupts(void);
#define _MD_BLOCK_CLOCK_INTERRUPTS      _amigaos_BlockClockInterrupts

NSPR_API(void) _amigaos_UnblockClockInterrupts(void);
#define _MD_UNBLOCK_CLOCK_INTERRUPTS _amigaos_UnblockClockInterrupts

/* in amigaos_misc */
NSPR_API(char *) _amigaos_GetEnv(const char *);
#define _MD_PUT_ENV                                     _amigaos_PutEnv

NSPR_API(PRIntn) _amigaos_PutEnv(const char *);
#define _MD_GET_ENV                                     _amigaos_GetEnv

NSPR_API(PRSize) _amigaos_GetRandomNoise(void *buf, PRSize  size);
#define _PR_MD_GetRandomNoise		_amigaos_GetRandomNoise

/* in amigaos_socketio.c */
NSPR_API(void) _amigaos_InitIO(void);
#define _MD_INIT_IO                             _amigaos_InitIO

NSPR_API(PRInt32) _amigaos_CloseSocket(PRInt32);
#define _MD_CLOSE_SOCKET                        _amigaos_CloseSocket

NSPR_API(PRInt32) _amigaos_Connect(PRFileDesc *, const PRNetAddr *, PRUint32, PRIntervalTime);
#define _MD_CONNECT                                     _amigaos_Connect

NSPR_API(PRInt32) _amigaos_Accept(PRFileDesc *, PRNetAddr *, PRUint32 *, PRIntervalTime);
#define _MD_ACCEPT                                      _amigaos_Accept

NSPR_API(PRInt32) _amigaos_Bind(PRFileDesc *, const PRNetAddr *, PRUint32);
#define _MD_BIND                                        _amigaos_Bind

NSPR_API(PRInt32) _amigaos_Listen(PRFileDesc *, PRIntn);
#define _MD_LISTEN                                      _amigaos_Listen

NSPR_API(PRInt32) _amigaos_Shutdown(PRFileDesc *, PRIntn);
#define _MD_SHUTDOWN                            _amigaos_Shutdown

NSPR_API(PRInt32) _amigaos_Recv(PRFileDesc *, void *, PRInt32 , PRInt32, PRIntervalTime);
#define _MD_RECV                                        _amigaos_Recv

NSPR_API(PRInt32) _amigaos_Send(PRFileDesc *, const void *, PRInt32, PRInt32, PRIntervalTime);
#define _MD_SEND                                        _amigaos_Send

NSPR_API(PRStatus) _amigaos_GetSockName(PRFileDesc *, PRNetAddr *, PRUint32 *);
#define _MD_GETSOCKNAME                         _amigaos_GetSockName

NSPR_API(PRStatus) _amigaos_GetPeerName(PRFileDesc *, PRNetAddr *, PRUint32 *);
#define _MD_GETPEERNAME                         _amigaos_GetPeerName

NSPR_API(PRStatus) _amigaos_GetSockOpt(PRFileDesc *, PRInt32, PRInt32, char*, PRInt32*);
#define _MD_GETSOCKOPT                          _amigaos_GetSockOpt

NSPR_API(PRStatus) _amigaos_SetSockOpt(PRFileDesc *, PRInt32, PRInt32, const char*, PRInt32);
#define _MD_SETSOCKOPT                          _amigaos_SetSockOpt

NSPR_API(PRInt32) _amigaos_SendTo(PRFileDesc *, const void *, PRInt32, PRIntn, const PRNetAddr *, PRUint32, PRIntervalTime);
#define _MD_SENDTO                                      _amigaos_SendTo

NSPR_API(PRInt32) _amigaos_RecvFrom(PRFileDesc *, void *, PRInt32, PRIntn, PRNetAddr *, PRUint32 *, PRIntervalTime);
#define _MD_RECVFROM                            _amigaos_RecvFrom

NSPR_API(PRInt32) _amigaos_SocketPair(int, int, int, PRInt32 *);
#define _MD_SOCKETPAIR                          _amigaos_SocketPair

NSPR_API(PRInt32) _amigaos_Socket(PRInt32, PRInt32, PRInt32);
#define _MD_SOCKET                                      _amigaos_Socket

NSPR_API(PRInt32) _amigaos_SocketAvailable(PRFileDesc *);
#define _MD_SOCKETAVAILABLE             _amigaos_SocketAvailable
#define _MD_PIPEAVAILABLE                       _amigaos_SocketAvailable

NSPR_API(PRInt32) _amigaos_Pr_Poll(PRPollDesc *, PRIntn, PRIntervalTime);
#define _MD_PR_POLL                                     _amigaos_Pr_Poll

NSPR_API(PRInt32) _amigaos_Select(PRInt32, fd_set *, fd_set *, fd_set *, struct timeval *);
#define _MD_SELECT                                      _amigaos_Select

NSPR_API(PRInt32) _amigaos_Writev(PRFileDesc *, const PRIOVec *, PRInt32, PRIntervalTime);
#define _MD_WRITEV                                      _amigaos_Writev

NSPR_API(PRStatus) _amigaos_gethostname(char *name, PRUint32 namelen);
#define _MD_GETHOSTNAME         _amigaos_gethostname

NSPR_API(PRStatus) _amigaos_getsysinfo(PRSysInfo cmd, char *name, PRUint32 namelen);
#define _MD_GETSYSINFO          _amigaos_getsysinfo


/* in amigaos_segment.h */

#define _MD_INIT_SEGS()

NSPR_API(PRStatus) _amigaos_AllocSegment(PRSegment *, PRUint32, void *);
#define _MD_ALLOC_SEGMENT                       _amigaos_AllocSegment

NSPR_API(void) _amigaos_FreeSegment(PRSegment *);
#define _MD_FREE_SEGMENT                        _amigaos_FreeSegment

/* in amigaos_process */
NSPR_API(PRProcess *)_amigaos_CreateProcess(const char *, char *const *, char *const *, const PRProcessAttr *);
#define _MD_CREATE_PROCESS                      _amigaos_CreateProcess

NSPR_API(PRStatus) _amigaos_DetachProcess(PRProcess *);
#define _MD_DETACH_PROCESS                      _amigaos_DetachProcess

NSPR_API(PRStatus) _amigaos_WaitProcess(PRProcess *, PRInt32 *);
#define _MD_WAIT_PROCESS                        _amigaos_WaitProcess

NSPR_API(PRStatus) _amigaos_KillProcess(PRProcess *);
#define _MD_KILL_PROCESS                        _amigaos_KillProcess


/* in amigaos_filemap.h */
NSPR_API(PRStatus) _amigaos_CreateFileMap(struct PRFileMap *fmap, PRInt64 size);
#define _MD_CREATE_FILE_MAP 				_amigaos_CreateFileMap

NSPR_API(PRInt32) _amigaos_GetMemMapAlignment(void);
#define _MD_GET_MEM_MAP_ALIGNMENT 			_amigaos_GetMemMapAlignment

NSPR_API(void *) _amigaos_MemMap(struct PRFileMap *fmap, PRInt64 offset,
        PRUint32 len);
#define _MD_MEM_MAP 						_amigaos_MemMap

NSPR_API(PRStatus) _amigaos_MemUnmap(void *addr, PRUint32 size);
#define _MD_MEM_UNMAP 						_amigaos_MemUnmap

NSPR_API(PRStatus) _amigaos_CloseFileMap(struct PRFileMap *fmap);
#define _MD_CLOSE_FILE_MAP 					_amigaos_CloseFileMap


#endif /* nspr_amigaos_defs_h___ */

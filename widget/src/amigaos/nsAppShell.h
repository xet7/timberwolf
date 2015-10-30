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


#ifndef nsAppShell_h__
#define nsAppShell_h__

#include "nsBaseAppShell.h"
#include <exec/exec.h>
#include <exec/ports.h>
#include <exec/tasks.h>

/**
 * Native AmigaOS Application shell wrapper
 */

class nsAppShell : public nsBaseAppShell
{
public:
	nsAppShell();
	nsresult        Init();

	virtual void    ScheduleNativeEventCallback();
	virtual PRBool  ProcessNextNativeEvent(PRBool mayWait);
	virtual        ~nsAppShell();

private:

	struct MsgPort *mSharedWindowPort;
	struct MsgPort *mAppShellPort;
	struct MsgPort *mInternalPort;
	void *			mMessageItemPool;
	PRBool			mDestroyed;

public:
	void * GetMessagePort()
	{
		return (void *)mSharedWindowPort;
	}

	void SynthesizeRedrawEvent(void *destWindow);
};

#endif // nsAppShell_h__

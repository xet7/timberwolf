/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* vim:expandtab:shiftwidth=4:tabstop=4:
 */
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


#ifndef __nsAmigaOSIMModule_h__
#define __nsAmigaOSIMModule_h__

#include "nsString.h"
#include "nsAutoPtr.h"
#include "nsTArray.h"
#include "nsGUIEvent.h"

class nsWindow;

class nsAmigaOSIMModule
{
public:
	nsrefcnt AddRef()
	{
		NS_PRECONDITION(PRInt32(mRefCnt) >= 0, "mRefCnt is negative");
		++mRefCnt;
		NS_LOG_ADDREF(this, mRefCnt, "nsAmigaOSIMModule", sizeof(*this));
		return mRefCnt;
	}

	nsrefcnt Release()
	{
		NS_PRECONDITION(mRefCnt != 0, "mRefCnt is alrady zero");
		--mRefCnt;
		NS_LOG_RELEASE(this, mRefCnt, "nsAmigaOSIMModule");
		if (mRefCnt == 0)
		{
			mRefCnt = 1; /* stabilize */
			delete this;
			return 0;
		}
		return mRefCnt;
	}

	protected:
	    nsAutoRefCnt mRefCnt;

	public:
	    nsAmigaOSIMModule(nsWindow *aOwnerWindow);
	    ~nsAmigaOSIMModule();

	    PRBool IsEnabled();

	    void OnFocusWindow(nsWindow *aWindow);
	    void OnBlurWindow(nsWindow *aWindow);
	    void OnDestroyWindow(nsWindow *aWindow);
	    void OnFocusChangeInGecko(PRBool aFocus);

	    // OnKeyEvent is called when aWindow gets a native key press event or a
	    // native key release event.  If this returns TRUE, the key event was
	    // filtered by IME.  Otherwise, this returns FALSE.
	    // NOTE: When the keypress event starts composition, this returns TRUE but
	    //       this dispatches keydown event before compositionstart event.
	    PRBool OnKeyEvent(nsWindow* aWindow, struct IntuiMessage * aEvent,
	                      PRBool aKeyDownEventWasSent = PR_FALSE);

	    // IME related nsIWidget methods.
	    nsresult ResetInputState(nsWindow* aCaller);
	    nsresult SetInputMode(nsWindow* aCaller, const IMEContext* aContext);
	    nsresult GetInputMode(IMEContext* aContext);
	    nsresult CancelIMEComposition(nsWindow* aCaller);

	    // If a software keyboard has been opened, this returns TRUE.
	    // Otherwise, FALSE.
	    static PRBool IsVirtualKeyboardOpened();

	protected:
	    nsWindow* mOwnerWindow;
	    nsWindow* mLastFocusedWindow;

	    // IME enabled state and other things defined in IMEContext.
	    // Use following helper methods if you don't need the detail of the status.
	    IMEContext mIMEContext;

	    // mCompositionStart is the start offset of the composition string in the
	    // current content.  When <textarea> or <input> have focus, it means offset
	    // from the first character of them.  When a HTML editor has focus, it
	    // means offset from the first character of the root element of the editor.
	    PRUint32 mCompositionStart;

	    // mCompositionString is the current composing string. Even if this is
	    // empty, we can be composing.  See mIsComposing.
	    nsString mCompositionString;

	    // mIsComposing is set to TRUE when we dispatch the composition start
	    // event.  And it's set to FALSE when we dispatches the composition end
	    // event.  Note that mCompositionString can be empty string even if this is
	    // TRUE.
	    PRPackedBool mIsComposing;
	    // mIsIMFocused is set to TRUE when we call gtk_im_context_focus_in(). And
	    // it's set to FALSE when we call gtk_im_context_focus_out().
	    PRPackedBool mIsIMFocused;
	    // mFilterKeyEvent is used by OnKeyEvent().  If the commit event should
	    // be processed as simple key event, this is set to TRUE by the commit
	    // handler.
	    PRPackedBool mFilterKeyEvent;
	    // When mIgnoreNativeCompositionEvent is TRUE, all native composition
	    // should be ignored except that the compositon should be restarted in
	    // another content (nsIContent).  Don't refer this value directly, use
	    // ShouldIgnoreNativeCompositionEvent().
	    PRPackedBool mIgnoreNativeCompositionEvent;
	    // mKeyDownEventWasSent is used by OnKeyEvent() and
	    // DispatchCompositionStart().  DispatchCompositionStart() dispatches
	    // a keydown event if the composition start is caused by a native
	    // keypress event.  If this is true, the keydown event has been dispatched.
	    // Then, DispatchCompositionStart() doesn't dispatch keydown event.
	    PRPackedBool mKeyDownEventWasSent;
};


#endif

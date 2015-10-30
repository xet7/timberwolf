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


#include <proto/exec.h>

#define __NOLIBBASE__
#define __NOGLOBALIFACE__

#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/icon.h>
#include <proto/dos.h>
#include <proto/layers.h>
#include <proto/keymap.h>
#include <proto/timer.h>
#include <proto/diskfont.h>
#include <intuition/pointerclass.h>
#include <intuition/intuition.h>
#include <proto/Picasso96API.h>
#include <libraries/Picasso96.h>
#include <graphics/blitattr.h>
#include <libraries/diskfont.h>
#include <libraries/diskfonttag.h>
#include <graphics/gfx.h>

#include "prlink.h"
#include "prinrval.h"

#include "nsWindow.h"
#include "nsToolkit.h"
#include "nsIDeviceContext.h"
#include "nsIRenderingContext.h"
#include "nsIRegion.h"
#include "nsIRollupListener.h"
#include "nsIMenuRollup.h"
#include "nsIDOMNode.h"

#include "nsWidgetsCID.h"
#include "nsIDragService.h"

#include "nsDragService.h"

#include "nsWidgetAtoms.h"

#include "nsAppShell.h"

#ifdef MOZ_ENABLE_STARTUP_NOTIFICATION
#define SN_API_NOT_YET_FROZEN
#include <startup-notification-1.0/libsn/sn.h>
#endif

#include "nsIPrefService.h"
#include "nsIPrefBranch.h"
#include "nsIServiceManager.h"
#include "nsIStringBundle.h"
#include "nsGfxCIID.h"

/* For SetIcon */
#include "nsAppDirectoryServiceDefs.h"
#include "nsXPIDLString.h"
#include "nsIFile.h"
#include "nsILocalFile.h"
#include "nsTArray.h"

/* SetCursor(imgIContainer*) */
#include "imgIContainer.h"
#include "nsGfxCIID.h"

#include "nsIInterfaceRequestorUtils.h"
#include "nsAutoPtr.h"

#include "gfxContext.h"
#include "gfxImageSurface.h"
//#include "gfxAmigaOSSurface.h"

#include <stdlib.h>

#include <graphics/blitattr.h>

//#define DEBUG_FLASH 1

#if defined(MOZ_SPLASHSCREEN)
#include "nsSplashScreen.h"
#endif

static NS_DEFINE_IID(kCDragServiceCID,  NS_DRAGSERVICE_CID);

extern struct IntuitionIFace *IIntuition;
extern struct GraphicsIFace *IGraphics;
extern struct DOSIFace *IDOS;
extern struct KeymapIFace *IKeymap;
extern struct TimerIFace *ITimer;
extern struct LayersIFace *ILayers;
extern struct P96IFace *IP96;
extern struct DiskfontIFace *IDiskfont;

uint32 nsWindow::sNextWindowID = 0;
uint64 nsWindow::sLastButtonDownTime = 0;
uint64 nsWindow::sLastButtonUpTime = 0;
uint32 nsWindow::sLastButtonMask = 0;
uint32 nsWindow::sLastButtonDownX = 0;
uint32 nsWindow::sLastButtonDownY = 0;
nsWindow *nsWindow::sLastDragMotionWindow = NULL;
BOOL nsWindow::sLastButtonClickCount = 0;

PRBool sInDragSession = PR_FALSE;

static nsIRollupListener*          gRollupListener;
static nsIMenuRollup*              gMenuRollup;
static nsWeakPtr                   gRollupWindow;
static PRBool                      gConsumeRollupEvent;

PRBool check_for_rollup(struct IntuiMessage *msg, PRBool aIsWheel, PRBool aAlwaysRollup);

static nsTArray<nsWindow *> sWindowList;
static nsTArray<nsWindow *> sTopWindowList;
static nsWindow *sCurrentTop = nsnull;

static uint32 nsAmigaTranslateUnicode(uint16 Code, uint32 Qualifier);

#define DEBUG_TIMING

NS_IMPL_ISUPPORTS_INHERITED1(nsWindow, nsBaseWidget, nsISupportsWeakReference)

nsWindow::nsWindow() :
	mIsTopLevel(PR_FALSE),
	mIsVisible(PR_FALSE),
	mParent(nsnull),
	mEnabled(PR_TRUE),
	mFocus(nsnull),
	mCurrent(nsnull),
	mIsDestroyed(PR_FALSE),
	mIsTransparent(PR_FALSE),
	mNeedsRedraw(PR_FALSE),
	mIgnoreChangewnd(PR_FALSE),
	mAllowUserChange(PR_FALSE),
	mIsNativeWindow(PR_FALSE),
	mNeedsShow(PR_FALSE),
	mIntuitionWindow(NULL),
	mSharedPort(NULL),
	mAppShell(NULL),
	mCursor(NULL),
	mTitleStorage(NULL),
	mLastX(0),
	mLastY(0),
	mAlphaMap(NULL),
	mAlphaClip(NULL),
	mTransparencyBitmapWidth(0),
	mTransparencyBitmapHeight(0),
	mOldAlphaClip(NULL)
{
	mWindowID = sNextWindowID++;
}


nsWindow::~nsWindow()
{
	if (!mIsDestroyed)
		Destroy();
}


PRBool
nsWindow::ValidateWindow(nsWindow *w)
{
	for (size_t i = 0; i < sWindowList.Length(); i++)
	{
		if (sWindowList[i] == w)
			return PR_TRUE;
	}

	return PR_FALSE;
}

nsWindow *
nsWindow::FindTopLevel()
{
	nsWindow *toplevel = this;

	while (toplevel)
	{
		if (toplevel->mIsTopLevel)
			return toplevel;

		toplevel = toplevel->mParent;
	}

	LOG(("nsWindow[%d]::FindToplevel can't find toplevel window\n", mWindowID));
	return this;
}


nsWindow *
nsWindow::FindNative()
{
	nsWindow *parent = this;

	while (parent)
	{
		if (parent->mIsNativeWindow)
			return parent;

		parent = parent->mParent;
	}

	LOG(("nsWindow[%d]::FindNative can't find native window\n", mWindowID));
	return this;
}

NS_IMETHODIMP
nsWindow::Create(nsIWidget *aParent,
				 nsNativeWidget aNativeParent,
		         const nsIntRect &aRect,
		         EVENT_CALLBACK aHandleEventFunction,
		         nsIDeviceContext *aContext,
		         nsIAppShell *aAppShell,
		         nsIToolkit *aToolkit,
		         nsWidgetInitData *aInitData)
{
	uint32 zoomTag = TAG_IGNORE;

	mWindowType = aInitData->mWindowType;
	mIsTopLevel = (mWindowType == eWindowType_toplevel)
			   || (mWindowType == eWindowType_dialog)
			   || (mWindowType == eWindowType_invisible);
//	|| (mWindowType == eWindowType_popup)

	LOG(("nsWindow[%d]::Create aParent = %p, aNativeParent = %p, mIsTopLevel = %s, type = %d\n",
			mWindowID, aParent, aNativeParent, mIsTopLevel ? "true" : "false", mWindowType));

	nsWindow *parent = static_cast<nsWindow *>(aParent);

    if (aNativeParent)
    {
        if (aParent)
        {
        	LOG(("nsWindow[%d]::Create Ignoring native parent since parent was specified\n", mWindowID));
        }
        else
            parent = static_cast<nsWindow*>(aNativeParent);
    }

    LOG(("nsWindow[%d]::Create aRect = (%d, %d - %d x %d)\n", mWindowID, aRect.x, aRect.y, aRect.width, aRect.height));
	mBounds = aRect;
	mParent = parent;

	uint32 x = mBounds.x;
	uint32 y = mBounds.y;

	nsIWidget *baseParent = mIsTopLevel ? nsnull : mParent;

//	if (mWindowType == eWindowType_popup)
//	{
//		/* Move the popup mBounds relative to it's parent window */
//		nsWindow *root = FindTopLevel();
//		if (root)
//		{
//			LOG(("nsWindow[%d]::Create adjusting bounds for popup, root = (%d, %d), popup = (%d, %d), new = (%d, %d)\n", mWindowID,
//					root->mBounds.x, root->mBounds.y, mBounds.x, mBounds.y, mBounds.x - root->mBounds.x, mBounds.y - root->mBounds.y));
//			mBounds.x = mBounds.x - root->mBounds.x;
//			mBounds.y = mBounds.y - root->mBounds.y;
//		}
//	}

	BaseCreate(baseParent, aRect, aHandleEventFunction, aContext,
			aAppShell, aToolkit, aInitData);

	nsAppShell *appShell = (nsAppShell *)aAppShell;

	if (appShell == 0)
	{
		/* No appshell given. This happens if the window is a popup window.
		 * We go up and take the appshell of the parent.
		 */

		nsWindow *parentWin = mParent;
		appShell = (nsAppShell *)parentWin->mAppShell;
	}

	mAppShell = (void *)appShell;

	if (mIsTopLevel)
	{
		sWindowList.AppendElement(this);
		sTopWindowList.AppendElement(this);
		mSharedPort = (MsgPort *)appShell->GetMessagePort();
	}
	else
	{
		sWindowList.AppendElement(this);
		nsWindow *root = FindTopLevel();
		mSharedPort = root->mSharedPort;
		zoomTag = TAG_IGNORE;
	}

	if (mWindowType == eWindowType_popup)
	{
		// Find the root of the parent window
		nsWindow *w = parent->FindTopLevel();
		LOG(("nsWindow[%d]::Create Adding popup to parent's (%d) root (%d)\n",
				mWindowID, parent->mWindowID, w ? w->mWindowID : -1));

		if (w)
		{
			w->mPopupList.AppendElement(this);
		}
		zoomTag = TAG_IGNORE;
	}

	LOG(("nsWindow[%d]::Create MessagePort = %p\n", mWindowID, mSharedPort));
	NS_ASSERTION(mSharedPort, "No message port for this window");

	// Open the actual window
	if ( (mWindowType == eWindowType_dialog)
	  || (mWindowType == eWindowType_popup)
	  || (mWindowType == eWindowType_toplevel)
	  || (mWindowType == eWindowType_invisible) )
	{
		uint32 titleTag = TAG_IGNORE;
		uint32 windowFlags =
					WFLG_ACTIVATE
				  | WFLG_SMART_REFRESH
				  | WFLG_REPORTMOUSE
				  | WFLG_RMBTRAP
				  ;

		uint32 windowIDCMP =
					IDCMP_REFRESHWINDOW
				  | IDCMP_MOUSEBUTTONS
				  | IDCMP_MOUSEMOVE
				  | IDCMP_CLOSEWINDOW
				  | IDCMP_RAWKEY
				  | IDCMP_EXTENDEDKEYBOARD
				  | IDCMP_EXTENDEDMOUSE
				  | IDCMP_ACTIVEWINDOW
				  | IDCMP_SIZEVERIFY
				  | IDCMP_CHANGEWINDOW
//				  | IDCMP_INTUITICKS
				  ;

//		if (aInitData->mWindowType != eWindowType_popup)
//			windowIDCMP |= IDCMP_CHANGEWINDOW; // <- We always need that to move popups, except on popups themselves


		if (aInitData->mBorderStyle == eBorderStyle_none ||
			aInitData->mWindowType == eWindowType_popup)
		{
			windowFlags |= WFLG_BORDERLESS;
			zoomTag = TAG_IGNORE;
		}
//		else if (aInitData->mWindowType == eWindowType_dialog)
//		{
//			titleTag = WA_Title;
//			windowFlags |= WFLG_CLOSEGADGET | WFLG_DRAGBAR | WFLG_DEPTHGADGET;
//			if (aInitData->mBorderStyle != eBorderStyle_default)
//			{
//				windowFlags |= WFLG_SIZEGADGET;
//				zoomTag = WA_Zoom;
//			}
//		}
		else
		{
			windowFlags |= WFLG_CLOSEGADGET | WFLG_DRAGBAR | WFLG_DEPTHGADGET;
			titleTag = WA_Title;
			mAllowUserChange = PR_TRUE;

			if (aInitData->mBorderStyle & eBorderStyle_all ||
				aInitData->mBorderStyle & eBorderStyle_title ||
				aInitData->mBorderStyle == eBorderStyle_default)
			{
				titleTag = WA_Title;
				windowFlags |= WFLG_CLOSEGADGET | WFLG_DRAGBAR | WFLG_DEPTHGADGET;
			}
			if (aInitData->mBorderStyle & eBorderStyle_all ||
				aInitData->mBorderStyle & eBorderStyle_resizeh ||
				aInitData->mBorderStyle == eBorderStyle_default)
			{
				windowFlags |= WFLG_SIZEGADGET;
				zoomTag = WA_Zoom;
			}
			if (aInitData->mBorderStyle & eBorderStyle_all ||
				aInitData->mBorderStyle & eBorderStyle_close ||
				aInitData->mBorderStyle == eBorderStyle_default)
			{
				windowFlags |= WFLG_CLOSEGADGET | WFLG_DRAGBAR;
			}
			if (aInitData->mBorderStyle & eBorderStyle_minimize ||
				aInitData->mBorderStyle & eBorderStyle_maximize ||
				aInitData->mBorderStyle && eBorderStyle_all ||
				aInitData->mBorderStyle == eBorderStyle_default)
			{
				windowFlags |= WFLG_SIZEGADGET;
				zoomTag = WA_Zoom;
			}
		}

		if (mWindowType == eWindowType_invisible)
			mIsVisible = PR_FALSE;
		else
			mIsVisible = PR_TRUE;

		LOG(("nsWindow[%d]::Create Need to open native window (WFLG = %p, IDCMP = %p)\n",
				mWindowID, windowFlags, windowIDCMP));

		IBox zoomBox;
		zoomBox.Left = 0;
		zoomBox.Top = 0;
		zoomBox.Height = 1024;
		zoomBox.Width = 1024;

		BOOL hide = FALSE;
		if (!mIsVisible || (mBounds.width <= 1) || (mBounds.height <= 1))
		{
			hide = TRUE;

			if (mIsVisible)
				mNeedsShow = PR_TRUE;
		}

		mIntuitionWindow = IIntuition->OpenWindowTags(NULL,
			titleTag,			"Timberwolf 4.0.1",
			WA_Flags,			windowFlags,
			WA_IDCMP,			windowIDCMP,
			WA_Left,			x,
			WA_Top,				y,
			WA_InnerWidth,		mBounds.width + (mBounds.width == 0 ? 1 : 0),
			WA_InnerHeight,		mBounds.height + (mBounds.height == 0 ? 1 : 0),
			WA_WindowName,		"Timberwolf",
			WA_UserPort,		mSharedPort,
			WA_NotifyDepth,		TRUE,
			WA_Hidden,			hide,
			WA_ToolBox,			mWindowType == eWindowType_popup ? TRUE : FALSE,
			WA_MinWidth, 		1,
			WA_MinHeight,		1,
			WA_MaxWidth,		~0,
			WA_MaxHeight,		~0,
			zoomTag,			&zoomBox,
		TAG_DONE);

		LOG(("nsWindow[%d]::Create mIntuitionWindow = %p, requested size (%dx%d) actual size (%dx%d)\n",
				mWindowID, mIntuitionWindow, mBounds.width, mBounds.height,
				mIntuitionWindow->Width, mIntuitionWindow->Height));

		if (mIntuitionWindow == NULL)
			return NS_ERROR_FAILURE;

		mIntuitionWindow->UserData = (BYTE *)this;

		mIsNativeWindow = PR_TRUE;
	}
	else if (mWindowType == eWindowType_child)
	{
		mIsVisible = PR_TRUE;
		nsWindow *root = FindTopLevel();
		mIntuitionWindow = root->mIntuitionWindow;
	}

#ifdef AMIGAOS_ENABLE_IME
	if (mWindowType != eWindowType_popup)
	{
		if (mIsTopLevel)
			mIMModule = new nsAmigaOSIMModule(this);
		else
		{
			nsWindow *root = FindTopLevel();
			mIMModule = root->mIMModule;
		}
	}
#endif

	// Resize so that everything is set to the right dimensions
	if (!mIsNativeWindow)
		Resize(mBounds.x, mBounds.y, mBounds.width, mBounds.height, PR_FALSE);

	nsGUIEvent event(PR_TRUE, NS_CREATE, this);
	nsEventStatus status;
	DispatchEvent(&event, status);

	if (mIsTopLevel)
		OnActivateEvent();

	return NS_OK;
}


static void
StripIntuiMessages(struct Window *win)
{
    struct IntuiMessage *msg;
    struct Node *succ;
    struct MsgPort *mp = win->UserPort;

    msg = (struct IntuiMessage *) mp->mp_MsgList.lh_Head;

    while((succ = msg->ExecMessage.mn_Node.ln_Succ))
    {
    	if( msg->IDCMPWindow == win)
    	{
    		IExec->Remove((struct Node *)msg);

    		IExec->ReplyMsg((struct Message *)msg);
    	}

    	msg = (struct IntuiMessage *)succ;
    }
}

NS_IMETHODIMP
nsWindow::Destroy(void)
{
	LOG(("nsWindow[%d]::Destroy called\n", mWindowID));

	if (mIsDestroyed || !mIntuitionWindow)
		return NS_OK;

	mIsDestroyed = PR_TRUE;

	nsWindow *top = FindTopLevel();
	if (top->mFocus == this)
		top->mFocus = nsnull;

	if (top->mCurrent == this)
		top->mCurrent = nsnull;

	if (sCurrentTop == this)
		sCurrentTop = nsnull;

	if (mIsTopLevel)
		sTopWindowList.RemoveElement(this);

	if (mWindowType == eWindowType_popup)
		 top->mPopupList.RemoveElement(this);

	sWindowList.RemoveElement(this);
	mBounds.x = -1000;
	mBounds.y = -1000;

//XXX	ClearCachedResources();

//XXX
//	nsCOMPtr<nsIWidget> rollupWidget = do_QueryReferent(gRollupWindow);
//    if (static_cast<nsIWidget *>(this) == rollupWidget.get())
//    {
//        if (gRollupListener)
//            gRollupListener->Rollup(nsnull, nsnull);
//        NS_IF_RELEASE(gMenuRollup);
//        gRollupWindow = nsnull;
//        gRollupListener = nsnull;
//    }

	nsIWidget *f = GetFirstChild();
	while (f)
	{
		nsIWidget *n = f->GetNextSibling();
		f->Destroy();
		f = n;
	}

	if (mWindowType == eWindowType_popup)
	{
		// Find the root of the parent window
		nsWindow *w = mParent->FindTopLevel();
		if (w)
		{
			w->mPopupList.RemoveElement(this);
		}
	}

#ifdef AMIGAOS_ENABLE_IME
	if (mIMModule)
		mIMModule->OnDestroyWindow(this);
#endif

	nsBaseWidget::Destroy();

	if (mIsNativeWindow && mIntuitionWindow)
	{
		/* Make sure we strip any pending intuition messages from the shared
		 * port
		 */
		IExec->Forbid();
		StripIntuiMessages(mIntuitionWindow);
		mIntuitionWindow->UserPort = 0;
		IIntuition->ModifyIDCMP(mIntuitionWindow, 0);
		IExec->Permit();


		//DestroyTransparencyBitmap();

		IIntuition->CloseWindow(mIntuitionWindow);
		mIntuitionWindow = NULL;

		mIsNativeWindow = PR_FALSE;
	}

	OnDestroy();
	
	return NS_OK;
}


NS_IMETHODIMP
nsWindow::ConfigureChildren(const nsTArray<nsIWidget::Configuration>& config)
{
    for (PRUint32 i = 0; i < config.Length(); ++i) {
        nsWindow *w = static_cast<nsWindow*>(config[i].mChild);

        if (w->mBounds.Size() != config[i].mBounds.Size())
        {
        	w->Resize(config[i].mBounds.x,
        			  config[i].mBounds.y,
        			  config[i].mBounds.width,
        			  config[i].mBounds.height,
        			  PR_FALSE);
        }
        else if (w->mBounds.TopLeft() != config[i].mBounds.TopLeft())
        {
        	w->Move(config[i].mBounds.x, config[i].mBounds.y);
        }
    }

    return NS_OK;
}

NS_IMETHODIMP
nsWindow::SetParent(nsIWidget* aNewParent)
{
	LOG(("nsWindow[%d]::SetParent called\n", mWindowID));

	// Prevent premature deletion
	nsCOMPtr<nsIWidget> kungFuDeathGrip = this;

	mParent->RemoveChild(this);

	mParent = static_cast<nsWindow *>(aNewParent);

	if (mParent)
	{
		mParent->AddChild(this);
	}

#ifdef MOZ_LOGGING
	if (mParent)
	{
		LOG(("\tNew parent id is %d\n", mParent->mWindowID));
	}
#endif

	return NS_OK;
}


nsIWidget *
nsWindow::GetParent(void)
{
	return mParent;
}


float
nsWindow::GetDPI()
{
	uint32 dpiX = IDiskfont->GetDiskFontCtrl(DFCTRL_XDPI);
	uint32 dpiY = IDiskfont->GetDiskFontCtrl(DFCTRL_YDPI);

	return (float)(dpiY > dpiX ? dpiY : dpiX);
}


NS_IMETHODIMP
nsWindow::Show(PRBool aState)
{
#if defined(MOZ_SPLASHSCREEN)&& !defined(DEBUG)
	// we're about to show the first toplevel window,
	// so kill off any splash screen if we had one
	nsSplashScreen *splash = nsSplashScreen::Get();
	if (splash && splash->IsOpen() &&
			(mWindowType == eWindowType_toplevel ||
					mWindowType == eWindowType_dialog ||
					mWindowType == eWindowType_popup))
	{
		splash->Close();
	}
#endif

	if (aState == mIsVisible)
		return NS_OK;

	LOG(("nsWindow[%d]::Show called with state %s\n",
			mWindowID, aState ? "true" : "false"));

	mIsVisible = aState;

	if (mIsNativeWindow)
	{
		uint32 state = (aState == PR_TRUE) ? FALSE : TRUE;
		IIntuition->SetWindowAttr(mIntuitionWindow, WA_Hidden,
				(void *)state, sizeof(state));
	}

	if (aState)
	{
		// If we just became visible, resize and invalidate
		Resize(mBounds.x, mBounds.y, mBounds.width, mBounds.height, PR_FALSE);
		//Invalidate(nsIntRect(0, 0, mBounds.width, mBounds.height), PR_FALSE);
		Invalidate(mBounds, PR_FALSE);
	}
	//Invalidate(mBounds, PR_FALSE);
	
	return NS_OK;
}


NS_IMETHODIMP
nsWindow::SetModal(PRBool aModal)
{
	return NS_OK;
}


NS_IMETHODIMP
nsWindow::IsVisible(PRBool & aState)
{
	aState = mIsVisible;

	return NS_OK;
}


NS_IMETHODIMP
nsWindow::ConstrainPosition(PRBool aAllowSlop,
                           PRInt32 *aX,
                           PRInt32 *aY)
{
//	if (mIsTopLevel)
//	{
//		*aX = 0;
//		*aY = 0;
//	}

	return NS_OK;
}


NS_IMETHODIMP
nsWindow::Move(PRInt32 aX,
               PRInt32 aY)
{
	LOG(("nsWindow[%d]::Move to %d, %d\n", mWindowID, aX, aY));

	return Resize(aX, aY, mBounds.width, mBounds.height, PR_TRUE);
}


NS_IMETHODIMP
nsWindow::Resize(PRInt32 aWidth,
                 PRInt32 aHeight,
                 PRBool  aRepaint)
{
	LOG(("nsWindow[%d]::Resize to %d, %d (with%s repaint)\n",
			mWindowID, aWidth, aHeight, aRepaint ? "" : "out"));
	return Resize(mBounds.x, mBounds.y, aWidth, aHeight, aRepaint);
}


NS_IMETHODIMP
nsWindow::Resize(PRInt32 aX,
                 PRInt32 aY,
                 PRInt32 aWidth,
                 PRInt32 aHeight,
                 PRBool aRepaint)
{
	LOG(("nsWindow[%d]::Resize to %d, %d - %dx%d(with%s repaint)\n",
			mWindowID, aX, aY, aWidth, aHeight, aRepaint ? "" : "out"));

/*XXXif (mWindowID == 1 && aWidth > 100) __asm volatile ("trap");*/

	PRBool sizeChanged = aWidth != mBounds.width || aHeight != mBounds.height;

	int32 dx;
	int32 dy;

	dx = mBounds.x - aX;
	dy = mBounds.y - aY;

	mBounds.x = aX;
	mBounds.y = aY;
	mBounds.SizeTo(nsIntSize(aWidth, aHeight));

//	if (mWindowType == eWindowType_popup)
//	{
//		/* Move the popup mBounds relative to it's parent window */
//		nsWindow *root = FindTopLevel();
//		if (root)
//		{
//			LOG(("nsWindow[%d]::Resize adjusting bounds for popup, root = (%d, %d), popup = (%d, %d), new = (%d, %d)\n", mWindowID,
//					root->mBounds.x, root->mBounds.y, aX, aY, aX - root->mBounds.x, aY - root->mBounds.y));
//			mBounds.x = aX - root->mBounds.x;
//			mBounds.y = aY - root->mBounds.y;
//		}
//	}

	if (mIsNativeWindow)
	{
		NativeWindowResize(aX, aY, aWidth, aHeight, aRepaint);
		MovePopups(dx, dy);
		OnResizeEvent(mBounds);

		if (sizeChanged && mIsTransparent)
			ResizeTransparencyBitmap(aWidth, aHeight);
	}
	else if (sizeChanged)
		OnResizeEvent(mBounds);

	if (aRepaint)
		OnDraw();

	LOG(("nsWindow[%d]::Resize mBounds: %d, %d - %dx%d\n",
			mWindowID, mBounds.x, mBounds.y, mBounds.width, mBounds.height));
			
	return NS_OK;
}



NS_IMETHODIMP
nsWindow::SetZIndex(PRInt32 aZIndex)
{
	LOG(("nsWindow[%d]::SetZIndex %d\n", mWindowID, aZIndex));

	nsBaseWidget::SetZIndex(aZIndex);
	
	return NS_OK;
}



NS_IMETHODIMP
nsWindow::PlaceBehind(nsTopLevelWidgetZPlacement aPlacement,
                      nsIWidget *aWidget,
                      PRBool aActivate)
{
	LOG(("nsWindow[%d]::PlaceBehind\n", mWindowID));

	if (mIsDestroyed)
		return NS_OK;

	if (aPlacement == eZPlacementBottom)
	{
		sWindowList.RemoveElement(this);
		sWindowList.AppendElement(this);

		if (mWindowType == eWindowType_popup)
		{
			nsWindow *w = FindTopLevel();

			if (w)
			{
				w->mPopupList.RemoveElement(this);
				w->mPopupList.AppendElement(this);
			}
		}
		return NS_OK;
	}
	else if (aPlacement == eZPlacementTop)
	{
		sWindowList.RemoveElement(this);
		sWindowList.InsertElementAt(0, this);
		if (mWindowType == eWindowType_popup)
		{
			nsWindow *w = FindTopLevel();

			if (w)
			{
				w->mPopupList.RemoveElement(this);
				w->mPopupList.InsertElementAt(0, this);
			}
		}
		return NS_OK;
	}
	else if (aPlacement == eZPlacementBelow)
	{
		nsWindow *target = static_cast<nsWindow *>(aWidget);
		sWindowList.RemoveElement(this);
		bool found = false;

		for (size_t i = 0; i < sWindowList.Length(); i++)
		{
			if (sWindowList[i] == target)
			{
				sWindowList.InsertElementAt(i, this);
				found = true;
				break;
			}
		}
		if (!found)
			sWindowList.AppendElement(this);

		if (mWindowType == eWindowType_popup)
		{
			nsWindow *w = FindTopLevel();

			if (w)
			{
				bool found = false;

				for (size_t i = 0; i < w->mPopupList.Length(); i++)
				{
					if (w->mPopupList[i] == target)
					{
						w->mPopupList.InsertElementAt(i, this);
						found = true;
						break;
					}
				}

				if (!found)
					w->mPopupList.AppendElement(this);
			}


		}
	}

	return NS_OK;
}



NS_IMETHODIMP
nsWindow::SetSizeMode(PRInt32 aMode)
{
	LOG(("nsWindow[%d]::SetSizeMode\n", mWindowID));

	PRInt32 currentState = mSizeMode;

	nsresult rv = nsBaseWidget::SetSizeMode(aMode);

	switch (aMode)
	{
	case nsSizeMode_Maximized:
		NativeWindowMaximize(PR_TRUE);
		break;
	case nsSizeMode_Minimized:
		NativeWindowIconify(PR_TRUE);
		break;
	case nsSizeMode_Fullscreen:
		NativeWindowMakeFullscreen(PR_TRUE);
		break;
	default:
		switch (currentState)
		{
		case nsSizeMode_Maximized:
			NativeWindowMaximize(PR_FALSE);
			break;
		case nsSizeMode_Minimized:
			NativeWindowIconify(PR_FALSE);
			break;
		case nsSizeMode_Fullscreen:
			NativeWindowMakeFullscreen(PR_FALSE);
			break;
		}
		break;
	}

	return rv;
}



NS_IMETHODIMP
nsWindow::Enable(PRBool aState)
{
	LOG(("nsWindow[%d]::Enable aState = %s\n",
			mWindowID, aState ? "true" : "false"));

	mEnabled = aState;

	return NS_OK;
}


NS_IMETHODIMP
nsWindow::IsEnabled(PRBool *aState)
{
	*aState = mEnabled;

	return NS_OK;
}



NS_IMETHODIMP
nsWindow::Invalidate(const nsIntRect &aRect,
                     PRBool aIsSynchronous)
{
	LOGDRAW(("nsWindow[%d]::Invalidate (is %ssynchronous, %d,%d - %dx%d)\n",
			mWindowID,	aIsSynchronous ? "" : "a",
			aRect.x, aRect.y, aRect.width, aRect.height));

	if (!aIsSynchronous)
	{
		if (mIsNativeWindow)
		{
			mDamageRects.Or(aRect, mDamageRects);
			if (mAppShell)
				((nsAppShell *)mAppShell)->SynthesizeRedrawEvent(this);
		}
		else
		{
			uint32 x = 0,
				   y = 0;

			nsWindow *w = this;
			while (w && !w->mIsNativeWindow)
			{
				x += w->mBounds.x;
				y += w->mBounds.y;

				w = w->mParent;
			}

			nsIntRect newRect(x+aRect.x, y+aRect.y, aRect.width, aRect.height);
			w->Invalidate(newRect, aIsSynchronous);
		}
	}
	else
	{
		OnDraw(const_cast<nsIntRect*>(&aRect));
	}
	
	return NS_OK;
}



NS_IMETHODIMP
nsWindow::Update()
{
	LOGDRAW(("nsWindow[%d]::Update\n", mWindowID));
	OnDraw();
	return NS_OK;
}


NS_IMETHODIMP
nsWindow::SetFocus(PRBool aRaise)
{
	nsWindow *w = FindTopLevel();
	if (!w)
		return NS_OK;

	LOGEVENT(("nsWindow[%d]::SetFocus called (Current focus %d, current top %d, raise = %s)\n",
			mWindowID, w ? (w->mFocus  ? w->mFocus->mWindowID : -1) : -1,
			sCurrentTop ? sCurrentTop->mWindowID : -1,
			aRaise ? "true" : "false"));

    w->mFocus = this;

    if (aRaise)
    {
    	if (w != sCurrentTop)
    	{
    		LOGEVENT(("nsWindow[%d]::SetFocus Focus changed from %d to %d\n", sCurrentTop ? sCurrentTop->mWindowID : -1, w->mWindowID));

    		if (sCurrentTop)
    			sCurrentTop->OnDeactivateEvent();

    		sCurrentTop = w;
    		w->OnActivateEvent();
    		IIntuition->WindowToFront(w->mIntuitionWindow);

    		sTopWindowList.RemoveElement(w);
    		sTopWindowList.InsertElementAt(0, w);
    	}
    }


	return NS_OK;
}


NS_IMETHODIMP
nsWindow::GetScreenBounds(nsIntRect &aRect)
{
    aRect = nsIntRect(nsIntPoint(0, 0), mBounds.Size());

	if (mIsTopLevel)
	{
		struct IBox windowBox;
		IIntuition->GetWindowAttrs(mIntuitionWindow, WA_WindowBox, &windowBox, sizeof(windowBox));
		aRect.MoveTo(windowBox.Left, windowBox.Top);
	}
	else
	{
		aRect.MoveTo(WidgetToScreenOffset());
	}

//	printf("Screen bounds of %d: %d, %d - %dx%d\n", mWindowID, aRect.x, aRect.y, aRect.width, aRect.height);
	return NS_OK;
}



nsIntPoint
nsWindow::WidgetToScreenOffset()
{
	//XXX Really ?
	uint32 x = 0,
		   y = 0;
	nsWindow *top = FindTopLevel();

	nsWindow *w = this;
	while (w && !w->mIsTopLevel)
	{
		x += w->mBounds.x;
		y += w->mBounds.y;

		w = w->mParent;
	}


	/* Account for the window decoration of the intuition window */
	if (mWindowType != eWindowType_popup)
	{
		x += top->mIntuitionWindow->BorderLeft + top->mIntuitionWindow->LeftEdge;
		y += top->mIntuitionWindow->BorderTop + top->mIntuitionWindow->TopEdge;
	}
//	else
//	{
//		printf(" %d, %d -> %d, %d (top: %d, %d)\n", x, y, mBounds.x,mBounds.y, top->mBounds.x, top->mBounds.y);
//		x += top->mBounds.x;
//		y += top->mBounds.y;
//	}

	//LOGDRAW(("nsWindow[%d]::WidgetToScreenOffset returned %d, %d\n", mWindowID, x, y));

//	printf("nsWindow[%d]::WidgetToScreenOffset (new) returned %d, %d\n", mWindowID, x, y);
	return nsIntPoint(x,y);
}



nsEventStatus
nsWindow::DispatchEvent(nsGUIEvent *aEvent)
{
	nsEventStatus status;

	DispatchEvent(aEvent, status);
	return status;
}



NS_IMETHODIMP
nsWindow::DispatchEvent(nsGUIEvent *aEvent, nsEventStatus &aStatus)
{
	aStatus = nsEventStatus_eIgnore;

	if (mEventCallback)
		aStatus = (*mEventCallback)(aEvent);

	return NS_OK;
}



NS_IMETHODIMP
nsWindow::MakeFullScreen(PRBool aFullScreen)
{
	LOG(("nsWindow[%d]::MakeFullScreen aFullScreen = %s\n",
			mWindowID, aFullScreen ? "true" : "false"));

	NativeWindowMakeFullscreen(aFullScreen);
	return NS_OK;
}



NS_IMETHODIMP
nsWindow::SetWindowClass(const nsAString& xulWinType)
{
	return NS_OK;
}


NS_IMETHODIMP
nsWindow::SetForegroundColor(const nscolor &aColor)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}


NS_IMETHODIMP
nsWindow::SetBackgroundColor(const nscolor &aColor)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}


NS_IMETHODIMP
nsWindow::SetCursor(imgIContainer* aCursor,
                     PRUint32 aHotspotX,
                     PRUint32 aHotspotY)
{
	//XXX Implement
	return NS_ERROR_FAILURE;
}


NS_IMETHODIMP
nsWindow::SetCursor(nsCursor aCursor)
{
	struct Window *window = mIntuitionWindow;

	if (mCursor)
	{
		// Shortcut if the cursor already has the right shape
		if (mCursor->GetType() == aCursor)
		{
			mCursor->Set();
			return NS_OK;
		}
		delete mCursor;
		mCursor = NULL;
	}

	STRPTR name = NULL;
	switch(aCursor)
	{
	case eCursor_standard:
		IIntuition->SetWindowPointer(window,
					WA_Pointer, NULL,
				TAG_DONE);
		return NS_OK;
	case eCursor_wait:
		IIntuition->SetWindowPointer(window,
					WA_BusyPointer, TRUE,
				TAG_DONE);
		return NS_OK;
	case eCursor_select:
		name = (char *)"def_textpointer";
		break;
	case eCursor_hyperlink:
		name = (char *)"def_linkpointer";
		break;
	case eCursor_n_resize:
		name = (char *)"def_northresizepointer";
		break;
	case eCursor_s_resize:
		name = (char *)"def_southresizepointer";
		break;
	case eCursor_w_resize:
		name = (char *)"def_westresizepointer";
		break;
	case eCursor_e_resize:
		name = (char *)"def_eastresizepointer";
		break;
	case eCursor_ne_resize:
		name = (char *)"def_northeastresizepointer";
		break;
	case eCursor_sw_resize:
		name = (char *)"def_southwestresizepointer";
		break;
	case eCursor_se_resize:
		name = (char *)"def_southeastresizepointer";
		break;
	case eCursor_nw_resize:
		name = (char *)"def_northwestresizepointer";
		break;
	case eCursor_crosshair:
		name = (char *)"def_crosspointer";
		break;
	case eCursor_move:
		name = (char *)"def_handpointer";
		break;
	case eCursor_help:
		name = (char *)"def_helppointer";
		break;
	case eCursor_copy:
		name = (char *)"def_copypointer";
		break;
	case eCursor_alias:
		name = (char *)"def_aliaspointer";
		break;
	case eCursor_context_menu:
		name = (char *)"def_contextmenupointer";
		break;
	case eCursor_cell:
		name = (char *)"def_cellpointer";
		break;
	case eCursor_grab:
		name = (char *)"def_handpointer";
		break;
	case eCursor_grabbing:
		name = (char *)"def_handpointer";
		break;
	case eCursor_spinning:
		name = (char *)"def_progresspointer";
		break;
	case eCursor_zoom_in:
		name = (char *)"def_zoominpointer";
		break;
	case eCursor_zoom_out:
		name = (char *)"def_zoomoutpointer";
		break;
	case eCursor_not_allowed:
		name = (char *)"def_notallowedpointer";
		break;
	case eCursor_col_resize:
		name = (char *)"def_columnresizepointer";
		break;
	case eCursor_row_resize:
		name = (char *)"def_rowresizepointer";
		break;
	case eCursor_no_drop:
		name = (char *)"def_nodroppointer";
		break;
	case eCursor_vertical_text:
		name = (char *)"def_verticaltextpointer";
		break;
	case eCursor_all_scroll:
		name = (char *)"def_scrollallpointer";
		break;
	case eCursor_nwse_resize:
		name = (char *)"def_northwestsoutheastresizepointer";
		break;
	case eCursor_nesw_resize:
		name = (char *)"def_northeastsouthwestresizepointer";
		break;
	case eCursor_ns_resize:
		name = (char *)"def_northsouthresizepointer";
		break;
	case eCursor_ew_resize:
		name = (char *)"def_eastwestresizepointer";
		break;
	case eCursor_none:
		name = NULL;
		break;
	default:
		return NS_ERROR_UNEXPECTED;
	}

	if (name)
	{
		char buffer[1024];
		strcpy(buffer, "ENVARC:Sys/");
		strcat(buffer, name);
		mCursor = new nsAmigaOSCursor(window, buffer, aCursor);
		if (mCursor)
			mCursor->Set();
	}

	return NS_OK;
}


NS_IMETHODIMP
nsWindow::HideWindowChrome(PRBool aShouldHide)
{
	//XXX implent me ?
	return NS_OK;
}


void*
nsWindow::GetNativeData(PRUint32 aDataType)
{
	switch (aDataType)
	{
	case NS_NATIVE_WIDGET:
		return this;

	case NS_NATIVE_WINDOW:
		return mIntuitionWindow;

	case NS_NATIVE_DISPLAY:
		return NULL;	//XXX return screen ?
	}

	return nsnull;
}


NS_IMETHODIMP
nsWindow::SetTitle(const nsAString& aTitle)
{
	if (!mIsNativeWindow)
		return NS_OK;

	const char *titleString = NS_LossyConvertUTF16toASCII(aTitle).get();

	LOG(("nsWindow[%d]::SetTitle %s\n", mWindowID, titleString));

	if (mTitleStorage)
	{
		free(mTitleStorage);
		mTitleStorage = 0;
	}

	if (mIntuitionWindow)
	{
		mTitleStorage = strdup(titleString);
		IIntuition->SetWindowTitles(mIntuitionWindow,
				(CONST_STRPTR)mTitleStorage, (CONST_STRPTR)mTitleStorage);
	}

	return NS_OK;
}


NS_IMETHODIMP
nsWindow::SetIcon(const nsAString& aIconSpec)
{
	LOG(("nsWindow[%d]::SetIcon not yet implemented\n", mWindowID));

	return NS_OK;
}



NS_IMETHODIMP
nsWindow::EnableDragDrop(PRBool aEnable)
{
	LOG(("nsWindow[%d]::EnableDragDrop with aEnable = %s\n", mWindowID, aEnable ? "true" : "false"));
	return NS_OK;
}


NS_IMETHODIMP
nsWindow::CaptureMouse(PRBool aCapture)
{
	//XXX not implemented
	return NS_OK;
}


NS_IMETHODIMP
nsWindow::CaptureRollupEvents(nsIRollupListener *aListener,
                              nsIMenuRollup *aMenuRollup,
                              PRBool aDoCapture,
                              PRBool aConsumeRollupEvent)
{
    if (!mIntuitionWindow)
        return NS_OK;

    LOG(("nsWindow[%d]::CaptureRollupEvents (aDoCapture = %s, aConsumeRollupEvent = %s)\n",  mWindowID,
    		aDoCapture ? "PR_TRUE" : "PR_FALSE",
    		aConsumeRollupEvent ? "PR_TRUE" : "PR_FALSE"
    		));

    if (aDoCapture)
    {
        gConsumeRollupEvent = aConsumeRollupEvent;
        gRollupListener = aListener;
        NS_IF_RELEASE(gMenuRollup);
        gMenuRollup = aMenuRollup;
        NS_IF_ADDREF(aMenuRollup);
        gRollupWindow = do_GetWeakReference(static_cast<nsIWidget*>
                                                       (this));
    }
    else
    {
        gRollupListener = nsnull;
        NS_IF_RELEASE(gMenuRollup);
        gRollupWindow = nsnull;
    }

    return NS_OK;
}


NS_IMETHODIMP
nsWindow::GetAttention(PRInt32 aCycleCount)
{
	FlashWindow();
	IIntuition->DisplayBeep(NULL);

	return NS_OK;
}



NS_IMETHODIMP
nsWindow::BeginResizeDrag(nsGUIEvent* aEvent,
						  PRInt32 aHorizontal,
						  PRInt32 aVertical)
{
	//XXX Not implemented
	LOG(("nsWindow[%d]::BeginResizeDrag not yet implemented\n", mWindowID));

	return NS_OK;
}



NS_IMETHODIMP
nsWindow::ResetInputState()
{
	LOG(("nsWindow[%d]::ResetInputState\n", mWindowID));
#ifdef AMIGAOS_ENABLE_IME
	return mIMModule ? mIMModule->ResetInputState(this) : NS_OK;
#endif
	return NS_OK;
}


NS_IMETHODIMP
nsWindow::SetInputMode(const IMEContext& aContext)
{
	//XXX See ResetInputState
	LOG(("nsWindow[%d]::SetInputMode\n", mWindowID));
#ifdef AMIGAOS_ENABLE_IME
	return mIMModule ? mIMModule->SetInputMode(this, &aContext) : NS_OK;
#endif
	return NS_OK;
}


NS_IMETHODIMP
nsWindow::GetInputMode(IMEContext& aContext)
{
#ifdef AMIGAOS_ENABLE_IME
	if (!mIMModule)
	{
		aContext.mStatus = nsIWidget::IME_STATUS_DISABLED;
		return NS_OK;
	}
	return mIMModule->GetInputMode(&aContext);
#else
	aContext.mStatus = nsIWidget::IME_STATUS_DISABLED;
	return NS_OK;
#endif
}

NS_IMETHODIMP
nsWindow::CancelIMEComposition()
{
	//XXX See above
	LOG(("nsWindow[%d]::CancelIMEComposition\n", mWindowID));
#ifdef AMIGAOS_ENABLE_IME
    return mIMModule ? mIMModule->CancelIMEComposition(this) : NS_OK;
#endif
    return NS_OK;
}


NS_IMETHODIMP
nsWindow::OnIMEFocusChange(PRBool aFocus)
{
	//XXX See above
	LOG(("nsWindow[%d]::OnIMEFocusChange\n", mWindowID));
#ifdef AMIGAOS_ENABLE_IME
    if (mIMModule)
      mIMModule->OnFocusChangeInGecko(aFocus);
#endif

    // XXX Return NS_ERROR_NOT_IMPLEMENTED, see bug 496360.
    return NS_ERROR_NOT_IMPLEMENTED;
}



NS_IMETHODIMP
nsWindow::OnIMETextChange(PRUint32 aStart, PRUint32 aOldEnd, PRUint32 aNewEnd)
{
	//XXX See above
	LOG(("nsWindow[%d]::OnIMETextChange\n", mWindowID));

    return NS_OK;
}

NS_IMETHODIMP
nsWindow::OnIMESelectionChange(void)
{
	//XXX See above
	LOG(("nsWindow[%d]::OnIMETextChange\n", mWindowID));

    return NS_OK;
}


mozilla::layers::LayerManager*
nsWindow::GetLayerManager(LayerManagerPersistence aPersistence,
                          bool* aAllowRetaining)
{
    if (aAllowRetaining)
        *aAllowRetaining = true;

    if (mLayerManager)
        return mLayerManager;

    LOGDRAW(("nsWindow[%d]::GetLayerManager creating LayerManager\n",
    		mWindowID));

    mLayerManager = CreateBasicLayerManager();

    return mLayerManager;
}



gfxASurface*
nsWindow::GetThebesSurface()
{
	//XXX Just this ? Check Android vs. GTK2 implementation
	LOGDRAW(("nsWindow[%d]::GetThebesSurface called\n", mWindowID));

	gfxIntSize surfaceSize(mBounds.width, mBounds.height);
	mThebesSurface = new gfxImageSurface(surfaceSize,
			gfxASurface::ImageFormatRGB24);

	return mThebesSurface;
}


NS_IMETHODIMP
nsWindow::ReparentNativeWidget(nsIWidget* aNewParent)
{
	return NS_OK;
}



PRBool
nsWindow::HandleAmigaIDCMP(void *message)
{
	//nsWindow *top = FindTopLevel();
	//nsWindow *currentFocus = top->mFocus;
	struct IntuiMessage *imsg
		= reinterpret_cast<struct IntuiMessage *>(message);
	PRBool ret = PR_FALSE;
	nsIntPoint mousePoint(imsg->MouseX - imsg->IDCMPWindow->BorderLeft,
			imsg->MouseY - imsg->IDCMPWindow->BorderTop);
	nsWindow *top = FindTopLevel();

	//LOGEVENT(("nsWindow[%d]::HandleAmigaIDCMP called\n", mWindowID));

	switch (imsg->Class)
	{
	case IDCMP_CLOSEWINDOW:
		{
			nsGUIEvent event(PR_TRUE, NS_XUL_CLOSE, this);
			InitEvent(event, nsnull, nsnull);
			DispatchEvent(&event);
		}
		break;
	case IDCMP_MOUSEBUTTONS:
		{
			if (sInDragSession)
			{
				/* We're in drag/drop, so any button will cause a drop action. Regardless of
				 * the outcome, the drag session will end
				 */
				if (sLastDragMotionWindow)
				{
					sLastDragMotionWindow->OnDragDropEvent(imsg);
					sInDragSession = PR_FALSE;
					sLastDragMotionWindow = 0;
				}
			}
// Fall through and handle the event ?			else
			{
				nsWindow *w = FindWindowForPoint(mousePoint, imsg);
				if (w)
					w->OnMouseButtonEvent(imsg);
				else
				{
					LOGEVENT(("nsWindow[%d]::HandleAmigaIDCMP no window for mouse button event\n", mWindowID));
				}
			}
		}
		break;
	case IDCMP_CHANGEWINDOW:
		// This only handled user changes. Therefore, it must check whether
		// the window can actually be changed by the user.
		if (imsg->Code == CWCODE_MOVESIZE && !mIgnoreChangewnd && mAllowUserChange)
		{
			LOGEVENT(("nsWindow[%d]::HandleAmigaIDCMP CHANGEWINDOW\n", mWindowID));

			if (mWindowType == eWindowType_toplevel || mWindowType == eWindowType_dialog)
				check_for_rollup(imsg, PR_FALSE,  PR_TRUE);

			nsIntRect oldRect(mBounds);
			nscoord w = mIntuitionWindow->Width	- mIntuitionWindow->BorderLeft
					- mIntuitionWindow->BorderRight;
			nscoord h = mIntuitionWindow->Height - mIntuitionWindow->BorderTop
					- mIntuitionWindow->BorderBottom;
			nscoord y = mIntuitionWindow->TopEdge + mIntuitionWindow->BorderTop;
			nscoord x = mIntuitionWindow->LeftEdge + mIntuitionWindow->BorderLeft;

			int32 dx, dy;

			dy = mBounds.y - (mIntuitionWindow->TopEdge + mIntuitionWindow->BorderTop);
			dx = mBounds.x - (mIntuitionWindow->LeftEdge + mIntuitionWindow->BorderLeft);
			MovePopups(dx, dy);

			mBounds.width = w;
			mBounds.height = h;
			mBounds.x = x;
			mBounds.y = y;

			LOG(("nsWindow[%d]::HandleAmigaIDCMP old: %d, %d - %dx%d\n",
					mWindowID, oldRect.x, oldRect.y, oldRect.width, oldRect.height));
			LOG(("nsWindow[%d]::HandleAmigaIDCMP mBounds: %d, %d - %dx%d\n",
					mWindowID, mBounds.x, mBounds.y, mBounds.width, mBounds.height));

			if (mBounds.x != oldRect.x || mBounds.x != oldRect.x
			 || mBounds.width != oldRect.width || mBounds.height != oldRect.height)
			{
				if (w != oldRect.width || h != oldRect.height)
				{
					mNeedsRedraw = PR_TRUE;
				}

				OnResizeEvent(mBounds);
			}

			if (mNeedsRedraw)
			{
				OnDraw(NULL);
				mNeedsRedraw = PR_FALSE;
			}
		}

		// Do the redraw if necessary, even for windows that ignore resize
		// This is necessary since program triggered resize just finished
		if (mNeedsRedraw)
		{
			OnDraw(NULL);
			mNeedsRedraw = PR_FALSE;
		}

		mIgnoreChangewnd = PR_FALSE;
		break;
	case IDCMP_MOUSEMOVE:
		{
			if (sInDragSession)
			{
				/* During drag and drop, we handle the dispatch a bit different:
				 * - As long as the mouse is over a window, use that.
				 * - When there is no window, but we can find another toplevel window, use the new toplevel
				 * - If there is no window, continue sending drag events to the last
				 *   drag window, unles ...
				 */
				nsWindow *w = FindWindowForPoint(mousePoint, imsg);

				if (!w)
				{
					/* Look for a new toplevel window: First, transform coordinate into screen
					 * coordinates, then walk through our window list, checking for any top level
					 * window that matches the coordinates
					 */

					uint32 sx = imsg->MouseX + imsg->IDCMPWindow->LeftEdge;
					uint32 sy = imsg->MouseY + imsg->IDCMPWindow->TopEdge;

					for (size_t i = 0; i < sTopWindowList.Length(); i++)
					{
						if (sTopWindowList[i]->mBounds.Contains(sx, sy))
						{
							uint32 rx = sx - sTopWindowList[i]->mIntuitionWindow->LeftEdge;
							uint32 ry = sy - sTopWindowList[i]->mIntuitionWindow->TopEdge;
							nsIntPoint mousePoint(rx - imsg->IDCMPWindow->BorderLeft, ry - imsg->IDCMPWindow->BorderTop);
							w = sTopWindowList[i]->FindWindowForPoint(mousePoint, NULL);
							break;
						}
					}
				}

				if (!w)
					w = sLastDragMotionWindow;

				if (w)
				{
					struct IntuiMessage imsg2 = *imsg;

					// To screen
					uint32 sx = imsg->MouseX + imsg->IDCMPWindow->LeftEdge;
					uint32 sy = imsg->MouseY + imsg->IDCMPWindow->TopEdge;

					// To new window
					imsg2.MouseX = sx - w->mIntuitionWindow->LeftEdge;
					imsg2.MouseY = sy - w->mIntuitionWindow->TopEdge;

					w->OnDragMotionEvent(&imsg2);
				}
			}
			else
			{
				nsWindow *w = FindWindowForPoint(mousePoint, imsg);
				if (w)
					w->OnMouseMotionEvent(imsg);
			}
		}
		break;
	case IDCMP_EXTENDEDMOUSE:
		{
			nsWindow *w = FindWindowForPoint(mousePoint, imsg);

			if (w && imsg->Code == IMSGCODE_INTUIWHEELDATA)
				w->OnMouseWheelEvent(imsg);
		}
		break;
	case IDCMP_ACTIVEWINDOW:
		{
			// Synthetic mouse button event for toolbox window
			nsWindow *w = (nsWindow *)imsg->IDCMPWindow->UserData;
			if (w && w->mWindowType == eWindowType_popup && imsg->Code == AWCODE_INTERIM)
				w->OnMouseButtonEvent(imsg);

			if (w)
				w->OnActivateEvent();
		}
		break;
	case IDCMP_INACTIVEWINDOW:
		{
			nsWindow *w = (nsWindow *)imsg->IDCMPWindow->UserData;

			w = (nsWindow *)imsg->IDCMPWindow->UserData;
			if (w)
				w->OnDeactivateEvent();
		}
		break;
	case IDCMP_RAWKEY:
		if (top->mFocus)
			top->mFocus->OnKeyEvent(imsg);
		break;
	}

	return ret;
}


void
nsWindow::OnMouseWheelEvent(struct IntuiMessage *imsg)
{
	if (imsg->Code != IMSGCODE_INTUIWHEELDATA)
		return;

	struct IntuiWheelData *d = (struct IntuiWheelData *)imsg->IAddress;

	if (d->WheelY)
	{
		nsMouseScrollEvent event(PR_TRUE, NS_MOUSE_SCROLL, this);
		InitEvent(event, NULL, imsg);

		event.delta = (PRInt32)d->WheelY;
		event.scrollFlags = nsMouseScrollEvent::kIsVertical;

		DispatchEvent(&event);
	}

	if (d->WheelX)
	{
		nsMouseScrollEvent event(PR_TRUE, NS_MOUSE_SCROLL, this);
		InitEvent(event, NULL, imsg);

		event.delta = (PRInt32)d->WheelX;
		event.scrollFlags = nsMouseScrollEvent::kIsHorizontal;

		DispatchEvent(&event);
	}
}


void
nsWindow::OnResizeEvent(const nsIntRect &aRect)
{
    int w = aRect.width;
    int h = aRect.height;
    int x = aRect.x;
    int y = aRect.y;

	LOGEVENT(("nsWindow[%d]::OnResizeEvent %d,%d - %dx%d\n", mWindowID, x, y, w, h));

    nsSizeEvent event(PR_TRUE, NS_SIZE, this);
    InitEvent(event);

    nsIntRect newRect = aRect;
    event.windowSize = &newRect;
    event.mWinWidth = w;
    event.mWinHeight = h;
    event.refPoint.x = x;
    event.refPoint.y = y;

    //mBounds.width = w;
    //mBounds.height = h;

    DispatchEvent(&event);

	/* XXX Invalidate the whole window. Need to check if the resize
	 * event already does this
	 */
	//Invalidate(mBounds, PR_TRUE);
}



void
nsWindow::OnDestroy(void)
{
	if (mOnDestroyCalled)
		return;

	LOGEVENT(("nsWindow[%d]::OnDestroy called\n", mWindowID));

	mOnDestroyCalled = PR_TRUE;

	// Reference to prevent deletion
	nsCOMPtr<nsIWidget> kungFuDeathGrup = this;

	nsBaseWidget::OnDestroy();
	nsBaseWidget::Destroy();
	mParent = nsnull;

	nsGUIEvent event(PR_TRUE, NS_DESTROY, this);
	nsEventStatus status;
	DispatchEvent(&event, status);
}


void
nsWindow::OnMouseButtonEvent(struct IntuiMessage *imsg)
{
	LOGEVENT(("nsWindow[%d]::OnMouseButtonEvent called\n", mWindowID));
	//nsEventStatus status;
	PRUint16 button;
	PRUint32 eventType;
	nsWindow *topw = FindTopLevel();

	switch (imsg->Code)
	{
	case SELECTDOWN:
		sLastButtonMask |= 1;
	case AWCODE_INTERIM:
		button = nsMouseEvent::eLeftButton;
		eventType = NS_MOUSE_BUTTON_DOWN;
		break;
	case MENUDOWN:
		sLastButtonMask |= 2;
		button = nsMouseEvent::eRightButton;
		eventType = NS_MOUSE_BUTTON_DOWN;
		break;
	case MIDDLEDOWN:
		sLastButtonMask |= 4;
		button = nsMouseEvent::eMiddleButton;
		eventType = NS_MOUSE_BUTTON_DOWN;
		break;
	case SELECTUP:
		sLastButtonMask &= ~1;
		button = nsMouseEvent::eLeftButton;
		eventType = NS_MOUSE_BUTTON_UP;
		break;
	case MENUUP:
		sLastButtonMask &= ~2;
		button = nsMouseEvent::eRightButton;
		eventType = NS_MOUSE_BUTTON_UP;
		break;
	case MIDDLEUP:
		sLastButtonMask &= ~4;
		button = nsMouseEvent::eMiddleButton;
		eventType = NS_MOUSE_BUTTON_UP;
		break;
	}

	nsMouseEvent event(PR_TRUE, eventType, this, nsMouseEvent::eReal);
	InitEvent(event, NULL, imsg);

	event.clickCount 	= 1;
	event.button 		= button;

	if (imsg->Code == AWCODE_INTERIM)
	{
		event.refPoint.x = topw->mLastX;
		event.refPoint.y = topw->mLastY;
	}
	else
	{
		topw->mLastX = event.refPoint.x;
		topw->mLastY = event.refPoint.y;
	}


	LOGEVENT(("nsWindow[%d]::OnMouseButtonEvent (%s,%s) at %d, %d, Qualifier = (%d, %d, %d, %d)\n", mWindowID,
			button == nsMouseEvent::eLeftButton ? "Left" : (button == nsMouseEvent::eRightButton ? "Right" : "Middle"),
			eventType == NS_MOUSE_BUTTON_DOWN ? "Down" : "Up",
			event.refPoint.x, event.refPoint.y,
			event.isAlt, event.isControl, event.isMeta, event.isShift));

	if (eventType == NS_MOUSE_BUTTON_DOWN && button == nsMouseEvent::eLeftButton)
	{
		PRBool rolledUp = check_for_rollup(imsg, PR_FALSE, PR_FALSE);
		if (gConsumeRollupEvent && rolledUp)
			return;

		if (IIntuition->DoubleClick((uint32)(sLastButtonDownTime >> 32), (uint32)(sLastButtonDownTime & 0xffffffffULL),
				imsg->Seconds, imsg->Micros))
		{
			// Potential double-click. Make sure the mouse did not move too much
			if (abs(imsg->MouseX-sLastButtonDownX) < 5 && abs(imsg->MouseY-sLastButtonDownY) < 5)
			{
				sLastButtonClickCount += 1;

				event.clickCount = sLastButtonClickCount + 1;
			}
			else
				sLastButtonClickCount = 0;
		}
		else
			sLastButtonClickCount = 0;

		sLastButtonDownTime = ((uint64)imsg->Seconds) << 32 | imsg->Micros;
		if (imsg->Code != AWCODE_INTERIM)
		{
			sLastButtonDownX = imsg->MouseX;
			sLastButtonDownY = imsg->MouseY;
		}
	}
	else
	{
		sLastButtonUpTime = ((uint64)imsg->Seconds) << 32 | imsg->Micros;
	}

	DispatchEvent(&event);

    // right menu click on AmigaOS should also pop up a context menu
    if (eventType == NS_MOUSE_BUTTON_DOWN && button == nsMouseEvent::eRightButton &&
        NS_LIKELY(!mIsDestroyed))
    {
        nsMouseEvent contextMenuEvent(PR_TRUE, NS_CONTEXTMENU, this,
                                      nsMouseEvent::eReal);
        InitEvent(contextMenuEvent, NULL, imsg);
        DispatchEvent(&contextMenuEvent);
    }
}


void
nsWindow::OnMouseMotionEvent(struct IntuiMessage *imsg)
{
	/* First of all, check whether we need to synthesize mouse enter/exit
	 * events.
	 */
	nsWindow *window = this;

//	while (window && window->mWindowType == eWindowType_popup)
//		window = window->mParent;

	if (!window)
		return;

	nsMouseEvent event(PR_TRUE, NS_MOUSE_MOVE, window, nsMouseEvent::eReal);
	window->InitEvent(event, NULL, imsg);

	nsWindow *topw = FindTopLevel();
	topw->mLastX = event.refPoint.x;
	topw->mLastY = event.refPoint.y;

	LOGEVENT(("nsWindow[%d]::OnMouseMotionEvent %d, %d\n", window->mWindowID, event.refPoint.x, event.refPoint.y));

	window->DispatchEvent(&event);
}

void
nsWindow::OnMouseEnterEvent(struct IntuiMessage *imsg)
{
	LOGEVENT(("nsWindow[%d]::OnMouseEnter\n", mWindowID));
	nsMouseEvent event(PR_TRUE, NS_MOUSE_ENTER, this, nsMouseEvent::eReal);
	InitEvent(event, NULL, imsg);

	DispatchEvent(&event);
}

void
nsWindow::OnMouseLeaveEvent(struct IntuiMessage *imsg)
{
	LOGEVENT(("nsWindow[%d]::OnMouseExit\n", mWindowID));
	nsMouseEvent event(PR_TRUE, NS_MOUSE_EXIT, this, nsMouseEvent::eReal);
	InitEvent(event, NULL, imsg);

	DispatchEvent(&event);
}


void
nsWindow::OnActivateEvent(void)
{
	LOGEVENT(("nsWindow[%d]::OnActivateEvent called\n", mWindowID));

	nsWindow *top = FindTopLevel();

	nsGUIEvent event(PR_TRUE, NS_ACTIVATE, top);
	nsEventStatus status;
	DispatchEvent(&event, status);
}

void
nsWindow::OnDeactivateEvent(void)
{
	LOGEVENT(("nsWindow[%d]::OnDeactivateEvent\n", mWindowID));

	nsWindow *top = FindTopLevel();

	nsGUIEvent event(PR_TRUE, NS_DEACTIVATE, top);
	nsEventStatus status;
	DispatchEvent(&event, status);
}


inline PRBool
is_context_menu_key(const nsKeyEvent& aKeyEvent)
{
    return
    	(
    		(aKeyEvent.keyCode == NS_VK_F10
    	  && aKeyEvent.isShift
    	  && !aKeyEvent.isControl
    	  && !aKeyEvent.isMeta
    	  && !aKeyEvent.isAlt
    	  )
    	  ||
            (aKeyEvent.keyCode == NS_VK_CONTEXT_MENU
          && !aKeyEvent.isShift
          && !aKeyEvent.isControl
          && !aKeyEvent.isMeta
          && !aKeyEvent.isAlt
          )
    	);
}


void
nsWindow::OnKeyEvent(struct IntuiMessage *imsg)
{
	LOGEVENT(("nsWindow[%d]::OnKeyEvent\n", mWindowID));

//    PRBool IMEWasEnabled = PR_FALSE;
//    if (mIMModule)
//    {
//    	IMEWasEnabled = mIMModule->IsEnabled();
//    	if (mIMModule->OnKeyEvent(this, aEvent))
//            return TRUE;
//    }

#define map(a,b) case a: event.keyCode = b; break

	PRUint32 msgType;

	if (imsg->Code & IECODE_UP_PREFIX)
		msgType = NS_KEY_UP;
	else
		msgType = NS_KEY_DOWN;

	nsKeyEvent event(PR_TRUE, msgType, this);
	InitEvent(event, NULL, imsg);

	switch(imsg->Code & ~IECODE_UP_PREFIX)
	{
     map(0x45, NS_VK_ESCAPE);
     map(0x50, NS_VK_F1);
     map(0x51, NS_VK_F2);
     map(0x52, NS_VK_F3);
     map(0x53, NS_VK_F4);
     map(0x54, NS_VK_F5);
     map(0x55, NS_VK_F6);
     map(0x56, NS_VK_F7);
     map(0x57, NS_VK_F8);
     map(0x58, NS_VK_F9);
     map(0x59, NS_VK_F10);
     map(0x4B, NS_VK_F11);
     map(0x6F, NS_VK_F12);
     map(0x00, NS_VK_BACK_QUOTE);
     map(0x01, NS_VK_1);
     map(0x02, NS_VK_2);
     map(0x03, NS_VK_3);
     map(0x04, NS_VK_4);
     map(0x05, NS_VK_5);
     map(0x06, NS_VK_6);
     map(0x07, NS_VK_7);
     map(0x08, NS_VK_8);
     map(0x09, NS_VK_9);
     map(0x0A, NS_VK_0);
     map(0x0B, NS_VK_SUBTRACT);
     map(0x0C, NS_VK_EQUALS);
     map(0x0D, NS_VK_BACK_SLASH);
     map(0x41, NS_VK_BACK);
     map(0x46, NS_VK_DELETE);
     map(0x5F, NS_VK_HELP);
     map(0x5A, 0);
     map(0x5B, 0);
     map(0x5C, NS_VK_DIVIDE);
     map(0x5D, NS_VK_MULTIPLY);
     map(0x42, NS_VK_TAB);
     map(0x10, NS_VK_Q);
     map(0x11, NS_VK_W);
     map(0x12, NS_VK_E);
     map(0x13, NS_VK_R);
     map(0x14, NS_VK_T);
     map(0x15, NS_VK_Y);
     map(0x16, NS_VK_U);
     map(0x17, NS_VK_I);
     map(0x18, NS_VK_O);
     map(0x19, NS_VK_P);
     map(0x1A, NS_VK_OPEN_BRACKET);
     map(0x1B, NS_VK_CLOSE_BRACKET);
     map(0x44, NS_VK_RETURN);
     map(0x3D, NS_VK_NUMPAD7);
     map(0x3E, NS_VK_NUMPAD8);
     map(0x3F, NS_VK_NUMPAD9);
     map(0x4A, NS_VK_SUBTRACT);
     map(0x63, NS_VK_CONTROL);
     map(0x62, NS_VK_CAPS_LOCK);
     map(0x20, NS_VK_A);
     map(0x21, NS_VK_S);
     map(0x22, NS_VK_D);
     map(0x23, NS_VK_F);
     map(0x24, NS_VK_G);
     map(0x25, NS_VK_H);
     map(0x26, NS_VK_J);
     map(0x27, NS_VK_K);
     map(0x28, NS_VK_L);
     map(0x29, NS_VK_SEMICOLON);
     map(0x2A, NS_VK_QUOTE);
     map(0x2B, 0); //NS_VK_HASH); // German keyboard
     map(0x4C, NS_VK_UP);
     map(0x2D, NS_VK_NUMPAD4);
     map(0x2E, NS_VK_NUMPAD5);
     map(0x2F, NS_VK_NUMPAD6);
     map(0x5E, NS_VK_ADD);
     map(0x60, NS_VK_SHIFT);
     map(0x30, 0); //NS_VK_LESS); // German keyboard
     map(0x31, NS_VK_Z);
     map(0x32, NS_VK_X);
     map(0x33, NS_VK_C);
     map(0x34, NS_VK_V);
     map(0x35, NS_VK_B);
     map(0x36, NS_VK_N);
     map(0x37, NS_VK_M);
     map(0x38, NS_VK_COMMA);
     map(0x39, NS_VK_PERIOD);
     map(0x3A, NS_VK_SLASH);
     map(0x61, NS_VK_SHIFT);
     map(0x4F, NS_VK_LEFT);
     map(0x4D, NS_VK_DOWN);
     map(0x4E, NS_VK_RIGHT);
     map(0x1D, NS_VK_NUMPAD1);
     map(0x1E, NS_VK_NUMPAD2);
     map(0x1F, NS_VK_NUMPAD3);
     map(0x43, NS_VK_ENTER);
     map(0x64, NS_VK_ALT);
     map(0x66, NS_VK_META);
     map(0x40, NS_VK_SPACE);
     map(0x67, NS_VK_CONTROL);
     map(0x65, NS_VK_ALT);
     map(0x0f, NS_VK_NUMPAD0);
     map(0x3C, NS_VK_SEPARATOR);
     // Only on PS/2 or USB PC keyboards
     map(0x6D, NS_VK_PRINTSCREEN);
     map(0x6E, NS_VK_PAUSE);
     map(0x47, NS_VK_INSERT);
     map(0x70, NS_VK_HOME);
     map(0x48, NS_VK_PAGE_UP);
     map(0x71, NS_VK_END);
     map(0x49, NS_VK_PAGE_DOWN);
     map(0x6B, NS_VK_CONTEXT_MENU);
	 default:
		 event.keyCode = 0;
	 }

	event.charCode = nsAmigaTranslateUnicode(imsg->Code, imsg->Qualifier);
	event.isChar = event.charCode >= 32 ? PR_TRUE : PR_FALSE;

	if (is_context_menu_key(event))
	{
		LOGEVENT(("nsWindow[%d]::OnKeyEvent context menu key\n", mWindowID));

		nsMouseEvent contextMenuEvent(PR_TRUE, NS_CONTEXTMENU, this,
							nsMouseEvent::eReal, nsMouseEvent::eContextMenuKey);
		contextMenuEvent.refPoint = nsIntPoint(0,0);
		contextMenuEvent.isShift = PR_FALSE;
		contextMenuEvent.isControl = PR_FALSE;
		contextMenuEvent.isMeta = PR_FALSE;
		contextMenuEvent.isAlt = PR_FALSE;
		contextMenuEvent.clickCount = 1;
		contextMenuEvent.time = event.time;

		DispatchEvent(&contextMenuEvent);
	}
	else
	{
		if (!(imsg->Code & IECODE_UP_PREFIX))
		{
			LOGEVENT(("nsWindow[%d]::OnKeyEvent key %d maps to '%c' (Qualifiers %d, %d, %d, %d)\n", mWindowID,
				event.keyCode, event.isChar ? event.charCode : ' ',
				event.isAlt, event.isControl, event.isMeta, event.isShift));
		}

		nsKeyEvent backup = event;
		DispatchEvent(&event);

		if (!(imsg->Code & IECODE_UP_PREFIX))
		{
			nsKeyEvent event(PR_TRUE, NS_KEY_PRESS, this);
			InitEvent(event, NULL, imsg);
			event.keyCode = backup.keyCode;
			event.charCode = backup.charCode;
			event.isChar = backup.isChar;
			if (!(event.keyCode == NS_VK_SHIFT || event.keyCode == NS_VK_ALT
			 || event.keyCode == NS_VK_CONTROL || event.keyCode == NS_VK_META))
				DispatchEvent(&event);
		}
	}
}


void
nsWindow::InitEvent(nsKeyEvent& event,
					nsIntPoint* aPoint,
					struct IntuiMessage *imsg)
{
	InitEvent((nsGUIEvent&)event, aPoint, imsg);

	if (imsg)
	{
		event.isShift  	= imsg->Qualifier & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT)
				? PR_TRUE : PR_FALSE;
		event.isControl	= imsg->Qualifier & (IEQUALIFIER_CONTROL | IEQUALIFIER_RCOMMAND)
				? PR_TRUE : PR_FALSE;
		event.isAlt     = imsg->Qualifier & (IEQUALIFIER_LALT /*| IEQUALIFIER_RALT*/)
				? PR_TRUE : PR_FALSE;
		event.isMeta    = imsg->Qualifier & IEQUALIFIER_LCOMMAND
				? PR_TRUE : PR_FALSE;
	}
}

void
nsWindow::InitEvent(nsMouseScrollEvent& event,
					nsIntPoint* aPoint,
					struct IntuiMessage *imsg)
{
	InitEvent((nsGUIEvent&)event, aPoint, imsg);

	if (imsg)
	{
		event.isShift  	= imsg->Qualifier & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT)
				? PR_TRUE : PR_FALSE;
		event.isControl	= imsg->Qualifier & (IEQUALIFIER_CONTROL | IEQUALIFIER_RCOMMAND)
				? PR_TRUE : PR_FALSE;
		event.isAlt     = imsg->Qualifier & (IEQUALIFIER_LALT | IEQUALIFIER_RALT)
				? PR_TRUE : PR_FALSE;
		event.isMeta    = imsg->Qualifier & IEQUALIFIER_LCOMMAND
				? PR_TRUE : PR_FALSE;
	}
}

void
nsWindow::InitEvent(nsMouseEvent& event,
					nsIntPoint* aPoint,
					struct IntuiMessage *imsg)
{
	InitEvent((nsGUIEvent&)event, aPoint, imsg);

	if (imsg)
	{
		event.isShift  	= imsg->Qualifier & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT)
				? PR_TRUE : PR_FALSE;
		event.isControl	= imsg->Qualifier & (IEQUALIFIER_CONTROL | IEQUALIFIER_RCOMMAND)
				? PR_TRUE : PR_FALSE;
		event.isAlt     = imsg->Qualifier & (IEQUALIFIER_LALT | IEQUALIFIER_RALT)
				? PR_TRUE : PR_FALSE;
		event.isMeta    = imsg->Qualifier & IEQUALIFIER_LCOMMAND
				? PR_TRUE : PR_FALSE;
	}
}

void
nsWindow::InitEvent(nsGUIEvent& event,
					nsIntPoint* aPoint,
					struct IntuiMessage *imsg)
{
	if (nsnull == aPoint)
	{
		// use the point from the event
		if (imsg)
		{
			nsWindow *top = FindTopLevel();
			if (!top)
				top = this;

			//XXX Is this translation correct ?
//			if (0) //mWindowType == eWindowType_popup)
//			{
//				printf("popup (popup ID %d)-- m: %d,%d     b: %d,%d    e: %d,%d   mb: %d,%d\n", mWindowID,
//						imsg->MouseX, imsg->MouseY, mIntuitionWindow->BorderLeft, mIntuitionWindow->BorderTop,
//						mIntuitionWindow->LeftEdge, mIntuitionWindow->TopEdge, mBounds.x, mBounds.y);
//
////				event.refPoint.x = imsg->MouseX - mBounds.x;
////				event.refPoint.y = imsg->MouseY - mBounds.y;
//
////				event.refPoint.x = imsg->MouseX - top->mIntuitionWindow->BorderLeft	- mBounds.x - top->mIntuitionWindow->LeftEdge;
////				event.refPoint.y = imsg->MouseY - top->mIntuitionWindow->BorderTop - mBounds.y - top->mIntuitionWindow->TopEdge;
//
////				printf("screen: %d : %d\n", (imsg->MouseX + top->mIntuitionWindow->LeftEdge), (imsg->MouseY + top->mIntuitionWindow->TopEdge));
//				event.refPoint.x = 10;
//				event.refPoint.y = 10;
//			}
//			else
			{
//				printf("other (top ID %d) -- m: %d,%d     b: %d,%d    e: %d,%d   mb: %d,%d\n", top->mWindowID,
//						imsg->MouseX, imsg->MouseY, top->mIntuitionWindow->BorderLeft, top->mIntuitionWindow->BorderTop,
//						top->mIntuitionWindow->LeftEdge, top->mIntuitionWindow->TopEdge, mBounds.x, mBounds.y);
				event.refPoint.x = imsg->MouseX - top->mIntuitionWindow->BorderLeft
						- mBounds.x; // top->mIntuitionWindow->LeftEdge; //mBounds.x;
				event.refPoint.y = imsg->MouseY - top->mIntuitionWindow->BorderTop
						- mBounds.y; // top->mIntuitionWindow->TopEdge; //mBounds.y;
			}
		}
		else
		{
			event.refPoint.x = 0;
			event.refPoint.y = 0;
		}
	}
	else
	{   // use the point override if provided
		event.refPoint.x = aPoint->x;
		event.refPoint.y = aPoint->y;
	}

	event.time = PR_IntervalNow();
}



nsWindow*
nsWindow::FindWindowForPoint(const nsIntPoint& aPoint, struct IntuiMessage *imsg)
{
	nsWindow *win;
	nsIntPoint pt = aPoint;

	if (imsg)
		win = reinterpret_cast<nsWindow *>(imsg->IDCMPWindow->UserData);
	else
		win = this;


//	printf("Checking children of %d for (%d, %d)\n", win->mWindowID, pt.x, pt.y);

	// Special case: Popups of one of our children might still hit if they extend
	// beyond the child window's boundaries.
	for (int i = mPopupList.Length() - 1; i >= 0; i--)
	{
		nsWindow *w = mPopupList[i];

		nsIntRect realBounds = w->mBounds;
		nsWindow *top = w->FindTopLevel();
		if (top)
		{
			realBounds.x -= top->mIntuitionWindow->LeftEdge + top->mIntuitionWindow->BorderLeft;
			realBounds.y -= top->mIntuitionWindow->TopEdge + top->mIntuitionWindow->BorderTop;
		}

//		printf("Checking popup %d: %d,%d - %dx%d (relative %d, %d)\n",
//				w->mWindowID, w->mBounds.x, w->mBounds.y, w->mBounds.width, w->mBounds.height,
//				realBounds.x, realBounds.y);

		if (realBounds.Contains(pt) && w->mIsVisible)
			return w->FindWindowForPoint(pt, NULL);
	}


	nsIWidget *f = win->GetFirstChild();
	while (f)
	{
		nsWindow *w = static_cast<nsWindow*>(f);

		nsIntRect realBounds = w->mBounds;
		if (w->mWindowType == eWindowType_popup)
		{
			nsWindow *top = w->FindTopLevel();
			if (top)
			{
				realBounds.x -= top->mIntuitionWindow->LeftEdge + top->mIntuitionWindow->BorderLeft;
				realBounds.y -= top->mIntuitionWindow->TopEdge + top->mIntuitionWindow->BorderTop;
			}
		}

//		printf("Checking %d: %d,%d - %dx%d (relative %d, %d)\n",
//				w->mWindowID, w->mBounds.x, w->mBounds.y, w->mBounds.width, w->mBounds.height,
//				realBounds.x, realBounds.y);

		if (realBounds.Contains(pt) && w->mIsVisible)
			return w->FindWindowForPoint(pt, NULL);

		f = w->GetNextSibling();
	}

	nsIntRect realBounds = mBounds;
	if (mWindowType == eWindowType_popup)
	{
		nsWindow *top = FindTopLevel();
		if (top)
		{
			realBounds.x -= top->mIntuitionWindow->LeftEdge + top->mIntuitionWindow->BorderLeft;
			realBounds.y -= top->mIntuitionWindow->TopEdge + top->mIntuitionWindow->BorderTop;
		}
	}

	if (realBounds.Contains(pt))
	{
//		printf("Hit at %d\n", mWindowID);
		return this;
	}

	return nsnull;
}

void
nsWindow::NativeWindowResize(PRInt32 aX,
		                	 PRInt32 aY,
		                	 PRInt32 aWidth,
		                	 PRInt32 aHeight,
		                	 PRBool aRepaint)
{
	/* Account for window decoration. We place the desired origin of the
	 * drawable area to where aX, aY points.
	 */
	LOG(("nsWindow[%d]::NativeWindowResize mNeedsShow = %s\n", mWindowID, mNeedsShow ? "true" : "false"));
	int32 newX = aX;// - mIntuitionWindow->BorderLeft;
	int32 newY = aY;// - mIntuitionWindow->BorderTop;
	int32 newW = aWidth + mIntuitionWindow->BorderLeft + mIntuitionWindow->BorderRight;
	int32 newH = aHeight + mIntuitionWindow->BorderTop + mIntuitionWindow->BorderBottom;

//	if (newX < 0)
//		newX = 0;
//	if (newY < 0)
//		newY = 0;

//	if (mWindowType == eWindowType_popup)
//	{
//		/* Move the popup mBounds relative to it's parent window */
//		nsWindow *root = FindTopLevel();
//		if (root)
//		{
//			newX += root->mBounds.x;
//			newY += root->mBounds.y;
//		}
//	}

	mIgnoreChangewnd = PR_TRUE;

	IIntuition->ChangeWindowBox(mIntuitionWindow, newX, newY, newW, newH);

	if (mNeedsShow)
	{
		mNeedsShow = PR_FALSE;
		uint32 state = FALSE;
		IIntuition->SetWindowAttr(mIntuitionWindow, WA_Hidden,
				(void *)state, sizeof(state));
		IIntuition->ActivateWindow(mIntuitionWindow);
	}

	if (aRepaint)
		mNeedsRedraw = PR_TRUE;
}



void
nsWindow::NativeWindowMakeFullscreen(PRBool aState)
{
	if (aState)
	{
		// Make fullscreen
		struct Screen *scr = IIntuition->LockPubScreen(NULL);

		mPreFSBounds = mBounds;

		LONG w, h;

		IIntuition->GetScreenAttrs(scr,
				SA_Width,			&w,
				SA_Height,			&h,
			TAG_DONE);

		IIntuition->UnlockPubScreen(NULL, scr);

		mBounds.x = 0;
		mBounds.y = 0;
		mBounds.width = w;
		mBounds.height = h;
	}
	else
		mBounds = mPreFSBounds;

	Resize(mBounds.x, mBounds.y, mBounds.width, mBounds.height, PR_TRUE);

//	if (aState)
	{
		mIgnoreChangewnd = PR_TRUE;
		IIntuition->MoveWindow(mIntuitionWindow, - mIntuitionWindow->BorderLeft,  - mIntuitionWindow->BorderTop);
	}
}



void
nsWindow::NativeWindowMaximize(PRBool aState)
{
	//XXX Implement me
}



void
nsWindow::NativeWindowIconify(PRBool aState)
{
	//XXX Implement me
}

void
nsWindow::OnDraw(nsIntRect *aRect)
{
	LOGDRAW(("nsWindow[%d]::OnDraw called\n", mWindowID));

	// If not a toplevel window, send it to the toplevel now
	if (!mIsNativeWindow)
	{
		uint32 x = 0,
			   y = 0;

		nsWindow *w = this;
		while (w && !w->mIsNativeWindow)
		{
			x += w->mBounds.x;
			y += w->mBounds.y;

			w = w->mParent;
		}

		if (!aRect)
			aRect = &mBounds;

		nsIntRect newRect(*aRect);

		newRect.MoveBy(x,y);

		LOGDRAW(("nsWindow[%d]::OnDraw sending to toplevel\n", mWindowID));
		nsWindow *root = FindTopLevel();
		root->OnDraw(&newRect);
		return;
	}

	// ignore invisible windows
//	if (!mIsVisible)
//	{
//		LOGDRAW(("nsWindow[%d]::OnDraw ignored (invisible)\n", mWindowID));
//		return;
//	}

	nsIntRect fullRect(0, 0, mBounds.width, mBounds.height);

	if (!aRect)
		aRect = &fullRect;

	if (mWindowType == eWindowType_popup)
	{
		LOGDRAW(("nsWindow[%d]::OnDraw popup window\n", mWindowID));
		aRect = &fullRect;
	}

    if (GetLayerManager(nsnull)->GetBackendType() == LayerManager::LAYERS_BASIC)
    {

    	//XXX Use the thebes surface ?
    	nsRefPtr<gfxImageSurface> targetSurface =
    			new gfxImageSurface(gfxIntSize(aRect->width, aRect->height),
    					gfxASurface::ImageFormatRGB24);

#ifdef DEBUG_TIMING
    	LOGDRAW(("nsWindow[%d]::OnDraw begin DrawTo\n", mWindowID));
    	PRUint32 nowTime = PR_IntervalNow();
#endif
    	PRBool result = DrawTo(targetSurface, aRect);

#ifdef DEBUG_TIMING
    	PRUint32 endTime = PR_IntervalToMilliseconds(PR_IntervalNow() - nowTime);
    	LOGDRAW(("nsWindow[%d]::OnDraw end DrawTo (%d ms, for a %d,%d x %d,%d rectangle)\n",
    			mWindowID, endTime,
    			aRect->x, aRect->y, aRect->width, aRect->height
    			));
#endif

    	if (result)
    	{
    		// Blit into window
    		uint32 bltFmt = BLITT_ARGB32;
    		uint32 dstX = aRect->x;
    		uint32 dstY = aRect->y;

    		LOGDRAW(("nsWindow[%d]::OnDraw src rect %d, %d - %dx%d\n", mWindowID,
    				aRect->x, aRect->y, aRect->width, aRect->height));

#ifdef DEBUG_TIMING
    		PRUint32 nowTime = PR_IntervalNow();
#endif
    		IGraphics->BltBitMapTags(
    				BLITA_SrcX,			0, //aRect->x,
    				BLITA_SrcY,			0, //aRect->y,
    				BLITA_Width,		aRect->width,
    				BLITA_Height,		aRect->height,
    				BLITA_SrcType,		bltFmt,
    				BLITA_Source,		targetSurface->Data(),
    				BLITA_SrcBytesPerRow,targetSurface->Stride(),

    				BLITA_DestX,		dstX + mIntuitionWindow->BorderLeft,
    				BLITA_DestY,		dstY + mIntuitionWindow->BorderTop,
    				BLITA_DestType,		BLITT_RASTPORT,
    				BLITA_Dest,			mIntuitionWindow->RPort,
    			TAG_DONE);
#ifdef DEBUG_TIMING
    		PRUint32 endTime = PR_IntervalToMilliseconds(PR_IntervalNow() - nowTime);
    		LOGDRAW(("nsWindow[%d]::OnDraw blitting: %d ms\n",
    			mWindowID, endTime));
#endif
#ifdef DEBUG_FLASH
    		FlashRect(*aRect);
#endif
    		if(eTransparencyTransparent == GetTransparencyMode())
    		{
    			UpdateTransparencyBitmap(dstX + mIntuitionWindow->BorderLeft,
    					dstY + mIntuitionWindow->BorderTop, aRect->width, aRect->height,
    					targetSurface->Data(), targetSurface->Stride());
    			ApplyTransparencyBitmap();
    		}

    	}
    }
    else
    {
    	LOGDRAW(("nsWindow[%d]::OnDraw no layer manager\n", mWindowID));
    }
}


PRBool
nsWindow::DrawTo(gfxASurface *targetSurface, nsIntRect *aRect)
{
	PRBool ok = PR_TRUE;

	if (!mIsVisible)
	{
//		LOGDRAW(("nsWindow[%d]::DrawTo (invisible)\n", mWindowID));
		return PR_FALSE;
	}

//	LOGDRAW(("nsWindow[%d]::DrawTo (%d,%d - %dx%d)\n", mWindowID,
//			aRect->x, aRect->y, aRect->width, aRect->height));

	nsIntRect boundsRect(*aRect);
	nsWindow *coveringChild = NULL;
	nsEventStatus status;

	gfxPoint offset;
	if (targetSurface)
		offset = targetSurface->GetDeviceOffset();

	// Check if a child window completely covers this
	for (nsIWidget *kid = mFirstChild; kid; kid = kid->GetNextSibling())
	{
		nsWindow *w = static_cast<nsWindow *>(kid);

		if (w->mBounds.IsEmpty())
			continue;

		if (w->mBounds.Contains(boundsRect))
			coveringChild = w;
	}

	// If we're not fully covered by a child window, we need to render
	// this one
	if (!coveringChild)
	{
//		LOGDRAW(("nsWindow[%d]::DrawTo no child fully covers us\n", mWindowID));

		nsPaintEvent event(PR_TRUE, NS_PAINT, this);
		event.region = boundsRect;
		event.refPoint.x = aRect->x;
		event.refPoint.y = aRect->y;

		switch (GetLayerManager(nsnull)->GetBackendType())
		{
			case LayerManager::LAYERS_BASIC:
			{

				nsRefPtr<gfxContext> ctx = new gfxContext(targetSurface);

				AutoLayerManagerSetup
					setupLayerManager(this, ctx, BasicLayerManager::BUFFER_NONE);

#ifdef DEBUG_TIMING
    	    	PRUint32 nowTime = PR_IntervalNow();
    	    	LOGDRAW(("nsWindow[%d]::DrawTo DispatchEvent starting\n", mWindowID));
#endif
				status = DispatchEvent(&event);
#ifdef DEBUG_TIMING
				PRUint32 endTime = PR_IntervalToMilliseconds(PR_IntervalNow() - nowTime);
				LOGDRAW(("nsWindow[%d]::DrawTo DispatchEvent(%d ms)\n", mWindowID, endTime));
#endif
				break;
			}

			default:
				NS_ERROR("Invalid layer manager");
		}
	}

	// Paint child windows
	for (nsIWidget *kid = mFirstChild; kid; kid = kid->GetNextSibling())
	{
		nsWindow *w = static_cast<nsWindow *>(kid);
		if (w->mWindowType != eWindowType_child
		 ||	w->mBounds.IsEmpty()
		 ||	!w->mBounds.Intersects(boundsRect))
			continue;

		if (targetSurface)
			targetSurface->SetDeviceOffset(offset
					- gfxPoint(aRect->x, aRect->y));

//		LOGDRAW(("nsWindow[%d]::DrawTo drawing child at %d, %d\n", mWindowID,
//				mBounds.x, mBounds.y));

		ok = w->DrawTo(targetSurface, aRect);

		if (!ok)
			break;
	}

	if (targetSurface)
		targetSurface->SetDeviceOffset(offset);

	return ok;
}



void
nsWindow::FlashWindow(void)
{
	uint32 x = 0,
		   y = 0;
	nsWindow *w = this;
	while (w && !w->mIsNativeWindow)
	{
		x += w->mBounds.x;
		y += w->mBounds.y;

		w = w->mParent;
	}

	/* Account for the window decoration of the intuition window */
	x += mIntuitionWindow->BorderLeft;
	y += mIntuitionWindow->BorderTop;

	for (int i = 0; i < 8; i++)
	{
		IDOS->Delay(1);
		IGraphics->SetDrMd(mIntuitionWindow->RPort, COMPLEMENT);
		IGraphics->RectFill(mIntuitionWindow->RPort, x, y, x + mBounds.width - 1, y + mBounds.height - 1);
	}
}


void
nsWindow::FlashRect(nsIntRect &aRect)
{
//	nsIntPoint pt = WidgetToWindowOffset();
	nsIntPoint pt(mIntuitionWindow->BorderLeft, mIntuitionWindow->BorderTop);

	for (int i = 0; i < 4; i++)
	{
		IDOS->Delay(5);
		IGraphics->SetDrMd(mIntuitionWindow->RPort, COMPLEMENT);
		IGraphics->RectFill(mIntuitionWindow->RPort,
				pt.x + aRect.x, pt.y + aRect.y,
				pt.x + aRect.width - 1, pt.y + aRect.height - 1);
	}
}

nsIntPoint
nsWindow::WidgetToWindowOffset()
{
	uint32 x = 0,
		   y = 0;
	nsWindow *w = this;
	while (w && !w->mIsTopLevel)
	{
		x += w->mBounds.x;
		y += w->mBounds.y;

		w = w->mParent;
	}

	/*XXX: Actually, since the top level includes the window position in it
	 * it's mBounds, we should be OK by simply NOT using the top level
	 * bounds (thus effectively assuming they are 0,0);
	 */
	/* Account for the window decoration of the intuition window */
	x += mIntuitionWindow->BorderLeft;
	y += mIntuitionWindow->BorderTop;

	return nsIntPoint(x, y);
}

void
nsWindow::DrawDamagedRegions(void)
{
	nsIntRect rect = mDamageRects.GetBounds();

	while (!rect.IsEmpty())
	{
		nsIntRegion copyRect = mDamageRects;

		mDamageRects.SetEmpty();

		nsIntRegionRectIterator it(copyRect);
		while (const nsIntRect* sr = it.Next())
		{
			OnDraw(const_cast<nsIntRect *>(sr));
		}

		rect = mDamageRects.GetBounds();
	}
}


static uint32 nsAmigaTranslateUnicode(uint16 Code, uint32 Qualifier)
{
	struct InputEvent ie;
    uint16 res;
    char buffer[10];

    ie.ie_Class 		= IECLASS_RAWKEY;
    ie.ie_SubClass 		= 0;
    ie.ie_Code  		= Code & ~(IECODE_UP_PREFIX);
    ie.ie_Qualifier 	= Qualifier & (IEQUALIFIER_RALT|IEQUALIFIER_LSHIFT|IEQUALIFIER_RSHIFT);
    ie.ie_EventAddress 	= NULL;

    res = IKeymap->MapRawKey(&ie, buffer, 10, 0);

    if (res != 1)
    	return 0;
    else
    	return buffer[0];
}

void nsWindow::MovePopups(int32 dx, int32 dy)
{
//	LOG(("nsWindow[%d]::MovePopups %d popups to move (by %d, %d)\n", mWindowID, mPopupList.Length(), dx, dy));
//
//	for (int i = 0; i < mPopupList.Length(); i++)
//	{
//		nsWindow *w = mPopupList[i];
//		while (w && !w->mIsTopLevel)
//			w = w->mParent;
//
//		nsWindow *topw = w->FindTopLevel();
//
//		IIntuition->MoveWindow(w->mIntuitionWindow, -dx, -dy);
//		IIntuition->MoveWindowInFrontOf(w->mIntuitionWindow, topw->mIntuitionWindow);

//		IIntuition->ChangeWindowBox(w->mIntuitionWindow,
//				w->mIntuitionWindow->LeftEdge - dx,
//				w->mIntuitionWindow->TopEdge - dy,
//				w->mIntuitionWindow->Width,
//				w->mIntuitionWindow->Height);
//	}
}


PRBool
check_for_rollup(struct IntuiMessage *msg, PRBool aIsWheel, PRBool aAlwaysRollup)
{
	LOG(("check_for_rollup"));

	PRBool retVal = PR_FALSE;
    nsCOMPtr<nsIWidget> rollupWidget = do_QueryReferent(gRollupWindow);

    if (rollupWidget && gRollupListener) {
        Window *currentPopup =
            (Window *)rollupWidget->GetNativeData(NS_NATIVE_WINDOW);
        if (aAlwaysRollup || !(msg->IDCMPWindow == currentPopup)) {
            PRBool rollup = PR_TRUE;
            if (aIsWheel) {
                gRollupListener->ShouldRollupOnMouseWheelEvent(&rollup);
                retVal = PR_TRUE;
            }
            // if we're dealing with menus, we probably have submenus and
            // we don't want to rollup if the click is in a parent menu of
            // the current submenu
            PRUint32 popupsToRollup = PR_UINT32_MAX;
            if (gMenuRollup && !aAlwaysRollup) {
                nsAutoTArray<nsIWidget*, 5> widgetChain;
                PRUint32 sameTypeCount = gMenuRollup->GetSubmenuWidgetChain(&widgetChain);
                for (PRUint32 i=0; i<widgetChain.Length(); ++i) {
                    nsIWidget* widget = widgetChain[i];
                    Window* currWindow =
                        (Window*) widget->GetNativeData(NS_NATIVE_WINDOW);
                    if (msg->IDCMPWindow == currWindow) {
                      // don't roll up if the mouse event occurred within a
                      // menu of the same type. If the mouse event occurred
                      // in a menu higher than that, roll up, but pass the
                      // number of popups to Rollup so that only those of the
                      // same type close up.
                      if (i < sameTypeCount) {
                        rollup = PR_FALSE;
                      }
                      else {
                        popupsToRollup = sameTypeCount;
                      }
                      break;
                    }
                } // foreach parent menu widget
            } // if rollup listener knows about menus

            // if we've determined that we should still rollup, do it.
            if (rollup) {
            	LOG(("check_for_rollup: Rolling up\n"));
                gRollupListener->Rollup(popupsToRollup, nsnull);
                if (popupsToRollup == PR_UINT32_MAX) {
                    retVal = PR_TRUE;
                }
            }
        }
    } else {
        gRollupWindow = nsnull;
        gRollupListener = nsnull;
        NS_IF_RELEASE(gMenuRollup);
    }

    return retVal;
}



void
nsWindow::OnDragMotionEvent(struct IntuiMessage *imsg, void *aData)
{
	LOGEVENT(("nsWindow[%d]::OnDragMotionEvent \n", mWindowID));

    nsCOMPtr<nsIDragService> dragService = do_GetService(kCDragServiceCID);

    LOGEVENT(("nsWindow[%d]::OnDragMotionEvent dragService = %p\n", mWindowID, (void *)dragService));

	CheckNeedDragLeaveEnter(imsg, this, dragService);

	dragService->FireDragEventAtSource(NS_DRAGDROP_DRAG);

	nsDragEvent event(PR_TRUE, NS_DRAGDROP_OVER, this);
	InitEvent(event, NULL, imsg);
	DispatchEvent(&event);

	((nsDragService *)dragService.get())->UpdateDragPosition(mIntuitionWindow, imsg);
}



void
nsWindow::OnDragEnter(struct IntuiMessage *imsg)
{
	LOGEVENT(("nsWindow[%d]::OnDragEnter\n", mWindowID));

	nsDragEvent event(PR_TRUE, NS_DRAGDROP_ENTER, this);
	InitEvent(event, NULL, imsg);
	DispatchEvent(&event);
}



void
nsWindow::OnDragLeave(struct IntuiMessage *imsg)
{
	LOGEVENT(("nsWindow[%d]::OnDragLeave\n", mWindowID));

	nsDragEvent event(PR_TRUE, NS_DRAGDROP_EXIT, this);
	InitEvent(event, NULL, imsg);
	DispatchEvent(&event);

	nsCOMPtr<nsIDragService> dragService = do_GetService(kCDragServiceCID);

	if (dragService)
	{
		nsCOMPtr<nsIDragSession> currentDragSession;
		dragService->GetCurrentSession(getter_AddRefs(currentDragSession));

		if (currentDragSession)
		{
			nsCOMPtr<nsIDOMNode> sourceNode;
			currentDragSession->GetSourceNode(getter_AddRefs(sourceNode));

			if (!sourceNode)
			{
				// We're leaving a window while doing a drag that was
				// initiated in a different app. End the drag session,
				// since we're done with it for now (until the user
				// drags back into mozilla).
				dragService->EndDragSession(PR_FALSE);
			}
		}
	}
}



void
nsWindow::OnDragDropEvent(struct IntuiMessage *imsg, void *aData)
{
	LOGEVENT(("nsWindow[%d]::OnDragDropEvent \n", mWindowID));

	/* This is a synthetic event. This means that the imsg's data does not
	 * necessarily correspond to this window.
	 * We transform the coordinates to screen coordinates first using imsg, and
	 * then "fake" an appropriate imsg.
	 */
	struct IntuiMessage imsg2 = *imsg;

	// To screen
	uint32 sx = imsg->MouseX + imsg->IDCMPWindow->LeftEdge;
	uint32 sy = imsg->MouseY + imsg->IDCMPWindow->TopEdge;

	// To new window
	imsg2.MouseX = sx - mIntuitionWindow->LeftEdge;
	imsg2.MouseY = sy - mIntuitionWindow->TopEdge;
	imsg2.IDCMPWindow = mIntuitionWindow;

	nsCOMPtr<nsIDragService> dragService = do_GetService(kCDragServiceCID);

	CheckNeedDragLeaveEnter(&imsg2, this, dragService);

	nsDragEvent event(PR_TRUE, NS_DRAGDROP_OVER, this);
	InitEvent(event, NULL, &imsg2);
	DispatchEvent(&event);

	if (!mIsDestroyed)
	{
		nsDragEvent event(PR_TRUE, NS_DRAGDROP_DROP, this);
		InitEvent(event, NULL, &imsg2);
		DispatchEvent(&event);
	}

	((nsDragService *)dragService.get())->SetDragEndPoint(event.refPoint);

	dragService->EndDragSession(PR_TRUE);

	sLastDragMotionWindow = 0;
}



void
nsWindow::CheckNeedDragLeaveEnter(struct IntuiMessage *imsg, nsWindow *aCurrent, nsIDragService *aDragService)
{
	LOGEVENT(("nsWindow[%d]::CheckNeedDragLeaveEnter aDragService = %p, sLastDragMotionWindow = %p\n", mWindowID, (void *)aDragService, sLastDragMotionWindow));

	if (sLastDragMotionWindow)
	{
		if (sLastDragMotionWindow == aCurrent)
		{
			UpdateDragStatus(nsIDragService::DRAGDROP_ACTION_MOVE, aDragService);
			return;
		}

		// Send drag leave event
		nsRefPtr<nsWindow> kungFuDeathGrip = sLastDragMotionWindow;
		sLastDragMotionWindow->OnDragLeave(imsg);

	}

	aDragService->StartDragSession();

	// Update status and generate enter event
	UpdateDragStatus(nsIDragService::DRAGDROP_ACTION_MOVE, aDragService);
	aCurrent->OnDragEnter(imsg);

	sLastDragMotionWindow = this;
}



void
nsWindow::UpdateDragStatus(int action, nsIDragService *aDragService)
{
	nsCOMPtr<nsIDragSession> session;
	aDragService->GetCurrentSession(getter_AddRefs(session));

	if (session)
		session->SetDragAction(action);
}


void
nsWindow::SetTransparencyMode(nsTransparencyMode aMode)
{
	if (aMode == eTransparencyTransparent)
		LOG(("nsWindow[%d]::SetTransparencyMode %s\n", mWindowID, aMode == eTransparencyTransparent ? "eTransparencyTransparent" :  "eTransparencyOpaque"));

	if (!mIsNativeWindow)
	{
		nsWindow *root = FindNative();
		if (root)
			root->SetTransparencyMode(aMode);

		return;
	}

	PRBool isTransparent = (aMode == eTransparencyTransparent);

	if (mIsTransparent == isTransparent)
		return;

	if (!isTransparent)
	{
		LOG(("nsWindow[%d]::SetTransparencyMode resetting %s\n", mWindowID, aMode == eTransparencyTransparent ? "eTransparencyTransparent" :  "eTransparencyOpaque"));
		RemoveAlphaClip();
		DestroyTransparencyBitmap();
	}

	mIsTransparent = isTransparent;


}

nsTransparencyMode
nsWindow::GetTransparencyMode()
{
	if (!mIsNativeWindow)
	{
		nsWindow *root = FindNative();
		if (root)
			return root->GetTransparencyMode();

		return eTransparencyOpaque;
	}

//	LOG(("nsWindow[%d]::GetTransparencyMode %s\n", mWindowID, mIsTransparent ? "eTransparencyTransparent" :  "eTransparencyOpaque"));
    return mIsTransparent ? eTransparencyTransparent : eTransparencyOpaque;
}

void
nsWindow::DestroyTransparencyBitmap()
{
	if (!mAlphaClip)
		return;

	LOG(("nsWindow[%d]::DestroyTransparencyBitmap\n", mWindowID));

	struct Screen *scr = IIntuition->LockPubScreen(NULL);
	ILayers->FreeClipRect(&(scr->LayerInfo), (struct ClipRect *)mAlphaClip);
	mAlphaMap = 0;
	mAlphaClip = 0;
	mTransparencyBitmapWidth = 0;
	mTransparencyBitmapHeight = 0;
	IIntuition->UnlockPubScreen(NULL, scr);
}


void
nsWindow::ResizeTransparencyBitmap(PRInt32 aNewWidth, PRInt32 aNewHeight)
{
//	if (!mAlphaMap)
//		return;


	if (aNewWidth == mTransparencyBitmapWidth && aNewHeight == mTransparencyBitmapHeight)
		return;

	LOG(("nsWindow[%d]::ResizeTransparencyBitmap\n", mWindowID));

	if (aNewWidth == 0 || aNewHeight == 0)
	{
		RemoveAlphaClip();
		DestroyTransparencyBitmap();

		return;
	}


	struct Screen *scr = IIntuition->LockPubScreen(NULL);

	// Create alpha map (ugly hack alert)
	struct TagItem bmtags[3] = { { BMATags_PixelFormat, PIXF_ALPHA8 }, { BMATags_Friend, (ULONG)scr->RastPort.BitMap}, { TAG_END, 0 } };
	struct BitMap *newAlpha = IGraphics->AllocBitMap(aNewWidth, aNewHeight, 8, BMF_DISPLAYABLE|BMF_CLEAR|BMF_CHECKVALUE, (struct BitMap *)bmtags);

	// Copy old to new
	if (mAlphaMap)
	{
		IGraphics->BltBitMapTags(
			BLITA_SrcX,				0,
			// BLITA_SrcY,				0,
			BLITA_SrcType,			BLITT_BITMAP,
			BLITA_Source,			mAlphaMap,

			BLITA_DestX,			0,
			BLITA_DestY,			0,
			BLITA_DestType,			BLITT_BITMAP,
			BLITA_Dest,				newAlpha,

			BLITA_Width,			aNewWidth > mTransparencyBitmapWidth ? mTransparencyBitmapWidth : aNewWidth,
			BLITA_Height,			aNewHeight > mTransparencyBitmapHeight ? mTransparencyBitmapHeight : aNewHeight,

		TAG_DONE);

		IGraphics->FreeBitMap((struct BitMap *)mAlphaMap);
	}

	mAlphaMap = newAlpha;
	mTransparencyBitmapWidth = aNewWidth;
	mTransparencyBitmapHeight = aNewHeight;

	IIntuition->UnlockPubScreen(NULL, scr);
}

void
nsWindow::ApplyTransparencyBitmap()
{
	if (!mAlphaMap)
		return;

	LOG(("nsWindow[%d]::ApplyTransparencyBitmap\n", mWindowID));

	if (!mAlphaClip)
	{
		struct Screen *scr = IIntuition->LockPubScreen(NULL);

		// Create a new clip rectangle if we didn't have one yet
		mAlphaClip = (struct ClipRect *)ILayers->AllocClipRect(&(scr->LayerInfo));

		if (!mAlphaClip)
			return;

		((struct ClipRect *)mAlphaClip)->bounds.MinX = 0;
		((struct ClipRect *)mAlphaClip)->bounds.MinY = 0;
		((struct ClipRect *)mAlphaClip)->bounds.MaxX = mTransparencyBitmapWidth - 1;
		((struct ClipRect *)mAlphaClip)->bounds.MaxY = mTransparencyBitmapHeight - 1;
		((struct ClipRect *)mAlphaClip)->Next = NULL;
		((struct ClipRect *)mAlphaClip)->BitMap = (struct BitMap *)mAlphaMap;

		IIntuition->UnlockPubScreen(NULL, scr);
	}

	LOG(("nsWindow[%d]::ApplyTransparencyBitmap applying\n", mWindowID));
	SetAlphaClip();
}


void
nsWindow::UpdateTransparencyBitmap(PRInt32 x, PRInt32 y, PRInt32 width, PRInt32 height,
		void *data, PRInt32 stride)
{
	LOG(("nsWindow[%d]::UpdateTransparencyBitmap\n", mWindowID));

	struct RenderInfo ri;

	if (!mAlphaMap)
	{
		struct Screen *scr = IIntuition->LockPubScreen(NULL);

		struct TagItem bmtags[3] = { { BMATags_PixelFormat, PIXF_ALPHA8 }, { BMATags_Friend, (ULONG)scr->RastPort.BitMap}, { TAG_END, 0 } };
		mAlphaMap = IGraphics->AllocBitMap(width, height, 8, BMF_DISPLAYABLE|BMF_CLEAR|BMF_CHECKVALUE, (struct BitMap *)bmtags);

		IIntuition->UnlockPubScreen(NULL, scr);

		if (!mAlphaMap)
			return;
	}

//	IIntuition->SetWindowAttrs(mIntuitionWindow,
//					WA_AlphaClips, 		NULL,
//					WA_BackFill,		LAYERS_BACKFILL,
//				TAG_DONE);

	int32 lock = IP96->p96LockBitMap((struct BitMap *)mAlphaMap, (uint8*) &ri, sizeof(ri));
	LOG(("nsWindow[%d]::UpdateTransparencyBitmap converting from %p to %p\n", mWindowID, data, ri.Memory));

	if (ri.Memory)
	{
		for (PRInt32 row = y; row < height; row++)
		{
			uint32 *cairoPixel = reinterpret_cast<uint32 *>((uint8 *)data + row * stride);
			uint8 *alphaPixel = reinterpret_cast<uint8*>(reinterpret_cast<uint8*>(ri.Memory) + row * ri.BytesPerRow);

			for (PRInt32 col = x; col < width; col++)
			{
				uint32 pix = *cairoPixel++;
				*alphaPixel++ = (pix >> 24) & 0xff;;
			}
		}
	}

	if (lock)
		IP96->p96UnlockBitMap((struct BitMap *)mAlphaMap, lock);

}


void
nsWindow::RemoveAlphaClip(void)
{
	IIntuition->SetWindowAttrs(mIntuitionWindow,
			WA_AlphaClips, 		mOldAlphaClip,
			WA_BackFill,		LAYERS_BACKFILL,
			TAG_DONE);
}

void
nsWindow::SetAlphaClip(void)
{
	IIntuition->GetWindowAttrs(mIntuitionWindow, WA_AlphaClips, &mOldAlphaClip, sizeof(mOldAlphaClip));
	IIntuition->SetWindowAttrs(mIntuitionWindow,
				WA_AlphaClips, 	mAlphaClip,
				WA_BackFill, 	LAYERS_NOBACKFILL,
			TAG_DONE);
}

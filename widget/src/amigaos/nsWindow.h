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

#ifndef __nsWindow_h__
#define __nsWindow_h__

#include "nsAutoPtr.h"
#include "nsBaseWidget.h"
#include "nsGUIEvent.h"
#include "nsWeakReference.h"
#include "nsIDragService.h"
#include "nsITimer.h"
#include "nsWidgetAtoms.h"
#include "gfxASurface.h"
#include "nsAmigaOSCursor.h"
#ifdef AMIGAOS_ENABLE_IME
#include "nsAmigaOSIMModule.h"
#endif

#include "nsTArray.h"

#ifdef PR_LOGGING
#define FORCE_PR_LOG
#include "prlog.h"

extern PRLogModuleInfo *gWidgetLog;
extern PRLogModuleInfo *gWidgetDrawLog;
extern PRLogModuleInfo *gWidgetEventLog;

#define LOG(args) PR_LOG(gWidgetLog, 4, args)
#define LOGDRAW(args) PR_LOG(gWidgetDrawLog, 4, args)
#define LOGEVENT(args) PR_LOG(gWidgetEventLog, 4, args)

#else

#define LOG(args)
#define LOGDRAW(args)
#define LOGEVENT(args)

#endif

class nsWindow :
	public nsBaseWidget,
	public nsSupportsWeakReference
{
	public:
		using nsBaseWidget::GetLayerManager;

		nsWindow();
		virtual ~nsWindow();

		NS_DECL_ISUPPORTS_INHERITED

		// nsIWidget methods
		NS_IMETHOD Create(nsIWidget *aParent,
						  nsNativeWidget aNativeParent,
				          const nsIntRect &aRect,
				          EVENT_CALLBACK aHandleEventFunction,
				          nsIDeviceContext *aContext,
				          nsIAppShell *aAppShell,
				          nsIToolkit *aToolkit,
				          nsWidgetInitData *aInitData);
		NS_IMETHOD Destroy(void);
		NS_IMETHOD ConfigureChildren(const nsTArray<nsIWidget::Configuration>& aConfigurations);
	    NS_IMETHOD SetParent(nsIWidget* aNewParent);
	    virtual nsIWidget *GetParent(void);
	    virtual float GetDPI();
	    NS_IMETHOD Show(PRBool aState);
	    NS_IMETHOD SetModal(PRBool aModal);
	    NS_IMETHOD IsVisible(PRBool & aState);
	    NS_IMETHOD ConstrainPosition(PRBool aAllowSlop,
	                                 PRInt32 *aX,
	                                 PRInt32 *aY);
	    NS_IMETHOD Move(PRInt32 aX,
	                    PRInt32 aY);
	    NS_IMETHOD Resize(PRInt32 aWidth,
	                      PRInt32 aHeight,
	                      PRBool  aRepaint);
	    NS_IMETHOD Resize(PRInt32 aX,
	                      PRInt32 aY,
	                      PRInt32 aWidth,
	                      PRInt32 aHeight,
	                      PRBool aRepaint);
	    NS_IMETHOD SetZIndex(PRInt32 aZIndex);
	    NS_IMETHOD PlaceBehind(nsTopLevelWidgetZPlacement aPlacement,
	                           nsIWidget *aWidget,
	                           PRBool aActivate);
	    NS_IMETHOD SetSizeMode(PRInt32 aMode);
	    NS_IMETHOD Enable(PRBool aState);
	    NS_IMETHOD IsEnabled(PRBool *aState);
	    NS_IMETHOD Invalidate(const nsIntRect &aRect,
	                          PRBool aIsSynchronous);
	    NS_IMETHOD Update();
	    NS_IMETHOD SetFocus(PRBool aRaise = PR_FALSE);
	    NS_IMETHOD GetScreenBounds(nsIntRect &aRect);
	    virtual nsIntPoint WidgetToScreenOffset();
	    NS_IMETHOD DispatchEvent(nsGUIEvent *aEvent, nsEventStatus &aStatus);
	    nsEventStatus DispatchEvent(nsGUIEvent *aEvent);
	    NS_IMETHOD MakeFullScreen(PRBool aFullScreen);
	    NS_IMETHOD SetWindowClass(const nsAString& xulWinType);
	    NS_IMETHOD SetForegroundColor(const nscolor &aColor);
	    NS_IMETHOD SetBackgroundColor(const nscolor &aColor);
	    NS_IMETHOD SetCursor(imgIContainer* aCursor,
	                         PRUint32 aHotspotX,
	                         PRUint32 aHotspotY);
	    NS_IMETHOD SetCursor(nsCursor aCursor);
	    NS_IMETHOD HideWindowChrome(PRBool aShouldHide);
	    virtual void* GetNativeData(PRUint32 aDataType);
	    NS_IMETHOD SetTitle(const nsAString& aTitle);
	    NS_IMETHOD SetIcon(const nsAString& aIconSpec);
	    NS_IMETHOD EnableDragDrop(PRBool aEnable);
	    NS_IMETHOD CaptureMouse(PRBool aCapture);
	    NS_IMETHOD CaptureRollupEvents(nsIRollupListener *aListener,
	                                   nsIMenuRollup *aMenuRollup,
	                                   PRBool aDoCapture,
	                                   PRBool aConsumeRollupEvent);
	    NS_IMETHOD GetAttention(PRInt32 aCycleCount);
	    NS_IMETHOD BeginResizeDrag(nsGUIEvent* aEvent, PRInt32 aHorizontal, PRInt32 aVertical);
	    NS_IMETHOD ResetInputState();
	    NS_IMETHOD SetInputMode(const IMEContext& aContext);
	    NS_IMETHOD GetInputMode(IMEContext& aContext);
	    NS_IMETHOD CancelIMEComposition();
	    NS_IMETHOD OnIMEFocusChange(PRBool aFocus);
	    NS_IMETHOD OnIMETextChange(PRUint32 aStart, PRUint32 aOldEnd, PRUint32 aNewEnd);
	    NS_IMETHOD OnIMESelectionChange(void);
	    LayerManager* GetLayerManager(LayerManagerPersistence aPersistence = LAYER_MANAGER_CURRENT,
	                                  bool* aAllowRetaining = nsnull);
	    gfxASurface* GetThebesSurface();

	    NS_IMETHOD ReparentNativeWidget(nsIWidget* aNewParent);

	    // Event handling
	    virtual PRBool HandleAmigaIDCMP(void *message);
	    void OnActivateEvent(void);
	    void OnDeactivateEvent(void);
	    void OnMouseEnterEvent(struct IntuiMessage *imsg);
	    void OnMouseLeaveEvent(struct IntuiMessage *imsg);
	    void OnMouseButtonEvent(struct IntuiMessage *imsg);
	    void OnMouseMotionEvent(struct IntuiMessage *imsg);
	    void OnMouseWheelEvent(struct IntuiMessage *imsg);
	    void OnKeyEvent(struct IntuiMessage *imsg);
	    void OnResizeEvent(const nsIntRect &aRect);
	    void OnDestroy(void);
	    void OnDraw(nsIntRect *aRect = nsnull);

	    // Drag and Drop support
	    void OnDragMotionEvent(struct IntuiMessage *imsg, void *aData = 0);
	    void OnDragEnter(struct IntuiMessage *imsg);
	    void OnDragLeave(struct IntuiMessage *imsg);
	    void OnDragDropEvent(struct IntuiMessage *imsg, void *aData = 0);

	    void InitEvent(nsGUIEvent &event, nsIntPoint *aPoint = 0, struct IntuiMessage *imsg = 0);
	    void InitEvent(nsMouseEvent &event, nsIntPoint *aPoint = 0, struct IntuiMessage *imsg = 0);
	    void InitEvent(nsMouseScrollEvent &event, nsIntPoint *aPoint = 0, struct IntuiMessage *imsg = 0);
	    void InitEvent(nsKeyEvent &event, nsIntPoint *aPoint = 0, struct IntuiMessage *imsg = 0);

	    // Other
	    nsWindow* FindWindowForPoint(const nsIntPoint& pt, struct IntuiMessage *imsg = 0);
	    nsIntPoint WidgetToWindowOffset();

	    void DrawDamagedRegions(void);

	    // Transparency
	    void SetTransparencyMode(nsTransparencyMode aMode);
	    nsTransparencyMode GetTransparencyMode();
	    void ResizeTransparencyBitmap(PRInt32 aNewWidth, PRInt32 aNewHeight);
	    void ApplyTransparencyBitmap();
	    void DestroyTransparencyBitmap();
	    void UpdateTransparencyBitmap(PRInt32 x, PRInt32 y, PRInt32 width, PRInt32 height,
	    		void *data, PRInt32 stride);


	    static PRBool ValidateWindow(nsWindow *w);
	protected:

	    void CheckNeedDragLeaveEnter(struct IntuiMessage *imsg, nsWindow *current, nsIDragService *aDragService);
	    void UpdateDragStatus(int action, nsIDragService *aDragService);

	    nsWindow *FindTopLevel();
	    nsWindow *FindNative();
	    void NativeWindowResize(PRInt32 aX,
	    		                PRInt32 aY,
	    		                PRInt32 aWidth,
	    		                PRInt32 aHeight,
	    		                PRBool aRepaint);
	    void NativeWindowMakeFullscreen(PRBool aState);
	    void NativeWindowMaximize(PRBool aState);
	    void NativeWindowIconify(PRBool aState);

	    PRBool DrawTo(gfxASurface *targetSurface, nsIntRect *aRect = nsnull);

	    void FlashWindow(void);
	    void FlashRect(nsIntRect &aRect);

	    nsIntRegion mDamageRects;

	    nsWindow *mParent;
	    nsWindow *mFocus;				// Receiving input
	    nsWindow *mCurrent;				// For creating enter/exit events

	    PRPackedBool mIsTopLevel;
	    PRPackedBool mIsVisible;
	    PRPackedBool mEnabled;
	    PRPackedBool mIsDestroyed;
	    PRPackedBool mIsTransparent;

	    PRPackedBool mNeedsRedraw;		// Set after resize to let the IDCMP handler redraw
	    PRPackedBool mIgnoreChangewnd;	// Set before resize to prevent IDCMP resize
	    PRPackedBool mAllowUserChange;	// Set if the window has dragbar/size gadget
	    PRPackedBool mIsNativeWindow;	// Set if the window is a native window

	    PRPackedBool mNeedsShow;		// Set when the window is hidden, but needs to be shown (creation of 0x0 windows)

	    /* This is the pointer to the window we are using
	     * Note that this is either created when this is a toplevel window,
	     * or copied from the toplevel for anything else
	     */
	    struct Window *mIntuitionWindow;

	    /* Communication message port
	     * This is also inherited from the parent or from the AppShell, depending
	     * on whether it's a toplevel or not
	     */
	    struct MsgPort *mSharedPort;

	    void *mAppShell;

	    nsAmigaOSCursor *mCursor;

	    PRBool mIMEComposing;
	    nsString mIMEComposingText;
	    nsAutoTArray<nsTextRange, 4> mIMERanges;

	    IMEContext mIMEContext;

	    nsRefPtr<gfxASurface> mThebesSurface;


	    PRInt32 mTransparencyBitmapWidth;
	    PRInt32 mTransparencyBitmapHeight;
		void *mAlphaMap;
		void *mAlphaClip;
		void *mOldAlphaClip;


	private:
	    // Used for debugging purposes
	    uint32 mWindowID;

	    static uint32 sNextWindowID;
	    char *mTitleStorage;

	    // List of all popup windows rooted on this one, need to move
	    // when window is moved
	    nsTArray<nsWindow *> mPopupList;

	    void MovePopups(int32 dx, int32 dy);

	    uint32 mLastX;
	    uint32 mLastY;
	    static uint64 sLastButtonDownTime;
	    static uint64 sLastButtonUpTime;
	    static uint32 sLastButtonMask;
	    static uint32 sLastButtonDownX;
	    static uint32 sLastButtonDownY;
	    static BOOL sLastButtonClickCount;

	    static nsWindow *sLastDragMotionWindow;

	    nsIntRect mPreFSBounds;

	    void RemoveAlphaClip(void);
	    void SetAlphaClip(void);

	private:
#ifdef AMIGAOS_ENABLE_IME
	    nsRefPtr<nsAmigaOSIMModule> mIMModule;
#endif
};

#endif /* __nsWindow_h__ */


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


#include <stdio.h>
#include "nsDragService.h"
#include "nsIDocument.h"
#include "nsIRegion.h"
#include "nsITransferable.h"
#include "nsIServiceManager.h"
#include "nsISupportsPrimitives.h"
#include "nsVoidArray.h"
#include "nsXPIDLString.h"
#include "nsPrimitiveHelpers.h"
#include "nsWidgetsCID.h"
#include "nsCRT.h"
#include "gfxASurface.h"
#include "gfxContext.h"

#include "prlog.h"
#include "nsIPresShell.h"
#include "nsPresContext.h"
#include "nsIFrame.h"
#include "nsIView.h"
#include "nsIWidget.h"
#include "nsPixelConvert.h"

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/layers.h>
#include <graphics/blitattr.h>
#include <intuition/intuition.h>

static NS_DEFINE_CID(kCDragServiceCID,   NS_DRAGSERVICE_CID);

extern PRBool sInDragSession;

//NS_IMPL_ADDREF_INHERITED(nsDragService, nsBaseDragService)
//NS_IMPL_RELEASE_INHERITED(nsDragService, nsBaseDragService)
//NS_IMPL_QUERY_INTERFACE2(nsDragService, nsIDragService, nsIDragSession )

//-------------------------------------------------------------------------
//
// DragService constructor
// Enable logging: 'export NSPR_LOG_MODULES=nsDragService:5'
//-------------------------------------------------------------------------
nsDragService::nsDragService()
{
	mBitmap = NULL;
	mWindow = NULL;
	mAlphaMap = NULL;
	mAlphaClip = NULL;
}

//-------------------------------------------------------------------------
//
// DragService destructor
//
//-------------------------------------------------------------------------
nsDragService::~nsDragService()
{
	if (mWindow)
	{
		IIntuition->CloseWindow((struct Window *)mWindow);
		mWindow = 0;
	}

	if (mBitmap)
	{
		IGraphics->FreeBitMap((struct BitMap *)mBitmap);
		mBitmap = 0;
	}

	if (mAlphaClip)
	{
		struct Screen *scr = IIntuition->LockPubScreen(NULL);
		ILayers->FreeClipRect(&(scr->LayerInfo), (struct ClipRect *)mAlphaClip);
		// Freed by above call IGraphics->FreeBitMap((struct BitMap *)mAlphaMap);
		mAlphaMap = 0;
		mAlphaClip = 0;
		IIntuition->UnlockPubScreen(NULL, scr);
	}
}

//-------------------------------------------------------------------------
//
// nsIDragService : InvokeDragSession
//
// Called when a drag is being initiated from within mozilla
// The code here gets the BView, the dragRect, builds the DragMessage and
// starts the drag.
//
//-------------------------------------------------------------------------
NS_IMETHODIMP
nsDragService::InvokeDragSession (nsIDOMNode *aDOMNode,
                                  nsISupportsArray * aArrayTransferables,
                                  nsIScriptableRegion * aRegion,
                                  PRUint32 aActionType)
{
    nsresult rv = nsBaseDragService::InvokeDragSession(aDOMNode,
                                                       aArrayTransferables,
                                                       aRegion, aActionType);
    NS_ENSURE_SUCCESS(rv, rv);

    sInDragSession = PR_TRUE;

    if (!aArrayTransferables)
    	return NS_ERROR_INVALID_ARG;

    mSourceDataItems = aArrayTransferables;

    nsIntRect dragRect;
    nsPresContext *pc;
    nsRefPtr<gfxASurface> surface;

	DrawDrag(aDOMNode, aRegion, mScreenX, mScreenY, &dragRect, getter_AddRefs(surface), &pc);

    PRInt32 sx = mScreenX, sy = mScreenY;
    ConvertToUnscaledDevPixels(pc, &sx, &sy);

    mOffsetX = sx - dragRect.x;
    mOffsetY = sy - dragRect.y;

    if (!surface)
    	return PR_FALSE;

    mBitmap = nsPixelConvert::ASurfaceToBitmap(surface, dragRect, &mAlphaMap);

    if (mAlphaMap)
    {
    	struct Screen *scr = IIntuition->LockPubScreen(NULL);
    	mAlphaClip = (struct ClipRect *)ILayers->AllocClipRect(&(scr->LayerInfo));

    	if (mAlphaClip)
    	{
    		((struct ClipRect *)mAlphaClip)->bounds.MinX = 0;
    		((struct ClipRect *)mAlphaClip)->bounds.MinY = 0;
    		((struct ClipRect *)mAlphaClip)->bounds.MaxX = dragRect.width - 1;
    		((struct ClipRect *)mAlphaClip)->bounds.MaxY = dragRect.height - 1;
    		((struct ClipRect *)mAlphaClip)->Next = NULL;
    		((struct ClipRect *)mAlphaClip)->BitMap = (struct BitMap *)mAlphaMap;
    	}

    	IIntuition->UnlockPubScreen(NULL, scr);
    }

	mWindow = IIntuition->OpenWindowTags(NULL,
				WA_Left, 			mScreenX,
				WA_Top, 			mScreenY,
				WA_Activate,		FALSE,
				WA_ToolBox,			TRUE,
				WA_IDCMP,			NULL,
				WA_InnerWidth,		dragRect.width,
				WA_InnerHeight, 	dragRect.height,
				WA_Flags, 			WFLG_BORDERLESS,
				mAlphaClip ? WA_BackFill:TAG_IGNORE,	LAYERS_NOBACKFILL,
				WA_AlphaClips,		mAlphaClip,
				WA_FadeTime, 		200000,
				WA_NoHitThreshold, 	255,
			TAG_DONE);

	int32 error = IGraphics->BltBitMapTags(
			BLITA_SrcX,			0, //aRect->x,
			BLITA_SrcY,			0, //aRect->y,
			BLITA_SrcType,		BLITT_BITMAP,
			BLITA_Source,		mBitmap,

			BLITA_DestX,		((struct Window *)mWindow)->BorderLeft,
			BLITA_DestY,		((struct Window *)mWindow)->BorderTop,
			BLITA_DestType,		BLITT_RASTPORT,
			BLITA_Dest,			((struct Window *)mWindow)->RPort,

			BLITA_Width,		dragRect.width,
			BLITA_Height,		dragRect.height,

		TAG_DONE);

    StartDragSession();

    return NS_OK;
}

//-------------------------------------------------------------------------
//
// nsIDragService : StartDragSession
//
// We overwrite this so we can log it
//
//-------------------------------------------------------------------------
NS_IMETHODIMP
nsDragService::StartDragSession()
{
    return nsBaseDragService::StartDragSession();
}

//-------------------------------------------------------------------------
//
// nsIDragService : EndDragSession
//
// We overwrite this so we can log it
//
//-------------------------------------------------------------------------
NS_IMETHODIMP
nsDragService::EndDragSession(PRBool aDoneDrag)
{
//    PR_LOG(sDragLm, PR_LOG_DEBUG, ("nsDragService::EndDragSession()"));
    //Don't reset drag info, keep it until there is a new drag, in case a negotiated drag'n'drop wants the info.
    //We need the draginfo as we are ending starting the dragsession
    //on entering/exiting different views (nsWindows) now.
    //That way the dragsession is always ended when we go outside mozilla windows, but we do throw away the
    // mSourceDocument and mSourceNode. We do hold on to the nsTransferable if it was a internal drag.
    //ResetDragInfo();
	SetDragAction(DRAGDROP_ACTION_NONE);

	if (mWindow)
	{
		IIntuition->CloseWindow((struct Window *)mWindow);
		mWindow = 0;
	}

	if (mBitmap)
	{
		IGraphics->FreeBitMap((struct BitMap *)mBitmap);
		mBitmap = 0;
	}

	if (mAlphaClip)
	{
		struct Screen *scr = IIntuition->LockPubScreen(NULL);
		ILayers->FreeClipRect(&(scr->LayerInfo), (struct ClipRect *)mAlphaClip);
		// Freed by above call IGraphics->FreeBitMap((struct BitMap *)mAlphaMap);
		mAlphaMap = 0;
		mAlphaClip = 0;
		IIntuition->UnlockPubScreen(NULL, scr);
	}



    return nsBaseDragService::EndDragSession(aDoneDrag);
}

void
nsDragService::UpdateDragPosition(struct Window *mRefWindow, struct IntuiMessage *imsg)
{
	// Extract screen coordinates
	uint32 x = mRefWindow->LeftEdge + imsg->MouseX - mOffsetX;
	uint32 y = mRefWindow->TopEdge + imsg->MouseY - mOffsetY;

	IIntuition->ChangeWindowBox((struct Window *)mWindow, x, y, ((struct Window *)mWindow)->Width, ((struct Window *)mWindow)->Height);

}

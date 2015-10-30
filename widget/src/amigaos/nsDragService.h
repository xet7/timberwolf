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


#ifndef nsDragService_h__
#define nsDragService_h__

#include "nsBaseDragService.h"
#include "nsIObserver.h"


#include <intuition/intuition.h>

class nsDragService : public nsBaseDragService
{
public:
	nsDragService();
	virtual ~nsDragService();

	// nsIDragService
	NS_IMETHOD InvokeDragSession(nsIDOMNode *aDOMNode,
			nsISupportsArray *anArrayTransferables,
			nsIScriptableRegion *aRegion,
			PRUint32 aActionType);

	NS_IMETHOD StartDragSession();
	NS_IMETHOD EndDragSession(PRBool aDoneDrag);

	void UpdateDragPosition(struct Window *mRefWindow, struct IntuiMessage *imsg);

private:
	nsCOMPtr<nsISupportsArray> mSourceDataItems;

	void *mBitmap;
	void *mAlphaMap;
	void *mAlphaClip;
	void *mWindow;

	uint32 mOffsetX;
	uint32 mOffsetY;

};

#endif // nsDragService_h__

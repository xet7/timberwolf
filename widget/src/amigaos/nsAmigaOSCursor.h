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

#ifndef nsAmigaOSCursor_h__
#define nsAmigaOSCursor_h__

#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/icon.h>
#include <proto/dos.h>
#include <proto/layers.h>
#include <proto/keymap.h>
#include <intuition/pointerclass.h>
#include <graphics/blitattr.h>


class nsAmigaOSCursor
{
public:
	nsAmigaOSCursor(struct Window *forWindow, const char *name, nsCursor type);
	nsAmigaOSCursor(struct Window *forWindow, uint8 *imageData,
			uint32 width, uint32 height,
			int32 offsetx, int32 offsety);

	~nsAmigaOSCursor();

	void Set(void);
	void Unset(void);

	nsCursor GetType(void)
	{
		return mType;
	}

private:

	void Construct();

	uint8 *mImageBuffer;
	struct BitMap *mBitMap;
	uint32 mWidth;
	uint32 mHeight;
	int32 mHotSpotX;
	int32 mHotSpotY;
	struct Window *mWindow;
	Object *mPointerObject;
	nsCursor mType;
};

#endif //nsPopupWindow_h__

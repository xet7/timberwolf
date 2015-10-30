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

#include "nsWindow.h"
#include "nsAmigaOSCursor.h"

#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/icon.h>
#include <proto/dos.h>
#include <proto/layers.h>
#include <proto/keymap.h>
#include <intuition/pointerclass.h>
#include <graphics/blitattr.h>
#include <workbench/icon.h>

// nsAmigaOSCursor class
nsAmigaOSCursor::nsAmigaOSCursor(Window *forWindow, const char *name, nsCursor type)
	:
		mImageBuffer(NULL),
		mWidth(0),
		mHeight(0),
		mHotSpotX(0),
		mHotSpotY(0),
		mWindow(forWindow),
		mPointerObject(NULL),
		mType(type)
{
	// Get the cursor icon from disk
	struct DiskObject *dob = IIcon->GetIconTags(name,
		ICONGETA_FailIfUnavailable,			TRUE,
		ICONGETA_GetPaletteMappedIcon,		FALSE,
	TAG_DONE);
	if (!dob) return;

	// Extract the image data
	uint32 imageFormat;
	uint8 *imageData;
	uint32 width, height;
	struct BitMap *bm;

	uint32 num = IIcon->IconControl(dob,
		ICONCTRLA_GetImageDataFormat, &imageFormat,
		ICONCTRLA_GetImageData1, &imageData,
		ICONCTRLA_GetWidth,		&width,
		ICONCTRLA_GetHeight,	&height,
		ICONCTRLA_GetBitMap1,	&bm,
	TAG_DONE);

	if (num == 5 && imageFormat == IDFMT_DIRECTMAPPED)
	{
		// Allocate data storage for the image
		mImageBuffer = new uint8[width*height*4];
		memcpy(mImageBuffer, imageData, width*height*4);
		mWidth = width;
		mHeight = height;

		// Get the hotspot locations via Tooltypes XOFFSET and YOFFSET
		STRPTR *toolTypes = dob->do_ToolTypes;
		STRPTR x = IIcon->FindToolType(toolTypes, "XOFFSET");
		STRPTR y = IIcon->FindToolType(toolTypes, "YOFFSET");
		if (x) mHotSpotX = strtol(x, NULL, 0);
		if (y) mHotSpotY = strtol(y, NULL, 0);

		uint32
			bmWidth = IGraphics->GetBitMapAttr(bm, BMA_WIDTH),
			bmHeight = IGraphics->GetBitMapAttr(bm, BMA_HEIGHT);

		// Make the backup bitmap
		mBitMap = IGraphics->AllocBitMap(
			bmWidth,
			bmHeight,
			IGraphics->GetBitMapAttr(bm, BMA_DEPTH),
			IGraphics->GetBitMapAttr(bm, BMA_FLAGS),
			bm);
		if (mBitMap)
		{
			IGraphics->BltBitMapTags(
				BLITA_SrcX,				0,
				BLITA_SrcY,				0,
				BLITA_Width,			bmWidth,
				BLITA_Height,			bmHeight,
				BLITA_SrcType,			BLITT_BITMAP,
				BLITA_Source,			bm,

				BLITA_DestX,			0,
				BLITA_DestY,			0,
				BLITA_DestType,			BLITT_BITMAP,
				BLITA_Dest,				mBitMap,

				TAG_DONE
			);
		}

		// With all the data present, go and construct the pointer
		Construct();
	}

	IIcon->FreeDiskObject(dob);
}

nsAmigaOSCursor::nsAmigaOSCursor(struct Window *forWindow, uint8 *imageData,
			uint32 width, uint32 height,
			int32 offsetx, int32 offsety)
	:
		mImageBuffer(NULL),
		mWidth(width),
		mHeight(height),
		mHotSpotX(offsetx),
		mHotSpotY(offsety),
		mWindow(forWindow),
		mPointerObject(NULL)
{
	mImageBuffer = new uint8[width*height*4];
	memcpy(mImageBuffer, imageData, width*height*4);

	// Make the backup bitmap
	mBitMap = IGraphics->AllocBitMap(
		mWidth,
		mHeight,
		8, BMF_CLEAR,
		NULL);

	if (mBitMap)
	{
		IGraphics->BltBitMapTags(
			BLITA_SrcX,				0,
			BLITA_SrcY,				0,
			BLITA_Width,			mWidth,
			BLITA_Height,			mHeight,
			BLITA_SrcType,			BLITT_ARGB32,
			BLITA_SrcBytesPerRow,	mWidth * 4,
			BLITA_Source,			mImageBuffer,

			BLITA_DestX,			0,
			BLITA_DestY,			0,
			BLITA_DestType,			BLITT_BITMAP,
			BLITA_Dest,				mBitMap,

			TAG_DONE
		);
	}

	Construct();
}

void
nsAmigaOSCursor::Construct(void)
{
	mPointerObject = (Object *)IIntuition->NewObject(NULL, "pointerclass",
		POINTERA_XOffset,			-mHotSpotX,
		POINTERA_YOffset,			-mHotSpotY,
		POINTERA_XResolution,		POINTERXRESN_SCREENRES,
		POINTERA_YResolution,		POINTERXRESN_SCREENRES,
		POINTERA_ImageData,			mImageBuffer,
		POINTERA_Width,				mWidth,
		POINTERA_Height,			mHeight,

		// Backwards compatibility crap
		POINTERA_BitMap,			mBitMap,
		POINTERA_WordWidth,			mWidth / 16,
	TAG_DONE);
}

void
nsAmigaOSCursor::Set(void)
{
	if (mPointerObject && mWindow)
	{
		IIntuition->SetWindowPointer(mWindow,
			WA_Pointer, 	mPointerObject,
		TAG_DONE);
	}
}


void
nsAmigaOSCursor::Unset(void)
{
	if (mPointerObject && mWindow)
	{
		IIntuition->SetWindowPointer(mWindow,
			WA_Pointer, 	NULL,
		TAG_DONE);
	}
}

nsAmigaOSCursor::~nsAmigaOSCursor()
{
	Unset();
	if (mPointerObject) IIntuition->DisposeObject(mPointerObject);
	if (mImageBuffer) delete [] mImageBuffer;
}

// --------------------------------------------------------------------------


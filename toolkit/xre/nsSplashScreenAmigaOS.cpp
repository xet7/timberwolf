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

#include "nsSplashScreen.h"
#include "nsSplashScreenAmigaOS.h"

#include <stdio.h>
#include <string.h>


#include <intuition/intuition.h>
#include <graphics/gfx.h>
#include <graphics/blitattr.h>


class nsSplashScreenAmigaOS;

static nsSplashScreenAmigaOS *gSplashScreen = NULL;

class nsSplashScreenAmigaOS : public nsSplashScreen
{
public:
    nsSplashScreenAmigaOS();
    virtual ~nsSplashScreenAmigaOS();

    virtual void Open();
    virtual void Close();
    virtual void Update(PRInt32 progress);
protected:
    struct Window *mWindow;
    PRInt32 mProgress;

private:
    struct Library *mIntuitionBase;
    struct IntuitionIFace *mIIntuition;
    struct Library *mGfxBase;
    struct GraphicsIFace *mIGraphics;
};

class ____splashScreenSentinel
{
public:
	~____splashScreenSentinel()
	{
		if (gSplashScreen) delete gSplashScreen;
	}
} _____sentinel;


nsSplashScreen*
nsSplashScreen::Get()
{
#if !defined (DEBUG)
	return gSplashScreen;
#else
	return 0;
#endif
}

nsSplashScreen*
nsSplashScreen::GetOrCreate()
{
#if !defined(DEBUG)
	if (!gSplashScreen)
		gSplashScreen = new nsSplashScreenAmigaOS();

	return gSplashScreen;
#else
	return 0;
#endif
}

nsSplashScreenAmigaOS::nsSplashScreenAmigaOS()
	: mWindow(NULL), mProgress(-1)
{
	mIntuitionBase = IExec->OpenLibrary("intuition.library", 0L);
	mIIntuition = (struct IntuitionIFace *)IExec->GetInterface(mIntuitionBase, "main", 1, NULL);
	mGfxBase = IExec->OpenLibrary("graphics.library", 0L);
	mIGraphics = (struct GraphicsIFace *)IExec->GetInterface(mGfxBase, "main", 1, NULL);

	gSplashScreen = NULL;
}

nsSplashScreenAmigaOS::~nsSplashScreenAmigaOS()
{
	Close();

	if (mIIntuition) IExec->DropInterface((struct Interface *)mIIntuition);
	if (mIntuitionBase) IExec->CloseLibrary(mIntuitionBase);
	if (mIGraphics) IExec->DropInterface((struct Interface *)mIGraphics);
	if (mGfxBase) IExec->CloseLibrary(mGfxBase);
}

void
nsSplashScreenAmigaOS::Open()
{
	if (mIsOpen)
		return;

	struct Screen *scr = mIIntuition->LockPubScreen(NULL);
	uint32 scrWidth, scrHeight;

	mIIntuition->GetScreenAttrs(scr,
			SA_Width, &scrWidth,
			SA_Height, &scrHeight,
		TAG_DONE);

	mIIntuition->UnlockPubScreen(NULL, scr);

	uint32 winX = (scrWidth - splash_img.width) / 2;
	uint32 winY = (scrHeight - splash_img.height) / 2;

	mWindow = mIIntuition->OpenWindowTags(NULL,
			WA_Flags,			WFLG_BORDERLESS,
			WA_IDCMP,			0,
			WA_Left,			winX,
			WA_Top,				winY,
			WA_InnerWidth,		splash_img.width,
			WA_InnerHeight,		splash_img.height,
			WA_WindowName,		"Timberwolf Splash",
			WA_Hidden,			FALSE,
		TAG_DONE);

	mIGraphics->BltBitMapTags(
			BLITA_SrcX,			0,
			BLITA_SrcY,			0,
			BLITA_Width,		splash_img.width,
			BLITA_Height,		splash_img.height,
			BLITA_SrcType,		BLITT_RGB24,
			BLITA_Source,		splash_img.pixel_data,
			BLITA_SrcBytesPerRow,splash_img.bytes_per_pixel * splash_img.width,
			BLITA_DestX,		0,
			BLITA_DestY,		0,
			BLITA_DestType,		BLITT_RASTPORT,
			BLITA_Dest,			mWindow->RPort,
		TAG_DONE);

	mIsOpen = PR_TRUE;
}

void
nsSplashScreenAmigaOS::Close()
{
	if (!mIsOpen)
		return;

	if (mWindow)
	{
		mIIntuition->CloseWindow(mWindow);
		mWindow = 0;
	}

	mIsOpen = PR_FALSE;
}

void
nsSplashScreenAmigaOS::Update(PRInt32 progress)
{
	if (!mIsOpen)
		return;

	mProgress = progress;

	// Current image has progress bar 183,149 - 410,153
	uint32 pbWidth = (410 - 183) * progress / 100;

	mIGraphics->SetAPen(mWindow->RPort, 3);
	mIGraphics->SetDrMd(mWindow->RPort, JAM1);
	mIGraphics->RectFill(mWindow->RPort, 183, 149, pbWidth + 183, 153);
}

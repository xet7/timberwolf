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


#include "gfxAmigaOSSurface.h"

#include <proto/exec.h>
#include <proto/Picasso96API.h>

static struct P96IFace *myIP96 = NULL;

class __autoopen
{
public:
    __autoopen()
    {
        struct Library *lib = IExec->OpenLibrary("Picasso96API.library", 0);
        myIP96 = (struct P96IFace *)IExec->GetInterface(lib, "main", 1, 0);
    };

    ~__autoopen()
    {
        struct Library *lib = myIP96->Data.LibBase;
        IExec->DropInterface((struct Interface *)myIP96);
        IExec->CloseLibrary(lib);
    };
} __myautoopen;


gfxAmigaOSSurface::gfxAmigaOSSurface(struct BitMap *bm) :
    mBitmap(bm), mOwnsBitmap(PR_FALSE)
{
    Init(cairo_amigaos_surface_create(bm));
}

gfxAmigaOSSurface::gfxAmigaOSSurface(unsigned long width, unsigned long height,
		struct BitMap *friendBm, RGBFTYPE color)
{
	if (friendBm && color == (RGBFTYPE)-1)
	{
		 color = (RGBFTYPE)IP96->p96GetBitMapAttr(friendBm, P96BMA_RGBFORMAT);

	}
    int depth = 32;

    switch (color)
    {
        case RGBFB_A8R8G8B8:
            depth = 32;
            break;
        case RGBFB_R8G8B8:
            depth = 32;
            break;
        case RGBFB_ALPHA8:
            depth = 8;
            break;
        default:
        	depth = 32;
        	color = RGBFB_A8R8G8B8;
        	break;
    }

    mBitmap = myIP96->p96AllocBitMap(width, height,
                        depth, BMF_DISPLAYABLE, friendBm, color);
    mOwnsBitmap = PR_TRUE;

    Init(cairo_amigaos_surface_create(mBitmap));
}

gfxAmigaOSSurface::~gfxAmigaOSSurface()
{
    if (mOwnsBitmap && mBitmap)
    {
        myIP96->p96FreeBitMap(mBitmap);
        mBitmap = NULL;
    }
}

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



#ifndef GFX_AMIGAOSSURFACE_H
#define GFX_AMIGAOSSURFACE_H


#include "gfxASurface.h"
#include <cairo.h>
#include <cairo-amigaos.h>

class gfxAmigaOSSurface : public gfxASurface {
public:
    gfxAmigaOSSurface(struct BitMap *bm);
    gfxAmigaOSSurface(unsigned long width, unsigned long height, struct BitMap *friendBm, RGBFTYPE color = RGBFB_A8R8G8B8);
    virtual ~gfxAmigaOSSurface();

    const struct BitMap * GetBitmap()
    {
    	return mBitmap;
    }

private:
    struct BitMap *mBitmap;
    PRPackedBool   mOwnsBitmap;
};

#endif /* GFX_AMIGAOSSURFACE_H */

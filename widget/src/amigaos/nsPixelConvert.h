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

#include "gfxASurface.h"
#include "gfxImageSurface.h"
#include "gfxContext.h"

#include "imgIContainer.h"

#include "nsAutoPtr.h"

namespace nsPixelConvert
{
	void *ASurfaceToBitmap(gfxASurface *aSurface, nsIntRect &aSrcRect, void ** alpha = 0);
	void* ImgSurfaceToBitmap(gfxImageSurface *aImgSurface, nsIntRect &aSrcRect, void ** alpha = 0);
}

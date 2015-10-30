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
#include <proto/dos.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/Picasso96API.h>
#include <libraries/Picasso96.h>
#include <graphics/blitattr.h>
#include <intuition/intuition.h>
#include <graphics/gfx.h>
#include "gfxASurface.h"
#include "gfxImageSurface.h"
#include "gfxContext.h"

#include "imgIContainer.h"

#include "nsAutoPtr.h"

#include "nsPixelConvert.h"

// Shamelessly ripped from GTK code
inline uint8
unpremultiply (uint8 color, uint8 alpha)
{
    if (alpha == 0)
        return 0;
    // plus alpha/2 to round instead of truncate
    return (color * 255 + alpha / 2) / alpha;
}

inline uint32
packPixel(RGBFTYPE format, uint8 a, uint8 r, uint8 g, uint8 b)
{
	return (a << 24) | (r << 16) | (g << 8) | b;
}

namespace nsPixelConvert
{

	void* ImgSurfaceToBitmap(gfxImageSurface *aImageSurface, nsIntRect &aSrcRect, void ** alpha)
	{
		struct Screen *scr = IIntuition->LockPubScreen(NULL);
		struct BitMap *bm = IGraphics->AllocBitMap(aSrcRect.width, aSrcRect.height, 32, BMF_DISPLAYABLE, scr->RastPort.BitMap);
		struct BitMap *alphaMap = NULL;
		struct RenderInfo ri;

		if (alpha && aImageSurface->Format() == gfxASurface::ImageFormatARGB32)
		{
			struct TagItem bmtags[3] = { { BMATags_PixelFormat, PIXF_ALPHA8 }, { BMATags_Friend, (ULONG)scr->RastPort.BitMap}, { TAG_END, 0 } };
			alphaMap = IGraphics->AllocBitMap(aSrcRect.width, aSrcRect.height, 8, BMF_DISPLAYABLE|BMF_CLEAR|BMF_CHECKVALUE, (struct BitMap *)bmtags);
			*alpha = (void *)alphaMap;
		}

		IIntuition->UnlockPubScreen(NULL, scr);

		if (!bm)
			return NULL;

		if (alpha && !alphaMap && aImageSurface->Format() == gfxASurface::ImageFormatARGB32)
		{
			IGraphics->FreeBitMap(bm);
			return NULL;
		}

//		uint32 format = BLITT_ARGB32;
//		if (aImageSurface->Format() == gfxASurface::ImageFormatRGB24)
//			format = BLITT_RGB24;

//		printf("Image surface size: %dx%d, format = %p\n", aImageSurface->Width(), aImageSurface->Height(), aImageSurface->Format());
//		printf("Image surface data: base %p, stride %d\n", aImageSurface->Data(), aImageSurface->Stride());

		gfxASurface::gfxImageFormat format = aImageSurface->Format();
		int32 colorLock = IP96->p96LockBitMap(bm, (uint8*) &ri, sizeof(ri));

		for (PRInt32 row = 0; row < aSrcRect.height; row++)
		{
			uint32 *cairoPixel = reinterpret_cast<uint32 *>(aImageSurface->Data() + row * aImageSurface->Stride());
			uint32 *colorPixel = reinterpret_cast<uint32*>(reinterpret_cast<uint8*>(ri.Memory) + row * ri.BytesPerRow);

			if (format == gfxASurface::ImageFormatARGB32)
			{
				for (PRInt32 col = 0; col < aSrcRect.width; col++)
				{
					uint32 pix = *cairoPixel++;
					uint8 a = (pix >> 24) & 0xff;

					*colorPixel++ = packPixel(ri.RGBFormat,
										a,
										unpremultiply((pix >> 16) & 0xff, a),
										unpremultiply((pix >>  8) & 0xff, a),
										unpremultiply((pix      ) & 0xff, a)
							);
				}
			}
			else
			{
				for (PRInt32 col = 0; col < aSrcRect.width; col++)
				{
					uint32 pix = *cairoPixel++;
					*colorPixel++ = packPixel(ri.RGBFormat, 0xff, (pix >> 16) & 0xff, (pix >>  8) & 0xff, pix & 0xff);
				}
			}
		}

		if (colorLock)
			IP96->p96UnlockBitMap(bm, colorLock);

		if (alphaMap && format == gfxASurface::ImageFormatARGB32)
		{
			int32 lock = IP96->p96LockBitMap(alphaMap, (uint8*) &ri, sizeof(ri));

			if (ri.Memory)
			{
				for (PRInt32 row = 0; row < aSrcRect.height; row++)
				{
					uint32 *cairoPixel = reinterpret_cast<uint32 *>(aImageSurface->Data() + row * aImageSurface->Stride());
					uint8 *alphaPixel = reinterpret_cast<uint8*>(reinterpret_cast<uint8*>(ri.Memory) + row * ri.BytesPerRow);

					for (PRInt32 col = 0; col < aSrcRect.width; col++)
					{
						uint32 pix = *cairoPixel++;
						*alphaPixel++ = (pix >> 24) & 0xff;;
					}
				}
			}

			if (lock)
				IP96->p96UnlockBitMap(alphaMap, lock);
		}

		return bm;
	}

	void *ASurfaceToBitmap(gfxASurface *aSurface, nsIntRect &aSrcRect, void ** alpha)
	{
		if (aSurface->CairoStatus())
		{
			NS_ERROR("invalid surface");
			return nsnull;
		}

		nsRefPtr<gfxImageSurface> imgSurface;
		if (aSurface->GetType() == gfxASurface::SurfaceTypeImage)
		{
			imgSurface = static_cast<gfxImageSurface*>(static_cast<gfxASurface*>(aSurface));
		}
		else
		{
			imgSurface = new gfxImageSurface(gfxIntSize(aSrcRect.width, aSrcRect.height),
					gfxImageSurface::ImageFormatARGB32);

			if (!imgSurface)
				return nsnull;

			nsRefPtr<gfxContext> context = new gfxContext(imgSurface);
			if (!context)
				return nsnull;

			//aSurface->SetDeviceOffset(gfxPoint(aSrcRect.x, aSrcRect.height));
			context->SetOperator(gfxContext::OPERATOR_SOURCE);
			context->SetSource(aSurface);
			context->Paint();
		}

		return ImgSurfaceToBitmap(imgSurface, aSrcRect, alpha);
	}
}

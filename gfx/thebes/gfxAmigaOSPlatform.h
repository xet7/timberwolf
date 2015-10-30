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



#ifndef GFX_AMIGAOS_PLATFORM_H
#define GFX_AMIGAOS_PLATFORM_H

#include "gfxPlatform.h"
#include "nsString.h"
#include "nsVoidArray.h"
#include "nsDataHashtable.h"
#include <fontconfig/fontconfig.h>

typedef struct FT_LibraryRec_ *FT_Library;

class gfxFontconfigUtils;
class FontFamily;
class FontEntry;

class THEBES_API gfxAmigaOSPlatform : public gfxPlatform {
public:
    gfxAmigaOSPlatform();
    virtual ~gfxAmigaOSPlatform();

    static gfxAmigaOSPlatform *GetPlatform() {
        return (gfxAmigaOSPlatform*) gfxPlatform::GetPlatform();
    }

    already_AddRefed<gfxASurface> CreateOffscreenSurface(const gfxIntSize& size,
                                                         gfxASurface::gfxContentType contentType);

    nsresult GetFontList(nsIAtom *aLangGroup,
                         const nsACString& aGenericFamily,
                         nsTArray<nsString>& aListOfFonts);

    nsresult UpdateFontList();

    nsresult ResolveFontName(const nsAString& aFontName,
                             FontResolverCallback aCallback,
                             void *aClosure, PRBool& aAborted);

    nsresult GetStandardFamilyName(const nsAString& aFontName, nsAString& aFamilyName);

    gfxFontGroup *CreateFontGroup(const nsAString &aFamilies,
                                  const gfxFontStyle *aStyle,
                                  gfxUserFontSet* aUserFontSet);

    FontFamily *FindFontFamily(const nsAString& aName);
    FontEntry *FindFontEntry(const nsAString& aFamilyName, const gfxFontStyle& aFontStyle);

    already_AddRefed<gfxFont> FindFontForChar(PRUint32 aCh, gfxFont *aFont);
    PRBool GetPrefFontEntries(const nsCString& aLangGroup, nsTArray<nsRefPtr<gfxFontEntry> > *aFontEntryList);
    void SetPrefFontEntries(const nsCString& aLangGroup, nsTArray<nsRefPtr<gfxFontEntry> >& aFontEntryList);

    static PRInt32 DPI() {
        if (sDPI == -1) {
            InitDPI();
        }
        NS_ASSERTION(sDPI > 0, "Something is wrong");
        return sDPI;
    }

    FT_Library GetFTLibrary();

protected:
    static void InitDPI();

    static PRInt32 sDPI;
    static gfxFontconfigUtils *sFontconfigUtils;

};

#endif

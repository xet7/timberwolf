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


 #include "gfxAmigaOSPlatform.h"
 
 #include "gfxFontconfigUtils.h"

#include "nsUnicharUtils.h"
#include "gfxFontconfigUtils.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include "gfxFT2Fonts.h"

#include "cairo.h"

#include "gfxImageSurface.h"

#include <proto/exec.h>

PRInt32 gfxAmigaOSPlatform::sDPI = -1;


gfxFontconfigUtils *gfxAmigaOSPlatform::sFontconfigUtils = nsnull;
typedef nsDataHashtable<nsStringHashKey, nsRefPtr<FontFamily> > FontTable;
typedef nsDataHashtable<nsCStringHashKey, nsTArray<nsRefPtr<gfxFontEntry> > > PrefFontTable;
static FontTable *gPlatformFonts = NULL;
static FontTable *gPlatformFontAliases = NULL;
static PrefFontTable *gPrefFonts = NULL;
static gfxSparseBitSet *gCodepointsWithNoFonts = NULL;
static FT_Library gPlatformFTLibrary = NULL;

gfxAmigaOSPlatform::gfxAmigaOSPlatform()
{
    if (!sFontconfigUtils)
    {
    	IExec->Forbid();
        sFontconfigUtils = gfxFontconfigUtils::GetFontconfigUtils();
        IExec->Permit();
    }

    FT_Init_FreeType(&gPlatformFTLibrary);

    gPlatformFonts = new FontTable();
    gPlatformFonts->Init(100);
    gPlatformFontAliases = new FontTable();
    gPlatformFontAliases->Init(100);
    gPrefFonts = new PrefFontTable();
    gPrefFonts->Init(100);
    gCodepointsWithNoFonts = new gfxSparseBitSet();
    UpdateFontList();
}

gfxAmigaOSPlatform::~gfxAmigaOSPlatform()
{
    gfxFontconfigUtils::Shutdown();
    sFontconfigUtils = nsnull;

    delete gPlatformFonts;
    gPlatformFonts = NULL;
    delete gPlatformFontAliases;
    gPlatformFontAliases = NULL;
    delete gPrefFonts;
    gPrefFonts = NULL;
    delete gCodepointsWithNoFonts;
    gCodepointsWithNoFonts = NULL;

    FT_Done_FreeType(gPlatformFTLibrary);
    gPlatformFTLibrary = NULL;
}

already_AddRefed<gfxASurface>
gfxAmigaOSPlatform::CreateOffscreenSurface(const gfxIntSize& size,
                                           gfxASurface::gfxContentType contentType)
{
    gfxASurface *newSurface = nsnull;
 //   RGBFTYPE format;
	gfxASurface::gfxImageFormat imageFormat = gfxASurface::FormatFromContent(contentType);
	
    /* TODO: Can't get any friend bitmap here, what do we do ?
     * Possibility would be to use the first screen's bitmap.
     */
//    struct BitMap *friendBm = NULL;

    if (contentType == gfxASurface::CONTENT_COLOR)
    {
//		format = RGBFB_A8R8G8B8;
		imageFormat = gfxASurface::ImageFormatRGB16_565;
    }

    //if (imageFormat == gfxASurface::ImageFormatA1) {
        newSurface = new gfxImageSurface(size, imageFormat);
//    } else {
//        newSurface = new gfxAmigaOSSurface(size.width, size.height, friendBm, format);
//    }

    NS_ADDREF(newSurface);
    return newSurface;
}

nsresult
gfxAmigaOSPlatform::GetFontList(nsIAtom *aLangGroup,
                            const nsACString& aGenericFamily,
                            nsTArray<nsString>& aListOfFonts)
{
    return sFontconfigUtils->GetFontList(aLangGroup, aGenericFamily,
                                         aListOfFonts);
}

nsresult
gfxAmigaOSPlatform::UpdateFontList()
{
    FcPattern *pat = NULL;
    FcObjectSet *os = NULL;
    FcFontSet *fs = NULL;
    PRInt32 result = -1;

    pat = FcPatternCreate();
    os = FcObjectSetBuild(FC_FAMILY, FC_FILE, FC_INDEX, FC_WEIGHT, FC_SLANT, FC_WIDTH, NULL);

    fs = FcFontList(NULL, pat, os);


    for (int i = 0; i < fs->nfont; i++) {
        char *str;

        if (FcPatternGetString(fs->fonts[i], FC_FAMILY, 0, (FcChar8 **) &str) != FcResultMatch)
            continue;

        //printf("Family: %s\n", str);

        nsAutoString name(NS_ConvertUTF8toUTF16(nsDependentCString(str)).get());
        nsAutoString key(name);
        ToLowerCase(key);
        nsRefPtr<FontFamily> ff;
        if (!gPlatformFonts->Get(key, &ff)) {
            ff = new FontFamily(name);
            gPlatformFonts->Put(key, ff);
        }

        FontEntry *fe = new FontEntry(ff->Name());
        ff->AddFontEntry(fe);

        if (FcPatternGetString(fs->fonts[i], FC_FILE, 0, (FcChar8 **) &str) == FcResultMatch) {
            fe->mFilename = nsDependentCString(str);
            //printf(" - file: %s\n", str);
        }

        int x;
        if (FcPatternGetInteger(fs->fonts[i], FC_INDEX, 0, &x) == FcResultMatch) {
            //printf(" - index: %d\n", x);
            fe->mFTFontIndex = x;
        } else {
            fe->mFTFontIndex = 0;
        }

        fe->mWeight = gfxFontconfigUtils::GetThebesWeight(fs->fonts[i]);
        //printf(" - weight: %d\n", fe->mWeight);

        fe->mItalic = PR_FALSE;
        if (FcPatternGetInteger(fs->fonts[i], FC_SLANT, 0, &x) == FcResultMatch) {
            switch (x) {
            case FC_SLANT_ITALIC:
            case FC_SLANT_OBLIQUE:
                fe->mItalic = PR_TRUE;
            }
            //printf(" - slant: %d\n", x);
        }

        //if (FcPatternGetInteger(fs->fonts[i], FC_WIDTH, 0, &x) == FcResultMatch)
            //printf(" - width: %d\n", x);
        // XXX deal with font-stretch stuff later
    }

    if (pat)
        FcPatternDestroy(pat);
    if (os)
        FcObjectSetDestroy(os);
    if (fs)
        FcFontSetDestroy(fs);

    return sFontconfigUtils->UpdateFontList();
}

nsresult
gfxAmigaOSPlatform::ResolveFontName(const nsAString& aFontName,
                                FontResolverCallback aCallback,
                                void *aClosure,
                                PRBool& aAborted)
{

    nsAutoString name(aFontName);
    ToLowerCase(name);

    nsRefPtr<FontFamily> ff;
    if (gPlatformFonts->Get(name, &ff) ||
        gPlatformFontAliases->Get(name, &ff)) {
        aAborted = !(*aCallback)(ff->Name(), aClosure);
        return NS_OK;
    }

    nsCAutoString utf8Name = NS_ConvertUTF16toUTF8(aFontName);

    FcPattern *npat = FcPatternCreate();
    FcPatternAddString(npat, FC_FAMILY, (FcChar8*)utf8Name.get());
    FcObjectSet *nos = FcObjectSetBuild(FC_FAMILY, NULL);
    FcFontSet *nfs = FcFontList(NULL, npat, nos);

    for (int k = 0; k < nfs->nfont; k++) {
        FcChar8 *str;
        if (FcPatternGetString(nfs->fonts[k], FC_FAMILY, 0, (FcChar8 **) &str) != FcResultMatch)
            continue;
        nsAutoString altName = NS_ConvertUTF8toUTF16(nsDependentCString(reinterpret_cast<char*>(str)));
        ToLowerCase(altName);
        if (gPlatformFonts->Get(altName, &ff)) {
            //printf("Adding alias: %s -> %s\n", utf8Name.get(), str);
            gPlatformFontAliases->Put(name, ff);
            aAborted = !(*aCallback)(NS_ConvertUTF8toUTF16(nsDependentCString(reinterpret_cast<char*>(str))), aClosure);
            goto DONE;
        }
    }

    FcPatternDestroy(npat);
    FcObjectSetDestroy(nos);
    FcFontSetDestroy(nfs);

    {
    npat = FcPatternCreate();
    FcPatternAddString(npat, FC_FAMILY, (FcChar8*)utf8Name.get());
    FcPatternDel(npat, FC_LANG);
    FcConfigSubstitute(NULL, npat, FcMatchPattern);
    FcDefaultSubstitute(npat);

    nos = FcObjectSetBuild(FC_FAMILY, NULL);
    nfs = FcFontList(NULL, npat, nos);

    FcResult fresult;

    FcPattern *match = FcFontMatch(NULL, npat, &fresult);
    if (match)
        FcFontSetAdd(nfs, match);

    for (int k = 0; k < nfs->nfont; k++) {
        FcChar8 *str;
        if (FcPatternGetString(nfs->fonts[k], FC_FAMILY, 0, (FcChar8 **) &str) != FcResultMatch)
            continue;
        nsAutoString altName = NS_ConvertUTF8toUTF16(nsDependentCString(reinterpret_cast<char*>(str)));
        ToLowerCase(altName);
        if (gPlatformFonts->Get(altName, &ff)) {
            //printf("Adding alias: %s -> %s\n", utf8Name.get(), str);
            gPlatformFontAliases->Put(name, ff);
            aAborted = !(*aCallback)(NS_ConvertUTF8toUTF16(nsDependentCString(reinterpret_cast<char*>(str))), aClosure);
            goto DONE;
        }
    }
    }
 DONE:
    FcPatternDestroy(npat);
    FcObjectSetDestroy(nos);
    FcFontSetDestroy(nfs);

    return NS_OK;
}

nsresult
gfxAmigaOSPlatform::GetStandardFamilyName(const nsAString& aFontName, nsAString& aFamilyName)
{
    return sFontconfigUtils->GetStandardFamilyName(aFontName, aFamilyName);
}

gfxFontGroup *
gfxAmigaOSPlatform::CreateFontGroup(const nsAString &aFamilies,
                                const gfxFontStyle *aStyle,
                                gfxUserFontSet *aUserFontSet)
{
    return new gfxFT2FontGroup(aFamilies, aStyle, aUserFontSet);
}


void
gfxAmigaOSPlatform::InitDPI()
{
	if (sDPI <= 0) {
		// Fall back to something sane
		sDPI = 96;
	}
}


FT_Library
gfxAmigaOSPlatform::GetFTLibrary()
{
    return gPlatformFTLibrary;
}

FontFamily *
gfxAmigaOSPlatform::FindFontFamily(const nsAString& aName)
{
	nsAutoString name(aName);
	ToLowerCase(name);

	nsRefPtr<FontFamily> ff;
	if (!gPlatformFonts->Get(name, &ff)) {
		return nsnull;
	}
	return ff.get();
}

FontEntry *
gfxAmigaOSPlatform::FindFontEntry(const nsAString& aName, const gfxFontStyle& aFontStyle)
{
	nsRefPtr<FontFamily> ff = FindFontFamily(aName);
	if (!ff)
		return nsnull;

	return ff->FindFontEntry(aFontStyle);
}

static PLDHashOperator
FindFontForCharProc(nsStringHashKey::KeyType aKey,
                    nsRefPtr<FontFamily>& aFontFamily,
                    void* aUserArg)
{
    FontSearch *data = (FontSearch*)aUserArg;
    aFontFamily->FindFontForChar(data);
    return PL_DHASH_NEXT;
}

already_AddRefed<gfxFont>
gfxAmigaOSPlatform::FindFontForChar(PRUint32 aCh, gfxFont *aFont)
{
    if (!gPlatformFonts || !gCodepointsWithNoFonts)
        return nsnull;

    // is codepoint with no matching font? return null immediately
    if (gCodepointsWithNoFonts->test(aCh)) {
        return nsnull;
    }

    FontSearch data(aCh, aFont);

    // find fonts that support the character
    gPlatformFonts->Enumerate(FindFontForCharProc, &data);

    if (data.mBestMatch) {
        nsRefPtr<gfxFT2Font> font =
            gfxFT2Font::GetOrMakeFont(static_cast<FontEntry*>(data.mBestMatch.get()),
                                      aFont->GetStyle());
        gfxFont* ret = font.forget().get();
        return already_AddRefed<gfxFont>(ret);
    }

    // no match? add to set of non-matching codepoints
    gCodepointsWithNoFonts->set(aCh);

    return nsnull;
}

PRBool
gfxAmigaOSPlatform::GetPrefFontEntries(const nsCString& aKey, nsTArray<nsRefPtr<gfxFontEntry> > *aFontEntryList)
{
    return gPrefFonts->Get(aKey, aFontEntryList);
}

void
gfxAmigaOSPlatform::SetPrefFontEntries(const nsCString& aKey, nsTArray<nsRefPtr<gfxFontEntry> >& aFontEntryList)
{
    gPrefFonts->Put(aKey, aFontEntryList);
}

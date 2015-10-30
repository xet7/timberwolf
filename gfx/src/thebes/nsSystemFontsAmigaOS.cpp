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


#include "nsIDeviceContext.h"
#include "nsSystemFontsAmigaOS.h"

#include <proto/intuition.h>
#include <proto/exec.h>
#include <intuition/intuition.h>
#include <graphics/gfx.h>
#include <graphics/text.h>


static struct Library *s_IntuitionBase = NULL;
static struct IntuitionIFace *s_IIntuition = NULL;

nsSystemFontsAmigaOS::nsSystemFontsAmigaOS()
{
	s_IntuitionBase = IExec->OpenLibrary("intuition.library", 0L);
	s_IIntuition = (struct IntuitionIFace *)IExec->GetInterface(s_IntuitionBase, "main", 1, NULL);

	GetSystemFontInfo("Default", &mDefaultFontName, &mDefaultFontStyle);
	GetSystemFontInfo("Menu", &mMenuFontName, &mMenuFontStyle);
	GetSystemFontInfo("Caption", &mCaptionFontName, &mCaptionFontStyle);

	IExec->DropInterface((struct Interface *)s_IIntuition);
	IExec->CloseLibrary(s_IntuitionBase);
}

nsresult
nsSystemFontsAmigaOS::GetSystemFontInfo(const char *aClassName, nsString *aFontName,
                                        gfxFontStyle *aFontStyle) const
{
	float fontSize = 16.0;
	uint32 fontFlags = 0;

	struct Screen *scr = s_IIntuition->LockPubScreen(NULL);
	struct Window *win = s_IIntuition->OpenWindowTags(NULL,
			WA_Left, 	10,
			WA_Top,		10,
			WA_Width,	10,
			WA_Height,	10,
			WA_Hidden,	TRUE,
		TAG_DONE);

	if (win)
	{
		struct TextFont *font = win->RPort->Font;

		fontSize = (float)font->tf_YSize;
		fontFlags = font->tf_Style;

		s_IIntuition->CloseWindow(win);
	};

	s_IIntuition->UnlockPubScreen(NULL, scr);

    aFontStyle->style = (fontFlags & FSF_ITALIC) ? NS_FONT_STYLE_ITALIC : NS_FONT_STYLE_NORMAL;
    aFontStyle->systemFont = PR_TRUE;
    *aFontName = NS_LITERAL_STRING("sans-serif");
    aFontStyle->weight = (fontFlags & FSF_BOLD) ? NS_FONT_WEIGHT_BOLD : NS_FONT_WEIGHT_NORMAL;
    aFontStyle->stretch = (fontFlags & FSF_EXTENDED) ?NS_FONT_STRETCH_EXPANDED : NS_FONT_STRETCH_NORMAL;
    aFontStyle->size = fontSize * 0.8;
    return NS_OK;
}

nsresult nsSystemFontsAmigaOS::GetSystemFont(nsSystemFontID aID,
                                          nsString *aFontName,
                                          gfxFontStyle *aFontStyle) const
{
  switch (aID) {
    case eSystemFont_PullDownMenu:
    case eSystemFont_Menu:
      *aFontName = mMenuFontName;
      *aFontStyle = mMenuFontStyle;
      return NS_OK;
    case eSystemFont_Caption:             // css2 bold
      *aFontName = mCaptionFontName;
      *aFontStyle = mCaptionFontStyle;
      return NS_OK;
    case eSystemFont_List:
    case eSystemFont_Field:
    case eSystemFont_Icon :
    case eSystemFont_MessageBox :
    case eSystemFont_SmallCaption :
    case eSystemFont_StatusBar :
    case eSystemFont_Window:              // css3
    case eSystemFont_Document:
    case eSystemFont_Workspace:
    case eSystemFont_Desktop:
    case eSystemFont_Info:
    case eSystemFont_Dialog:
    case eSystemFont_Button:
    case eSystemFont_Tooltips:            // moz
    case eSystemFont_Widget:
    default:
      *aFontName = mDefaultFontName;
      *aFontStyle = mDefaultFontStyle;
      return NS_OK;
  }
  NS_NOTREACHED("huh?");
}


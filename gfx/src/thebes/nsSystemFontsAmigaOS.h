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


#ifndef _NS_SYSTEMFONTSAMIGAOS_H_
#define _NS_SYSTEMFONTSAMIGAOS_H_

#include <gfxFont.h>

class nsSystemFontsAmigaOS
{
public:
    nsSystemFontsAmigaOS();

    nsresult GetSystemFont(nsSystemFontID anID, nsString *aFontName,
                           gfxFontStyle *aFontStyle) const;

private:
    nsresult GetSystemFontInfo(const char *aClassName, nsString *aFontName,
                              gfxFontStyle *aFontStyle) const;

    /*
     * The following system font constants exist:
     *
     * css2: http://www.w3.org/TR/REC-CSS2/fonts.html#x27
     * eSystemFont_Caption, eSystemFont_Icon, eSystemFont_Menu,
     * eSystemFont_MessageBox, eSystemFont_SmallCaption,
     * eSystemFont_StatusBar,
     * // css3
     * eSystemFont_Window, eSystemFont_Document,
     * eSystemFont_Workspace, eSystemFont_Desktop,
     * eSystemFont_Info, eSystemFont_Dialog,
     * eSystemFont_Button, eSystemFont_PullDownMenu,
     * eSystemFont_List, eSystemFont_Field,
     * // moz
     * eSystemFont_Tooltips, eSystemFont_Widget
     */
    nsString mDefaultFontName, mMenuFontName, mCaptionFontName;
    gfxFontStyle mDefaultFontStyle, mMenuFontStyle, mCaptionFontStyle;
};

#endif /* _NS_SYSTEMFONTSAMIGAOS_H_ */

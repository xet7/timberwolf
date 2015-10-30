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


#include "nsLookAndFeel.h"
#include "nsFont.h"
#include "nsSize.h"
#include <proto/intuition.h>
#include <intuition/screens.h>

extern struct IntuitionIFace *IIntuition;

//#define NS_LOOKANDFEEL_DEBUG 1



void IntuitionColorMap::Init(struct Screen *scr)
{
	if (mInitialized)
		return;

	uint32 cm[1+3*256];

	cm[0] = 256 << 16;

	int32 res = IIntuition->GetScreenAttr(scr, SA_Colors32, cm, sizeof(cm));


	for (int i = 0; i < 256; i++)
	{
		mColorArray[i] = NS_RGB(cm[1+3*i] >> 24, cm[1+3*i+1] >> 24, cm[1+3*i+2] >> 24);
		//printf("0x%08lx 0x%08lx 0x%08lx ---> 0x%08lx\n", cm[1+3*i], cm[1+3*i+1], cm[1+3*i+2], mColorArray[i]);
	}

	mInitialized = true;
}

nsLookAndFeel::nsLookAndFeel() : nsXPLookAndFeel()
{
	struct Screen *scr = IIntuition->LockPubScreen(NULL);
	mColorMap.Init(scr);
	IIntuition->UnlockPubScreen(NULL, scr);
}

nsLookAndFeel::~nsLookAndFeel()
{
}

#ifdef NS_LOOKANDFEEL_DEBUG
static const char *colorToString[] =
{
	    "eColor_WindowBackground",
	    "eColor_WindowForeground",
	    "eColor_WidgetBackground",
	    "eColor_WidgetForeground",
	    "eColor_WidgetSelectBackground",
	    "eColor_WidgetSelectForeground",
	    "eColor_Widget3DHighlight",
	    "eColor_Widget3DShadow",
	    "eColor_TextBackground",
	    "eColor_TextForeground",
	    "eColor_TextSelectBackground",
	    "eColor_TextSelectForeground",
	    "eColor_TextSelectBackgroundDisabled",
	    "eColor_TextSelectBackgroundAttention",
	    "eColor_TextHighlightBackground",
	    "eColor_TextHighlightForeground",

	    "eColor_IMERawInputBackground",
	    "eColor_IMERawInputForeground",
	    "eColor_IMERawInputUnderline",
	    "eColor_IMESelectedRawTextBackground",
	    "eColor_IMESelectedRawTextForeground",
	    "eColor_IMESelectedRawTextUnderline",
	    "eColor_IMEConvertedTextBackground",
	    "eColor_IMEConvertedTextForeground",
	    "eColor_IMEConvertedTextUnderline",
	    "eColor_IMESelectedConvertedTextBackground",
	    "eColor_IMESelectedConvertedTextForeground",
	    "eColor_IMESelectedConvertedTextUnderline",

	    "eColor_SpellCheckerUnderline",

	    "eColor_activeborder",
	    "eColor_activecaption",
	    "eColor_appworkspace",
	    "eColor_background",
	    "eColor_buttonface",
	    "eColor_buttonhighlight",
	    "eColor_buttonshadow",
	    "eColor_buttontext",
	    "eColor_captiontext",
	    "eColor_graytext",
	    "eColor_highlight",
	    "eColor_highlighttext",
	    "eColor_inactiveborder",
	    "eColor_inactivecaption",
	    "eColor_inactivecaptiontext",
	    "eColor_infobackground",
	    "eColor_infotext",
	    "eColor_menu",
	    "eColor_menutext",
	    "eColor_scrollbar",
	    "eColor_threeddarkshadow",
	    "eColor_threedface",
	    "eColor_threedhighlight",
	    "eColor_threedlightshadow",
	    "eColor_threedshadow",
	    "eColor_window",
	    "eColor_windowframe",
	    "eColor_windowtext",

	    "eColor__moz_buttondefault",
	    "eColor__moz_field",
	    "eColor__moz_fieldtext",
	    "eColor__moz_dialog",
	    "eColor__moz_dialogtext",
	    "eColor__moz_dragtargetzone",

	    "eColor__moz_cellhighlight",
	    "eColor__moz_cellhighlighttext",
	    "eColor__moz_html_cellhighlight",
	    "eColor__moz_html_cellhighlighttext",
	    "eColor__moz_buttonhoverface",
	    "eColor__moz_buttonhovertext",
	    "eColor__moz_menuhover",
	    "eColor__moz_menuhovertext",
	    "eColor__moz_menubartext",
	    "eColor__moz_menubarhovertext",

	    "eColor__moz_eventreerow",
	    "eColor__moz_oddtreerow",

	    "eColor__moz_mac_chrome_active",
	    "eColor__moz_mac_chrome_inactive",
	    "eColor__moz_mac_focusring",
	    "eColor__moz_mac_menuselect",
	    "eColor__moz_mac_menushadow",
	    "eColor__moz_mac_menutextdisable",
	    "eColor__moz_mac_menutextselect",
	    "eColor__moz_mac_disabledtoolbartext",

	    "eColor__moz_mac_alternateprimaryhighlight",
	    "eColor__moz_mac_secondaryhighlight",

	    "eColor__moz_win_mediatext",
	    "eColor__moz_win_communicationstext",

	    "eColor__moz_nativehyperlinktext",

	    "eColor__moz_comboboxtext",
	    "eColor__moz_combobox",

	    "eColor_LAST_COLOR"
};
#endif

nsresult nsLookAndFeel::NativeGetColor(const nsColorID aID, nscolor &aColor)
{
	nsresult res = NS_OK;
	struct Screen *scr = IIntuition->LockPubScreen(NULL);
	struct DrawInfo *dri = IIntuition->GetScreenDrawInfo(scr);
	int32 pen = -1;

	switch (aID) {
	case eColor_WindowBackground:
		aColor = NS_RGB(0xff, 0xff, 0xff);
		pen = BACKGROUNDPEN;
		break;
	case eColor_WindowForeground:
		aColor = NS_RGB(0x00, 0x00, 0x00);
		pen = TEXTPEN;
		break;
	case eColor_WidgetBackground:
		aColor = NS_RGB(0xdd, 0xdd, 0xdd);
		pen = FOREGROUNDPEN;
		break;
	case eColor_WidgetForeground:
		aColor = NS_RGB(0x00, 0x00, 0x00);
		pen = GADGETPEN;
		break;
	case eColor_WidgetSelectBackground:
		aColor = NS_RGB(0xdd, 0xdd, 0xdd);
		pen = SELECTPEN;
		break;
	case eColor_WidgetSelectForeground:
		aColor = NS_RGB(0x00, 0x00, 0x80);
		pen = SELECTTEXTPEN;
		break;
	case eColor_Widget3DHighlight:
		aColor = NS_RGB(0xa0, 0xa0, 0xa0);
		pen = SHINEPEN;
		break;
	case eColor_Widget3DShadow:
		aColor = NS_RGB(0x40, 0x40, 0x40);
		pen = SHADOWPEN;
		break;
	case eColor_TextBackground:
		aColor = NS_RGB(0xff, 0xff, 0xff);
		pen = BACKGROUNDPEN;
		break;
	case eColor_TextForeground:
		aColor = NS_RGB(0x00, 0x00, 0x00);
		pen = TEXTPEN;
		break;
	case eColor_TextSelectBackground:
	case eColor_IMESelectedRawTextBackground:
	case eColor_IMESelectedConvertedTextBackground:
		aColor = NS_RGB(0xdd, 0xdd, 0xdd);
		pen = SELECTPEN;
		break;
	case eColor_TextSelectForeground:
	case eColor_IMESelectedRawTextForeground:
	case eColor_IMESelectedConvertedTextForeground:
		aColor = NS_RGB(0x00, 0x00, 0x00);
		pen = SELECTTEXTPEN;
		break;
	case eColor_IMERawInputBackground:
	case eColor_IMEConvertedTextBackground:
		aColor = NS_TRANSPARENT;
		break;
	case eColor_IMERawInputForeground:
	case eColor_IMEConvertedTextForeground:
		aColor = NS_SAME_AS_FOREGROUND_COLOR;
		break;
	case eColor_IMERawInputUnderline:
	case eColor_IMEConvertedTextUnderline:
		aColor = NS_SAME_AS_FOREGROUND_COLOR;
		break;
	case eColor_IMESelectedRawTextUnderline:
	case eColor_IMESelectedConvertedTextUnderline:
		aColor = NS_TRANSPARENT;
		break;
	case eColor_activeborder:
		aColor = NS_RGB(0x88, 0x88, 0x88);
		pen = FILLPEN;
		break;
	case eColor_activecaption:
		aColor = NS_RGB(0x88, 0x88, 0x88);
		pen = FILLPEN;
		break;
	break;
	case eColor_appworkspace:
		aColor = NS_RGB(0xd8, 0xd8, 0xd8);
		pen = BACKGROUNDPEN;
		break;
	case eColor_background:
		aColor = NS_RGB(0xdd, 0xdd, 0xdd);
		pen = BACKGROUNDPEN;
		break;
	case eColor_buttonface:
	case eColor__moz_buttonhoverface:
		aColor = NS_RGB(0xdd, 0xdd, 0xdd);
		pen = FOREGROUNDPEN;
		break;
	case eColor_buttonhighlight:
		aColor = NS_RGB(0xff, 0xff, 0xff);
		pen = SHINEPEN;
		break;
	case eColor_buttonshadow:
		aColor = NS_RGB(0x77, 0x77, 0x77);
		pen = SHADOWPEN;
		break;
	case eColor_buttontext:
	case eColor__moz_buttonhovertext:
		aColor = NS_RGB(0x00, 0x00, 0x00);
		pen = TEXTPEN;
		break;
	case eColor_captiontext:
		aColor = NS_RGB(0x00, 0x00, 0x00);
		pen = FILLTEXTPEN;
		break;
	case eColor_graytext:
		aColor = NS_RGB(0x77, 0x77, 0x77);
		pen = DISABLEDTEXTPEN;
		break;
	case eColor_highlight:
	case eColor__moz_html_cellhighlight:
	case eColor__moz_menuhover:
		aColor = NS_RGB(0x60, 0x60, 0xf0);
		pen = FILLPEN;
		break;
	case eColor_highlighttext:
	case eColor__moz_html_cellhighlighttext:
	case eColor__moz_menuhovertext:
		aColor = NS_RGB(0x00, 0x00, 0x00);
		pen = FILLTEXTPEN;
		break;
	case eColor_inactiveborder:
		aColor = NS_RGB(0x55, 0x55, 0x55);
		pen = INACTIVEFILLPEN;
		break;
	case eColor_inactivecaption:
		aColor = NS_RGB(0xdd, 0xdd, 0xdd);
		pen = INACTIVEFILLPEN;
		break;
	case eColor_inactivecaptiontext:
		aColor = NS_RGB(0x77, 0x77, 0x77);
		pen = INACTIVEFILLTEXTPEN;
		break;
	case eColor_infobackground:
		aColor = NS_RGB(0xff, 0xff, 0xd0);
		pen = -1; // No real Amiga-like color
		break;
	case eColor_infotext:
		aColor = NS_RGB(0x00, 0x00, 0x00);
		pen = -1; // No real Amiga-like color
		break;
	case eColor_menu:
		aColor = NS_RGB(0xdd, 0xdd, 0xdd);
		pen = MENUBACKGROUNDPEN;
		break;
	case eColor_menutext:
		aColor = NS_RGB(0x00, 0x00, 0x00);
		pen = MENUTEXTPEN;
		break;
	case eColor_scrollbar:
		aColor = NS_RGB(0xaa, 0xaa, 0xaa);
		pen = FOREGROUNDPEN;
		break;
	case eColor_threeddarkshadow:
		aColor = NS_RGB(0x77, 0x77, 0x77);
		pen = SHADOWPEN;
		break;
	case eColor_threedface:
		aColor = NS_RGB(0xdd, 0xdd, 0xdd);
		pen = FOREGROUNDPEN;
		break;
	case eColor_threedhighlight:
		aColor = NS_RGB(0xff, 0xff, 0xff);
		pen = SHINEPEN;
		break;
	case eColor_threedlightshadow:
		aColor = NS_RGB(0xdd, 0xdd, 0xdd);
		pen = SHINEPEN;
		break;
	case eColor_threedshadow:
		aColor = NS_RGB(0x99, 0x99, 0x99);
		pen = SHADOWPEN;
		break;
	case eColor_window:
		aColor = NS_RGB(0xff, 0xff, 0xff);
		pen = BACKGROUNDPEN;
		break;
	case eColor_windowframe:
		aColor = NS_RGB(0xcc, 0xcc, 0xcc);
		pen = FILLFLATPEN;
		break;
	case eColor_windowtext:
		aColor = NS_RGB(0x00, 0x00, 0x00);
		pen = TEXTPEN;
		break;
	case eColor__moz_eventreerow:
		aColor = NS_RGB(0xff, 0xff, 0xff);
		pen = HALFSHINEPEN;
		break;
	case eColor__moz_oddtreerow:
		aColor = NS_RGB(0xff, 0xff, 0xff);
		pen = HALFSHADOWPEN;
		break;
	case eColor__moz_field:
		aColor = NS_RGB(0xff, 0xff, 0xff);
		pen = -1; //BACKGROUNDPEN;
		break;
	case eColor__moz_fieldtext:
		aColor = NS_RGB(0x00, 0x00, 0x00);
		pen = -1; //TEXTPEN;
		break;
	case eColor__moz_dialog:
		aColor = NS_RGB(0xdd, 0xdd, 0xdd);
		pen = BACKGROUNDPEN;
		break;
	case eColor__moz_cellhighlight:
		aColor = NS_RGB(0xdd, 0xdd, 0xdd);
		pen = FILLPEN;
		break;
	case eColor__moz_dialogtext:
		aColor = NS_RGB(0x00, 0x00, 0x00);
		pen = TEXTPEN;
		break;
	case eColor__moz_cellhighlighttext:
		aColor = NS_RGB(0x00, 0x00, 0x00);
		pen = FILLTEXTPEN;
		break;
	case eColor__moz_dragtargetzone:
		aColor = NS_RGB(0x63, 0x63, 0xCE);
		pen = -1;
		break;
	case eColor__moz_buttondefault:
		aColor = NS_RGB(0x77, 0x77, 0x77);
		break;

	case eColor__moz_menubartext:
		aColor = NS_RGB(0x00, 0x00, 0x00);
		pen = TEXTPEN;
		break;
	case eColor__moz_menubarhovertext:
		aColor = NS_RGB(0x00, 0x00, 0x00);
		pen = TEXTPEN;
		break;
	case eColor__moz_nativehyperlinktext:
		aColor = NS_RGB(0x00, 0x00, 0x00);
		pen = HIGHLIGHTTEXTPEN;
		break;
	case eColor__moz_comboboxtext:
		aColor = NS_RGB(0x00, 0x00, 0x00);
		pen = TEXTPEN;
		break;
	case eColor__moz_combobox:
		aColor = NS_RGB(0xff, 0xff, 0xff);
		pen = BACKGROUNDPEN;
		break;
	case eColor_LAST_COLOR:
	default:
		aColor = NS_RGB(0xff, 0xff, 0xff);
		res    = NS_ERROR_FAILURE;
		break;
	}

	if (pen != -1)
	{
		if (dri->dri_NumPens >= pen)
		{
			aColor = mColorMap[dri->dri_Pens[pen]];
		}
	}

#ifdef NS_LOOKANDFEEL_DEBUG
	printf("Using %p for color %s\n", aColor, colorToString[aID]);
#endif

	IIntuition->FreeScreenDrawInfo(scr, dri);
	IIntuition->UnlockPubScreen(NULL, scr);

	return res;
}

#ifdef NS_LOOKANDFEEL_DEBUG
static const char *metricToString[] = {
    "eMetric_WindowTitleHeight",
    "eMetric_WindowBorderWidth",
    "eMetric_WindowBorderHeight",
    "eMetric_Widget3DBorder",
    "eMetric_TextFieldBorder",
    "eMetric_TextFieldHeight",
    "eMetric_TextVerticalInsidePadding",
    "eMetric_TextShouldUseVerticalInsidePadding",
    "eMetric_TextHorizontalInsideMinimumPadding",
    "eMetric_TextShouldUseHorizontalInsideMinimumPadding",
    "eMetric_ButtonHorizontalInsidePaddingNavQuirks",
    "eMetric_ButtonHorizontalInsidePaddingOffsetNavQuirks",
    "eMetric_CheckboxSize",
    "eMetric_RadioboxSize",
    "eMetric_ListShouldUseHorizontalInsideMinimumPadding",
    "eMetric_ListHorizontalInsideMinimumPadding",
    "eMetric_ListShouldUseVerticalInsidePadding",
    "eMetric_ListVerticalInsidePadding",
    "eMetric_CaretBlinkTime",
    "eMetric_CaretWidth",
    "eMetric_ShowCaretDuringSelection",
    "eMetric_SelectTextfieldsOnKeyFocus",
    "eMetric_SubmenuDelay",
    "eMetric_MenusCanOverlapOSBar",
    "eMetric_SkipNavigatingDisabledMenuItem",
    "eMetric_DragFullWindow",
    "eMetric_DragThresholdX",
    "eMetric_DragThresholdY",
    "eMetric_UseAccessibilityTheme",
    "eMetric_IsScreenReaderActive",
    "eMetric_ScrollArrowStyle",
    "eMetric_ScrollSliderStyle",
    "eMetric_ScrollButtonLeftMouseButtonAction",
    "eMetric_ScrollButtonMiddleMouseButtonAction",
    "eMetric_ScrollButtonRightMouseButtonAction",
    "eMetric_TreeOpenDelay",
    "eMetric_TreeCloseDelay",
    "eMetric_TreeLazyScrollDelay",
    "eMetric_TreeScrollDelay",
    "eMetric_TreeScrollLinesMax",
    "eMetric_TabFocusModel",
    "eMetric_WindowsDefaultTheme",
    "eMetric_AlertNotificationOrigin",
    "eMetric_ScrollToClick",
    "eMetric_IMERawInputUnderlineStyle",
    "eMetric_IMESelectedRawTextUnderlineStyle",
    "eMetric_IMEConvertedTextUnderlineStyle",
    "eMetric_IMESelectedConvertedTextUnderline",
    "eMetric_ImagesInMenus"
    };
#endif


NS_IMETHODIMP nsLookAndFeel::GetMetric(const nsMetricID aID, PRInt32 & aMetric)
{
#ifdef NS_LOOKANDFEEL_DEBUG
	printf("nsLookAndFeel::GetMetric (int version) aID = %s", metricToString[aID]);
#endif

    // Set these before they can get overridden in the nsXPLookAndFeel.
    switch (aID) {
    case eMetric_ScrollButtonLeftMouseButtonAction:
        aMetric = 0;
        return NS_OK;
    case eMetric_ScrollButtonMiddleMouseButtonAction:
        aMetric = 1;
        return NS_OK;
    case eMetric_ScrollButtonRightMouseButtonAction:
        aMetric = 2;
        return NS_OK;
    default:
        break;
    }

	nsresult res = nsXPLookAndFeel::GetMetric(aID, aMetric);
	if (NS_SUCCEEDED(res))
		return res;

	res = NS_OK;


	switch (aID)
	{
	case eMetric_CaretBlinkTime:
		aMetric = 500;
		break;
	case eMetric_CaretWidth:
		aMetric = 1;
		break;
	case eMetric_ShowCaretDuringSelection:
		aMetric = 1;
		break;
	case eMetric_SelectTextfieldsOnKeyFocus:
		aMetric = 0;
		break;
	case eMetric_SubmenuDelay:
		aMetric = 500;
		break;
	case eMetric_MenusCanOverlapOSBar:
		aMetric = 0;
		break;
	case eMetric_ScrollArrowStyle:
		aMetric = eMetric_ScrollArrowStyleSingle;
		break;
	case eMetric_ScrollSliderStyle:
		aMetric = eMetric_ScrollThumbStyleProportional;
		break;
	case eMetric_TreeOpenDelay:
		aMetric = 1000;
		break;
	case eMetric_TreeCloseDelay:
		aMetric = 1000;
		break;
	case eMetric_TreeLazyScrollDelay:
		aMetric = 150;
		break;
	case eMetric_TreeScrollDelay:
		aMetric = 100;
		break;
	case eMetric_TreeScrollLinesMax:
		aMetric = 3;
		break;
	case eMetric_IMERawInputUnderlineStyle:
	case eMetric_IMEConvertedTextUnderlineStyle:
		aMetric = NS_UNDERLINE_STYLE_SOLID;
		break;
	case eMetric_IMESelectedRawTextUnderlineStyle:
	case eMetric_IMESelectedConvertedTextUnderline:
		aMetric = NS_UNDERLINE_STYLE_NONE;
		break;
	case eMetric_DragThresholdX:
	case eMetric_DragThresholdY:
		aMetric = 5;
		break;
	case eMetric_SkipNavigatingDisabledMenuItem:
	case eMetric_ImagesInMenus:
	case eMetric_ImagesInButtons:
		aMetric = 1;
		break;
	case eMetric_ScrollbarsCanOverlapContent:
	case eMetric_MenuBarDrag:
		aMetric = 0;
		break;
	case eMetric_UseAccessibilityTheme:
	case eMetric_WindowsDefaultTheme:
	case eMetric_DWMCompositor:
	case eMetric_WindowsClassic:
	case eMetric_TouchEnabled:
	case eMetric_MacGraphiteTheme:
	case eMetric_MaemoClassic:
	case eMetric_WindowsThemeIdentifier:
		aMetric = 0;
		res = NS_ERROR_NOT_IMPLEMENTED;
		break;
	case eMetric_SpellCheckerUnderlineStyle:
		aMetric = NS_UNDERLINE_STYLE_WAVY;
		break;
//	case eMetric_TabFocusModel:
	case eMetric_ChosenMenuItemsShouldBlink:
		aMetric = 1;
		break;
//	case eMetric_AlertNotificationOrigin:
//	case eMetric_ScrollToClick:
//
//		break;
	default:
//#ifdef NS_LOOKANDFEEL_DEBUG
		printf("GetMetric: %d not found\n", aID);
//#endif
		aMetric = 0;
		res = NS_ERROR_FAILURE;
	}
#ifdef NS_LOOKANDFEEL_DEBUG
	printf("  returns %d (res %d)\n", aMetric, res);
#endif
	return res;
}

#ifdef NS_LOOKANDFEEL_DEBUG
static const char *floatMetricToString[] = {
    "eMetricFloat_TextFieldVerticalInsidePadding",
    "eMetricFloat_TextFieldHorizontalInsidePadding",
    "eMetricFloat_TextAreaVerticalInsidePadding",
    "eMetricFloat_TextAreaHorizontalInsidePadding",
    "eMetricFloat_ListVerticalInsidePadding",
    "eMetricFloat_ListHorizontalInsidePadding",
    "eMetricFloat_ButtonVerticalInsidePadding",
    "eMetricFloat_ButtonHorizontalInsidePadding",
    "eMetricFloat_IMEUnderlineRelativeSize",
    "eMetricFloat_CaretAspectRatio"
};
#endif

NS_IMETHODIMP nsLookAndFeel::GetMetric(const nsMetricFloatID aID, float & aMetric)
{
#ifdef NS_LOOKANDFEEL_DEBUG
	printf("nsLookAndFeel::GetMetric (float version) aID = %s", floatMetricToString[aID]);
#endif
	nsresult res = nsXPLookAndFeel::GetMetric(aID, aMetric);
	if (NS_SUCCEEDED(res))
		return res;

	res = NS_OK;

	switch (aID) {
	case eMetricFloat_IMEUnderlineRelativeSize:
		aMetric = 1.0f;
		break;
	/*case eMetricFloat_CaretAspectRatio:
		aMetric = 1.0f;
		break;*/
	default:
#ifdef NS_LOOKANDFEEL_DEBUG
		printf("Unknown metric %d\n", aID);
#endif
		aMetric = -1.0;
		res = NS_ERROR_FAILURE;
	}
#ifdef NS_LOOKANDFEEL_DEBUG
	printf("  returns float value %f (res %d)\n", aMetric, res);
#endif
	return res;
}

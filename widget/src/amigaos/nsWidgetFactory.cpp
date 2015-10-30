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


#include "mozilla/ModuleUtils.h"
#include "nsIModule.h"
#include "nsCOMPtr.h"
#include "nsWidgetsCID.h"

#include "nsWindow.h"
#include "nsPopupWindow.h"
#include "nsChildView.h"
#include "nsSound.h"
#include "nsToolkit.h"
#include "nsAppShell.h"
#include "nsAppShellSingleton.h"
#include "nsLookAndFeel.h"
#include "nsFilePicker.h"
#include "nsBidiKeyboard.h"
#include "nsScreenManagerAmiga.h"
#include "nsIdleServiceAmiga.h"
// Printing:
// aka    nsDeviceContextSpecAmiga.h
#include "nsDeviceContextSpecAmiga.h"
#include "nsPrintOptionsAmiga.h"
#include "nsPrintSession.h"

// Drag & Drop, Clipboard
#include "nsTransferable.h"
#include "nsClipboard.h"
#include "nsClipboardHelper.h"
#include "nsHTMLFormatConverter.h"
#include "nsDragService.h"

#include "nsIPrefService.h"
#include "nsIPrefBranch.h"

//
//NS_GENERIC_FACTORY_CONSTRUCTOR(nsWindow)
//NS_GENERIC_FACTORY_CONSTRUCTOR(nsPopupWindow)
//NS_GENERIC_FACTORY_CONSTRUCTOR(nsChildView)
//NS_GENERIC_FACTORY_CONSTRUCTOR(nsToolkit)
//NS_GENERIC_FACTORY_CONSTRUCTOR(nsLookAndFeel)
//NS_GENERIC_FACTORY_CONSTRUCTOR(nsTransferable)
//NS_GENERIC_FACTORY_CONSTRUCTOR(nsClipboard)
//NS_GENERIC_FACTORY_CONSTRUCTOR(nsClipboardHelper)
//NS_GENERIC_FACTORY_CONSTRUCTOR(nsHTMLFormatConverter)
//NS_GENERIC_FACTORY_CONSTRUCTOR(nsDragService)
//NS_GENERIC_FACTORY_CONSTRUCTOR(nsSound)
//NS_GENERIC_FACTORY_CONSTRUCTOR(nsFilePicker)
//NS_GENERIC_FACTORY_CONSTRUCTOR(nsBidiKeyboard)
//NS_GENERIC_FACTORY_CONSTRUCTOR(nsScreenManagerAmiga)
//
//NS_GENERIC_FACTORY_CONSTRUCTOR(nsDeviceContextSpecAmiga)
//NS_GENERIC_FACTORY_CONSTRUCTOR_INIT(nsPrintOptionsAmiga, Init)
//NS_GENERIC_FACTORY_CONSTRUCTOR_INIT(nsPrintSession, Init)
////NS_GENERIC_FACTORY_CONSTRUCTOR(nsPrinterEnumeratorAmiga)


NS_GENERIC_FACTORY_CONSTRUCTOR(nsWindow)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsChildView)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsPopupWindow)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsToolkit)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsHTMLFormatConverter)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsTransferable)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsLookAndFeel)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsClipboard)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsClipboardHelper)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsDragService)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsBidiKeyboard)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsSound)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsScreenManagerAmiga)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsIdleServiceAmiga)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsFilePicker)

NS_DEFINE_NAMED_CID(NS_WINDOW_CID);
NS_DEFINE_NAMED_CID(NS_CHILD_CID);
NS_DEFINE_NAMED_CID(NS_APPSHELL_CID);
NS_DEFINE_NAMED_CID(NS_LOOKANDFEEL_CID);
NS_DEFINE_NAMED_CID(NS_FILEPICKER_CID);
NS_DEFINE_NAMED_CID(NS_SOUND_CID);
NS_DEFINE_NAMED_CID(NS_TRANSFERABLE_CID);
NS_DEFINE_NAMED_CID(NS_CLIPBOARD_CID);
NS_DEFINE_NAMED_CID(NS_CLIPBOARDHELPER_CID);
NS_DEFINE_NAMED_CID(NS_DRAGSERVICE_CID);
NS_DEFINE_NAMED_CID(NS_HTMLFORMATCONVERTER_CID);
NS_DEFINE_NAMED_CID(NS_BIDIKEYBOARD_CID);
NS_DEFINE_NAMED_CID(NS_SCREENMANAGER_CID);
NS_DEFINE_NAMED_CID(NS_THEMERENDERER_CID);
NS_DEFINE_NAMED_CID(NS_IDLE_SERVICE_CID);
NS_DEFINE_NAMED_CID(NS_POPUP_CID);
NS_DEFINE_NAMED_CID(NS_TOOLKIT_CID);
#ifdef NS_PRINTING
NS_DEFINE_NAMED_CID(NS_PRINTSETTINGSSERVICE_CID);
NS_DEFINE_NAMED_CID(NS_PRINTER_ENUMERATOR_CID);
NS_DEFINE_NAMED_CID(NS_PRINTSESSION_CID);
NS_DEFINE_NAMED_CID(NS_DEVICE_CONTEXT_SPEC_CID);
NS_DEFINE_NAMED_CID(NS_PRINTDIALOGSERVICE_CID);
#endif



static const mozilla::Module::CIDEntry kWidgetCIDs[] = {
    { &kNS_WINDOW_CID, false, NULL, nsWindowConstructor },
    { &kNS_CHILD_CID, false, NULL, nsChildViewConstructor },
    { &kNS_APPSHELL_CID, false, NULL, nsAppShellConstructor },
    { &kNS_LOOKANDFEEL_CID, false, NULL, nsLookAndFeelConstructor },
    { &kNS_FILEPICKER_CID, false, NULL, nsFilePickerConstructor },
    { &kNS_SOUND_CID, false, NULL, nsSoundConstructor },
    { &kNS_TRANSFERABLE_CID, false, NULL, nsTransferableConstructor },
    { &kNS_CLIPBOARD_CID, false, NULL, nsClipboardConstructor },
    { &kNS_CLIPBOARDHELPER_CID, false, NULL, nsClipboardHelperConstructor },
    { &kNS_DRAGSERVICE_CID, false, NULL, nsDragServiceConstructor },
    { &kNS_HTMLFORMATCONVERTER_CID, false, NULL, nsHTMLFormatConverterConstructor },
    { &kNS_BIDIKEYBOARD_CID, false, NULL, nsBidiKeyboardConstructor },
    { &kNS_SCREENMANAGER_CID, false, NULL, nsScreenManagerAmigaConstructor },
    { &kNS_IDLE_SERVICE_CID, false, NULL, nsIdleServiceAmigaConstructor },
    { &kNS_POPUP_CID, false, NULL, nsPopupWindowConstructor },
    { &kNS_TOOLKIT_CID, false, NULL, nsToolkitConstructor },
    { NULL }
};

static const mozilla::Module::ContractIDEntry kWidgetContracts[] = {
    { "@mozilla.org/widget/window/amiga;1", &kNS_WINDOW_CID },
    { "@mozilla.org/widgets/child_window/amiga;1", &kNS_CHILD_CID },
    { "@mozilla.org/widget/appshell/amiga;1", &kNS_APPSHELL_CID },
    { "@mozilla.org/widget/lookandfeel;1", &kNS_LOOKANDFEEL_CID },
    { "@mozilla.org/filepicker;1", &kNS_FILEPICKER_CID },
    { "@mozilla.org/sound;1", &kNS_SOUND_CID },
    { "@mozilla.org/widget/transferable;1", &kNS_TRANSFERABLE_CID },
    { "@mozilla.org/widget/clipboard;1", &kNS_CLIPBOARD_CID },
    { "@mozilla.org/widget/clipboardhelper;1", &kNS_CLIPBOARDHELPER_CID },
    { "@mozilla.org/widget/dragservice;1", &kNS_DRAGSERVICE_CID },
    { "@mozilla.org/widget/htmlformatconverter;1", &kNS_HTMLFORMATCONVERTER_CID },
    { "@mozilla.org/widget/bidikeyboard;1", &kNS_BIDIKEYBOARD_CID },
    { "@mozilla.org/gfx/screenmanager;1", &kNS_SCREENMANAGER_CID },
//    { "@mozilla.org/chrome/chrome-native-theme;1", &kNS_THEMERENDERER_CID },
    { "@mozilla.org/widget/idleservice;1", &kNS_IDLE_SERVICE_CID },
    { "@mozilla.org/widgets/popup_window/amiga;1", &kNS_POPUP_CID },
    { "@mozilla.org/widget/toolkit/amiga;1", &kNS_TOOLKIT_CID },
#ifdef NS_PRINTING
    { "@mozilla.org/gfx/printsettings-service;1", &kNS_PRINTSETTINGSSERVICE_CID },
    { "@mozilla.org/gfx/printerenumerator;1", &kNS_PRINTER_ENUMERATOR_CID },
    { "@mozilla.org/gfx/printsession;1", &kNS_PRINTSESSION_CID },
    { "@mozilla.org/gfx/devicecontextspec;1", &kNS_DEVICE_CONTEXT_SPEC_CID },
    { NS_PRINTDIALOGSERVICE_CONTRACTID, &kNS_PRINTDIALOGSERVICE_CID },
#endif
    { NULL }
};

static void
nsWidgetAmigaModuleDtor()
{
  //NYI nsSound::Shutdown();
  //XXX Do we need this ? nsWindow::ReleaseGlobals();
  nsAppShellShutdown();
}

static const mozilla::Module kWidgetModule = {
    mozilla::Module::kVersion,
    kWidgetCIDs,
    kWidgetContracts,
    NULL,
    NULL,
    nsAppShellInit,
    nsWidgetAmigaModuleDtor
};

NSMODULE_DEFN(nsWidgetAmigaOSModule) = &kWidgetModule;

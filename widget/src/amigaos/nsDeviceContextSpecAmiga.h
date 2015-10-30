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


#ifndef nsDeviceContextSpecB_h___
#define nsDeviceContextSpecB_h___

#include "nsCOMPtr.h"
#include "nsIDeviceContextSpec.h"
#include "nsVoidArray.h"
#include "nsIPrintSettings.h"
#include "nsIPrintOptions.h"
// For public interface?
#include "nsIWidget.h"
//#include "nsPrintdAmiga.h"

class nsDeviceContextSpecAmiga : public nsIDeviceContextSpec
{
public:
/**
 * Construct a nsDeviceContextSpecAmiga, which is an object which contains and manages a mac printrecord
 * @update  dc 12/02/98
 */
  nsDeviceContextSpecAmiga();

  NS_DECL_ISUPPORTS

/**
 * Initialize the nsDeviceContextSpecAmiga for use.  This will allocate a printrecord for use
 * @update   dc 2/16/98
 * @param aWidget         Unused
 * @param aPS             Settings for this print job
 * @param aIsPrintPreview Unused
 * @return error status
 */
  NS_IMETHOD Init(nsIWidget *aWidget,
                  nsIPrintSettings* aPS,
                  PRBool aIsPrintPreview);


/**
 * Closes the printmanager if it is open.
 * @update   dc 2/13/98
 * @return error status
 */
  NS_IMETHOD ClosePrintManager();

  NS_IMETHOD GetSurfaceForPrinter(gfxASurface **nativeSurface) {
    return NS_ERROR_NOT_IMPLEMENTED;
  }

/**
 * Destructor for nsDeviceContextSpecAmiga, this will release the printrecord
 * @update  dc 2/16/98
 */
protected:
  virtual ~nsDeviceContextSpecAmiga();

public:
  int InitializeGlobalPrinters();
  void FreeGlobalPrinters();

  NS_IMETHOD BeginDocument(PRUnichar * aTitle, PRUnichar * aPrintToFileName, PRInt32 aStartPage, PRInt32 aEndPage);
  NS_IMETHOD EndDocument();
  NS_IMETHOD BeginPage() { return NS_OK; }
  NS_IMETHOD EndPage() { return NS_OK; }

protected:
  nsCOMPtr<nsIPrintSettings> mPrintSettings;
};

//-------------------------------------------------------------------------
// Printer Enumerator
//-------------------------------------------------------------------------
class nsPrinterEnumeratorAmiga : public nsIPrinterEnumerator
{
public:
  nsPrinterEnumeratorAmiga();
  NS_DECL_ISUPPORTS
  NS_DECL_NSIPRINTERENUMERATOR

protected:
};

#endif

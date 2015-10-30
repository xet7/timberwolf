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


#include "nsIWidget.h"
#include "nsDeviceContextSpecAmiga.h"

#include "nsIPrefService.h"
#include "nsIPrefBranch.h"
#include "prenv.h" /* for PR_GetEnv */
#include "nsIServiceManager.h"
#include "nsReadableUtils.h"
#include "nsStringEnumerator.h"
#include "nsCRT.h"


nsDeviceContextSpecAmiga::nsDeviceContextSpecAmiga()
{
}

nsDeviceContextSpecAmiga::~nsDeviceContextSpecAmiga()
{
}

NS_IMPL_ISUPPORTS1(nsDeviceContextSpecAmiga, nsIDeviceContextSpec)

NS_IMETHODIMP nsDeviceContextSpecAmiga::Init(nsIWidget *aWidget,
                                            nsIPrintSettings* aPS,
                                            PRBool aIsPrintPreview)
{
	NS_NOTYETIMPLEMENTED("nsDeviceContextSpecAmiga::Init");
	return NS_ERROR_FAILURE;
}

/** -------------------------------------------------------
 * Closes the printmanager if it is open.
 *  @update   dc 2/15/98
 */
NS_IMETHODIMP nsDeviceContextSpecAmiga :: ClosePrintManager()
{
	NS_NOTYETIMPLEMENTED("nsDeviceContextSpecAmiga::ClosePrintManager");
	return NS_OK;
}

NS_IMETHODIMP nsDeviceContextSpecAmiga::BeginDocument(PRUnichar * aTitle, PRUnichar * aPrintToFileName,
		PRInt32 aStartPage, PRInt32 aEndPage)
{
	return NS_ERROR_FAILURE;
}


NS_IMETHODIMP nsDeviceContextSpecAmiga::EndDocument()
{
	return NS_ERROR_FAILURE;
}
//  Printer Enumerator
nsPrinterEnumeratorAmiga::nsPrinterEnumeratorAmiga()
{
}

NS_IMPL_ISUPPORTS1(nsPrinterEnumeratorAmiga, nsIPrinterEnumerator)

NS_IMETHODIMP nsPrinterEnumeratorAmiga::GetPrinterNameList(nsIStringEnumerator **aPrinterNameList)
{
	NS_NOTYETIMPLEMENTED("nsPrinterEnumeratorAmiga::GetPrinterNameList");
	return NS_OK;
}

/* readonly attribute wstring defaultPrinterName; */
NS_IMETHODIMP nsPrinterEnumeratorAmiga::GetDefaultPrinterName(PRUnichar * *aDefaultPrinterName)
{
  NS_ENSURE_ARG_POINTER(aDefaultPrinterName);
  *aDefaultPrinterName = nsnull;
  return NS_OK;
}

/* void initPrintSettingsFromPrinter (in wstring aPrinterName, in nsIPrintSettings aPrintSettings); */
NS_IMETHODIMP nsPrinterEnumeratorAmiga::InitPrintSettingsFromPrinter(const PRUnichar *aPrinterName, nsIPrintSettings *aPrintSettings)
{
    return NS_OK;
}

NS_IMETHODIMP nsPrinterEnumeratorAmiga::DisplayPropertiesDlg(const PRUnichar *aPrinter, nsIPrintSettings *aPrintSettings)
{
  return NS_OK;
}





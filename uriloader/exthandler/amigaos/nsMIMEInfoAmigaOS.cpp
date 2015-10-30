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



#include "nsMIMEInfoAmigaOS.h"
#include "nsILocalFile.h"

#include <stdio.h>

nsMIMEInfoAmigaOS::~nsMIMEInfoAmigaOS()
{
}


NS_IMETHODIMP
nsMIMEInfoAmigaOS::GetHasDefaultHandler(PRBool * _retval)
{
	  *_retval = !mDefaultAppDescription.IsEmpty();
//	  printf("nsMIMEInfoAmigaOS::GetHasDefaultHandler (%s) = %s\n", mSchemeOrType.get(),  *_retval ? "true" : "false");

	  return NS_OK;
}

nsresult
nsMIMEInfoAmigaOS::LaunchDefaultWithFile(nsIFile* aFile)
{
//	printf("nsMIMEInfoAmigaOS::LaunchDefaultWithFile\n");
	nsCAutoString nativePath;
	aFile->GetNativePath(nativePath);

	return LaunchWithIProcess(mDefaultApplication, nativePath);
}

nsresult
nsMIMEInfoAmigaOS::LoadUriInternal(nsIURI * aURL)
{
	nsresult rv = NS_OK;
//	printf("nsMIMEInfoAmigaOS::LoadUriInternal\n");
	return rv;
}

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


#include "nsOSHelperAppService.h"
#include "nsMIMEInfoAmigaOS.h"
#include "nsISupports.h"
#include "nsString.h"
#include "nsReadableUtils.h"
#include "nsUnicharUtils.h"
#include "nsXPIDLString.h"
#include "nsIURL.h"
#include "nsIFileStreams.h"
#include "nsILineInputStream.h"
#include "nsILocalFile.h"
#include "nsIProcess.h"
#include "nsIPrefService.h"
#include "nsIPrefBranch.h"
#include "nsNetCID.h"
#include "nsXPCOM.h"
#include "nsISupportsPrimitives.h"
#include "nsHashtable.h"
#include "nsCRT.h"
#include "nsDirectoryServiceDefs.h"
#include "nsDirectoryServiceUtils.h"
#include "prenv.h"      // for PR_GetEnv()
#include "nsAutoPtr.h"

#include "nsAmigaOSMIMETypes.h"

#define LOG(args) PR_LOG(mLog, PR_LOG_DEBUG, args)
#define LOG_ENABLED() PR_LOG_TEST(mLog, PR_LOG_DEBUG)

nsOSHelperAppService::nsOSHelperAppService() : nsExternalHelperAppService()
{

}

nsOSHelperAppService::~nsOSHelperAppService()
{

}

nsresult nsOSHelperAppService::OSProtocolHandlerExists(const char * aProtocolScheme, PRBool * aHandlerExists)
{
	*aHandlerExists = PR_FALSE;
//	printf("No protocol handler for scheme %s\n", aProtocolScheme);
	return NS_OK;
}

already_AddRefed<nsIMIMEInfo>
nsOSHelperAppService::GetMIMEInfoFromOS(const nsACString& aType, const nsACString& aFileExt, PRBool* aFound)
{
//	printf("nsOSHelperAppService::GetMIMEInfoFromOS (%s, %s)\n", nsPromiseFlatCString(aType).get(), nsPromiseFlatCString(aFileExt).get());
	*aFound = PR_TRUE;
	nsMIMEInfoBase* retval = GetFromType(PromiseFlatCString(aType)).get();
	PRBool hasDefault = PR_FALSE;
	if (retval)
		retval->GetHasDefaultHandler(&hasDefault);
	if (!retval || !hasDefault) {
		nsRefPtr<nsMIMEInfoBase> miByExt = GetFromExtension(PromiseFlatCString(aFileExt));

		// If we had no extension match, but a type match, use that
		if (!miByExt && retval)
			return retval;
		// If we had an extension match but no type match, set the mimetype and use
		// it
		if (!retval && miByExt) {
			if (!aType.IsEmpty())
				miByExt->SetMIMEType(aType);
			miByExt.swap(retval);

			return retval;
		}
		// If we got nothing, make a new mimeinfo
		if (!retval) {
			*aFound = PR_FALSE;
			retval = new nsMIMEInfoAmigaOS(aType);
			if (retval) {
				NS_ADDREF(retval);
				if (!aFileExt.IsEmpty())
					retval->AppendExtension(aFileExt);
			}

			return retval;
		}

		// Copy the attributes of retval (mimeinfo from type) onto miByExt, to
		// return it
		// but reset to just collected mDefaultAppDescription (from ext)
		nsAutoString byExtDefault;
		miByExt->GetDefaultDescription(byExtDefault);
		retval->SetDefaultDescription(byExtDefault);
		retval->CopyBasicDataTo(miByExt);

		miByExt.swap(retval);
	}

	return retval;
}

NS_IMETHODIMP
nsOSHelperAppService::GetProtocolHandlerInfoFromOS(const nsACString &aScheme,
                                                   PRBool *found,
                                                   nsIHandlerInfo **_retval)
{
//	printf("nsOSHelperAppService::GetProtocolHandlerInfoFromOS (%s)\n", nsPromiseFlatCString(aScheme).get());

	NS_ASSERTION(!aScheme.IsEmpty(), "No scheme was specified!");

	nsresult rv = OSProtocolHandlerExists(nsPromiseFlatCString(aScheme).get(),
			found);
	if (NS_FAILED(rv))
		return rv;

	nsMIMEInfoAmigaOS *handlerInfo =
			new nsMIMEInfoAmigaOS(aScheme, nsMIMEInfoBase::eProtocolInfo);
	NS_ENSURE_TRUE(handlerInfo, NS_ERROR_OUT_OF_MEMORY);
	NS_ADDREF(*_retval = handlerInfo);

	if (!*found) {
		// Code that calls this requires an object regardless if the OS has
		// something for us, so we return the empty object.
		return rv;
	}

	nsAutoString desc;
	GetApplicationDescription(aScheme, desc);
	handlerInfo->SetDefaultDescription(desc);

	return rv;
}

NS_IMETHODIMP nsOSHelperAppService::GetApplicationDescription(const nsACString& aScheme, nsAString& _retval)
{
//	printf("nsOSHelperAppService::GetApplicationDescription not yet (fully) implemented (%s)\n", nsPromiseFlatCString(aScheme).get());
	return NS_ERROR_NOT_AVAILABLE;
}

nsresult nsOSHelperAppService::GetFileTokenForPath(const PRUnichar * platformAppPath, nsIFile ** aFile)
{
//	printf("nsOSHelperAppService::GetFileTokenForPath (%s)\n", NS_LossyConvertUTF16toASCII(platformAppPath).get());
	if (! *platformAppPath) { // empty filename--return error
		NS_WARNING("Empty filename passed in.");
		return NS_ERROR_INVALID_ARG;
	}


	return NS_ERROR_FILE_NOT_FOUND;
}

already_AddRefed<nsMIMEInfoBase>
nsOSHelperAppService::GetFromType(const nsCString& aMIMEType)
{
	for (int i = 0; mimeTypes[i].type; i++)
	{
		if (strcasecmp(aMIMEType.get(), mimeTypes[i].type) == 0)
		{
			nsCAutoString mimeType(mimeTypes[i].type);
			nsAutoString description = NS_ConvertASCIItoUTF16(mimeTypes[i].description);;

			nsMIMEInfoAmigaOS *mimeInfo = new nsMIMEInfoAmigaOS(mimeType);
			NS_ADDREF(mimeInfo);

			if (mimeTypes[i].handler)
			{
				nsCAutoString path(mimeTypes[i].handler);
				nsAutoString handler = NS_ConvertASCIItoUTF16(mimeTypes[i].handlerDesc);

				nsCOMPtr<nsILocalFile> localFile (do_CreateInstance(NS_LOCAL_FILE_CONTRACTID));
				localFile->InitWithNativePath(path);

				NS_IF_ADDREF(localFile);

				mimeInfo->SetDescription(nsAutoString(description));
				mimeInfo->SetDefaultApplication(localFile);
				mimeInfo->SetDefaultDescription(handler);
				mimeInfo->SetPreferredAction(nsIMIMEInfo::useHelperApp);
			}
			else
			{
				mimeInfo->SetDescription(description);
				mimeInfo->SetPreferredAction(nsIMIMEInfo::saveToDisk);
			}

			return mimeInfo;
		}
	}

	return nsnull;
}

already_AddRefed<nsMIMEInfoBase>
nsOSHelperAppService::GetFromExtension(const nsCString& aFileExt)
{
	for (int i = 0; mimeTypes[i].type; i++)
	{
		/* Try to find a match in the extensions string. */
		const char *ext = mimeTypes[i].extensions;
		const char *fileExt = aFileExt.get();
		int fileExtSize = strlen(fileExt);

		while (ext)
		{
			if (ext[0] == ' ')
				ext++;

			if (strncasecmp(fileExt, ext, fileExtSize) == 0)
			{
				/* Partial match, it's a full match if the next char is either a space or 0 */
				if (ext[fileExtSize] == 0 || ext[fileExtSize] == ' ')
					break;
			}

			ext = strchr(ext+1, ' ');
		}

		/* ext now points to the match, or is NULL. In the latter case, continue searching */
		if (!ext)
			continue;

		nsCAutoString mimeType(mimeTypes[i].type);
		nsAutoString description = NS_ConvertASCIItoUTF16(mimeTypes[i].description);

		nsMIMEInfoAmigaOS *mimeInfo = new nsMIMEInfoAmigaOS(mimeType);
		NS_ADDREF(mimeInfo);

		if (mimeTypes[i].handler)
		{
			nsCAutoString path(mimeTypes[i].handler);
			nsAutoString handler = NS_ConvertASCIItoUTF16(mimeTypes[i].handlerDesc);

			nsCOMPtr<nsILocalFile> localFile (do_CreateInstance(NS_LOCAL_FILE_CONTRACTID));
			localFile->InitWithNativePath(path);

			NS_IF_ADDREF(localFile);

			mimeInfo->SetDescription(description);
			mimeInfo->SetDefaultApplication(localFile);
			mimeInfo->SetDefaultDescription(handler);
			mimeInfo->SetPreferredAction(nsIMIMEInfo::useHelperApp);
		}
		else
		{
			mimeInfo->SetDescription(description);
			mimeInfo->SetPreferredAction(nsIMIMEInfo::saveToDisk);
		}

		return mimeInfo;
	}

	return nsnull;
}

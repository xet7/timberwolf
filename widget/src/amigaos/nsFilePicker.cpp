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


#include "nsCOMPtr.h"
#include "nsNetUtil.h"
#include "nsIServiceManager.h"
#include "nsIPlatformCharset.h"
#include "nsFilePicker.h"
#include "nsILocalFile.h"
#include "nsIURL.h"
#include "nsIStringBundle.h"
#include "nsReadableUtils.h"
#include "nsEscape.h"
#include "nsEnumeratorUtils.h"
#include "nsString.h"
#include "nsWindow.h"

#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/asl.h>
#include <proto/dos.h>

#include <classes/requester.h>
#include <proto/requester.h>


NS_IMPL_THREADSAFE_ISUPPORTS1(nsFilePicker, nsIFilePicker)


//-------------------------------------------------------------------------
//
// nsFilePicker constructor
//
//-------------------------------------------------------------------------
nsFilePicker::nsFilePicker()
: mMode(nsIFilePicker::modeOpen),
  mSelectedType(0),
  mAllowURLs(PR_FALSE)
{

}

//-------------------------------------------------------------------------
//
// nsFilePicker destructor
//
//-------------------------------------------------------------------------
nsFilePicker::~nsFilePicker()
{

}

//-------------------------------------------------------------------------
//
// Show - Display the file dialog
//
//-------------------------------------------------------------------------
NS_IMETHODIMP nsFilePicker::Show(PRInt16 *aReturn)
{
	NS_ENSURE_ARG_POINTER(aReturn);

	PRInt16 res = nsIFilePicker::returnCancel;
//	nsWindow *parent = static_cast<nsWindow *>(mParentWidget);
	struct Window *parentWin = NULL; //parent->GetIntuitionWindow();

	struct FileRequester *freq = (struct FileRequester *)
			IAsl->AllocAslRequest(ASL_FileRequest, NULL);

	if (freq)
	{
		char *titleString = ToNewCString(mTitle);
		char *initialDrawer = ToNewCString(mDefault);
		char *initialPattern = ToNewCString(mDefaultExtension);

		char dirBuffer[1024];
		strncpy(dirBuffer, initialDrawer, 1023);
		dirBuffer[1023] = '\0';
		char *pathEnd = IDOS->PathPart(dirBuffer);
		*pathEnd = '\0';

		if (IAsl->AslRequestTags(freq,
				ASLFR_TitleText,		(uint32) titleString,
				ASLFR_InitialDrawer,	(uint32) dirBuffer,
				ASLFR_InitialFile,		(uint32) IDOS->FilePart(initialDrawer),
				ASLFR_DoMultiSelect,	mMode == nsIFilePicker::modeOpenMultiple ? TRUE : FALSE,
				ASLFR_DoSaveMode,		mMode == nsIFilePicker::modeSave ? TRUE : FALSE,
				ASLFR_DrawersOnly,		mMode == nsIFilePicker::modeGetFolder ? TRUE : FALSE,
				ASLFR_DoPatterns,		TRUE,
			TAG_DONE))
		{
			if (mMode == nsIFilePicker::modeOpenMultiple)
			{
				mFiles.Clear();

				for (int i = 0; i < freq->fr_NumArgs; i++)
				{
					char buffer[1024];
					IDOS->NameFromLock(freq->fr_ArgList[i].wa_Lock, buffer, 1023);
					IDOS->AddPart((STRPTR)buffer, freq->fr_ArgList[i].wa_Name, 1023);

					nsCOMPtr<nsILocalFile> file;

					nsresult rv = NS_NewNativeLocalFile(nsDependentCString(buffer),
							PR_FALSE, getter_AddRefs(file));

					if (NS_SUCCEEDED(rv))
						mFiles.AppendObject(file);
				}
			}
			else
			{
				char buffer[1024];
				strncpy(buffer, freq->fr_Drawer, 1023);
				buffer[1023] = '\0';
				if (strlen(buffer) == 0 || buffer[0] == ':' || buffer[0] == '/')
				{
					BPTR dirLock = IDOS->Lock(buffer, SHARED_LOCK);
					if (dirLock)
					{
						IDOS->NameFromLock(dirLock, buffer, 1023);
						IDOS->UnLock(dirLock);
					};

				}
				IDOS->AddPart((STRPTR)buffer, freq->fr_File, 1023);


				mFileURL.Assign("file:///");
				mFileURL.Append(buffer);
			}

			res = nsIFilePicker::returnOK;
		}

		IAsl->FreeAslRequest(freq);
		NS_Free(titleString);
		NS_Free(initialDrawer);
		NS_Free(initialPattern);
	}

	*aReturn = res;


	return NS_OK;
}


NS_IMETHODIMP
nsFilePicker::SetDefaultString(const nsAString& aString)
{
	mDefault = aString;

	return NS_OK;
}

NS_IMETHODIMP
nsFilePicker::GetDefaultString(nsAString& aString)
{
	// Per API...
	return NS_ERROR_FAILURE;
}

NS_IMETHODIMP
nsFilePicker::SetDefaultExtension(const nsAString& aExtension)
{
	mDefaultExtension = aExtension;

	return NS_OK;
}

NS_IMETHODIMP
nsFilePicker::GetDefaultExtension(nsAString& aExtension)
{
	aExtension = mDefaultExtension;

	return NS_OK;
}

NS_IMETHODIMP
nsFilePicker::GetFilterIndex(PRInt32 *aFilterIndex)
{
	*aFilterIndex = mSelectedType;

	return NS_OK;
}

NS_IMETHODIMP
nsFilePicker::SetFilterIndex(PRInt32 aFilterIndex)
{
	mSelectedType = aFilterIndex;

	return NS_OK;
}

NS_IMETHODIMP
nsFilePicker::GetFile(nsILocalFile **aFile)
{
	NS_ENSURE_ARG_POINTER(aFile);

	*aFile = nsnull;
	nsCOMPtr<nsIURI> uri;
	nsresult rv = GetFileURL(getter_AddRefs(uri));
	if (!uri)
		return rv;

	nsCOMPtr<nsIFileURL> fileURL(do_QueryInterface(uri, &rv));
	NS_ENSURE_SUCCESS(rv, rv);

	nsCOMPtr<nsIFile> file;
	rv = fileURL->GetFile(getter_AddRefs(file));
	NS_ENSURE_SUCCESS(rv, rv);

	return CallQueryInterface(file, aFile);
}

NS_IMETHODIMP
nsFilePicker::GetFileURL(nsIURI **aFileURL)
{
	*aFileURL = nsnull;
	return NS_NewURI(aFileURL, mFileURL);
}

NS_IMETHODIMP
nsFilePicker::GetFiles(nsISimpleEnumerator **aFiles)
{
	NS_ENSURE_ARG_POINTER(aFiles);

	if (mMode == nsIFilePicker::modeOpenMultiple) {
		return NS_NewArrayEnumerator(aFiles, mFiles);
	}

	return NS_ERROR_FAILURE;
}


//-------------------------------------------------------------------------
void nsFilePicker::InitNative(nsIWidget *aParent,
		const nsAString& aTitle,
		PRInt16 aMode)
{
	mParentWidget = aParent;
	mTitle.Assign(aTitle);
	mMode = aMode;
}

NS_IMETHODIMP
nsFilePicker::AppendFilters(PRInt32 aFilterMask)
{
	mAllowURLs = !!(aFilterMask & filterAllowURLs);
	return nsBaseFilePicker::AppendFilters(aFilterMask);
}

NS_IMETHODIMP
nsFilePicker::AppendFilter(const nsAString& aTitle, const nsAString& aFilter)
{
	if (aFilter.EqualsLiteral("..apps")) {
		// No platform specific thing we can do here, really....
		return NS_OK;
	}

	nsCAutoString filter, name;
	CopyUTF16toUTF8(aFilter, filter);
	CopyUTF16toUTF8(aTitle, name);

	mFilters.AppendElement(filter);
	mFilterNames.AppendElement(name);

	return NS_OK;
}

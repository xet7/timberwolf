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



#include "nscore.h"
#include "plstr.h"
#include <stdio.h>
#include "nsIURL.h"
#include "nsString.h"
#include "nsIFileURL.h"
#include "nsSound.h"
#include "nsNetUtil.h"

#include "nsDirectoryServiceDefs.h"

#include "nsNativeCharsetUtils.h"

#include <proto/exec.h>
#include <proto/datatypes.h>
#include <proto/intuition.h>
#include <datatypes/datatypesclass.h>
#include <datatypes/soundclass.h>


NS_IMPL_ISUPPORTS2(nsSound, nsISound, nsIStreamLoaderObserver)

////////////////////////////////////////////////////////////////////////
nsSound::nsSound() : mInitialized(PR_FALSE)
{
	//printf("%s\n", __PRETTY_FUNCTION__);
}

nsSound::~nsSound()
{
	//printf("%s\n", __PRETTY_FUNCTION__);
}

NS_IMETHODIMP nsSound::OnStreamComplete(nsIStreamLoader *aLoader,
                                        nsISupports *context,
                                        nsresult aStatus,
                                        PRUint32 dataLen,
                                        const PRUint8 *data)
{//printf("%s\n", __PRETTY_FUNCTION__);
	if (NS_FAILED(aStatus))
		return aStatus;

	uint32 sigNum = IExec->AllocSignal(-1);

	// Try to create a datatype object from the memory stream
	Object *dtobject = (Object *)IDataTypes->NewDTObject(NULL,
					DTA_SourceType, 		DTST_MEMORY,
					DTA_GroupID, 			GID_SOUND,
					DTA_SourceAddress,		(void *)data,
					DTA_SourceSize,			dataLen,
					SDTA_SignalTask,		IExec->FindTask(NULL),
					SDTA_SignalBit,			1L << sigNum,
				TAG_END);

	if (dtobject)
	{
		struct dtTrigger mydtt;

		mydtt.MethodID = DTM_TRIGGER;
		mydtt.dtt_GInfo = NULL;
		mydtt.dtt_Function = STM_PLAY;
		mydtt.dtt_Data = NULL;

		IDataTypes->DoDTMethod(dtobject, NULL, NULL, &mydtt);

		IExec->Wait(1L << sigNum);

		IDataTypes->DisposeDTObject(dtobject);
	}

	return NS_OK;

}

NS_IMETHODIMP nsSound::Init(void)
{//printf("%s\n", __PRETTY_FUNCTION__);
	if (mInitialized)
		return NS_OK;

	mInitialized = PR_TRUE;
	return NS_OK;
}



NS_METHOD nsSound::Beep()
{//printf("%s\n", __PRETTY_FUNCTION__);
	IIntuition->DisplayBeep(NULL);
	return NS_OK;
}

NS_METHOD nsSound::Play(nsIURL *aURL)
{//printf("%s\n", __PRETTY_FUNCTION__);
	nsresult rv;
	nsCOMPtr<nsIStreamLoader> loader;
	rv = NS_NewStreamLoader(getter_AddRefs(loader), aURL, this);
	return rv;
}

NS_IMETHODIMP nsSound::PlaySystemSound(const nsAString &aSoundAlias)
{//printf("%s\n", __PRETTY_FUNCTION__);

//	const char *str = NS_LossyConvertUTF16toASCII(aSoundAlias).get();
//	printf("PlaySystemSound %s", str);


	if (NS_IsMozAliasSound(aSoundAlias)) {
		NS_WARNING("nsISound::playSystemSound is called with \"_moz_\" events, they are obsolete, use nsISound::playEventSound instead");
		PRUint32 eventId;
		if (aSoundAlias.Equals(NS_SYSSOUND_ALERT_DIALOG))
			eventId = EVENT_ALERT_DIALOG_OPEN;
		else if (aSoundAlias.Equals(NS_SYSSOUND_CONFIRM_DIALOG))
			eventId = EVENT_CONFIRM_DIALOG_OPEN;
		else if (aSoundAlias.Equals(NS_SYSSOUND_MAIL_BEEP))
			eventId = EVENT_NEW_MAIL_RECEIVED;
		else if (aSoundAlias.Equals(NS_SYSSOUND_MENU_EXECUTE))
			eventId = EVENT_MENU_EXECUTE;
		else if (aSoundAlias.Equals(NS_SYSSOUND_MENU_POPUP))
			eventId = EVENT_MENU_POPUP;
		else
			return NS_OK;
		return PlayEventSound(eventId);
	}

	nsresult rv;
	nsCOMPtr <nsIURI> fileURI;

	// create a nsILocalFile and then a nsIFileURL from that
	nsCOMPtr <nsILocalFile> soundFile;
	rv = NS_NewLocalFile(aSoundAlias, PR_TRUE,
			getter_AddRefs(soundFile));
	NS_ENSURE_SUCCESS(rv,rv);

	rv = NS_NewFileURI(getter_AddRefs(fileURI), soundFile);
	NS_ENSURE_SUCCESS(rv,rv);

	nsCOMPtr<nsIFileURL> fileURL = do_QueryInterface(fileURI,&rv);
	NS_ENSURE_SUCCESS(rv,rv);

	rv = Play(fileURL);

	return rv;
}


NS_IMETHODIMP nsSound::PlayEventSound(PRUint32 aEventId)
{//printf("%s\n", __PRETTY_FUNCTION__);
    return NS_OK;
}


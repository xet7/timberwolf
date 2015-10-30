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


#ifndef nsClipboard_h__
#define nsClipboard_h__

/**
 * Native AmigaOS Clipboard wrapper
 */


#include "nsIClipboard.h"
#include "nsClipboardPrivacyHandler.h"
#include "nsAutoPtr.h"

#include <proto/exec.h>
#include <proto/iffparse.h>
#include <proto/datatypes.h>
#include <libraries/iffparse.h>

class nsClipboard : public nsIClipboard
{
public:
	NS_DECL_ISUPPORTS
	NS_DECL_NSICLIPBOARD

	nsClipboard();
	virtual ~nsClipboard();

private:
	nsCOMPtr<nsIClipboardOwner>  mSelectionOwner;
	nsCOMPtr<nsIClipboardOwner>  mGlobalOwner;
	nsCOMPtr<nsITransferable>    mSelectionTransferable;
	nsCOMPtr<nsITransferable>    mGlobalTransferable;
	nsRefPtr<nsClipboardPrivacyHandler> mPrivacyHandler;

	void CopyTextToClipboard(uint32 unit, char *buffer, uint32 len);
	char * CopyTextFromClipboard(uint32 unit);
	uint32 GetClipboardDataType(uint32 unit);

	enum
	{
		kNone,
		kText,
		kImage
	};

private:
	struct IFFHandle *mIFF;
};

#endif // nsClipboard_h__

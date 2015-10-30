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


#ifndef __nsSound_h__
#define __nsSound_h__

#include "nsISound.h"
#include "nsIStreamLoader.h"
#include "nsThreadUtils.h"
#include <datatypes/datatypesclass.h>

class nsSound : public nsISound,
				public nsIStreamLoaderObserver
{
public:
	nsSound();
	virtual ~nsSound();

	NS_DECL_ISUPPORTS
	NS_DECL_NSISOUND
	NS_DECL_NSISTREAMLOADEROBSERVER

private:
	PRBool	mInitialized;

};

#endif /* __nsSound_h__ */

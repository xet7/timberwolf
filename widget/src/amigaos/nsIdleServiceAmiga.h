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


#ifndef nsIdleServiceAmiga_h__
#define nsIdleServiceAmiga_h__

#include "nsIdleService.h"

class nsIdleServiceAmiga : public nsIdleService
{
public:
    NS_DECL_ISUPPORTS

    bool PollIdleTime(PRUint32* aIdleTime);
protected:
    bool UsePollMode();
};

#endif // nsIdleServiceAmiga_h__

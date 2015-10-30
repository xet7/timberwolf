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


#include "nsIdleServiceAmiga.h"
#include "nsIServiceManager.h"

NS_IMPL_ISUPPORTS2(nsIdleServiceAmiga, nsIIdleService, nsIdleService)

bool
nsIdleServiceAmiga::PollIdleTime(PRUint32 *aIdleTime)
{
    return false;
}

bool
nsIdleServiceAmiga::UsePollMode()
{
    return false;
}

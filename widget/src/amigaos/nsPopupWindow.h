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

 
#ifndef nsPopupWindow_h__
#define nsPopupWindow_h__

#include "nsWindow.h"

class nsPopupWindow : public nsWindow 
{
public:
	nsPopupWindow();

	NS_IMETHOD              Show(PRBool bState);



};
#endif //nsPopupWindow_h__

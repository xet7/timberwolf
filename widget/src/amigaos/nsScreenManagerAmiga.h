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


#ifndef nsScreenManagerAmiga_h___
#define nsScreenManagerAmiga_h___

#include "nsIScreenManager.h"
#include "nsIScreen.h"
#include "nsCOMPtr.h"

//------------------------------------------------------------------------

class nsScreenManagerAmiga : public nsIScreenManager
{
public:
  nsScreenManagerAmiga ( );
  virtual ~nsScreenManagerAmiga();

  NS_DECL_ISUPPORTS
  NS_DECL_NSISCREENMANAGER

private:

  nsIScreen* CreateNewScreenObject ( ) ;

    // cache the primary screen object to avoid memory allocation every time
  nsCOMPtr<nsIScreen> mCachedMainScreen;

};

#endif  // nsScreenManagerAmiga_h___

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


#ifndef nsScreenAmiga_h___
#define nsScreenAmiga_h___

#include "nsIScreen.h"

//------------------------------------------------------------------------

class nsScreenAmiga : public nsIScreen
{
public:
  nsScreenAmiga ( );
  virtual ~nsScreenAmiga();

  NS_DECL_ISUPPORTS
  NS_DECL_NSISCREEN

private:

};

#endif  // nsScreenAmiga_h___

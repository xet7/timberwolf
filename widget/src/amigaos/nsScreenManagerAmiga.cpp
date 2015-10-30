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

#include "nsScreenManagerAmiga.h"
#include "nsScreenAmiga.h"


nsScreenManagerAmiga :: nsScreenManagerAmiga ( )
{
  // nothing else to do. I guess we could cache a bunch of information
  // here, but we want to ask the device at runtime in case anything
  // has changed.
}


nsScreenManagerAmiga :: ~nsScreenManagerAmiga()
{
  // nothing to see here.
}


// addref, release, QI
NS_IMPL_ISUPPORTS1(nsScreenManagerAmiga, nsIScreenManager)


//
// CreateNewScreenObject
//
// Utility routine. Creates a new screen object from the given device handle
//
// NOTE: For this "single-monitor" impl, we just always return the cached primary
//        screen. This should change when a multi-monitor impl is done.
//
nsIScreen*
nsScreenManagerAmiga :: CreateNewScreenObject (  )
{
	  nsIScreen* retval = nsnull;
	  if ( !mCachedMainScreen )
	    mCachedMainScreen = new nsScreenAmiga ( );
	  NS_IF_ADDREF(retval = mCachedMainScreen.get());

	  return retval;
}


//
// ScreenForRect
//
// Returns the screen that contains the rectangle. If the rect overlaps
// multiple screens, it picks the screen with the greatest area of intersection.
//
// The coordinates are in pixels (not twips) and in screen coordinates.
//
NS_IMETHODIMP
nsScreenManagerAmiga :: ScreenForRect ( PRInt32 /*inLeft*/, PRInt32 /*inTop*/, PRInt32 /*inWidth*/,
                                       PRInt32 /*inHeight*/, nsIScreen **outScreen )
{
  GetPrimaryScreen ( outScreen );
  return NS_OK;

} // ScreenForRect


//
// GetPrimaryScreen
//
// The screen with the menubar/taskbar. This shouldn't be needed very
// often.
//
NS_IMETHODIMP
nsScreenManagerAmiga :: GetPrimaryScreen(nsIScreen * *aPrimaryScreen)
{
  *aPrimaryScreen = CreateNewScreenObject();    // addrefs
  return NS_OK;

} // GetPrimaryScreen


//
// GetNumberOfScreens
//
// Returns how many physical screens are available.
//
NS_IMETHODIMP
nsScreenManagerAmiga :: GetNumberOfScreens(PRUint32 *aNumberOfScreens)
{
  *aNumberOfScreens = 1;
  return NS_OK;

} // GetNumberOfScreens

NS_IMETHODIMP
nsScreenManagerAmiga :: ScreenForNativeWidget(void *nativeWidget, nsIScreen **aScreen)
{
	*aScreen = CreateNewScreenObject();    // addrefs
	return NS_OK;
}

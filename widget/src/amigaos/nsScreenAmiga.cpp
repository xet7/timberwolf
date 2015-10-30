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


#include "nsScreenAmiga.h"
#include <proto/intuition.h>

extern struct IntuitionIFace *IIntuition;

nsScreenAmiga :: nsScreenAmiga (  )
{
  // nothing else to do. I guess we could cache a bunch of information
  // here, but we want to ask the device at runtime in case anything
  // has changed.
}


nsScreenAmiga :: ~nsScreenAmiga()
{
  // nothing to see here.
}


// addref, release, QI
NS_IMPL_ISUPPORTS1(nsScreenAmiga, nsIScreen)


NS_IMETHODIMP
nsScreenAmiga :: GetRect(PRInt32 *outLeft, PRInt32 *outTop, PRInt32 *outWidth, PRInt32 *outHeight)
{
	struct Screen *pubScreen = IIntuition->LockPubScreen(NULL);


	if (pubScreen)
	{
		IIntuition->GetScreenAttr(pubScreen, SA_Top, outTop, sizeof(PRInt32));
		IIntuition->GetScreenAttr(pubScreen, SA_Left, outLeft, sizeof(PRInt32));
		IIntuition->GetScreenAttr(pubScreen, SA_Width, outWidth, sizeof(PRInt32));
		IIntuition->GetScreenAttr(pubScreen, SA_Height, outHeight, sizeof(PRInt32));

		IIntuition->UnlockPubScreen(NULL, pubScreen);
	}

//	printf("Screen size: %d, %d - %d x %d\n", *outTop, *outLeft, *outWidth, *outHeight);
	return NS_OK;
} // GetRect


NS_IMETHODIMP
nsScreenAmiga :: GetAvailRect(PRInt32 *outLeft, PRInt32 *outTop, PRInt32 *outWidth, PRInt32 *outHeight)
{

	struct Screen *pubScreen = IIntuition->LockPubScreen(NULL);

	if (pubScreen)
	{
		IIntuition->GetScreenAttr(pubScreen, SA_Top, outTop, sizeof(PRInt32));
		IIntuition->GetScreenAttr(pubScreen, SA_Left, outLeft, sizeof(PRInt32));
		IIntuition->GetScreenAttr(pubScreen, SA_Width, outWidth, sizeof(PRInt32));
		IIntuition->GetScreenAttr(pubScreen, SA_Height, outHeight, sizeof(PRInt32));

		IIntuition->UnlockPubScreen(NULL, pubScreen);
	}

//	printf("Screen size (avail): %d, %d - %d x %d\n", *outTop, *outLeft, *outWidth, *outHeight);

	return NS_OK;

} // GetAvailRect


NS_IMETHODIMP
nsScreenAmiga :: GetPixelDepth(PRInt32 *aPixelDepth)
{
	struct Screen *pubScreen = IIntuition->LockPubScreen(NULL);

	if (pubScreen)
	{
		IIntuition->GetScreenAttr(pubScreen, SA_Depth, aPixelDepth, sizeof(PRInt32));

		IIntuition->UnlockPubScreen(NULL, pubScreen);
	}

	return NS_OK;


} // GetPixelDepth


NS_IMETHODIMP
nsScreenAmiga :: GetColorDepth(PRInt32 *aColorDepth)
{
  return GetPixelDepth ( aColorDepth );

} // GetColorDepth



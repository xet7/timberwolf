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


#ifndef __nsLookAndFeel
#define __nsLookAndFeel
#include "nsXPLookAndFeel.h"
#include <proto/intuition.h>
#include <intuition/screens.h>

class IntuitionColorMap
{
public:
	IntuitionColorMap(void) :
		mInitialized(false)
	{
	}

	IntuitionColorMap(struct Screen *scr)
	{
		Init(scr);
	}

	nscolor operator[](int a)
	{
		return mColorArray[a];
	}

	void Init(struct Screen *scr);

private:
	nscolor mColorArray[256];
	bool mInitialized;
};

class nsLookAndFeel: public nsXPLookAndFeel {
public:
  nsLookAndFeel();
  virtual ~nsLookAndFeel();

  nsresult NativeGetColor(const nsColorID aID, nscolor &aColor);
  NS_IMETHOD GetMetric(const nsMetricID aID, PRInt32 & aMetric);
  NS_IMETHOD GetMetric(const nsMetricFloatID aID, float & aMetric);

private:
  IntuitionColorMap mColorMap;
};

#endif

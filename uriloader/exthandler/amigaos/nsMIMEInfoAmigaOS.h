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


#ifndef nsMIMEInfoAmigaOS_h_
#define nsMIMEInfoAmigaOS_h_

#include "nsMIMEInfoImpl.h"

class nsMIMEInfoAmigaOS : public nsMIMEInfoImpl {
  public:
    nsMIMEInfoAmigaOS(const char* aType = "") : nsMIMEInfoImpl(aType) {}
    nsMIMEInfoAmigaOS(const nsACString& aMIMEType) : nsMIMEInfoImpl(aMIMEType) {}
    nsMIMEInfoAmigaOS(const nsACString& aType, HandlerClass aClass) :
      nsMIMEInfoImpl(aType, aClass) {}
    virtual ~nsMIMEInfoAmigaOS();

    NS_IMETHOD GetHasDefaultHandler(PRBool * _retval);


  protected:
    virtual NS_HIDDEN_(nsresult) LaunchDefaultWithFile(nsIFile* aFile);
    virtual NS_HIDDEN_(nsresult) LoadUriInternal(nsIURI *aURI);
};

#endif

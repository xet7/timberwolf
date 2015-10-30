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


#ifndef nsOSHelperAppService_h__
#define nsOSHelperAppService_h__

// The OS helper app service is a subclass of nsExternalHelperAppService and is implemented on each
// platform. It contains platform specific code for finding helper applications for a given mime type
// in addition to launching those applications.

#include "nsExternalHelperAppService.h"
#include "nsCExternalHandlerService.h"
#include "nsCOMPtr.h"
#include "nsMIMEInfoImpl.h"

class nsMIMEInfoAmigaOS;

class nsOSHelperAppService : public nsExternalHelperAppService
{
public:
  nsOSHelperAppService();
  virtual ~nsOSHelperAppService();

  // method overrides for mime.types and mime.info look up steps
  already_AddRefed<nsIMIMEInfo> GetMIMEInfoFromOS(const nsACString& aMimeType,
                                                  const nsACString& aFileExt,
                                                  PRBool     *aFound);
  NS_IMETHOD GetProtocolHandlerInfoFromOS(const nsACString &aScheme,
                                          PRBool *found,
                                          nsIHandlerInfo **_retval);

  // override nsIExternalProtocolService methods
  nsresult OSProtocolHandlerExists(const char * aProtocolScheme, PRBool * aHandlerExists);
  NS_IMETHOD GetApplicationDescription(const nsACString& aScheme, nsAString& _retval);

  // GetFileTokenForPath must be implemented by each platform.
  // platformAppPath --> a platform specific path to an application that we got out of the
  //                     rdf data source. This can be a mac file spec, a unix path or a windows path depending on the platform
  // aFile --> an nsIFile representation of that platform application path.
  virtual nsresult GetFileTokenForPath(const PRUnichar * platformAppPath, nsIFile ** aFile);

protected:
  already_AddRefed<nsMIMEInfoBase> GetFromType(const nsCString& aMimeType);
  already_AddRefed<nsMIMEInfoBase> GetFromExtension(const nsCString& aFileExt);

};

#endif // nsOSHelperAppService_h__

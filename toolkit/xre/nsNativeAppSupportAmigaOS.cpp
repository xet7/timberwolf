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


#include "nsNativeAppSupportBase.h"
#include "nsCOMPtr.h"
#include "nsXPCOM.h"
#include "nsISupportsPrimitives.h"
#include "nsIObserverService.h"
#include "nsIAppStartup.h"
#include "nsServiceManagerUtils.h"
#include "prlink.h"
#include "nsXREDirProvider.h"
#include "nsReadableUtils.h"

#include "nsIFile.h"
#include "nsDirectoryServiceDefs.h"
#include "nsICommandLineRunner.h"
#include "nsIWindowMediator.h"
#include "nsIDOMWindowInternal.h"
#include "nsPIDOMWindow.h"
#include "nsIDocShell.h"
#include "nsIBaseWindow.h"
#include "nsIWidget.h"
#include "nsIWritablePropertyBag2.h"
#include "nsIPrefService.h"
#include "mozilla/Services.h"


class nsNativeAppSupportAmigaOS : public nsNativeAppSupportBase
{
public:
  NS_IMETHOD Start(PRBool* aRetVal);
  NS_IMETHOD Stop(PRBool *aResult);
  NS_IMETHOD Enable();

private:

};

NS_IMETHODIMP
nsNativeAppSupportAmigaOS::Start(PRBool *aRetVal)
{
  NS_ASSERTION(gAppData, "gAppData must not be null.");

  nsCAutoString applicationName;
  if (gAppData->vendor) {
      applicationName.Append(gAppData->vendor);
      applicationName.Append(".");
  }
  applicationName.Append(gAppData->name);
  ToLowerCase(applicationName);

//  printf("nsNativeAppSupportAmigaOS::Start  Launch %s\n", applicationName.get());
  *aRetVal = PR_TRUE;

  // Is there a request to suppress default binary launcher?
  nsCAutoString path;
  char* argv1 = getenv("MOZ_APP_LAUNCHER");

  if(!argv1) {
    // Tell the desktop the command for restarting us so that we can be part of XSMP session restore
    NS_ASSERTION(gDirServiceProvider, "gDirServiceProvider is NULL! This shouldn't happen!");
    nsCOMPtr<nsIFile> executablePath;
    nsresult rv;

    PRBool dummy;
    rv = gDirServiceProvider->GetFile(XRE_EXECUTABLE_FILE, &dummy, getter_AddRefs(executablePath));

    if (NS_SUCCEEDED(rv)) {
      // Strip off the -bin suffix to get the shell script we should run; this is what Breakpad does
      nsCAutoString leafName;
      rv = executablePath->GetNativeLeafName(leafName);
      if (NS_SUCCEEDED(rv) && StringEndsWith(leafName, NS_LITERAL_CSTRING("-bin"))) {
        leafName.SetLength(leafName.Length() - strlen("-bin"));
        executablePath->SetNativeLeafName(leafName);
      }

      executablePath->GetNativePath(path);
      argv1 = (char*)(path.get());
    }
  }

  return NS_OK;
}

NS_IMETHODIMP
nsNativeAppSupportAmigaOS::Stop(PRBool *aResult)
{
  NS_ENSURE_ARG(aResult);
  *aResult = PR_TRUE;

//  printf("nsNativeAppSupportAmigaOS::Stop\n");

  return NS_OK;
}

NS_IMETHODIMP
nsNativeAppSupportAmigaOS::Enable()
{
//  printf("nsNativeAppSupportAmigaOS::Enable\n");
  return NS_OK;
}

nsresult
NS_CreateNativeAppSupport(nsINativeAppSupport **aResult)
{
  nsNativeAppSupportBase* native = new nsNativeAppSupportAmigaOS();
  if (!native)
    return NS_ERROR_OUT_OF_MEMORY;

  *aResult = native;
  NS_ADDREF(*aResult);

  return NS_OK;
}

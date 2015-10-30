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


#include "nsToolkit.h"
#include "prmon.h"
#include "prprf.h"
#include "nsWidgetAtoms.h"

//
// Static thread local storage index of the Toolkit
// object associated with a given thread...
//
static PRUintn gToolkitTLSIndex = 0;

//-------------------------------------------------------------------------
//
// nsISupports implementation macro
//
//-------------------------------------------------------------------------
NS_IMPL_THREADSAFE_ISUPPORTS1(nsToolkit, nsIToolkit)


//-------------------------------------------------------------------------
//
// constructor
//
//-------------------------------------------------------------------------
nsToolkit::nsToolkit()
{
	//NS_NOTYETIMPLEMENTED("nsToolkit::nsToolkit");
}


//-------------------------------------------------------------------------
//
// destructor
//
//-------------------------------------------------------------------------
nsToolkit::~nsToolkit()
{

}



//-------------------------------------------------------------------------
//
//
//-------------------------------------------------------------------------
NS_METHOD nsToolkit::Init(PRThread *aThread)
{
//	NS_NOTYETIMPLEMENTED("nsToolkit::Init");
	nsWidgetAtoms::RegisterAtoms();

	return NS_OK;
}


//-------------------------------------------------------------------------
//
// Return the nsIToolkit for the current thread.  If a toolkit does not
// yet exist, then one will be created...
//
//-------------------------------------------------------------------------
NS_METHOD NS_GetCurrentToolkit(nsIToolkit* *aResult)
{
    nsIToolkit* toolkit = nsnull;
    nsresult rv = NS_OK;
    PRStatus status;

    // Create the TLS index the first time through...
    if (0 == gToolkitTLSIndex) {
        status = PR_NewThreadPrivateIndex(&gToolkitTLSIndex, NULL);
        if (PR_FAILURE == status) {
            rv = NS_ERROR_FAILURE;
        }
    }

    if (NS_SUCCEEDED(rv)) {
        toolkit = (nsIToolkit*)PR_GetThreadPrivate(gToolkitTLSIndex);

        //
        // Create a new toolkit for this thread...
        //
        if (!toolkit) {
            toolkit = new nsToolkit();

            if (!toolkit) {
                rv = NS_ERROR_OUT_OF_MEMORY;
            } else {
                NS_ADDREF(toolkit);
                toolkit->Init(PR_GetCurrentThread());
                //
                // The reference stored in the TLS is weak.  It is
                // removed in the nsToolkit destructor...
                //
                PR_SetThreadPrivate(gToolkitTLSIndex, (void*)toolkit);
            }
        } else {
            NS_ADDREF(toolkit);
        }
        *aResult = toolkit;
    }

    return rv;
}



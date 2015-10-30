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


#ifndef TOOLKIT_H
#define TOOLKIT_H

#include "nsIToolkit.h"

struct MethodInfo;

/**
 * Wrapper around the thread running the message pump.
 * The toolkit abstraction is necessary because the message pump must
 * execute within the same thread that created the widget under Win32.
 */

class nsToolkit : public nsIToolkit
{
public:
            NS_DECL_ISUPPORTS

                            nsToolkit();
            NS_IMETHOD      Init(PRThread *aThread);
private:
            virtual         ~nsToolkit();
            void            CreateUIThread(void);

protected:

};

#endif  // TOOLKIT_H

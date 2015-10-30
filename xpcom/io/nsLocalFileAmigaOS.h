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


/*
 * Implementation of nsIFile for AmigaOS systems.
 */

#ifndef _nsLocalFileAmigaOS_H_
#define _nsLocalFileAmigaOS_H_

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "nscore.h"
#include "nsString.h"
#include "nsReadableUtils.h"
#include "nsIHashable.h"
#include "nsIClassInfoImpl.h"

/**
 *  we need these for statfs()
 */
#ifdef HAVE_SYS_STATVFS_H
    #if defined(__osf__) && defined(__DECCXX)
        extern "C" int statvfs(const char *, struct statvfs *);
    #endif
    #include <sys/statvfs.h>
#endif

#ifdef HAVE_SYS_STATFS_H
    #include <sys/statfs.h>
#endif

#ifdef HAVE_STATVFS
    #define STATFS statvfs
#else
    #define STATFS statfs
#endif

// so we can statfs on freebsd
#if defined(__FreeBSD__)
    #define HAVE_SYS_STATFS_H
    #define STATFS statfs
    #include <sys/param.h>
    #include <sys/mount.h>
#endif

class NS_COM nsLocalFile : public nsILocalFile,
                           public nsIHashable
{
public:
    NS_DEFINE_STATIC_CID_ACCESSOR(NS_LOCAL_FILE_CID)

    nsLocalFile();

    static NS_METHOD nsLocalFileConstructor(nsISupports* outer, const nsIID& aIID, void* *aInstancePtr);

    // nsISupports
    NS_DECL_ISUPPORTS

    // nsIFile
    NS_DECL_NSIFILE

    // nsILocalFile
    NS_DECL_NSILOCALFILE

    // nsIHashable
    NS_DECL_NSIHASHABLE

public:
    static void GlobalInit();
    static void GlobalShutdown();

private:
    nsLocalFile(const nsLocalFile& other);
    ~nsLocalFile() {}

    void MakeAmiga(void);

protected:
// This stat cache holds the *last stat* - it does not invalidate.
// Call "FillStatCache" whenever you want to stat our file.
#ifdef HAVE_STAT64
    struct stat64 mCachedStat;
#else
    struct stat  mCachedStat;
#endif
    nsCString    mPath;

    void LocateNativeLeafName(nsACString::const_iterator &,
                              nsACString::const_iterator &);

    nsresult CopyDirectoryTo(nsIFile *newParent);
    nsresult CreateAllAncestors(PRUint32 permissions);
    nsresult GetNativeTargetPathName(nsIFile *newParent,
                                     const nsACString &newName,
                                     nsACString &_retval);

    PRBool FillStatCache();

    nsresult CreateAndKeepOpen(PRUint32 type, PRIntn flags,
                               PRUint32 permissions, PRFileDesc **_retval);
};

#endif /* _nsLocalFileAmigaOS_H_ */

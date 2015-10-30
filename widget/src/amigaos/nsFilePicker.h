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



#ifndef nsFilePicker_h__
#define nsFilePicker_h__

#ifndef DEBUG
// XXX adding mLastUsedDirectory and code for handling last used folder on File Open/Save
// caused crashes in DEBUG builds on some machines - see comments for bug 177720
// works well on "normal" builds
#define FILEPICKER_SAVE_LAST_DIR 1
#endif

#include "nsICharsetConverterManager.h"
#include "nsBaseFilePicker.h"
#include "nsString.h"
#include "nsIFileChannel.h"
#include "nsILocalFile.h"
#include "nsISimpleEnumerator.h"
#include "nsISupportsArray.h"
#include "nsCOMArray.h"

class nsFilePicker : public nsBaseFilePicker
{
public:
  NS_DECL_ISUPPORTS

  nsFilePicker();
  virtual ~nsFilePicker();

  // nsIFilePicker (less what's in nsBaseFilePicker)
  NS_IMETHODIMP AppendFilters(PRInt32 aFilterMask);
  NS_IMETHODIMP AppendFilter(const nsAString& aTitle, const nsAString& aFilter);
  NS_IMETHODIMP SetDefaultString(const nsAString& aString);
  NS_IMETHODIMP GetDefaultString(nsAString& aString);
  NS_IMETHODIMP SetDefaultExtension(const nsAString& aExtension);
  NS_IMETHODIMP GetDefaultExtension(nsAString& aExtension);
  NS_IMETHODIMP GetFilterIndex(PRInt32 *aFilterIndex);
  NS_IMETHODIMP SetFilterIndex(PRInt32 aFilterIndex);
  NS_IMETHODIMP GetFile(nsILocalFile **aFile);
  NS_IMETHODIMP GetFileURL(nsIURI **aFileURL);
  NS_IMETHODIMP GetFiles(nsISimpleEnumerator **aFiles);
  NS_IMETHODIMP Show(PRInt16 *aReturn);

  virtual void InitNative(nsIWidget *aParent, const nsAString& aTitle, PRInt16 aMode);

  static void Shutdown();

protected:
  nsCOMPtr<nsIWidget>    mParentWidget;
  nsCOMArray<nsILocalFile> mFiles;

  PRInt16   mMode;
  PRInt16   mSelectedType;
  PRBool    mAllowURLs;
  nsCString mFileURL;
  nsString  mTitle;
  nsString  mDefault;
  nsString  mDefaultExtension;

  nsTArray<nsCString> mFilters;
  nsTArray<nsCString> mFilterNames;

};

#endif // nsFilePicker_h__

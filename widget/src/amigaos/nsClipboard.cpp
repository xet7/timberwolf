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


#include "nsClipboard.h"
#include "nsCOMPtr.h"
#include "nsITransferable.h"
#include "nsWidgetsCID.h"
#include "nsXPIDLString.h"
#include "nsPrimitiveHelpers.h"
#include "nsXPCOM.h"
#include "nsISupportsPrimitives.h"
#include "nsString.h"


NS_IMPL_ISUPPORTS1(nsClipboard, nsIClipboard)

//-------------------------------------------------------------------------
//
// nsClipboard constructor
//
//-------------------------------------------------------------------------
nsClipboard::nsClipboard()
{
	mIFF = IIFFParse->AllocIFF();
}

//-------------------------------------------------------------------------
//
// nsClipboard destructor
//
//-------------------------------------------------------------------------
nsClipboard::~nsClipboard()
{
	if (mIFF)
		IIFFParse->FreeIFF(mIFF);
}


NS_IMETHODIMP
nsClipboard::SetData(nsITransferable *aTransferable,
                     nsIClipboardOwner *aOwner, PRInt32 aWhichClipboard)
{
    nsresult rv;
    if (!mPrivacyHandler)
    {
        rv = NS_NewClipboardPrivacyHandler(getter_AddRefs(mPrivacyHandler));
        NS_ENSURE_SUCCESS(rv, rv);
    }
    rv = mPrivacyHandler->PrepareDataForClipboard(aTransferable);
    NS_ENSURE_SUCCESS(rv, rv);

    EmptyClipboard(aWhichClipboard);

    // Get the types of supported flavors
    nsCOMPtr<nsISupportsArray> flavors;

    rv = aTransferable->FlavorsTransferableCanExport(getter_AddRefs(flavors));
    NS_ENSURE_SUCCESS(rv, rv);

    if (!flavors || NS_FAILED(rv))
        return NS_ERROR_FAILURE;

    PRBool imagesAdded = PR_FALSE;
    PRUint32 count;
    flavors->Count(&count);

    nsCOMPtr<nsISupports> tmp;
    PRUint32 len;

    for (PRUint32 i=0; i < count; i++)
    {
        nsCOMPtr<nsISupports> tastesLike;
        flavors->GetElementAt(i, getter_AddRefs(tastesLike));
        nsCOMPtr<nsISupportsCString> flavor = do_QueryInterface(tastesLike);

        if (flavor)
        {
            nsXPIDLCString flavorStr;
            flavor->ToString(getter_Copies(flavorStr));

            if (!strcmp(flavorStr, kUnicodeMime) || !strcmp(flavorStr, kTextMime) || !strcmp(flavorStr, kHTMLMime)
            		||!strcmp(flavorStr, kMozTextInternal))
            {
            	rv = aTransferable->GetTransferData(kUnicodeMime, getter_AddRefs(tmp), &len);
            	NS_ENSURE_SUCCESS(rv, rv);

            	nsCOMPtr<nsISupportsString> wideString;
            	wideString = do_QueryInterface(tmp);
            	if (!wideString)
            		return NS_ERROR_FAILURE;

            	nsAutoString usc2string;
            	wideString->GetData(usc2string);
                char *utf8string = ToNewUTF8String(usc2string);
                if (!utf8string)
                    return NS_ERROR_FAILURE;

                CopyTextToClipboard(0, utf8string, strlen(utf8string));

                nsMemory::Free(utf8string);
                break;
            }

        }
    }

	return NS_OK;
}

NS_IMETHODIMP
nsClipboard::GetData(nsITransferable *aTransferable, PRInt32 aWhichClipboard)
{
	if (!aTransferable)
		return NS_ERROR_FAILURE;

	nsCOMPtr<nsISupportsArray> flavors;
	nsresult rv;

	rv = aTransferable->FlavorsTransferableCanImport(getter_AddRefs(flavors));
	if (!flavors || NS_FAILED(rv))
		return NS_ERROR_FAILURE;

	PRUint32 count;
	flavors->Count(&count);

	for (PRUint32 i = 0; i < count; i++)
	{
		nsCOMPtr<nsISupports> genericFlavor;
		flavors->GetElementAt(i, getter_AddRefs(genericFlavor));

		nsCOMPtr<nsISupportsCString> currentFlavor;
		currentFlavor = do_QueryInterface(genericFlavor);

		if (currentFlavor)
		{
			nsXPIDLCString flavorStr;
			currentFlavor->ToString(getter_Copies(flavorStr));

			if (!strcmp(flavorStr, kUnicodeMime))
			{
				char *text = CopyTextFromClipboard(0);
				if (text)
				{
					NS_ConvertUTF8toUTF16 ucs2string(text);
					void *data = ToNewUnicode(ucs2string);
					PRUint32 length = ucs2string.Length() * 2;
					delete [] text;

					nsCOMPtr<nsISupports> wrapper;
					nsPrimitiveHelpers::CreatePrimitiveForData(flavorStr, data, length, getter_AddRefs(wrapper));
					aTransferable->SetTransferData(flavorStr, wrapper, length);

					nsMemory::Free(data);

					return NS_OK;
				}
			}
		}
	}
	return NS_OK;
}

NS_IMETHODIMP
nsClipboard::EmptyClipboard(PRInt32 aWhichClipboard)
{

	return NS_OK;
}

NS_IMETHODIMP
nsClipboard::HasDataMatchingFlavors(const char** aFlavorList, PRUint32 aLength,
                                    PRInt32 aWhichClipboard, PRBool *_retval)
{
	uint32 cbData = GetClipboardDataType(0);
	*_retval = PR_FALSE;

	if (cbData == kNone)
		return NS_OK;

	for (int i = 0; i < aLength; i++)
	{
		//printf("%s\n", aFlavorList[i]);
		if (!strncmp(aFlavorList[i], "text", 4) && cbData == kText)
		{
			*_retval = PR_TRUE;
			break;
		}
	}

	return NS_OK;
}

NS_IMETHODIMP
nsClipboard::SupportsSelectionClipboard(PRBool *_retval)
{
    *_retval = PR_FALSE; //XXX: We can actually support it, but let's keep it simple for now
    return NS_OK;
}


/* Read and write AmigaOS clipboard */

#define  ID_FTXT        MAKE_ID('F','T','X','T')
#define  ID_CHRS        MAKE_ID('C','H','R','S')

#define  ID_ILBM        MAKE_ID('I','L','B','M')

void
nsClipboard::CopyTextToClipboard(uint32 unit, char *buffer, uint32 len)
{
	int32 error = 0;

	if ((mIFF->iff_Stream = (uint32) IIFFParse->OpenClipboard(unit)))
	{
		IIFFParse->InitIFFasClip(mIFF);
		if (!(error = IIFFParse->OpenIFF(mIFF, IFFF_WRITE)))
		{
			if (!(error = IIFFParse->PushChunk(mIFF, ID_FTXT, ID_FORM, IFFSIZE_UNKNOWN)))
			{
				if (!(error = IIFFParse->PushChunk(mIFF, 0, ID_CHRS, IFFSIZE_UNKNOWN)))
				{
					if (IIFFParse->WriteChunkBytes(mIFF, buffer, len) != len)
					{
						error = IFFERR_WRITE;
					}

					IIFFParse->PopChunk(mIFF);
				}

				IIFFParse->PopChunk(mIFF);
			}

			IIFFParse->CloseIFF(mIFF);
		}

		IIFFParse->CloseClipboard((struct ClipboardHandle *)mIFF->iff_Stream);
	}

}

char *
nsClipboard::CopyTextFromClipboard(uint32 unit)
{
	char *res = NULL;
	int32 error = 0;

	if ((mIFF->iff_Stream = (uint32) IIFFParse->OpenClipboard(unit)))
	{
		IIFFParse->InitIFFasClip(mIFF);
		if (error = IIFFParse->OpenIFF(mIFF, IFFF_READ))
		{
			// Any error means we assume the clipboard to be empty
			IIFFParse->CloseClipboard((struct ClipboardHandle *)mIFF->iff_Stream);
			return 0;
		}

		IIFFParse->StopChunk(mIFF, ID_FTXT, ID_CHRS);

		while (1)
		{
			error = IIFFParse->ParseIFF(mIFF, IFFPARSE_SCAN);
			if (error == IFFERR_EOC)
				continue;
			else if (error)
				break;

			struct ContextNode *cn = IIFFParse->CurrentChunk(mIFF);

			if ((cn) && (cn->cn_Type == ID_FTXT) && (cn->cn_ID == ID_CHRS))
			{
				uint8 rdBuf[512];
				int32 rlen;

				res = new char[cn->cn_Size + 1];

				rlen = IIFFParse->ReadChunkBytes(mIFF, res, cn->cn_Size);
				if (rlen > 0)
				{
					res[rlen+1] = 0;
					break;
				}
			}
		}

		IIFFParse->CloseIFF(mIFF);
		IIFFParse->CloseClipboard((struct ClipboardHandle *)mIFF->iff_Stream);
	}

	return res;
}

uint32
nsClipboard::GetClipboardDataType(uint32 unit)
{
	int32 error = 0;
	uint32 result = kNone;

	if ((mIFF->iff_Stream = (uint32) IIFFParse->OpenClipboard(unit)))
	{
		IIFFParse->InitIFFasClip(mIFF);

		if (IIFFParse->OpenIFF(mIFF, IFFF_READ))
		{
			// Any error means we assume the clipboard to be empty
			IIFFParse->CloseClipboard((struct ClipboardHandle *)mIFF->iff_Stream);
			return kNone;
		}

		/* Start parsing. We basically just need to check the first 'FORMS' chunk.
		 * FTXT means we have text, ILBM means image data.
		 */
		while (1)
		{
			error = IIFFParse->ParseIFF(mIFF, IFFPARSE_RAWSTEP);

			/* error is either 0 (no error), or EOC (end of context). EOC is skipped,
			 * any other error means we're breaking off and return kNone.
			 */
			if (error == IFFERR_EOC)
				continue;
			else if (error != 0)
				break;

			struct ContextNode *cn = IIFFParse->CurrentChunk(mIFF);

			if (cn->cn_ID == ID_FORM && cn->cn_Type == ID_FTXT)
			{
				result = kText;
				break;
			}
		}

		IIFFParse->CloseIFF(mIFF);
		IIFFParse->CloseClipboard((struct ClipboardHandle *)mIFF->iff_Stream);
	}

	return result;
}

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


#include <primpl.h>

#define DEBUG_FILEMAP
#ifdef DEBUG_FILEMAP
#define dprintf(format, args...) {printf("[%s] " format, __FUNCTION__, ##args); fflush(stdout);}
#else
#define dprintf(format, args...)
#endif

PRStatus _amigaos_CreateFileMap(PRFileMap *fmap, PRInt64 size)
{
	// Currently does nothing but checking for writability. We don't support that yet
	if (fmap->prot != PR_PROT_READONLY)
	{
		PR_SetError(PR_NO_ACCESS_RIGHTS_ERROR, 0);
		dprintf("Opening filmap read/write\n");
		return PR_FAILURE;
	}

	return PR_SUCCESS;
}

PRInt32 _amigaos_GetMemMapAlignment(void)
{
	// Pretty arbitrary at the moment
	return 4096;
}

void * _amigaos_MemMap(
    PRFileMap *fmap,
    PRInt64 offset,
    PRUint32 len)
{
	PRUint32 rv = lseek(fmap->fd->secret->md.osfd, (int)offset, SEEK_SET);
	if (rv == -1)
	{
		dprintf("seek %d failed\n", (int)offset);
		return NULL;
	}

	void *base = malloc(len);
	if (base)
	{
		rv = read(fmap->fd->secret->md.osfd, base, (size_t)len);
		if (rv == len)
		{
			return base;
		}

		dprintf("Read failed, wanted %d, got %d\n", (int)len, rv);
		free(base);
	}

	return NULL;
}

PRStatus _amigaos_MemUnmap(void *addr, PRUint32 len)
{
	free(addr);
	return PR_SUCCESS;
}

PRStatus _amigaos_CloseFileMap(PRFileMap *fmap)
{
	return PR_SUCCESS;
}


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

PR_IMPLEMENT(PRStatus) _amigaos_AllocSegment(PRSegment *seg, PRUint32 size, void *vaddr)
{
	PR_ASSERT(seg != 0);
	PR_ASSERT(size != 0);
	PR_ASSERT(vaddr == 0);

	seg->vaddr = PR_MALLOC(size);
	if (!seg->vaddr)
		return PR_FAILURE;

	seg->size = size;

	return PR_SUCCESS;
}


PR_IMPLEMENT(void) _amigaos_FreeSegment(PRSegment *seg)
{
	PR_ASSERT(seg != 0);
	PR_ASSERT((seg->flags & _PR_SEG_VM) == 0);

	if (seg->vaddr != NULL)
		PR_DELETE(seg->vaddr);

	return;
}

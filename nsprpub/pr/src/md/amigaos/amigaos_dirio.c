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
#include <sys/dirent.h>

PR_IMPLEMENT(PRStatus) _amigaos_OpenDir(_MDDir *d, const char *name)
{
	int err;

    d->d = opendir(name);
    if (!d->d) 
    {
		err = _MD_ERRNO();
		_PR_MD_MAP_OPENDIR_ERROR(err);
		return PR_FAILURE;
    }
    return PR_SUCCESS;
}

PR_IMPLEMENT(PRInt32) _amigaos_CloseDir(_MDDir *d)
{
	int rv = 0, err;

	if (d->d) 
	{
		rv = closedir(d->d);
		if (rv == -1) 
		{
			err = _MD_ERRNO();
			_PR_MD_MAP_CLOSEDIR_ERROR(err);
        }
    }
    return rv;
}

PR_IMPLEMENT(char *) _amigaos_ReadDir(_MDDir *d, PRIntn flags)
{
	struct dirent *de;
	int err;

	for (;;) 
	{
    	/*
         * XXX: readdir() is not MT-safe. There is an MT-safe version
         * readdir_r() on some systems.
         */
		_MD_ERRNO() = 0;
		de = readdir(d->d);
		if (!de) 
		{
			err = _MD_ERRNO();
			_PR_MD_MAP_READDIR_ERROR(err);
			return 0;
		}        
		if ((flags & PR_SKIP_DOT) &&
			(de->d_name[0] == '.') && (de->d_name[1] == 0))
			continue;
		if ((flags & PR_SKIP_DOT_DOT) &&
			(de->d_name[0] == '.') && (de->d_name[1] == '.') &&
			(de->d_name[2] == 0))
			continue;
		if ((flags & PR_SKIP_HIDDEN) && (de->d_name[0] == '.'))
			continue;
		break;
	}
	return de->d_name;
}

PR_IMPLEMENT(PRInt32) _amigaos_MakeDir(const char *name, PRIntn mode)
{
	int rv, err;

	rv = mkdir(name, mode);
	if (rv < 0) 
	{
		err = _MD_ERRNO();
		_PR_MD_MAP_MKDIR_ERROR(err);
	}

	return rv;
}

PR_IMPLEMENT(PRInt32) _amigaos_RemoveDir(const char *name)
{
	int rv, err;

	rv = rmdir(name);
	if (rv == -1) 
	{
		err = _MD_ERRNO();
		_PR_MD_MAP_RMDIR_ERROR(err);
	}
	return rv;
}

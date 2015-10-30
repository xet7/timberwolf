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
#include <fcntl.h>
#include <sys/stat.h>

PR_IMPLEMENT(PRInt32) _amigaos_Open(const char *name, PRIntn flags, PRIntn mode)
{
    PRInt32 osflags;
    PRInt32 rv;

    if (flags & PR_RDWR)
    {
    	osflags = O_RDWR;
    }
    else if (flags & PR_WRONLY)
    {
    	osflags = O_WRONLY;
    }
    else
    {
    	osflags = O_RDONLY;
    }

    if (flags & PR_EXCL)
	osflags |= O_EXCL;

    if (flags & PR_APPEND)
	osflags |= O_APPEND;

    if (flags & PR_TRUNCATE)
	osflags |= O_TRUNC;

    if (flags & PR_SYNC)
	osflags |= O_SYNC;

    if (flags & PR_CREATE_FILE)
	osflags |= O_CREAT;

    rv = open(name, osflags, mode);

    if (rv < 0)
    {
	_PR_MD_MAP_OPEN_ERROR(_MD_ERRNO());
    }

    return rv;
}


PR_IMPLEMENT(PRInt32) _amigaos_CloseFile(PRInt32 osfd)
{
    PRInt32 rv;

    rv = close(osfd);

    if (rv == -1)
    {
	_PR_MD_MAP_CLOSE_ERROR(_MD_ERRNO());
    }

    return rv;
}


PR_IMPLEMENT(PRInt32) _amigaos_Read(PRFileDesc *fd, void *buffer, PRInt32 len)
{
    PRInt32 rv;

    rv = (PRInt32)read(fd->secret->md.osfd, buffer, (size_t)len);

    if (rv < 0)
    {
	_PR_MD_MAP_READ_ERROR(_MD_ERRNO());
    }

    return (PRInt32)rv;
}


PR_IMPLEMENT(PRInt32) _amigaos_Write(PRFileDesc *fd, const void *buffer, PRInt32 len)
{
    PRInt32 rv;

    rv = write(fd->secret->md.osfd, buffer, (size_t)len);

    if (rv < 0)
    {
	_PR_MD_MAP_WRITE_ERROR(_MD_ERRNO());
    }

    return (PRInt32)rv;
}


PR_IMPLEMENT(PRInt32) _amigaos_FSync(PRFileDesc *fd)
{
    PRInt32 rv, err;

    rv = fsync(fd->secret->md.osfd);
    if (rv == -1)
    {
        err = _MD_ERRNO();
        _PR_MD_MAP_FSYNC_ERROR(err);
    }
    return(rv);
}


PR_IMPLEMENT(PRInt32) _amigaos_Delete(const char *name)
{
    PRInt32 rv;

    rv = unlink(name);

    if (rv < 0)
    {
	_PR_MD_MAP_UNLINK_ERROR(_MD_ERRNO());
    }

    return rv;
}


PR_IMPLEMENT(PRInt32) _amigaos_Rename(const char *from, const char *to)
{
    PRInt32 rv;

    if (access(to, F_OK) == 0)
    {
	PR_SetError(PR_FILE_EXISTS_ERROR, 0);
	return -1;
    }

    rv = rename(from, to);
    if (rv < 0)
    {
	_PR_MD_MAP_RENAME_ERROR(_MD_ERRNO());
    }

    return rv;
}


PR_IMPLEMENT(PRInt32) _amigaos_Access(const char *name, PRAccessHow how)
{
    PRInt32 rv;
    int mode;

    switch (how)
    {
    case PR_ACCESS_WRITE_OK:
	mode = W_OK;
	break;
    case PR_ACCESS_READ_OK:
	mode = R_OK;
	break;
    case PR_ACCESS_EXISTS:
	mode = F_OK;
	break;
     default:
 	PR_SetError(PR_INVALID_ARGUMENT_ERROR, 0);
        return -1;
    }

    rv = access(name, mode);

    if (rv < 0)
    {
	_PR_MD_MAP_ACCESS_ERROR(_MD_ERRNO());
    }

    return rv;
}


PR_IMPLEMENT(PRInt32) _amigaos_Stat(const char *name, struct stat *sb)
{
    PRInt32 rv;

    rv = stat(name, sb);

    if (rv < 0)
    {
	_PR_MD_MAP_STAT_ERROR(_MD_ERRNO());
    }

    return rv;
}


PR_IMPLEMENT(void) _amigaos_MakeNonblock(PRFileDesc *fd)
{
    /* TODO: Does this actually work with newlib? */
    PRInt32 osfd = fd->secret->md.osfd;
    int flags;

    if (osfd <= 2)
    {
        /* Don't mess around with stdin, stdout or stderr */
        return;
    }

    flags = fcntl(osfd, F_GETFL, 0);
    fcntl(osfd, F_SETFL, flags | O_NONBLOCK);
}


PR_IMPLEMENT(PROffset32) _amigaos_LSeek(PRFileDesc *fd, PROffset32 offset, PRSeekWhence whence)
{
    PROffset32 rv, where;

    switch (whence)
    {
    case PR_SEEK_SET:
	where = SEEK_SET;
        break;
    case PR_SEEK_CUR:
    	where = SEEK_CUR;
        break;
    case PR_SEEK_END:
        where = SEEK_END;
        break;
    default:
	PR_SetError(PR_INVALID_ARGUMENT_ERROR, 0);
	return -1;
    }

    rv = lseek(fd->secret->md.osfd, offset, where);

    if (rv == -1)
    {
        _PR_MD_MAP_LSEEK_ERROR(_MD_ERRNO());
    }

    return rv;
}


PR_IMPLEMENT(PROffset64) _amigaos_LSeek64(PRFileDesc *fd, PROffset64 offset, PRSeekWhence whence)
{
    PROffset64 rv, where;

    if (offset > 0xffffffff)
    {
	PR_SetError(PR_FILE_TOO_BIG_ERROR, 0);
	return -1;
	}

    switch (whence)
    {
    case PR_SEEK_SET:
	where = SEEK_SET;
        break;
    case PR_SEEK_CUR:
	where = SEEK_CUR;
	break;
    case PR_SEEK_END:
	where = SEEK_END;
	break;
    default:
	PR_SetError(PR_INVALID_ARGUMENT_ERROR, 0);
	return -1;
    }

    rv = (PROffset64)lseek(fd->secret->md.osfd, (PROffset32)offset, where);

    if (rv == -1)
    {
        _PR_MD_MAP_LSEEK_ERROR(_MD_ERRNO());
    }

    return rv;
}


static int _amigaos_convert_stat_to_fileinfo(const struct stat *sb,  PRFileInfo *info)
{
    if (_IFREG & sb->st_mode)
        info->type = PR_FILE_FILE;
    else if (_IFDIR & sb->st_mode)
        info->type = PR_FILE_DIRECTORY;
    else
        info->type = PR_FILE_OTHER;

    info->size = sb->st_size;
    info->creationTime = (PRTime)sb->st_ctime * 1000;
	info->modifyTime = (PRTime)sb->st_mtime * 1000;

    return 0;
}

static int _amigaos_convert_stat_to_fileinfo64(const struct stat *sb,  PRFileInfo64 *info)
{
    if (_IFREG & sb->st_mode)
        info->type = PR_FILE_FILE;
    else if (_IFDIR & sb->st_mode)
        info->type = PR_FILE_DIRECTORY;
    else
        info->type = PR_FILE_OTHER;

    LL_I2L(info->size, sb->st_size);
	info->creationTime = (PRTime)sb->st_ctime * 1000;
	info->modifyTime = (PRTime)sb->st_mtime * 1000;

    return 0;
}

PR_IMPLEMENT(PRInt32) _amigaos_Getfileinfo(const char *fn, PRFileInfo *info)
{
    PRInt32 rv;
    struct stat sb;

    rv = stat(fn, &sb);
    if (rv < 0)
    {
        _PR_MD_MAP_STAT_ERROR(_MD_ERRNO());
    }
    else if (NULL != info)
    {
        rv = _amigaos_convert_stat_to_fileinfo(&sb, info);
    }

    return rv;
}


PR_IMPLEMENT(PRInt32) _amigaos_Getfileinfo64(const char *fn, PRFileInfo64 *info)
{
    PRInt32 rv;
    struct stat sb;

    rv = stat(fn, &sb);

    if (rv < 0)
    {
        _PR_MD_MAP_STAT_ERROR(_MD_ERRNO());
    }
    else if (NULL != info)
    {
        rv = _amigaos_convert_stat_to_fileinfo64(&sb, info);
    }
    return rv;
}


PR_IMPLEMENT(PRInt32) _amigaos_Getopenfileinfo(const PRFileDesc *fd, PRFileInfo *info)
{
    struct stat sb;
    PRInt32 rv;

    rv = fstat(fd->secret->md.osfd, &sb);

    if (rv < 0)
    {
        _PR_MD_MAP_FSTAT_ERROR(_MD_ERRNO());
    }
    else if (NULL != info)
    {
        rv = _amigaos_convert_stat_to_fileinfo(&sb, info);
    }
    return rv;
}


PR_IMPLEMENT(PRInt32) _amigaos_Getopenfileinfo64(const PRFileDesc *fd, PRFileInfo64 *info)
{
    struct stat sb;
    PRInt32 rv;

    rv = fstat(fd->secret->md.osfd, &sb);

    if (rv < 0)
    {
        _PR_MD_MAP_FSTAT_ERROR(_MD_ERRNO());
    }
    else if (NULL != info)
    {
        rv = _amigaos_convert_stat_to_fileinfo64(&sb, info);
    }
    return rv;
}


PR_IMPLEMENT(PRStatus) _amigaos_LockFile(PRInt32 f)
{
    // TODO: I have my doubts this works - HJF -
    PRInt32 rv;
    struct flock arg;

    arg.l_type = F_WRLCK;
    arg.l_whence = SEEK_SET;
    arg.l_start = 0;
    arg.l_len = 0;  /* until EOF */

    rv = fcntl(f, F_SETLKW, &arg);
    if (rv == 0)
	return PR_SUCCESS;

    _PR_MD_MAP_FLOCK_ERROR(_MD_ERRNO());
    return PR_FAILURE;
}


PR_IMPLEMENT(PRStatus) _amigaos_TLockFile(PRInt32 f)
{
    // TODO: I have my doubts this works (see above)   - HJF -
    PRInt32 rv;
    struct flock arg;

    arg.l_type = F_WRLCK;
    arg.l_whence = SEEK_SET;
    arg.l_start = 0;
    arg.l_len = 0;  /* until EOF */

    rv = fcntl(f, F_SETLK, &arg);
    if (rv == 0)
	return PR_SUCCESS;

    _PR_MD_MAP_FLOCK_ERROR(_MD_ERRNO());
    return PR_FAILURE;
}


PR_IMPLEMENT(PRStatus) _amigaos_UnlockFile(PRInt32 f)
{
    // TODO: same doubts as the above two. - HJF -
    PRInt32 rv;
    struct flock arg;

    arg.l_type = F_UNLCK;
    arg.l_whence = SEEK_SET;
    arg.l_start = 0;
    arg.l_len = 0;  /* until EOF */

    rv = fcntl(f, F_SETLK, &arg);
    if (rv == 0)
	return PR_SUCCESS;

    _PR_MD_MAP_FLOCK_ERROR(_MD_ERRNO());
    return PR_FAILURE;
}


PR_IMPLEMENT(void) _amigaos_InitFdInheritable(PRFileDesc *fd, PRBool imported)
{
    if (imported) {
        fd->secret->inheritable = _PR_TRI_UNKNOWN;
    } else {
        fd->secret->inheritable = _PR_TRI_FALSE;
    }
}


PR_IMPLEMENT(void) _amigaos_QueryFdInheritable(PRFileDesc *fd)
{
	fd->secret->inheritable = _PR_TRI_FALSE;
}

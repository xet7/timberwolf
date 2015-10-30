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


#ifndef pramigaerrors_h___
#define pramigaerrors_h___

#include <unistd.h>
#include <stddef.h>

PR_BEGIN_EXTERN_C

NSPR_API(void) _MD_amiga_map_default_error(int err);
#define	_PR_MD_MAP_DEFAULT_ERROR	_MD_amiga_map_default_error

NSPR_API(void) _MD_amiga_map_opendir_error(int err);
#define	_PR_MD_MAP_OPENDIR_ERROR	_MD_amiga_map_opendir_error

NSPR_API(void) _MD_amiga_map_closedir_error(int err);
#define	_PR_MD_MAP_CLOSEDIR_ERROR	_MD_amiga_map_closedir_error

NSPR_API(void) _MD_amiga_readdir_error(int err);
#define	_PR_MD_MAP_READDIR_ERROR	_MD_amiga_readdir_error

NSPR_API(void) _MD_amiga_map_unlink_error(int err);
#define	_PR_MD_MAP_UNLINK_ERROR	_MD_amiga_map_unlink_error

NSPR_API(void) _MD_amiga_map_stat_error(int err);
#define	_PR_MD_MAP_STAT_ERROR	_MD_amiga_map_stat_error

NSPR_API(void) _MD_amiga_map_fstat_error(int err);
#define	_PR_MD_MAP_FSTAT_ERROR	_MD_amiga_map_fstat_error

NSPR_API(void) _MD_amiga_map_rename_error(int err);
#define	_PR_MD_MAP_RENAME_ERROR	_MD_amiga_map_rename_error

NSPR_API(void) _MD_amiga_map_access_error(int err);
#define	_PR_MD_MAP_ACCESS_ERROR	_MD_amiga_map_access_error

NSPR_API(void) _MD_amiga_map_mkdir_error(int err);
#define	_PR_MD_MAP_MKDIR_ERROR	_MD_amiga_map_mkdir_error

NSPR_API(void) _MD_amiga_map_rmdir_error(int err);
#define	_PR_MD_MAP_RMDIR_ERROR	_MD_amiga_map_rmdir_error

NSPR_API(void) _MD_amiga_map_read_error(int err);
#define	_PR_MD_MAP_READ_ERROR	_MD_amiga_map_read_error

NSPR_API(void) _MD_amiga_map_write_error(int err);
#define	_PR_MD_MAP_WRITE_ERROR	_MD_amiga_map_write_error

NSPR_API(void) _MD_amiga_map_lseek_error(int err);
#define	_PR_MD_MAP_LSEEK_ERROR	_MD_amiga_map_lseek_error

NSPR_API(void) _MD_amiga_map_fsync_error(int err);
#define	_PR_MD_MAP_FSYNC_ERROR	_MD_amiga_map_fsync_error

NSPR_API(void) _MD_amiga_map_close_error(int err);
#define	_PR_MD_MAP_CLOSE_ERROR	_MD_amiga_map_close_error

NSPR_API(void) _MD_amiga_map_socket_error(int err);
#define	_PR_MD_MAP_SOCKET_ERROR	_MD_amiga_map_socket_error

NSPR_API(void) _MD_amiga_map_socketavailable_error(int err);
#define	_PR_MD_MAP_SOCKETAVAILABLE_ERROR	_MD_amiga_map_socketavailable_error

NSPR_API(void) _MD_amiga_map_recv_error(int err);
#define	_PR_MD_MAP_RECV_ERROR	_MD_amiga_map_recv_error

NSPR_API(void) _MD_amiga_map_recvfrom_error(int err);
#define	_PR_MD_MAP_RECVFROM_ERROR	_MD_amiga_map_recvfrom_error

NSPR_API(void) _MD_amiga_map_send_error(int err);
#define	_PR_MD_MAP_SEND_ERROR	_MD_amiga_map_send_error

NSPR_API(void) _MD_amiga_map_sendto_error(int err);
#define	_PR_MD_MAP_SENDTO_ERROR	_MD_amiga_map_sendto_error

NSPR_API(void) _MD_amiga_map_writev_error(int err);
#define	_PR_MD_MAP_WRITEV_ERROR	_MD_amiga_map_writev_error

NSPR_API(void) _MD_amiga_map_accept_error(int err);
#define	_PR_MD_MAP_ACCEPT_ERROR	_MD_amiga_map_accept_error

NSPR_API(void) _MD_amiga_map_connect_error(int err);
#define	_PR_MD_MAP_CONNECT_ERROR	_MD_amiga_map_connect_error

NSPR_API(void) _MD_amiga_map_bind_error(int err);
#define	_PR_MD_MAP_BIND_ERROR	_MD_amiga_map_bind_error

NSPR_API(void) _MD_amiga_map_listen_error(int err);
#define	_PR_MD_MAP_LISTEN_ERROR	_MD_amiga_map_listen_error

NSPR_API(void) _MD_amiga_map_shutdown_error(int err);
#define	_PR_MD_MAP_SHUTDOWN_ERROR	_MD_amiga_map_shutdown_error

NSPR_API(void) _MD_amiga_map_socketpair_error(int err);
#define	_PR_MD_MAP_SOCKETPAIR_ERROR	_MD_amiga_map_socketpair_error

NSPR_API(void) _MD_amiga_map_getsockname_error(int err);
#define	_PR_MD_MAP_GETSOCKNAME_ERROR	_MD_amiga_map_getsockname_error

NSPR_API(void) _MD_amiga_map_getpeername_error(int err);
#define	_PR_MD_MAP_GETPEERNAME_ERROR	_MD_amiga_map_getpeername_error

NSPR_API(void) _MD_amiga_map_getsockopt_error(int err);
#define	_PR_MD_MAP_GETSOCKOPT_ERROR	_MD_amiga_map_getsockopt_error

NSPR_API(void) _MD_amiga_map_setsockopt_error(int err);
#define	_PR_MD_MAP_SETSOCKOPT_ERROR	_MD_amiga_map_setsockopt_error

NSPR_API(void) _MD_amiga_map_open_error(int err);
#define	_PR_MD_MAP_OPEN_ERROR	_MD_amiga_map_open_error

NSPR_API(void) _MD_amiga_map_mmap_error(int err);
#define	_PR_MD_MAP_MMAP_ERROR	_MD_amiga_map_mmap_error

NSPR_API(void) _MD_amiga_map_gethostname_error(int err);
#define	_PR_MD_MAP_GETHOSTNAME_ERROR	_MD_amiga_map_gethostname_error

NSPR_API(void) _MD_amiga_map_select_error(int err);
#define	_PR_MD_MAP_SELECT_ERROR	_MD_amiga_map_select_error

NSPR_API(void) _MD_amiga_map_poll_error(int err);
#define _PR_MD_MAP_POLL_ERROR _MD_amiga_map_poll_error

NSPR_API(void) _MD_amiga_map_poll_revents_error(int err);
#define _PR_MD_MAP_POLL_REVENTS_ERROR _MD_amiga_map_poll_revents_error

NSPR_API(void) _MD_amiga_map_flock_error(int err);
#define	_PR_MD_MAP_FLOCK_ERROR	_MD_amiga_map_flock_error

NSPR_API(void) _MD_amiga_map_lockf_error(int err);
#define	_PR_MD_MAP_LOCKF_ERROR	_MD_amiga_map_lockf_error

PR_END_EXTERN_C

#endif /* pramigaerrors_h___ */

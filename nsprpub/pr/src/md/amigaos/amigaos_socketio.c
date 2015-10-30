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


#include <exec/exec.h>
#include <prinet.h>
#include <proto/exec.h>
#include <sys/filio.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/socket.h>
#include <sys/utsname.h>

#include "primpl.h"

//#define EWOULDBLOCK     EAGAIN          /* Operation would block */
//#define EINPROGRESS     36              /* Operation now in progress */
//#define ECONNABORTED    53              /* Software caused connection abort */

#define READ_FD        	1
#define WRITE_FD    	2

//#define DEBUG_SOCKETS
#ifdef DEBUG_SOCKETS
#define dprintf(format, args...) {printf("[%s / %s] " format, __FUNCTION__, IExec->FindTask(NULL)->tc_Node.ln_Name, ##args); fflush(stdout);}
#else
#define dprintf(format, args...)
#endif



/* Slightly modified UNIX version */
static PRInt32 socket_io_wait(PRInt32 osfd, PRInt32 fd_type,
		PRIntervalTime timeout)
{
	PRInt32 rv = -1;
	struct timeval tv;
	PRThread *me = _PR_MD_CURRENT_THREAD();
	PRIntervalTime epoch, now, elapsed, remaining;
	PRBool wait_for_remaining;
	PRInt32 syserror;
	fd_set rd_wr;

	dprintf("socket_io_wait(osfd = %d, fd_type = %d, timeout = %d)\n", osfd, fd_type, timeout);

	switch (timeout) {
	case PR_INTERVAL_NO_WAIT:
		PR_SetError(PR_IO_TIMEOUT_ERROR, 0);
		break;
	case PR_INTERVAL_NO_TIMEOUT:
		/*
		 * This is a special case of the 'default' case below.
		 * Please see the comments there.
		 */
		tv.tv_sec = _PR_INTERRUPT_CHECK_INTERVAL_SECS;
		tv.tv_usec = 0;
		FD_ZERO(&rd_wr);
		do {
			FD_SET(osfd, &rd_wr);

			PR_Sleep(PR_INTERVAL_NO_WAIT);

			if (fd_type == READ_FD)
				rv = _MD_SELECT(osfd + 1, &rd_wr, NULL, NULL, &tv);
			else
				rv = _MD_SELECT(osfd + 1, NULL, &rd_wr, NULL, &tv);

			if (rv == -1 && (syserror = _MD_ERRNO()) != EINTR) {
				_PR_MD_MAP_SELECT_ERROR(syserror);
				break;
			}
			dprintf("rv = %d, syserror = %d\n", rv, _MD_ERRNO());
			PR_Sleep(PR_INTERVAL_NO_WAIT);

			dprintf("blocked = %d, interrupt = %d\n", me->flags & _PR_INTERRUPT_BLOCKED, me->flags & _PR_INTERRUPT);
			if (_PR_PENDING_INTERRUPT(me)) {
				me->flags &= ~_PR_INTERRUPT;
				PR_SetError(PR_PENDING_INTERRUPT_ERROR, 0);
				rv = -1;
				dprintf("INTERRUPT\n");
				break;
			}
		} while (rv == 0 || (rv == -1 && syserror == EINTR));
		break;
	default:
		now = epoch = PR_IntervalNow();
		remaining = timeout;
		FD_ZERO(&rd_wr);
		do {
			/*
			 * We block in _MD_SELECT for at most
			 * _PR_INTERRUPT_CHECK_INTERVAL_SECS seconds,
			 * so that there is an upper limit on the delay
			 * before the interrupt bit is checked.
			 */
			wait_for_remaining = PR_TRUE;
			tv.tv_sec = PR_IntervalToSeconds(remaining);
			if (tv.tv_sec > _PR_INTERRUPT_CHECK_INTERVAL_SECS) {
				wait_for_remaining = PR_FALSE;
				tv.tv_sec = _PR_INTERRUPT_CHECK_INTERVAL_SECS;
				tv.tv_usec = 0;
			} else {
				tv.tv_usec = PR_IntervalToMicroseconds(
						remaining -
						PR_SecondsToInterval(tv.tv_sec));
			}
			FD_SET(osfd, &rd_wr);
			if (fd_type == READ_FD)
				rv = _MD_SELECT(osfd + 1, &rd_wr, NULL, NULL, &tv);
			else
				rv = _MD_SELECT(osfd + 1, NULL, &rd_wr, NULL, &tv);
			/*
			 * we don't consider EINTR a real error
			 */
			 if (rv == -1 && (syserror = _MD_ERRNO()) != EINTR) {
				 _PR_MD_MAP_SELECT_ERROR(syserror);
				 break;
			 }
			PR_Sleep(PR_INTERVAL_NO_WAIT);
			if (_PR_PENDING_INTERRUPT(me)) {
				me->flags &= ~_PR_INTERRUPT;
				PR_SetError(PR_PENDING_INTERRUPT_ERROR, 0);
				rv = -1;
				break;
			}
			/*
			 * We loop again if _MD_SELECT timed out or got interrupted
			 * by a signal, and the timeout deadline has not passed yet.
			 */
			if (rv == 0 || (rv == -1 && syserror == EINTR)) {
				/*
				 * If _MD_SELECT timed out, we know how much time
				 * we spent in blocking, so we can avoid a
				 * PR_IntervalNow() call.
				 */
				if (rv == 0) {
					if (wait_for_remaining) {
						now += remaining;
					} else {
						now += PR_SecondsToInterval(tv.tv_sec)
						+ PR_MicrosecondsToInterval(tv.tv_usec);
					}
				} else {
					now = PR_IntervalNow();
				}
				elapsed = (PRIntervalTime) (now - epoch);
				if (elapsed >= timeout) {
					PR_SetError(PR_IO_TIMEOUT_ERROR, 0);
					rv = -1;
					break;
				} else {
					remaining = timeout - elapsed;
				}
			}
		} while (rv == 0 || (rv == -1 && syserror == EINTR));
		break;
	}

	return(rv);
}

/* NativeThread stuff copied from uxpoll.c */
static PRInt32 NativeThreadSelect(
		PRPollDesc *pds, PRIntn npds, PRIntervalTime timeout)
{
	dprintf("NativeThreadSelect(npds = %d, timeout = %d)\n", npds, timeout);
	/*
	 * This code is almost a duplicate of w32poll.c's _PR_MD_PR_POLL().
	 */
	fd_set rd, wt, ex;
	PRFileDesc *bottom;
	PRPollDesc *pd, *epd;
	PRInt32 maxfd = -1, ready, err;
	PRIntervalTime remaining, elapsed, start;

	struct timeval tv, *tvp = NULL;

	FD_ZERO(&rd);
	FD_ZERO(&wt);
	FD_ZERO(&ex);

	ready = 0;
	for (pd = pds, epd = pd + npds; pd < epd; pd++)
	{
		PRInt16 in_flags_read = 0, in_flags_write = 0;
		PRInt16 out_flags_read = 0, out_flags_write = 0;

		if ((NULL != pd->fd) && (0 != pd->in_flags))
		{
			if (pd->in_flags & PR_POLL_READ)
			{
				in_flags_read = (pd->fd->methods->poll)(
						pd->fd, pd->in_flags & ~PR_POLL_WRITE, &out_flags_read);
			}
			if (pd->in_flags & PR_POLL_WRITE)
			{
				in_flags_write = (pd->fd->methods->poll)(
						pd->fd, pd->in_flags & ~PR_POLL_READ, &out_flags_write);
			}
			if ((0 != (in_flags_read & out_flags_read))
					|| (0 != (in_flags_write & out_flags_write)))
			{
				/* this one's ready right now */
				if (0 == ready)
				{
					/*
					 * We will have to return without calling the
					 * system poll/select function.  So zero the
					 * out_flags fields of all the poll descriptors
					 * before this one.
					 */
					PRPollDesc *prev;
					for (prev = pds; prev < pd; prev++)
					{
						prev->out_flags = 0;
					}
				}
				ready += 1;
				pd->out_flags = out_flags_read | out_flags_write;
			}
			else
			{
				pd->out_flags = 0;  /* pre-condition */

				/* make sure this is an NSPR supported stack */
				bottom = PR_GetIdentitiesLayer(pd->fd, PR_NSPR_IO_LAYER);
				PR_ASSERT(NULL != bottom);  /* what to do about that? */
				if ((NULL != bottom)
						&& (_PR_FILEDESC_OPEN == bottom->secret->state))
				{
					if (0 == ready)
					{
						PRInt32 osfd = bottom->secret->md.osfd;
						if (osfd > maxfd) maxfd = osfd;
						if (in_flags_read & PR_POLL_READ)
						{
							pd->out_flags |= _PR_POLL_READ_SYS_READ;
							FD_SET(osfd, &rd);
						}
						if (in_flags_read & PR_POLL_WRITE)
						{
							pd->out_flags |= _PR_POLL_READ_SYS_WRITE;
							FD_SET(osfd, &wt);
						}
						if (in_flags_write & PR_POLL_READ)
						{
							pd->out_flags |= _PR_POLL_WRITE_SYS_READ;
							FD_SET(osfd, &rd);
						}
						if (in_flags_write & PR_POLL_WRITE)
						{
							pd->out_flags |= _PR_POLL_WRITE_SYS_WRITE;
							FD_SET(osfd, &wt);
						}
						if (pd->in_flags & PR_POLL_EXCEPT) FD_SET(osfd, &ex);
					}
				}
				else
				{
					if (0 == ready)
					{
						PRPollDesc *prev;
						for (prev = pds; prev < pd; prev++)
						{
							prev->out_flags = 0;
						}
					}
					ready += 1;  /* this will cause an abrupt return */
					pd->out_flags = PR_POLL_NVAL;  /* bogii */
				}
			}
		}
		else
		{
			pd->out_flags = 0;
		}
	}

	if (0 != ready) return ready;  /* no need to block */

	remaining = timeout;
	start = PR_IntervalNow();

	retry:
	if (timeout != PR_INTERVAL_NO_TIMEOUT)
	{
		PRInt32 ticksPerSecond = PR_TicksPerSecond();
		tv.tv_sec = remaining / ticksPerSecond;
		tv.tv_usec = PR_IntervalToMicroseconds( remaining % ticksPerSecond );
		tvp = &tv;
	}

	/*
	 ** TODO: GLITCH ALERT!
	 ** newlib's select barfs when the number of fd's is too big.
	 ** I'll return PR_UNKNOWN_ERROR for that case for now
	 */
	if (maxfd > 256)
	{
		fprintf(stderr, "*** NOTICE: newlib select problem with > 256 maxfd\n");
		PR_SetError(PR_UNKNOWN_ERROR, ENOTSUP);
		return -1;
	}


	ready = _MD_SELECT(maxfd + 1, &rd, &wt, &ex, tvp);

	if (ready == -1 && errno == EINTR)
	{
		if (timeout == PR_INTERVAL_NO_TIMEOUT) goto retry;
		else
		{
			elapsed = (PRIntervalTime) (PR_IntervalNow() - start);
			if (elapsed > timeout)
			{
				ready = 0;  /* timed out */
			}
			else
			{
				remaining = timeout - elapsed;
				goto retry;
			}
		}
	}

	/*
	 ** Now to unravel the select sets back into the client's poll
	 ** descriptor list. Is this possibly an area for pissing away
	 ** a few cycles or what?
	 */
	if (ready > 0)
	{
		ready = 0;
		for (pd = pds, epd = pd + npds; pd < epd; pd++)
		{
			PRInt16 out_flags = 0;
			if ((NULL != pd->fd) && (0 != pd->in_flags))
			{
				PRInt32 osfd;
				bottom = PR_GetIdentitiesLayer(pd->fd, PR_NSPR_IO_LAYER);
				PR_ASSERT(NULL != bottom);

				osfd = bottom->secret->md.osfd;

				if (FD_ISSET(osfd, &rd))
				{
					if (pd->out_flags & _PR_POLL_READ_SYS_READ)
						out_flags |= PR_POLL_READ;
					if (pd->out_flags & _PR_POLL_WRITE_SYS_READ)
						out_flags |= PR_POLL_WRITE;
				}
				if (FD_ISSET(osfd, &wt))
				{
					if (pd->out_flags & _PR_POLL_READ_SYS_WRITE)
						out_flags |= PR_POLL_READ;
					if (pd->out_flags & _PR_POLL_WRITE_SYS_WRITE)
						out_flags |= PR_POLL_WRITE;
				}
				if (FD_ISSET(osfd, &ex)) out_flags |= PR_POLL_EXCEPT;
			}
			pd->out_flags = out_flags;
			if (out_flags) ready++;
		}
		PR_ASSERT(ready > 0);
	}
	else if (ready < 0)
	{
		err = errno;
		_PR_MD_MAP_SELECT_ERROR(err);
	}

	return ready;
}  /* NativeThreadSelect */



PR_IMPLEMENT(void) _amigaos_InitIO(void)
{
}

PR_IMPLEMENT(PRInt32) _amigaos_CloseSocket(PRInt32 osfd)
{
	close(osfd);
}

PR_IMPLEMENT(PRInt32) _amigaos_Connect(
		PRFileDesc *fd, const PRNetAddr *addr,
		PRUint32 addrlen, PRIntervalTime timeout)
{
	PRInt32 rv, err;
	PRThread *me = _PR_MD_CURRENT_THREAD();
	PRInt32 osfd = fd->secret->md.osfd;
	dprintf("_amigaos_Connect(%d)\n", osfd);
	PRNetAddr addrCopy;

//	/* Make the socket blocking */
//	PRUint32 zero = 0;
//	ioctl(osfd, FIONBIO, &zero);

	addrCopy = *addr;
	((struct sockaddr *) &addrCopy)->sa_len = addrlen;
	((struct sockaddr *) &addrCopy)->sa_family = addr->raw.family;
	dprintf("addr: sa_len = %d, sa_family = %d\n", addrlen, addr->raw.family);

	retry:
	if ((rv = connect(osfd, (struct sockaddr *)&addrCopy, addrlen)) == -1)
	{
		dprintf("connect: rv = %d\n", rv);
		dprintf("errno = %d, EINPROGRESS == %d\n", errno, EINPROGRESS);
		dprintf("fd->secret->nonblocking = %d\n", fd->secret->nonblocking);
		err = errno;
		if (!fd->secret->nonblocking && (err == EINPROGRESS))
		{
			dprintf("in progress");
			rv = socket_io_wait(osfd, WRITE_FD, timeout);
			dprintf("socket_io_wait: rv = %d\n", rv);
			if (rv == -1)
			{
				return -1;
			}

			return 0;
		}
        _PR_MD_MAP_CONNECT_ERROR(err);
	}
	dprintf("end of function, rv = %d\n", rv);
	return rv;
}

PR_IMPLEMENT(PRInt32) _amigaos_Accept(
		PRFileDesc *fd, PRNetAddr *addr,
		PRUint32 *addrlen, PRIntervalTime timeout)
		{
	PRInt32 osfd = fd->secret->md.osfd;
	PRInt32 rv, err;
	PRThread *me = _PR_MD_CURRENT_THREAD();

	while ((rv = accept(osfd, (struct sockaddr *) addr,
			(socklen_t *)addrlen)) == -1)
	{
		err = errno;
		if ((err == EAGAIN) || (err == EWOULDBLOCK) || (err == ECONNABORTED))
		{
			if (fd->secret->nonblocking)
			{
				break;
			}
			if ((rv = socket_io_wait(osfd, READ_FD, timeout)) < 0)
			{
				goto done;
			}
		}
		else
		{
			break;
		}
	}
	if (rv < 0) {
		_PR_MD_MAP_ACCEPT_ERROR(err);
	}
	done:
	if (rv != -1)
	{
		/* ignore the sa_len field of struct sockaddr */
		if (addr)
		{
			addr->raw.family = ((struct sockaddr *) addr)->sa_family;
		}
	}

	return(rv);
}

PR_IMPLEMENT(PRInt32) _amigaos_Bind(PRFileDesc *fd, const PRNetAddr *addr, PRUint32 addrlen)
{
	PRInt32 rv, err;
	PRNetAddr addrCopy;

	addrCopy = *addr;
	((struct sockaddr *) &addrCopy)->sa_len = addrlen;
	((struct sockaddr *) &addrCopy)->sa_family = addr->raw.family;
	rv = bind(fd->secret->md.osfd, (struct sockaddr *) &addrCopy, (int )addrlen);
	if (rv < 0)
	{
		err = _MD_ERRNO();
		_PR_MD_MAP_BIND_ERROR(err);
	}
	return(rv);
}

PR_IMPLEMENT(PRInt32) _amigaos_Listen(PRFileDesc *fd, PRIntn backlog)
{
	PRInt32 rv, err;

	rv = listen(fd->secret->md.osfd, backlog);
	if (rv < 0)
	{
		err = _MD_ERRNO();
		_PR_MD_MAP_LISTEN_ERROR(err);
	}
	return(rv);
}

PR_IMPLEMENT(PRInt32) _amigaos_Shutdown(PRFileDesc *fd, PRIntn how)
{
	PRInt32 rv, err;

	rv = shutdown(fd->secret->md.osfd, how);
	if (rv < 0) {
		err = _MD_ERRNO();
		_PR_MD_MAP_SHUTDOWN_ERROR(err);
	}
	return(rv);
}

PR_IMPLEMENT(PRInt32) _amigaos_Recv(PRFileDesc *fd, void *buf, PRInt32 amount,
		PRInt32 flags, PRIntervalTime timeout)
		{
	PRInt32 osfd = fd->secret->md.osfd;
	PRInt32 rv, err;
	PRThread *me = _PR_MD_CURRENT_THREAD();


	while ((rv = recv(osfd,buf,amount,flags)) == -1)
	{
		err = _MD_ERRNO();
		if ((err == EAGAIN) || (err == EWOULDBLOCK))
		{
			if (fd->secret->nonblocking)
			{
				break;
			}
			if ((rv = socket_io_wait(osfd, READ_FD, timeout)) < 0)
			{
				goto done;
			}
		}
		else if ((err == EINTR) && (!_PR_PENDING_INTERRUPT(me)))
		{
			continue;
		}
		else
		{
			break;
		}
	}
	if (rv < 0) {
		_PR_MD_MAP_RECV_ERROR(err);
	}
	done:
	return(rv);
		}

PR_IMPLEMENT(PRInt32) _amigaos_Send(PRFileDesc *fd, const void *buf, PRInt32 amount,
		PRInt32 flags, PRIntervalTime timeout)
		{
	PRInt32 osfd = fd->secret->md.osfd;
	PRInt32 rv, err;
	PRThread *me = _PR_MD_CURRENT_THREAD();

	while ((rv = send(osfd,buf,amount,flags)) == -1)
	{
		err = _MD_ERRNO();
		if ((err == EAGAIN) || (err == EWOULDBLOCK))
		{
			if (fd->secret->nonblocking)
			{
				break;
			}
			if ((rv = socket_io_wait(osfd, WRITE_FD, timeout))< 0)
			{
				goto done;
			}
		}
		else if ((err == EINTR) && (!_PR_PENDING_INTERRUPT(me)))
		{
			continue;
		}
		else
		{
			break;
		}
	}
	/*
	 * optimization; if bytes sent is less than "amount" call
	 * select before returning. This is because it is likely that
	 * the next send() call will return EWOULDBLOCK.
	 */
	 if ((!fd->secret->nonblocking) && (rv > 0) && (rv < amount)
			 && (timeout != PR_INTERVAL_NO_WAIT))
	 {
		 if (socket_io_wait(osfd, WRITE_FD, timeout)< 0)
		 {
			 rv = -1;
			 goto done;
		 }
	 }
	 if (rv < 0)
	 {
		 _PR_MD_MAP_SEND_ERROR(err);
	 }
	 done:
	 return(rv);
		}

PR_IMPLEMENT(PRStatus) _amigaos_GetSockName(PRFileDesc *fd, PRNetAddr *addr,
		PRUint32 *addrlen)
		{
	PRInt32 rv, err;

	rv = getsockname(fd->secret->md.osfd,
			(struct sockaddr *) addr, (socklen_t *)addrlen);
	if (rv == 0)
	{
		/* ignore the sa_len field of struct sockaddr */
		if (addr)
		{
			addr->raw.family = ((struct sockaddr *) addr)->sa_family;
		}
	}
	if (rv < 0)
	{
		err = _MD_ERRNO();
		_PR_MD_MAP_GETSOCKNAME_ERROR(err);
	}
	return rv==0?PR_SUCCESS:PR_FAILURE;
		}

PR_IMPLEMENT(PRStatus) _amigaos_GetPeerName(PRFileDesc *fd, PRNetAddr *addr,
		PRUint32 *addrlen)
		{
	PRInt32 rv, err;

	rv = getpeername(fd->secret->md.osfd,
			(struct sockaddr *) addr, (socklen_t *)addrlen);
	if (rv == 0)
	{
		/* ignore the sa_len field of struct sockaddr */
		if (addr)
		{
			addr->raw.family = ((struct sockaddr *) addr)->sa_family;
		}
	}
	if (rv < 0)
	{
		err = _MD_ERRNO();
		_PR_MD_MAP_GETPEERNAME_ERROR(err);
	}
	return rv==0?PR_SUCCESS:PR_FAILURE;
		}

PR_IMPLEMENT(PRStatus) _amigaos_GetSockOpt(PRFileDesc *fd, PRInt32 level,
		PRInt32 optname, char* optval, PRInt32* optlen)
		{
	PRInt32 rv, err;

	rv = getsockopt(fd->secret->md.osfd, level, optname, optval, (socklen_t *)optlen);
	if (rv < 0)
	{
		err = _MD_ERRNO();
		_PR_MD_MAP_GETSOCKOPT_ERROR(err);
	}
	return rv==0?PR_SUCCESS:PR_FAILURE;
		}

PR_IMPLEMENT(PRStatus) _amigaos_SetSockOpt(PRFileDesc *fd, PRInt32 level,
		PRInt32 optname, const char* optval, PRInt32 optlen)
{
	PRInt32 rv, err;

	rv = setsockopt(fd->secret->md.osfd, level, optname, optval, optlen);
	if (rv < 0)
	{
		err = _MD_ERRNO();
		_PR_MD_MAP_SETSOCKOPT_ERROR(err);
	}
	return rv==0?PR_SUCCESS:PR_FAILURE;
}

PR_IMPLEMENT(PRInt32) _amigaos_SendTo(
		PRFileDesc *fd, const void *buf, PRInt32 amount, PRIntn flags,
		const PRNetAddr *addr, PRUint32 addrlen, PRIntervalTime timeout)
		{
	PRInt32 osfd = fd->secret->md.osfd;
	PRInt32 rv, err;
	PRThread *me = _PR_MD_CURRENT_THREAD();
	PRNetAddr addrCopy;

	addrCopy = *addr;
	((struct sockaddr *) &addrCopy)->sa_len = addrlen;
	((struct sockaddr *) &addrCopy)->sa_family = addr->raw.family;

	while ((rv = sendto(osfd, buf, amount, flags,
			(struct sockaddr *) &addrCopy, addrlen)) == -1)
	{
		err = _MD_ERRNO();
		if ((err == EAGAIN) || (err == EWOULDBLOCK))
		{
			if (fd->secret->nonblocking)
			{
				break;
			}
			if ((rv = socket_io_wait(osfd, WRITE_FD, timeout))< 0)
			{
				goto done;
			}
		}
		else if ((err == EINTR) && (!_PR_PENDING_INTERRUPT(me)))
		{
			continue;
		}
		else
		{
			break;
		}
	}
	if (rv < 0)
	{
		_PR_MD_MAP_SENDTO_ERROR(err);
	}
	done:
	return(rv);
		}

PR_IMPLEMENT(PRInt32) _amigaos_RecvFrom(PRFileDesc *fd, void *buf, PRInt32 amount,
		PRIntn flags, PRNetAddr *addr, PRUint32 *addrlen,
		PRIntervalTime timeout)
		{
	PRInt32 osfd = fd->secret->md.osfd;
	PRInt32 rv, err;
	PRThread *me = _PR_MD_CURRENT_THREAD();

	while ((*addrlen = PR_NETADDR_SIZE(addr)),
			((rv = recvfrom(osfd, buf, amount, flags,
					(struct sockaddr *) addr, (socklen_t *)addrlen)) == -1))
	{
		err = _MD_ERRNO();
		if ((err == EAGAIN) || (err == EWOULDBLOCK))
		{
			if (fd->secret->nonblocking)
			{
				break;
			}
			if ((rv = socket_io_wait(osfd, READ_FD, timeout)) < 0)
			{
				goto done;
			}
		}
		else if ((err == EINTR) && (!_PR_PENDING_INTERRUPT(me)))
		{
			continue;
		}
		else
		{
			break;
		}
	}
	if (rv < 0)
	{
		_PR_MD_MAP_RECVFROM_ERROR(err);
	}
	done:
	if (rv != -1)
	{
		/* ignore the sa_len field of struct sockaddr */
		if (addr) {
			addr->raw.family = ((struct sockaddr *) addr)->sa_family;
		}
	}
	return(rv);
		}

PR_IMPLEMENT(PRInt32) _amigaos_SocketPair(int af, int type, int flags,
		PRInt32 *osfd)
		{
#if 0
PRInt32 rv, err;

rv = socketpair(af, type, flags, osfd);
if (rv < 0) {
	err = _MD_ERRNO();
	_PR_MD_MAP_SOCKETPAIR_ERROR(err);
}
return rv;
#else
	PR_SetError(PR_NOT_IMPLEMENTED_ERROR, 0);
return -1;
#endif
		}

PR_IMPLEMENT(PRInt32) _amigaos_Socket(PRInt32 domain, PRInt32 type, PRInt32 proto)
{
	PRInt32 osfd, err;
	PRUint32 one = 1;

	dprintf("domain = %d, type = %d, proto = %d\n", domain, type, proto);

	osfd = socket(domain, type, proto);

	dprintf("osfd = %d\n", osfd);
	dprintf("errno = %d\n", errno);
	if (osfd == -1)
	{
		err = _MD_ERRNO();
		_PR_MD_MAP_SOCKET_ERROR(err);
		return(osfd);
	}

//	/* Make the socket noblocking */
//	if (0 != ioctl(osfd, FIONBIO, &one))
//	{
//		printf("can't make nonblocking\n");
//		err = _MD_ERRNO();
//		close(osfd);
//		return -1;
//	}

	dprintf("returning %d\n", osfd);
	return(osfd);
}

PR_IMPLEMENT(PRInt32) _amigaos_SocketAvailable(PRFileDesc *fd)
{
	PRInt32 result;

	if (ioctl(fd->secret->md.osfd, FIONREAD, &result) < 0)
	{
		_PR_MD_MAP_SOCKETAVAILABLE_ERROR(_MD_ERRNO());
		return -1;
	}
	return result;
}

PR_IMPLEMENT(PRInt32) _amigaos_Pr_Poll(PRPollDesc *pds, PRIntn npds, PRIntervalTime timeout)
{
	PRInt32 rv = 0;
	PRThread *me = _PR_MD_CURRENT_THREAD();

	if (_PR_PENDING_INTERRUPT(me))
	{
		me->flags &= ~_PR_INTERRUPT;
		PR_SetError(PR_PENDING_INTERRUPT_ERROR, 0);
		return -1;
	}
	if (0 == npds) PR_Sleep(timeout);
	else
		rv = NativeThreadSelect(pds, npds, timeout);

	return rv;
}

PR_IMPLEMENT(PRInt32) _amigaos_Select(PRInt32 osfd, fd_set *readfs, fd_set *writefs, fd_set *exceptfs, struct timeval *timeout)
{
	PRInt32 rv;

	rv = select(osfd, readfs, writefs,exceptfs, timeout);
	return rv;
}


PR_IMPLEMENT(PRInt32) _amigaos_Writev(PRFileDesc *fd, const PRIOVec *iov, PRInt32 iov_size, PRIntervalTime timeout)
{
	int index;
	int sent = 0;
	int rv;

	for (index=0; index < iov_size; index++)
	{
		rv = _PR_MD_SEND(fd, iov[index].iov_base, iov[index].iov_len, 0, timeout);
		if (rv > 0)
			sent += rv;
		if ( rv != iov[index].iov_len )
		{
			if (rv < 0)
			{
				if (fd->secret->nonblocking
						&& (PR_GetError() == PR_WOULD_BLOCK_ERROR)
						&& (sent > 0))
				{
					return sent;
				}
				else
				{
					return -1;
				}
			}
			/* Only a nonblocking socket can have partial sends */
			PR_ASSERT(fd->secret->nonblocking);
			return sent;
		}
	}
	return sent;
}

PR_IMPLEMENT(PRStatus) _amigaos_gethostname(char *name, PRUint32 namelen)
{
	PRIntn rv;

	rv = gethostname(name, namelen);
	if (0 == rv) {
		return PR_SUCCESS;
	}
	_PR_MD_MAP_GETHOSTNAME_ERROR(_MD_ERRNO());
	return PR_FAILURE;
}


PR_IMPLEMENT(PRStatus) _amigaos_getsysinfo(PRSysInfo cmd, char *name, PRUint32 namelen)
{
	struct utsname info;

	PR_ASSERT((cmd == PR_SI_SYSNAME) || (cmd == PR_SI_RELEASE));

	if (uname(&info) == -1) {
		_PR_MD_MAP_DEFAULT_ERROR(errno);
		return PR_FAILURE;
	}
	if (PR_SI_SYSNAME == cmd)
		(void)PR_snprintf(name, namelen, info.sysname);
	else if (PR_SI_RELEASE == cmd)
		(void)PR_snprintf(name, namelen, info.release);
	else
		return PR_FAILURE;
	return PR_SUCCESS;
}


int _MD_amigaos_get_nonblocking_connect_error(int osfd)
{
    int err;
    socklen_t optlen = sizeof(err);
    if (getsockopt(osfd, SOL_SOCKET, SO_ERROR, (char *) &err, &optlen) == -1) {
        return errno;
    } else {
        return err;
    }
}


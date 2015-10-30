/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 * Thomas Frieden <thomas@friedenhq.org>
 * Portions created by the Initial Developer are Copyright (C) 2007
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s): Michael Martin
 *                 Chris Double (chris.double@double.co.nz)
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** *
 */
#include <stdlib.h>
#include <string.h>
#include "sydney_audio.h"

#include <exec/exec.h>
#include <exec/types.h>
#include <devices/ahi.h>
#include <devices/timer.h>
#include <proto/exec.h>
#include <proto/ahi.h>
#include <proto/timer.h>
#include <dos/dos.h>

#include <stdio.h>

struct sa_buffer
{
	uint32 size;
	uint32 writePos;
	void *data;
};

#define PLAYBUFFERSIZE 32768

typedef struct sa_buffer sa_buffer_t;

struct sa_stream {
	  /* audio format info */
	  unsigned int      mRate;
	  unsigned int      mChannels;
	  unsigned int 		mVolume;
	  unsigned int		mPan;

	  uint64			mBytesWritten;
	  uint64			mLastPosition;

	  /* Device data */
	  struct MsgPort 	*mPort;
	  struct AHIRequest *mRequest[2];
	  uint32			mCurrentBuffer;

	  /* Backup buffers */
	  sa_buffer_t		mBackup[2];

	  /* Timer device data */
	  struct MsgPort	*mTimerPort;
	  struct TimeRequest *mTimerRequest;
	  struct TimerIFace *mITimer;
	  BOOL				mDispatched[2];

	  struct TimeVal	mLastWrite;

	  struct Task *		mOwner;

	  uint8				mSilence[4];
	  uint64			mDummy;

	  /* Used to pause/resume */
	  uint32			mSignal;
	  BOOL 				mPause;
};


/* Buffer management */
static void
___buffer_init(sa_buffer_t *buffer)
{
	buffer->data = NULL;
	buffer->size = 0;
	buffer->writePos = 0;
}

static void
___buffer_destroy(sa_buffer_t *buffer)
{
	if (buffer->data)
		IExec->FreeVec(buffer->data);

	buffer->data = NULL;
	buffer->size = 0;
	buffer->writePos = 0;
}

static uint32
___buffer_submit(sa_buffer_t *buffer, const void *data, uint32 size)
{
	uint32 realSize = size;

	if ((size + buffer->writePos) > buffer->size)
		realSize = buffer->size - buffer->writePos;

	memcpy((void *)((uint8*)buffer->data + buffer->writePos), data, realSize);

	buffer->writePos += realSize;

	return size - realSize;
}

static uint32
___buffer_capacity(sa_buffer_t *buffer)
{
	return buffer->size - buffer->writePos;
}

static void
___buffer_reset(sa_buffer_t *buffer)
{
	buffer->writePos = 0;
}

static void *
___buffer_ensure_size(sa_buffer_t *buffer, uint32 size)
{
	if (buffer->size >= size)
		return buffer->data;

	___buffer_destroy(buffer);

	buffer->data = IExec->AllocVecTags(size, AVT_Type, MEMF_SHARED, TAG_DONE);
	if (buffer->data)
		buffer->size = size;

	return buffer->data;
}




static BOOL
__check_ahi_present(void)
{
	BOOL result = FALSE;
	struct MsgPort *port = (struct MsgPort *)IExec->AllocSysObject(ASOT_PORT, NULL);

	if (!port)
		return FALSE;

	struct AHIRequest *request = (struct AHIRequest *)IExec->AllocSysObjectTags(ASOT_IOREQUEST,
			ASOIOR_Size,		sizeof(struct AHIRequest),
			ASOIOR_ReplyPort,	port,
			TAG_DONE);

	if (request)
	{
		IExec->FreeSysObject(ASOT_PORT, (void *)port);
		return FALSE;
	};

	request->ahir_Version = 5;

	if (0 != (IExec->OpenDevice(AHINAME, 0, (struct IORequest *)request, 0)))
		result = FALSE;
	else
	{
		IExec->CloseDevice((struct IORequest *)request);
		result = TRUE;
	}

	IExec->FreeSysObject(ASOT_IOREQUEST, (void *)request);
	IExec->FreeSysObject(ASOT_PORT, (void *)port);

	return result;
}



/*
 * -----------------------------------------------------------------------------
 *  Error Handler to prevent output to stderr
 *  ----------------------------------------------------------------------------
 */
static void
quiet_error_handler(const char* file,
		int         line,
		const char* function,
		int         err,
		const char* format,
		...)
{
}

/*
 * -----------------------------------------------------------------------------
 * Startup and shutdown functions
 * -----------------------------------------------------------------------------
 */

int
sa_stream_create_pcm(
		sa_stream_t      ** _s,
		const char        * client_name,
		sa_mode_t           mode,
		sa_pcm_format_t     format,
		unsigned  int       rate,
		unsigned  int       n_channels)
{
	sa_stream_t *s = NULL;

	if (_s == NULL)
		return SA_ERROR_INVALID;

	if (!__check_ahi_present())
		return SA_ERROR_NOT_SUPPORTED;

	*_s = NULL;

	if (mode != SA_MODE_WRONLY)
		return SA_ERROR_NOT_SUPPORTED;

	if (format != SA_PCM_FORMAT_S16_NE)
		return SA_ERROR_NOT_SUPPORTED;

	s = IExec->AllocVecTags(sizeof(sa_stream_t), AVT_Type, MEMF_SHARED, TAG_DONE);
	if (!s)
		return SA_ERROR_OOM;

	s->mRate = rate;
	s->mChannels = n_channels;
	s->mBytesWritten = 0;
	s->mLastPosition = 0;

	s->mVolume = 0x10000;
	s->mPan = 0x08000;

	___buffer_init(&(s->mBackup[0]));
	___buffer_init(&(s->mBackup[1]));

	s->mPort = NULL;
	s->mPause = FALSE;

	s->mITimer = NULL;

	*_s = s;

	return SA_SUCCESS;
}


static inline void
__update_timestamp(sa_stream_t *s)
{
	s->mITimer->GetSysTime(&(s->mLastWrite));
}

int
sa_stream_open(sa_stream_t *s)
{

	if (s == NULL)
		return SA_ERROR_NO_INIT;

	if (s->mPort != NULL)
		return SA_ERROR_INVALID;


	return SA_SUCCESS;
}


int
___opendevice(sa_stream_t *s)
{
	if (s->mPort)
		return SA_SUCCESS;

	s->mPort = (struct MsgPort *)IExec->AllocSysObject(ASOT_PORT, NULL);
	if (!s->mPort)
		return SA_ERROR_INVALID;

	s->mRequest[0] = (struct AHIRequest *)IExec->AllocSysObjectTags(ASOT_IOREQUEST,
			ASOIOR_Size,		sizeof(struct AHIRequest),
			ASOIOR_ReplyPort,	s->mPort,
		TAG_DONE);

	if (!s->mRequest[0])
	{
		IExec->FreeSysObject(ASOT_PORT, (void *)s->mPort);
		s->mPort = NULL;
		return SA_ERROR_INVALID;
	};

	s->mRequest[0]->ahir_Version = 5;

	if (0 != (IExec->OpenDevice(AHINAME, 0, (struct IORequest *)s->mRequest[0], 0)))
	{
		printf("%s OpenDevice failed\n", __PRETTY_FUNCTION__);
		IExec->FreeSysObject(ASOT_IOREQUEST, (void *)s->mRequest[0]);
		IExec->FreeSysObject(ASOT_PORT, (void *)s->mPort);
		s->mPort = NULL;
		return SA_ERROR_INVALID;
	}

	s->mRequest[1] = (struct AHIRequest *)IExec->AllocSysObjectTags(ASOT_IOREQUEST,
				ASOIOR_Duplicate,		(void *)s->mRequest[0],
			TAG_DONE);

	/* Open timer.device */
	s->mTimerPort = (struct MsgPort *)IExec->AllocSysObject(ASOT_PORT, NULL);
	s->mTimerRequest = (struct TimeRequest *)IExec->AllocSysObjectTags(ASOT_IOREQUEST,
				ASOIOR_Size,		sizeof(struct TimeRequest),
				ASOIOR_ReplyPort,	s->mTimerPort,
			TAG_DONE);
	IExec->OpenDevice(TIMERNAME, UNIT_MICROHZ, (struct IORequest *)s->mTimerRequest, 0);
	s->mITimer = (struct TimerIFace *)IExec->GetInterface((struct Library *)s->mTimerRequest->Request.io_Device,
			"main", 1, NULL);

	__update_timestamp(s);

	s->mOwner = IExec->FindTask(0);
	s->mCurrentBuffer = 0;
	s->mSilence[0] = s->mSilence[1] = s->mSilence[2] = s->mSilence[3] = 0;
	s->mDummy = 0;
	s->mDispatched[0] = FALSE;
	s->mDispatched[1] = FALSE;

	s->mSignal = IExec->AllocSignal(-1);

	return SA_SUCCESS;
}

int
sa_stream_destroy(sa_stream_t *s)
{

	if (!s->mPort)
		return SA_SUCCESS;

	IExec->CloseDevice((struct IORequest *)s->mTimerRequest);
	IExec->FreeSysObject(ASOT_IOREQUEST, (void *)s->mTimerRequest);
	IExec->FreeSysObject(ASOT_PORT, (void *)s->mTimerPort);

	IExec->CloseDevice((struct IORequest *)s->mRequest[0]);
	IExec->FreeSysObject(ASOT_IOREQUEST, (void *)s->mRequest[1]);
	IExec->FreeSysObject(ASOT_IOREQUEST, (void *)s->mRequest[0]);
	IExec->FreeSysObject(ASOT_PORT, (void *)s->mPort);
	s->mPort = NULL;

	___buffer_destroy(&(s->mBackup[0]));
	___buffer_destroy(&(s->mBackup[1]));

	IExec->FreeVec((void *)s);

	return SA_SUCCESS;
}



/*
 * -----------------------------------------------------------------------------
 * Data read and write functions
 * -----------------------------------------------------------------------------
 */

int
sa_stream_write(sa_stream_t *s, const void *data, size_t nbytes)
{
	if (!s->mPort)
		___opendevice(s);

	if (IExec->FindTask(0) != s->mOwner)
		printf("*** ERROR: Not called from same thread\n");

	if (s->mPause)
	{
		IExec->Wait(1L << s->mSignal);
	}

	if (nbytes == 0 || data == NULL)
	{
		s->mRequest[s->mCurrentBuffer]->ahir_Std.io_Message.mn_Node.ln_Pri	= 0;
		s->mRequest[s->mCurrentBuffer]->ahir_Std.io_Command					= CMD_WRITE;
		s->mRequest[s->mCurrentBuffer]->ahir_Std.io_Data					= s->mSilence;
		s->mRequest[s->mCurrentBuffer]->ahir_Std.io_Length					= 4;
		s->mRequest[s->mCurrentBuffer]->ahir_Frequency						= s->mRate;
		s->mRequest[s->mCurrentBuffer]->ahir_Type							= s->mChannels == 2 ? AHIST_S16S : AHIST_M16S;
		s->mRequest[s->mCurrentBuffer]->ahir_Volume							= s->mVolume;
		s->mRequest[s->mCurrentBuffer]->ahir_Position						= s->mPan;
		s->mRequest[s->mCurrentBuffer]->ahir_Link 							= NULL;

		IExec->SendIO((struct IORequest *)s->mRequest[s->mCurrentBuffer]);

	}
	else
	{
		void *addr = ___buffer_ensure_size(&(s->mBackup[s->mCurrentBuffer]), PLAYBUFFERSIZE);

		uint32 submitSize = ___buffer_submit(&(s->mBackup[s->mCurrentBuffer]), data, nbytes);

		if (submitSize || ___buffer_capacity(&(s->mBackup[s->mCurrentBuffer])) == 0)
		{
			s->mRequest[s->mCurrentBuffer]->ahir_Std.io_Message.mn_Node.ln_Pri	= 0;
			s->mRequest[s->mCurrentBuffer]->ahir_Std.io_Command					= CMD_WRITE;
			s->mRequest[s->mCurrentBuffer]->ahir_Std.io_Data					= s->mBackup[s->mCurrentBuffer].data;
			s->mRequest[s->mCurrentBuffer]->ahir_Std.io_Length					= PLAYBUFFERSIZE;
			s->mRequest[s->mCurrentBuffer]->ahir_Frequency						= s->mRate;
			s->mRequest[s->mCurrentBuffer]->ahir_Type							= s->mChannels == 2 ? AHIST_S16S : AHIST_M16S;
			s->mRequest[s->mCurrentBuffer]->ahir_Volume							= s->mVolume;
			s->mRequest[s->mCurrentBuffer]->ahir_Position						= s->mPan;
			s->mRequest[s->mCurrentBuffer]->ahir_Link 							= s->mRequest[1 - s->mCurrentBuffer];

			IExec->SendIO((struct IORequest *)s->mRequest[s->mCurrentBuffer]);
			s->mDispatched[s->mCurrentBuffer] = TRUE;

			/* Wait for the other buffer to finish finish first, if we ever actually send one off */
			if (s->mDispatched[1 - s->mCurrentBuffer] == TRUE)
			{
				uint32 sigrec = IExec->Wait(SIGBREAKF_CTRL_C | (1L << s->mPort->mp_SigBit));
				if (sigrec & (1L << s->mPort->mp_SigBit))
					IExec->WaitIO((struct IORequest *)s->mRequest[1 - s->mCurrentBuffer]);
				else
					return SA_SUCCESS;

				___buffer_reset(&(s->mBackup[1 - s->mCurrentBuffer]));
			}

			s->mCurrentBuffer = 1 - s->mCurrentBuffer;
		}
	}
	s->mLastPosition = s->mBytesWritten;
	s->mBytesWritten += nbytes;

	__update_timestamp(s);

	return SA_SUCCESS;
}

/*
 * -----------------------------------------------------------------------------
 * General query and support functions
 * -----------------------------------------------------------------------------
 */

int
sa_stream_get_write_size(sa_stream_t *s, size_t *size)
{
	*size =  ___buffer_capacity(&(s->mBackup[s->mCurrentBuffer]));

//printf("%s %d available\n", __PRETTY_FUNCTION__, *size);

	return SA_SUCCESS;
}

int
sa_stream_get_position(sa_stream_t *s, sa_position_t position, int64_t *pos)
{
	if (!s->mITimer)
	{
		*pos = 0;
		return SA_SUCCESS;
	}

	struct TimeVal current;

	s->mITimer->GetSysTime(&current);
	s->mITimer->SubTime(&current, &s->mLastWrite);
//printf("%d   %d\n", current.Seconds, current.Microseconds);
	double fTime = (double)current.Seconds + (double)current.Microseconds / (double)1000000.0;

	*pos = s->mLastPosition + (uint64)((double)(2 * s->mChannels * s->mRate) * fTime);

//printf("%s: %lld, pos = %lld\n", __PRETTY_FUNCTION__, s->mBytesWritten, *pos);
	return SA_SUCCESS;
}


int
sa_stream_pause(sa_stream_t *s)
{printf("%s\n", __PRETTY_FUNCTION__);
	s->mPause = TRUE;

	return SA_SUCCESS;
}


int
sa_stream_resume(sa_stream_t *s)
{printf("%s\n", __PRETTY_FUNCTION__);
	s->mPause = FALSE;
	IExec->Signal(s->mOwner, 1L << s->mSignal);

	return SA_SUCCESS;
}


int
sa_stream_drain(sa_stream_t *s)
{printf("%s\n", __PRETTY_FUNCTION__);
	return SA_SUCCESS;
}



/*
 * -----------------------------------------------------------------------------
 * Extension functions
 * -----------------------------------------------------------------------------
 */

int
sa_stream_set_volume_abs(sa_stream_t *s, float vol)
{

	s->mVolume = (int) ((float)0x10000 * vol);

	return SA_SUCCESS;
}


int
sa_stream_get_volume_abs(sa_stream_t *s, float *vol)
{

	*vol = (float)s->mVolume / (float)0x10000;

	return SA_SUCCESS;
}



/*
 * -----------------------------------------------------------------------------
 * Unsupported functions
 * -----------------------------------------------------------------------------
 */
#define UNSUPPORTED(func)   func { return SA_ERROR_NOT_SUPPORTED; }

UNSUPPORTED(int sa_stream_create_opaque(sa_stream_t **s, const char *client_name, sa_mode_t mode, const char *codec))
UNSUPPORTED(int sa_stream_set_write_lower_watermark(sa_stream_t *s, size_t size))
UNSUPPORTED(int sa_stream_set_read_lower_watermark(sa_stream_t *s, size_t size))
UNSUPPORTED(int sa_stream_set_write_upper_watermark(sa_stream_t *s, size_t size))
UNSUPPORTED(int sa_stream_set_read_upper_watermark(sa_stream_t *s, size_t size))
UNSUPPORTED(int sa_stream_set_channel_map(sa_stream_t *s, const sa_channel_t map[], unsigned int n))
UNSUPPORTED(int sa_stream_set_xrun_mode(sa_stream_t *s, sa_xrun_mode_t mode))
UNSUPPORTED(int sa_stream_set_non_interleaved(sa_stream_t *s, int enable))
UNSUPPORTED(int sa_stream_set_dynamic_rate(sa_stream_t *s, int enable))
UNSUPPORTED(int sa_stream_set_driver(sa_stream_t *s, const char *driver))
UNSUPPORTED(int sa_stream_start_thread(sa_stream_t *s, sa_event_callback_t callback))
UNSUPPORTED(int sa_stream_stop_thread(sa_stream_t *s))
UNSUPPORTED(int sa_stream_change_device(sa_stream_t *s, const char *device_name))
UNSUPPORTED(int sa_stream_change_read_volume(sa_stream_t *s, const int32_t vol[], unsigned int n))
UNSUPPORTED(int sa_stream_change_write_volume(sa_stream_t *s, const int32_t vol[], unsigned int n))
UNSUPPORTED(int sa_stream_change_rate(sa_stream_t *s, unsigned int rate))
UNSUPPORTED(int sa_stream_change_meta_data(sa_stream_t *s, const char *name, const void *data, size_t size))
UNSUPPORTED(int sa_stream_change_user_data(sa_stream_t *s, const void *value))
UNSUPPORTED(int sa_stream_set_adjust_rate(sa_stream_t *s, sa_adjust_t direction))
UNSUPPORTED(int sa_stream_set_adjust_nchannels(sa_stream_t *s, sa_adjust_t direction))
UNSUPPORTED(int sa_stream_set_adjust_pcm_format(sa_stream_t *s, sa_adjust_t direction))
UNSUPPORTED(int sa_stream_set_adjust_watermarks(sa_stream_t *s, sa_adjust_t direction))
UNSUPPORTED(int sa_stream_get_mode(sa_stream_t *s, sa_mode_t *access_mode))
UNSUPPORTED(int sa_stream_get_codec(sa_stream_t *s, char *codec, size_t *size))
UNSUPPORTED(int sa_stream_get_pcm_format(sa_stream_t *s, sa_pcm_format_t *format))
UNSUPPORTED(int sa_stream_get_rate(sa_stream_t *s, unsigned int *rate))
UNSUPPORTED(int sa_stream_get_nchannels(sa_stream_t *s, int *nchannels))
UNSUPPORTED(int sa_stream_get_user_data(sa_stream_t *s, void **value))
UNSUPPORTED(int sa_stream_get_write_lower_watermark(sa_stream_t *s, size_t *size))
UNSUPPORTED(int sa_stream_get_read_lower_watermark(sa_stream_t *s, size_t *size))
UNSUPPORTED(int sa_stream_get_write_upper_watermark(sa_stream_t *s, size_t *size))
UNSUPPORTED(int sa_stream_get_read_upper_watermark(sa_stream_t *s, size_t *size))
UNSUPPORTED(int sa_stream_get_channel_map(sa_stream_t *s, sa_channel_t map[], unsigned int *n))
UNSUPPORTED(int sa_stream_get_xrun_mode(sa_stream_t *s, sa_xrun_mode_t *mode))
UNSUPPORTED(int sa_stream_get_non_interleaved(sa_stream_t *s, int *enabled))
UNSUPPORTED(int sa_stream_get_dynamic_rate(sa_stream_t *s, int *enabled))
UNSUPPORTED(int sa_stream_get_driver(sa_stream_t *s, char *driver_name, size_t *size))
UNSUPPORTED(int sa_stream_get_device(sa_stream_t *s, char *device_name, size_t *size))
UNSUPPORTED(int sa_stream_get_read_volume(sa_stream_t *s, int32_t vol[], unsigned int *n))
UNSUPPORTED(int sa_stream_get_write_volume(sa_stream_t *s, int32_t vol[], unsigned int *n))
UNSUPPORTED(int sa_stream_get_meta_data(sa_stream_t *s, const char *name, void*data, size_t *size))
UNSUPPORTED(int sa_stream_get_adjust_rate(sa_stream_t *s, sa_adjust_t *direction))
UNSUPPORTED(int sa_stream_get_adjust_nchannels(sa_stream_t *s, sa_adjust_t *direction))
UNSUPPORTED(int sa_stream_get_adjust_pcm_format(sa_stream_t *s, sa_adjust_t *direction))
UNSUPPORTED(int sa_stream_get_adjust_watermarks(sa_stream_t *s, sa_adjust_t *direction))
UNSUPPORTED(int sa_stream_get_state(sa_stream_t *s, sa_state_t *state))
UNSUPPORTED(int sa_stream_get_event_error(sa_stream_t *s, sa_error_t *error))
UNSUPPORTED(int sa_stream_get_event_notify(sa_stream_t *s, sa_notify_t *notify))
UNSUPPORTED(int sa_stream_read(sa_stream_t *s, void *data, size_t nbytes))
UNSUPPORTED(int sa_stream_read_ni(sa_stream_t *s, unsigned int channel, void *data, size_t nbytes))
UNSUPPORTED(int sa_stream_write_ni(sa_stream_t *s, unsigned int channel, const void *data, size_t nbytes))
UNSUPPORTED(int sa_stream_pwrite(sa_stream_t *s, const void *data, size_t nbytes, int64_t offset, sa_seek_t whence))
UNSUPPORTED(int sa_stream_pwrite_ni(sa_stream_t *s, unsigned int channel, const void *data, size_t nbytes, int64_t offset, sa_seek_t whence))
UNSUPPORTED(int sa_stream_get_read_size(sa_stream_t *s, size_t *size))
UNSUPPORTED(int sa_stream_get_min_write(sa_stream_t *s, size_t *samples))

const char *sa_strerror(int code) { return NULL; }





#if 0



#define BUFFERSIZE 2048
char buffer[BUFFERSIZE];

#include <stdio.h>

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		printf("Usage: %s <filename>\n", argv[0]);
		exit(0);
	}


	printf("Reading %s\n", argv[1]);
	FILE *fh = fopen(argv[1], "r");
	if (fh)
	{
		sa_stream_t *s;
		int readBytes = 0;

		sa_stream_create_pcm(&s, "name", SA_MODE_WRONLY, SA_PCM_FORMAT_S16_NE, 44100, 2);

		do
		{
			int i;

			readBytes = fread(buffer, 1, BUFFERSIZE, fh);

			for (i = 0; i < readBytes; i+= 4)
			{
				int t1 = buffer[i], t2 = buffer[i+2];
				buffer[i] = buffer[i+1];
				buffer[i+1] = t1;
				buffer[i+2] = buffer[i+3];
				buffer[i+3] = t2;
			}
			sa_stream_write(s, buffer, readBytes);
		} while (readBytes != 0);


		sa_stream_destroy(s);
		fclose(fh);
	}
	else
		printf("Can't open %s\n", argv[1]);
}
#endif

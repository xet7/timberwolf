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


#include "nsAppShell.h"

#include <exec/exectags.h>
#include <dos/dos.h>

#include <proto/intuition.h>
#include <proto/exec.h>
#include <proto/timer.h>
#include <proto/datatypes.h>
#include <proto/asl.h>
#include <proto/Picasso96API.h>
#include <proto/diskfont.h>

#include "nsWindow.h"

extern struct ExecIFace *IExec;

#undef DEBUG
//#define DEBUG


#ifdef PR_LOGGING
PRLogModuleInfo *gWidgetLog = nsnull;
PRLogModuleInfo *gWidgetDrawLog = nsnull;
PRLogModuleInfo *gWidgetEventLog = nsnull;
PRLogModuleInfo *gWidgetAppshellLog = nsnull;

#define LOGAPPSHELL(args) PR_LOG(gWidgetAppshellLog, 4, args)

#else

#define LOGAPPSHELL(args)

#endif

struct AppShellInternalMessage
{
	struct Message execMessage;
	uint32 data1;					// Usually command
	void *data2;					// Usually data
};

struct Library *IntuitionBase = NULL;
struct IntuitionIFace *IIntuition = NULL;
struct Library *GfxBase = NULL;
struct GraphicsIFace *IGraphics = NULL;
struct Library *IconBase = NULL;
struct IconIFace *IIcon = NULL;
struct Library *DOSBase = NULL;
struct DOSIFace *IDOS = NULL;
struct Library *LayersBase = NULL;
struct LayersIFace *ILayers = NULL;
struct Library *KeymapBase = NULL;
struct KeymapIFace *IKeymap = NULL;
struct Device *TimerBase = NULL;
struct TimerIFace *ITimer = NULL;
struct Library *DataTypesBase = NULL;
struct DataTypesIFace *IDataTypes = NULL;
struct Library *AslBase = NULL;
struct AslIFace *IAsl = NULL;
struct Library *IFFParseBase = NULL;
struct IFFParseIFace *IIFFParse = NULL;
struct Library *P96Base = NULL;
struct P96IFace *IP96 = NULL;
struct Library *DiskfontBase = NULL;
struct DiskfontIFace *IDiskfont = NULL;

class __autoinit_Libraries
{
public:
	__autoinit_Libraries()
	{
		IntuitionBase = IExec->OpenLibrary("intuition.library", 0L);
		IIntuition = (struct IntuitionIFace *)IExec->GetInterface(IntuitionBase, "main", 1, NULL);

		GfxBase = IExec->OpenLibrary("graphics.library", 0L);
		IGraphics = (struct GraphicsIFace *)IExec->GetInterface(GfxBase, "main", 1, NULL);

		IconBase = IExec->OpenLibrary("icon.library", 0L);
		IIcon = (struct IconIFace *)IExec->GetInterface(IconBase, "main", 1, NULL);

		DOSBase = IExec->OpenLibrary("dos.library", 0L);
		IDOS = (struct DOSIFace *)IExec->GetInterface(DOSBase, "main", 1, NULL);

		LayersBase = IExec->OpenLibrary("layers.library", 0L);
		ILayers = (struct LayersIFace *)IExec->GetInterface(LayersBase, "main", 1, NULL);

		KeymapBase = IExec->OpenLibrary("keymap.library", 0L);
		IKeymap = (struct KeymapIFace *)IExec->GetInterface(KeymapBase, "main", 1, NULL);

		DataTypesBase = IExec->OpenLibrary("datatypes.library", 0L);
		IDataTypes = (struct DataTypesIFace *)IExec->GetInterface(DataTypesBase, "main", 1, NULL);

		AslBase = IExec->OpenLibrary("asl.library", 0L);
		IAsl = (struct AslIFace *)IExec->GetInterface(AslBase, "main", 1, NULL);

		IFFParseBase = IExec->OpenLibrary("iffparse.library", 0L);
		IIFFParse = (struct IFFParseIFace *)IExec->GetInterface(IFFParseBase, "main", 1, NULL);

		P96Base = IExec->OpenLibrary("Picasso96API.library", 0L);
		IP96 = (struct P96IFace *)IExec->GetInterface(P96Base, "main", 1, NULL);

		DiskfontBase = IExec->OpenLibrary("diskfont.library", 0L);
		IDiskfont = (struct DiskfontIFace *)IExec->GetInterface(DiskfontBase, "main", 1, NULL);

	};

	~__autoinit_Libraries()
	{
		if (IIcon) IExec->DropInterface((struct Interface *)IIcon);
		if (IconBase) IExec->CloseLibrary(IconBase);
		if (IGraphics) IExec->DropInterface((struct Interface *)IGraphics);
		if (GfxBase) IExec->CloseLibrary(GfxBase);
		if (IIntuition) IExec->DropInterface((struct Interface *)IIntuition);
		if (IntuitionBase) IExec->CloseLibrary(IntuitionBase);
		if (IDOS) IExec->DropInterface((struct Interface *)IDOS);
		if (DOSBase) IExec->CloseLibrary(DOSBase);
		if (ILayers) IExec->DropInterface((struct Interface *)ILayers);
		if (LayersBase) IExec->CloseLibrary(LayersBase);
		if (IKeymap) IExec->DropInterface((struct Interface *)IKeymap);
		if (KeymapBase) IExec->CloseLibrary(KeymapBase);
		if (IDataTypes) IExec->DropInterface((struct Interface *)IDataTypes);
		if (DataTypesBase) IExec->CloseLibrary(DataTypesBase);
		if (IAsl) IExec->DropInterface((struct Interface *)IAsl);
		if (AslBase) IExec->CloseLibrary(AslBase);
		if (IIFFParse) IExec->DropInterface((struct Interface *)IIFFParse);
		if (IFFParseBase) IExec->CloseLibrary(IFFParseBase);
		if (IP96) IExec->DropInterface((struct Interface *)IP96);
		if (P96Base) IExec->CloseLibrary(P96Base);
		if (IDiskfont) IExec->DropInterface((struct Interface *)IDiskfont);
		if (DiskfontBase) IExec->CloseLibrary(DiskfontBase);
	}
} __libraries_singleton;

//-------------------------------------------------------------------------
//
// nsAppShell constructor
//
//-------------------------------------------------------------------------
nsAppShell::nsAppShell()
	: mSharedWindowPort(NULL), mAppShellPort(NULL)
{

}


//-------------------------------------------------------------------------
//
// Create the application shell
//
//-------------------------------------------------------------------------

NS_IMETHODIMP nsAppShell::Init()
{
#ifdef PR_LOGGING
    if (!gWidgetLog)
        gWidgetLog = PR_NewLogModule("Widget");
    if (!gWidgetDrawLog)
        gWidgetDrawLog = PR_NewLogModule("WidgetDraw");
    if (!gWidgetEventLog)
    	gWidgetEventLog = PR_NewLogModule("WidgetEvent");
    if (!gWidgetAppshellLog)
    	gWidgetAppshellLog = PR_NewLogModule("WidgetAppshell");
#endif

    LOGAPPSHELL(("nsAppShell::Init() from %p\n", IExec->FindTask(NULL)));

	mSharedWindowPort =
			(struct MsgPort *)IExec->AllocSysObjectTags(ASOT_PORT, TAG_DONE);
	mAppShellPort =
			(struct MsgPort *)IExec->AllocSysObjectTags(ASOT_PORT, TAG_DONE);
	mInternalPort =
				(struct MsgPort *)IExec->AllocSysObjectTags(ASOT_PORT, TAG_DONE);
	mMessageItemPool = IExec->AllocSysObjectTags(ASOT_ITEMPOOL,
			ASOITEM_MFlags,		MEMF_SHARED,
			ASOITEM_ItemSize,	sizeof(struct AppShellInternalMessage),
			ASOITEM_BatchSize,	100,
			ASOITEM_Protected,	TRUE,
			TAG_DONE);

	NS_ASSERTION(mSharedWindowPort != NULL, "Can't create shared WindowPort");
	NS_ASSERTION(mAppShellPort != NULL, "Can't create AppPort");
	NS_ASSERTION(mMessageItemPool != NULL, "Can't create Message ItemPool");
	NS_ASSERTION(mInternalPort != NULL, "Can't create internal message port");

	LOGAPPSHELL(("nsAppShell::Init() done, calling nsBaseAppShell::Init()\n"));

	nsresult res =  nsBaseAppShell::Init();

	LOGAPPSHELL(("nsBaseAppShell::Init() returned %d\n", res));

	mDestroyed = PR_FALSE;
	return res;
}



//-------------------------------------------------------------------------
//
// nsAppShell destructor
//
//-------------------------------------------------------------------------
nsAppShell::~nsAppShell()
{
	// Drain the port
	struct Message *m;

	if (mSharedWindowPort)
	{
		while ((m = IExec->GetMsg(mSharedWindowPort)))
			IExec->ReplyMsg(m);

		IExec->FreeSysObject(ASOT_PORT, mSharedWindowPort);

		mSharedWindowPort = NULL;
	}

	if (mAppShellPort)
	{
		while ((m = IExec->GetMsg(mAppShellPort)))
			IExec->ReplyMsg(m);

		IExec->FreeSysObject(ASOT_PORT, mAppShellPort);

		mAppShellPort = NULL;
	}

	if (mInternalPort)
	{
		while ((m = IExec->GetMsg(mInternalPort)))
			IExec->ReplyMsg(m);

		IExec->FreeSysObject(ASOT_PORT, mInternalPort);

		mInternalPort = NULL;
	}

	IExec->FreeSysObject(ASOT_ITEMPOOL, mMessageItemPool);
	mDestroyed = PR_TRUE;
}

PRBool nsAppShell::ProcessNextNativeEvent(PRBool mayWait)
{
	PRBool gotMessage = PR_FALSE;
	struct Message *m;

	if (!mAppShellPort)
		return PR_FALSE;

	LOGAPPSHELL(("ProcessNextNativeEvent in %p, mayWait = %s\n", IExec->FindTask(NULL), mayWait ? "true" : "false"));

	// Call the native event callback
	while (mAppShellPort && (m = IExec->GetMsg(mAppShellPort)))
	{
		//gotMessage = PR_TRUE;
		NativeEventCallback();
		IExec->ItemPoolFree(mMessageItemPool, (void *)m);
	}

	// Process window messages
	struct IntuiMessage *imsg;

	while (mSharedWindowPort &&
			(imsg = reinterpret_cast<struct IntuiMessage *>(IExec->GetMsg(mSharedWindowPort))))
	{
		gotMessage = PR_TRUE;

		LOGAPPSHELL(("Got IDCMP message\n"));

		PRBool didHandle = TRUE;
		if (imsg->IDCMPWindow)
		{
			nsWindow *win = reinterpret_cast<nsWindow *>(imsg->IDCMPWindow->UserData);
			didHandle = win->HandleAmigaIDCMP(imsg);
		}
		else
		{
			LOGAPPSHELL(("Stray IDCMP\n"));
		}

		if (didHandle == PR_FALSE)
		{
			// See what else we can do with this message
		}
		IExec->ReplyMsg(reinterpret_cast<struct Message *>(imsg));
	}

	// Process internal port
	struct AppShellInternalMessage *im;
	while (mInternalPort &&
			(im = reinterpret_cast<struct AppShellInternalMessage *>(IExec->GetMsg(mInternalPort))))
	{
		//gotMessage = PR_TRUE;
		if (im->data1 == 0)
		{
			nsWindow *w = reinterpret_cast<nsWindow *>(im->data2);
			if (nsWindow::ValidateWindow(w))
				w->DrawDamagedRegions();
		}
		IExec->ItemPoolFree(mMessageItemPool, (void *)im);
	}

	if (mayWait)
	{
		uint32 sigMask =
				  (1L << mAppShellPort->mp_SigBit)
				| (1L << mSharedWindowPort->mp_SigBit)
				| (1l << mInternalPort->mp_SigBit)
				| SIGBREAKF_CTRL_C
				;

		IExec->Wait(sigMask);
	}

	return gotMessage;
}
//-------------------------------------------------------------------------

void nsAppShell::ScheduleNativeEventCallback()
{
	LOGAPPSHELL(("ScheduleNativeEventCallback in %s\n", IExec->FindTask(NULL)->tc_Node.ln_Name));


	struct Message *m = (Message *)IExec->ItemPoolAlloc(mMessageItemPool);
	NS_ASSERTION(m != NULL, "Can't create notify message");

	IExec->PutMsg(mAppShellPort, m);
}


void
nsAppShell::SynthesizeRedrawEvent(void *destWindow)
{
	LOGAPPSHELL(("SynthesizeRedrawEvent in %p\n", IExec->FindTask(NULL)));


	struct AppShellInternalMessage *m = (AppShellInternalMessage *)IExec->ItemPoolAlloc(mMessageItemPool);
	NS_ASSERTION(m != NULL, "Can't create notify message");

	m->data1 = 0;
	m->data2 = destWindow;
	IExec->PutMsg(mInternalPort, &m->execMessage);
}


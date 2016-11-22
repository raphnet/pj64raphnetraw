/* raphnetraw input plugin
 * (C) 2016 Raphael Assenat
 *
 * Based on the N-Rage`s Dinput8 Plugin
 * (C) 2002, 2006  Norbert Wladyka
 *
 *  This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <windows.h>
#include <stdio.h>
#include "common.h"
#include "version.h"
#include "gcn64.h"
#include "gcn64lib.h"

// ProtoTypes //
static int prepareHeap();

// Global Variables //
HANDLE g_hHeap = NULL;				// Handle to our heap
FILE *logfptr = NULL;
static gcn64_hdl_t gcn64_handle = NULL;

// Types //
typedef struct _EMULATOR_INFO
{
	int fInitialisedPlugin;
	HWND hMainWindow;
	HINSTANCE hinst;
	LANGID Language;
} EMULATOR_INFO, *LPEMULATOR_INFO;


EMULATOR_INFO g_strEmuInfo;			// emulator info?  Stores stuff like our hWnd handle and whether the plugin is initialized yet

CRITICAL_SECTION g_critical;		// our critical section semaphore

#define M64MSG_INFO		0
#define M64MSG_ERROR	1

static void DebugMessage(int type, const char *message, ...)
{
	va_list args;

	if (logfptr) {
		va_start(args, message);
		vfprintf(logfptr, message, args);
		va_end(args);
		fflush(logfptr);
	}
}

static void DebugWriteA(const char *str)
{
	if (logfptr) {
		fputs(str, logfptr);
		fflush(logfptr);
	}
}


EXPORT BOOL APIENTRY DllMain( HINSTANCE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved )
{
	char logfilename[256];

	switch ( ul_reason_for_call )
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls( hModule );
		if( !prepareHeap())
			return FALSE;

		sprintf(logfilename, "%s\\%s\\raphnetraw.log", getenv("homedrive"), getenv("homepath"));
		logfptr = fopen(logfilename, "wct");

		DebugWriteA("*** DLL Attach | built on " __DATE__ " at " __TIME__")\n");
		ZeroMemory( &g_strEmuInfo, sizeof(g_strEmuInfo) );
		g_strEmuInfo.hinst = hModule;
		DebugWriteA("  (compiled in ANSI mode, language detection DISABLED.)\n");
		g_strEmuInfo.Language = 0;
		InitializeCriticalSection( &g_critical );
		gcn64_handle = NULL;
		break;

	case DLL_THREAD_ATTACH:
		break;

	case DLL_THREAD_DETACH:
		break;

	case DLL_PROCESS_DETACH:
		//CloseDLL();
		DebugWriteA("*** DLL Detach\n");

		DeleteCriticalSection( &g_critical );

		// Moved here from CloseDll... Heap is created from DllMain,
		// and now it's destroyed by DllMain... just safer code --rabid
		if( g_hHeap != NULL )
		{
			HeapDestroy( g_hHeap );
			g_hHeap = NULL;
		}

		if (logfptr) {
			fclose(logfptr);
			logfptr = NULL;
		}
		break;
	}
    return TRUE;
}

EXPORT void CALL GetDllInfo ( PLUGIN_INFO* PluginInfo )
{
	DebugWriteA("CALLED: GetDllInfo\n");
	sprintf(PluginInfo->Name,"%s version %d.%d.%d", PLUGIN_NAME, VERSION_PRINTF_SPLIT(PLUGIN_VERSION));

	PluginInfo->Type = PLUGIN_TYPE_CONTROLLER;
	PluginInfo->Version = SPECS_VERSION;
}

EXPORT void CALL DllAbout ( HWND hParent )
{
	char tmpbuf[128];

	DebugWriteA("CALLED: DllAbout\n");

	sprintf(tmpbuf, PLUGIN_NAME" version %d.%d.%d (Compiled on "__DATE__")\n\n"
					"by raphnet\n", VERSION_PRINTF_SPLIT(PLUGIN_VERSION));

	MessageBox( hParent, tmpbuf, "About", MB_OK | MB_ICONINFORMATION);

	return;
}

#if 0
EXPORT void CALL DllConfig ( HWND hParent )
{
	DebugWriteA("CALLED: DllConfig\n");
	return;
}
#endif

#if 0
EXPORT void CALL DllTest ( HWND hParent )
{
	DebugWriteA("CALLED: DllTest\n");
	return;
}
#endif

EXPORT void CALL InitiateControllers( CONTROL_INFO ControlInfo)
{
	int i;
	struct gcn64_list_ctx * lctx;
	struct gcn64_info inf;

	DebugWriteA("CALLED: InitiateControllers\n");
	if( !prepareHeap())
		return;

	if (!g_strEmuInfo.fInitialisedPlugin) {
		gcn64_init(1);
		DebugMessage(M64MSG_INFO, "gcn64_init\n");
	}

	lctx = gcn64_allocListCtx();
	if (!lctx) {
		DebugMessage(M64MSG_ERROR, "Could not allocate gcn64 list context\n");
		return;
	}

	/* This uses the first device we are able to open. TODO : Configurable per serial */
	while (gcn64_listDevices(&inf, lctx)) {
		gcn64_handle = gcn64_openDevice(&inf);
		if (!gcn64_handle) {
			DebugMessage(M64MSG_ERROR, "Could not open gcn64 device\n");
		}

		gcn64lib_suspendPolling(gcn64_handle, 1);

		DebugMessage(M64MSG_INFO, "Using USB device 0x%04x:0x%04x serial '%ls' name '%ls'",
			inf.usb_vid, inf.usb_pid, inf.str_serial, inf.str_prodname);

		break;
	}
	gcn64_freeListCtx(lctx);

	if (!gcn64_handle) {
		DebugMessage(M64MSG_INFO, "No adapters detected.\n");
	}

    g_strEmuInfo.hMainWindow   = ControlInfo.hMainWindow;

	EnterCriticalSection( &g_critical );


	if (gcn64_handle) {
		for (i=0; i<4; i++) {
			ControlInfo.Controls[i].RawData = 1;
			ControlInfo.Controls[i].Present = 1;
		}
	}

	g_strEmuInfo.fInitialisedPlugin = 1;

	LeaveCriticalSection( &g_critical );

	return;
}

EXPORT void CALL RomOpen (void)
{
	DebugWriteA("CALLED: RomOpen\n");

	return;
}

EXPORT void CALL RomClosed(void)
{
	DebugWriteA("CALLED: RomClosed\n");
	return;
}

EXPORT void CALL GetKeys(int Control, BUTTONS * Keys )
{
	return;
}

EXPORT void CALL ControllerCommand( int Control, BYTE * Command)
{
	// We don't need to use this because it will be echoed immediately afterwards
	// by a call to ReadController
	return;
}

EXPORT void CALL ReadController( int Control, BYTE * Command )
{
	if (Control != -1 && Command) {

		EnterCriticalSection( &g_critical );

		// Command structure
		// based on LaC's n64 hardware dox... v0.8 (unofficial release)
		//
		// Byte 0: nBytes to transmit
		// Byte 1: nBytes to receive
		// Byte 2: TX data start (first byte is the command)
		// Byte 2+[tx_len]: RX data start
		//
		// Request info (status):
		//
		// tx[1] = {0x00 }, expecting 3 bytes
		//
		// Get button status:
		//
		// tx[1] = {0x01 }, expecting 4 bytes
		//
		// Rumble commands: (Super smash bros)
		//
		// tx[35] = {0x03 0x80 0x01 0xfe 0xfe 0xfe 0xfe 0xfe 0xfe 0xfe 0xfe 0xfe
		// 			0xfe 0xfe 0xfe 0xfe 0xfe 0xfe 0xfe 0xfe 0xfe 0xfe 0xfe 0xfe 0xfe
		// 			0xfe 0xfe 0xfe 0xfe 0xfe 0xfe 0xfe 0xfe 0xfe 0xfe }, expecting 1 byte
		//
		// tx[3] = {0x02 0x80 0x01 }, expecting 33 bytes
		//
		// Memory card commands: (Mario Kart 64)
		//
		// tx[3] = {0x02 0x00 0x35 }, expecting 33 bytes
		// tx[35] = {0x03 0x02 0x19 0x00 0x71 0xbf 0x95 0xf5 0xfb 0xd7 0xc1 0x00 0x00
		// 			0x00 0x03 0x00 0x03 0x00 0x03 0x00 0x03 0x00 0x03 0x00 0x03 0x00
		// 			0x03 0x00 0x03 0x00 0x03 0x00 0x03 0x00 0x03 }, expecting 1 byte
		if (gcn64_handle) {
			int tx_len = Command[0];
			int rx_len = Command[1];
			int res;

			res = gcn64lib_rawSiCommand(gcn64_handle,
									Control,		// Channel
									Command + 2,	// TX data
									tx_len,		// TX data len
									Command + 2 + tx_len,	// RX data
									rx_len		// TX data len
								);
			if (res <= 0) {
				//
				// 0x00 - no error, operation successful.
				// 0x80 - error, device not present for specified command.
				// 0x40 - error, unable to send/recieve the number bytes for command type.
				//
				// Diagrams 1.2-1.4 shows that lower bits are preserved (i.e. The length
				// byte is not completely replaced).
				Command[1] &= 0xC0;
				Command[1] |= 0x80;
			}
		}

		LeaveCriticalSection( &g_critical );
	}

	return;
}

EXPORT void CALL WM_KeyDown( WPARAM wParam, LPARAM lParam )
{
	return;
}

EXPORT void CALL WM_KeyUp( WPARAM wParam, LPARAM lParam )
{
	return;
}

EXPORT void CALL CloseDLL (void)
{
	DebugWriteA("CALLED: CloseDLL\n");

	if (g_strEmuInfo.fInitialisedPlugin) {
		if (gcn64_handle) {
			gcn64_closeDevice(gcn64_handle);
		}
		g_strEmuInfo.fInitialisedPlugin = 0;
	}

	return;
}

// Prepare a global heap.  Use P_malloc and P_free as wrappers to grab/release memory.
static int prepareHeap()
{
	if( g_hHeap == NULL )
		g_hHeap = HeapCreate( 0, 4*1024, 0 );
	return (g_hHeap != NULL);
}

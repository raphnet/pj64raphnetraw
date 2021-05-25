/* raphnetraw input plugin
 * (C) 2016-2017 Raphael Assenat
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
#include "plugin_back.h"

// ProtoTypes //
static int prepareHeap();

// Global Variables //
HANDLE g_hHeap = NULL;				// Handle to our heap
FILE *logfptr = NULL;

// Types //
typedef struct _EMULATOR_INFO
{
	int fInitialisedPlugin;
	HWND hMainWindow;
	HINSTANCE hinst;
	LANGID Language;
} EMULATOR_INFO, *LPEMULATOR_INFO;

#define MAX_CONTROLLERS	4

#ifdef PORTS_1_AND_4
/* The VRU should be in port 4. On a dual-port adapter, this routes
 * the 2nd physical port to what the emulator sees as the 4th port. */
static int emu2adap_portmap[MAX_CONTROLLERS] = { 0, 2, 3, 1 };
#undef PLUGIN_NAME
#define PLUGIN_NAME "raphnetraw ports 1 and 4"
#else
#ifdef PORTS_1_AND_3
/* The Densha de GO controller should be in port 3. On a dual-port adapter,
 * this routes the 2nd physical port to what the emulator sees as the 4th port. */
static int emu2adap_portmap[MAX_CONTROLLERS] = { 0, 2, 1, 3 };
#undef PLUGIN_NAME
#define PLUGIN_NAME "raphnetraw ports 1 and 3"
#else
static int emu2adap_portmap[MAX_CONTROLLERS] = { 0, 1, 2, 3 };
#endif
#endif

#ifdef PORT_1_ONLY
#undef PLUGIN_NAME
#define PLUGIN_NAME "raphnetraw port 1 only"
#endif

#ifdef NO_BLOCK_IO
#undef PLUGIN_NAME
#define PLUGIN_NAME "raphnetraw 1 player NET version"
#endif

#define EMU_2_ADAP_PORT(a)     ((a) == -1 ? -1 : emu2adap_portmap[a])

EMULATOR_INFO g_strEmuInfo;			// emulator info?  Stores stuff like our hWnd handle and whether the plugin is initialized yet

CRITICAL_SECTION g_critical;		// our critical section semaphore

#define M64MSG_INFO		0
#define M64MSG_ERROR	1
#define M64MSG_WARNING	2

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
		logfptr = NULL;
#ifdef WITH_DEBUG_LOG
		logfptr = fopen(logfilename, "act");
#endif

		DebugWriteA("*** DLL Attach | built on " __DATE__ " at " __TIME__")\n");
		ZeroMemory( &g_strEmuInfo, sizeof(g_strEmuInfo) );
		g_strEmuInfo.hinst = hModule;
		g_strEmuInfo.Language = 0;
		InitializeCriticalSection( &g_critical );
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
	char tmpbuf[512];
#if defined(PORTS_1_AND_4)
	const char *specialBuild = "\n* * * Special build * * *\nAdapter ports 1 and 2 -> N64 ports 1 and 4\n(For use with VRU in port 4 with two-player adapter)\n";
#elif defined(PORTS_1_AND_3)
	const char *specialBuild = "\n* * * Special build * * *\nAdapter ports 1 and 2 -> N64 ports 1 and 3\n(For Densha de GO!)\n";
#elif defined(NO_BLOCK_IO)
	const char *specialBuild = "\n* * * Special build * * *\nNET version\n(For playing online)\n";
#elif defined(PORT_1_ONLY)
	const char *specialBuild = "\n* * * Special build * * *\nAdapter port 1 only\n(For maximum performance)\n";
#else
	const char *specialBuild = "Standard version";
#endif

	DebugWriteA("CALLED: DllAbout\n");

	snprintf(tmpbuf, sizeof(tmpbuf),
					PLUGIN_NAME" version %d.%d.%d (Compiled on "__DATE__")\n"
					"%s\n"
					"by raphnet\n", VERSION_PRINTF_SPLIT(PLUGIN_VERSION), specialBuild);

	MessageBox( hParent, tmpbuf, "About", MB_OK | MB_ICONINFORMATION);

	return;
}

EXPORT void CALL DllConfig ( HWND hParent )
{
	char tmpbuf[512];
	int n_controllers;

	if( !prepareHeap())
		return;

	pb_init(DebugMessage);
	n_controllers = pb_scanControllers();

	if (n_controllers <= 0) {
		DebugMessage(M64MSG_ERROR, "No adapters detected.\n");
		MessageBox( hParent, "raphnetraw: Adapter not detected.\n\nIf the adapter management tool is running,\nclose it.", "Warning", MB_OK | MB_ICONWARNING);
		return;
	}

	DebugWriteA("CALLED: DllConfig\n");

	snprintf(tmpbuf, sizeof(tmpbuf),
					"No calibration or configuration required.\n"
					"The controller will work, respond and feel exactly as it would in real life.\n\n"
					"Controller ports detected: %d", n_controllers);

	MessageBox( hParent, tmpbuf, "Raphnetraw Configuration", MB_OK | MB_ICONINFORMATION);

	return;
}

#if 0
EXPORT void CALL DllTest ( HWND hParent )
{
	DebugWriteA("CALLED: DllTest\n");
	return;
}
#endif

#if (SPECS_VERSION < 0x0101)
EXPORT void CALL InitiateControllers( void *hMainWindow, CONTROL Controls[4])
#else
EXPORT void CALL InitiateControllers( CONTROL_INFO ControlInfo)
#endif
{
	int i, n_controllers, adap_port;
	BOOL present;

	if( !prepareHeap())
		return;

#if (SPECS_VERSION < 0x0101)
	g_strEmuInfo.hMainWindow = hMainWindow;
#else
    g_strEmuInfo.hMainWindow = ControlInfo.hMainWindow;
#endif

	if (!g_strEmuInfo.fInitialisedPlugin) {
		pb_init(DebugMessage);
	}

	n_controllers = pb_scanControllers();

	if (n_controllers <= 0) {
		DebugMessage(M64MSG_ERROR, "No adapters detected.\n");
		MessageBox( g_strEmuInfo.hMainWindow, "raphnetraw: Adapter not detected.\n\nIf the adapter management tool is running,\nclose it.", "Warning", MB_OK | MB_ICONWARNING);
		return;
	}

	EnterCriticalSection( &g_critical );

	for (i=0; i<MAX_CONTROLLERS; i++) {
		adap_port = EMU_2_ADAP_PORT(i);

		if (adap_port < n_controllers) {
			present = 1;
		} else {
			present = 0;
		}

#ifdef PORT_1_ONLY
		if (i>0)
			present = 0;
#endif

#if (SPECS_VERSION < 0x0101)
		Controls[i].RawData = present;
		Controls[i].Present = present;
#else
		ControlInfo.Controls[i].RawData = present;
		ControlInfo.Controls[i].Present = present;
#endif
	}

	g_strEmuInfo.fInitialisedPlugin = 1;

	LeaveCriticalSection( &g_critical );

	return;
}

EXPORT void CALL RomOpen (void)
{
	DebugWriteA("CALLED: RomOpen\n");
	pb_romOpen();
}

EXPORT void CALL RomClosed(void)
{
	DebugWriteA("CALLED: RomClosed\n");
	pb_romClosed();
}

EXPORT void CALL GetKeys(int Control, BUTTONS * Keys )
{
}

EXPORT void CALL ControllerCommand( int Control, BYTE * Command)
{
	pb_controllerCommand(EMU_2_ADAP_PORT(Control), Command);
}

EXPORT void CALL ReadController( int Control, BYTE * Command )
{
	EnterCriticalSection( &g_critical );
	pb_readController(EMU_2_ADAP_PORT(Control), Command);
	LeaveCriticalSection( &g_critical );
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
		pb_shutdown();
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

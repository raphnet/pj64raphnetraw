#include <windows.h>
#include <strsafe.h>
#include <stdio.h>
#include <libgen.h>

typedef struct
{
	WORD Version;        /* Should be set to 0x0101 */
	WORD Type;           /* Set to PLUGIN_TYPE_CONTROLLER */
	char Name[100];      /* Name of the DLL */
	BOOL Reserved1;
	BOOL Reserved2;
} PLUGIN_INFO;

static void ErrorExit(LPTSTR lpszFunction)
{
    // Retrieve the system error message for the last-error code

    LPVOID lpMsgBuf;
    DWORD dw = GetLastError();

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

	fputs(lpMsgBuf, stdout);

    LocalFree(lpMsgBuf);
    ExitProcess(dw);
}

static void printPluginInfo(PLUGIN_INFO *pinf, const char *filename)
{
	char *f = strdup(filename), *bf;

	if (!f)
		return;
	bf = basename(f);

	printf("%s\n", pinf->Name);
	printf("%s\n", bf);
	printf("Type: %d\n", pinf->Type);
	printf("Spec version: 0x%04x\n", pinf->Version);

	free(f);
}

int main(int argc, char **argv)
{
	HMODULE lib;
	void (_cdecl *GetDllInfo)(PLUGIN_INFO * PluginInfo);
	PLUGIN_INFO pinf = { };
	int ret = 0;

	if (argc < 2) {
		printf("Usage: plugin_info.exe file.dll\n");
		return 1;
	}

	lib = LoadLibrary(argv[1]);
	if (lib == NULL) {
		printf("Failed to load '%s'...\n", argv[1]);
		ErrorExit("LoadLibrary");
	}

	GetDllInfo = (void*)GetProcAddress(lib, "GetDllInfo");
	if (GetDllInfo) {
		GetDllInfo(&pinf);
		printPluginInfo(&pinf, argv[1]);
	}
	else {
		fprintf(stderr, "GetDllInfo function not found\n");
		ret = -1;
	}

	FreeLibrary(lib);

	return ret;
}



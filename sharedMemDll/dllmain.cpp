// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"


extern DWORD WINAPI ThreadProc(LPVOID lpParam);

// The DLL entry-point function sets up shared memory using a 
// named file-mapping object. 

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{


	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
            // Create a named file mapping object
        if(!ProcAttach())
			return(FALSE);
 
        // Initialize memory if this is the first process
 
        break; 
 
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		ProcDetach();
		break;
	}
    UNREFERENCED_PARAMETER(hModule); 
    UNREFERENCED_PARAMETER(lpReserved); 
	return TRUE;
}


/** included by std_lib.h and std_flib.h to indicate how the program         *
 ** makes use of the defined functions:                                      *
 ** -  define STD_DLL_LINK if code is provided by the user of the DLL        *
 ** -  undef STD_DLL_LINK if code is used directely                          *
 ** -  define STD_DLL_USER only if code has to be passed to another DLL      */
#ifndef COMPILE_INCLUDED

#undef BORLAND_C45
#undef WATCOM_C
#undef REX_COMPILE
#define MSVC6
#define MSVC_EXPRESS
#undef CYGWIN_GCC


#ifdef CYGWIN_GCC
	#define strtok_r strtok_r_eggert
	#define HANDLE__ HINSTANCE
	#define _timeb timeb
	#define ___pascal
	#define far
#endif

#ifdef MSVC_EXPRESS
	#define HANDLE__ HINSTANCE
	#define ___pascal
	#define far
#endif

#ifndef DLL_LINK
	#ifdef MSVC6
		#define __pascal
		#define far
	#endif

   #ifdef REX_COMPILE
		#undef DLL_LINK
	#else
		#undef DLL_LINK        /* <- change if necessary */
	#endif
#endif


#ifdef DLL_LINK
   #define STD_DLL_LINK      /* <- DO NOT CHANGE!!                   *
                              *    DLL_LINK overwrites STD_DLL_LINK  */
#else
   #undef STD_DLL_LINK       /* <- change if necessary */
#endif

#ifndef STD_DLL_LINK
   #undef STD_DLL_USER       /* <- change if necessary */
#else
   #undef STD_DLL_USER       /* <- DO NOT CHANGE!! */
   #include <windows.h>
#endif



   #ifdef WATCOM_C
      #define DLLEXPORT __declspec(dllexport)
   #else
      #ifdef __WIN32__
         #define DLLEXPORT  far _stdcall _export
         extern CRITICAL_SECTION gCriticalSection;
      #else
         #define DLLEXPORT  far _pascal _export
      #endif
   #endif

#define FAR___ far


#define COMPILE_INCLUDED

#endif

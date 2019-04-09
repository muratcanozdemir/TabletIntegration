// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>



// TODO: reference additional headers your program requires here
#include <stdio.h>
#include <memory.h> 
#include <sys/timeb.h>
#include <time.h>


#ifdef __cplusplus    // If used by C++ code, 
extern "C" {          // we need to export the C interface
#endif
#include "std_lib.h"
#include "str_lib.h"
#include "matlb.h"
#ifdef __cplusplus
}
#endif

BOOL ProcAttach(void);
void ProcDetach(void);

#define SHMEMSIZE 65536 


#define MAX_FNAME_LENGTH	 500
#define ITEM_BSIZE			   8
#define PACKET_N_DOUBLE       11
#define CBUFF_N_PACKETS      731
#define PACKET_BSIZE          88  //= 11*8
#define CBUFF_BSIZE        64328  //=11*8*731

#define PROCESS_CNT_PTR_ADDRESS	CBUFF_BSIZE



#define FILENAME_PTR_ADDRESS		PROCESS_CNT_PTR_ADDRESS+4 // 4 is the size of PROCESS_CNT_PTR_ADDRESS
#define WRITE_PTR_ADDRESS			FILENAME_PTR_ADDRESS+MAX_FNAME_LENGTH+1  //
#define READ_PTR_ADDRESS			WRITE_PTR_ADDRESS+4  //  
#define THREADRUNNING_PTR_ADDRESS	READ_PTR_ADDRESS+4   //  flag signaling that writing thread continues running
#define HOLDMEM_PTR_ADDRESS			THREADRUNNING_PTR_ADDRESS+4  //  packet for holding data
#define PACKET_CNT_PTR_ADDRESS		HOLDMEM_PTR_ADDRESS+PACKET_BSIZE  // unsigned long packet counter
#define WRITE_CNT_PTR_ADDRESS		PACKET_CNT_PTR_ADDRESS+4 //  unsigned long write packet counter
#define FP_PTR_ADDRESS				WRITE_CNT_PTR_ADDRESS+4 //  file pointer
#define UPDATE_CB_PTR_ADDRESS		FP_PTR_ADDRESS+4 //  int  1: buffer has to be updated 0: buffer has not to be updated 
#define ERROR_PTR_ADDRESS			UPDATE_CB_PTR_ADDRESS+4 // int error code for write_to_circular_buffer: 0: no error 
															//												1: handle error
															//												2: WAIT_ABANDONED
															//												3: WAIT_TIMEOUT
															//												4: default
#define DEBUG_FNM_PTR_ADDRESS		ERROR_PTR_ADDRESS+4		//  Debug filename
#define START_TIME_PTR_ADDRESS		DEBUG_FNM_PTR_ADDRESS+MAX_FNAME_LENGTH+1 // double start_time

												// size of all variables=9*4+PACKET_BSIZE+2*(MAX_FNAME_LENGTH+1) + 8
												//						=9*4+  88        +     2*501             + 8
												//						= 1134
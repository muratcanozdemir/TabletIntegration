// sharedMemDll.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"


static LPVOID lpvMem = NULL;      // pointer to shared memory
static HANDLE hMapObject = NULL;  // handle to file mapping
static double DataPacketBuf[PACKET_N_DOUBLE];
static HANDLE ghMutex;
static HANDLE ghWriteEvent;
 

// The export mechanism used here is the __declspec(export)
// method supported by Microsoft Visual Studio, but any
// other export method supported by your development
// environment may be substituted.

#ifdef __cplusplus    // If used by C++ code, 
extern "C" {          // we need to export the C interface
#endif
 


// Low level routines not involving the MUTEX:

// SetSharedMem sets the contents of the shared memory 
 
__declspec(dllexport) void __cdecl SetSharedMem(char *lpszBuf) 
{ 
    char *lpszTmp; 
    DWORD dwCount=1;
 
    // Get the address of the shared memory block
 
    lpszTmp = (char *) lpvMem; 
 
    // Copy the null-terminated string into shared memory
 
    while (*lpszBuf && dwCount<SHMEMSIZE) 
    {
        *lpszTmp++ = *lpszBuf++; 
        dwCount++;
    }
    *lpszTmp = '\0'; 
} 
 
// GetSharedMem gets the contents of the shared memory
 
__declspec(dllexport) void __cdecl GetSharedMem(char *lpszBuf, int cchSize) 
{ 
    char *lpszTmp; 
 
    // Get the address of the shared memory block
 
    lpszTmp = (char *) lpvMem; 
 
    // Copy from shared memory into the caller's buffer
 
    while (*lpszTmp && --cchSize) 
        *lpszBuf++ = *lpszTmp++; 
    *lpszBuf = '\0'; 
}

__declspec(dllexport) void __cdecl SHM_getDebugFilename(char *lpszBuf, int cchSize)
{ 
char *lpszTmp;
 
	if(cchSize>MAX_FNAME_LENGTH+1)
		cchSize=MAX_FNAME_LENGTH+1;
	// Get the address of the shared memory block
	lpszTmp = (char *)lpvMem+DEBUG_FNM_PTR_ADDRESS;
	while (*lpszTmp && --cchSize) 
		*lpszBuf++ = *lpszTmp++; 
	*lpszBuf = '\0'; 


    return;
} 

__declspec(dllexport) void __cdecl setWriteIndex(int index) 
{ 
    char *lpszTmp; 
 
    // Get the address of the shared memory block
 
    lpszTmp = (char *)lpvMem+WRITE_PTR_ADDRESS; 
    *(int *)lpszTmp=index;
} 

__declspec(dllexport) int __cdecl getWriteIndex(void) 
{ 
    char *lpszTmp; 
 
    // Get the address of the shared memory block
 
    lpszTmp = (char *)lpvMem+WRITE_PTR_ADDRESS; 
    return(*(int *)lpszTmp);
} 

__declspec(dllexport) void __cdecl setReadIndex(int index) 
{ 
    char *lpszTmp; 
 
    // Get the address of the shared memory block
 
    lpszTmp = (char *)lpvMem+READ_PTR_ADDRESS; 
    *(int *)lpszTmp=index;
} 


__declspec(dllexport) int __cdecl getReadIndex(void) 
{ 
    char *lpszTmp; 
 
    // Get the address of the shared memory block
 
    lpszTmp = (char *)lpvMem+READ_PTR_ADDRESS; 
    return(*(int *)lpszTmp);
} 

__declspec(dllexport) void __cdecl setReadIndex_to_testvalue(void) 
{ 
    char *lpszTmp; 
	int testvalue;
 
    // Get the address of the shared memory block
 
    lpszTmp = (char *)lpvMem+READ_PTR_ADDRESS; 

	testvalue=sizeof(FILE *);
    *(int *)lpszTmp=testvalue;
} 

__declspec(dllexport) void __cdecl setIndexedDataPacket(double *Packet,int index) 
{ 
    char *lpszTmp;
	double *dp1,*dp2;
	unsigned short i;
    // Get the address of the shared memory block
    lpszTmp = (char *)lpvMem+index*PACKET_BSIZE;
	dp1=(double *)lpszTmp;
	dp2=Packet;
    for(i=0;i<PACKET_N_DOUBLE;i++) 
        *dp1++ = *dp2++; 
} 

__declspec(dllexport) double * __cdecl getIndexedDataPacket(double *Packet,int index) 
{ 
    char *lpszTmp;
	double *dp1,*dp2;
	unsigned short i;
    // Get the address of the shared memory block
    lpszTmp = (char *)lpvMem+index*PACKET_BSIZE;
	dp1=(double *)lpszTmp;
	dp2=Packet;
    for(i=0;i<PACKET_N_DOUBLE;i++) 
        *dp2++ = *dp1++; 

    return(Packet);
} 


__declspec(dllexport) double* __cdecl getLastWrittenDataPacket(double *Packet) 
{ 
    char *lpszTmp;
	double *dp1,*dp2;
	unsigned short i,index;
    // Get the address of the shared memory block
    index=getWriteIndex();
    lpszTmp = (char *)lpvMem+index*PACKET_BSIZE;
	dp1=(double *)lpszTmp;
	dp2=Packet;
    for(i=0;i<PACKET_N_DOUBLE;i++) 
        *dp2++ = *dp1++; 
    return(Packet);
} 

__declspec(dllexport) int __cdecl setNextDataPacket(double *Packet) 
{ 
	/* returns 0: success
	           1: overrun */
    char *lpszTmp;
	double *dp1,*dp2;
	unsigned short i,iw,ir;
    // Get the address of the shared memory block
    iw=getWriteIndex();
	iw=(iw+1) % CBUFF_N_PACKETS;
    lpszTmp = (char *)lpvMem+iw*PACKET_BSIZE;
	dp1=(double *)lpszTmp;
	dp2=Packet;
    for(i=0;i<PACKET_N_DOUBLE;i++) 
        *dp1++ = *dp2++; 

	setWriteIndex(iw);
	ir=getReadIndex();
	if(ir==iw)  //buffer overrun
		return(1);
	else
		return(0);
} 


__declspec(dllexport) double * __cdecl getIndexedDataBlock(double *Packets,int start_index,int end_index) 
{ 
    char *lpszTmp;
	double *dp1,*dp2;
	unsigned short i;
	unsigned short cnt;
    // Get the address of the shared memory block
    lpszTmp = (char *)lpvMem+start_index*PACKET_BSIZE;
	dp1=(double *)lpszTmp;
	dp2=Packets;
	cnt=(end_index-start_index+1)*PACKET_N_DOUBLE;
    for(i=0;i<cnt;i++) 
        *dp2++ = *dp1++; 

    return(Packets);
} 




__declspec(dllexport) void __cdecl Trigger_WriteEvent(void) 
{ 
	DWORD dwWaitResult;	


#ifndef KJHDKGLKSLKDLHDLKJH

		dwWaitResult = WaitForSingleObject( 
		    ghWriteEvent, // event handle
		    0);    // indefinite wait

		switch (dwWaitResult) 
		{
			// Event object was signaled
			case WAIT_OBJECT_0: 
				break;
			case WAIT_TIMEOUT:
				SetEvent(ghWriteEvent);
				break;
		};
#else
	SetEvent(ghWriteEvent);
#endif

}


__declspec(dllexport) int __cdecl getPacketWidth(void)
{
	return((int)PACKET_N_DOUBLE);
}

__declspec(dllexport) int __cdecl getNumOfPacketsInBuffer(void)
{
	return((int)CBUFF_N_PACKETS);
}

__declspec(dllexport) int __cdecl getPacketBSize(void)
{
	return(PACKET_N_DOUBLE*sizeof(double));
}

__declspec(dllexport) int __cdecl getMutexError(void)
{
    char *lpszTmp; 
 
    // Get the address of the shared memory block
 
    lpszTmp = (char *)lpvMem+ERROR_PTR_ADDRESS; 
    return(*(int *)lpszTmp);

}

__declspec(dllexport) int __cdecl getProcessCnt(void)
{
    char *lpszTmp; 
 
    // Get the address of the shared memory block
 
    lpszTmp = (char *)lpvMem+PROCESS_CNT_PTR_ADDRESS; 
    return(*(int *)lpszTmp);

}

__declspec(dllexport) unsigned long __cdecl get_HOLDMEM_PTR_ADDRESS(void)
{
unsigned long i;
	i=(unsigned long)(HOLDMEM_PTR_ADDRESS);
	return(i);
}






// High level routines involving the MUTEX:
__declspec(dllexport) int __cdecl read_i_equals_write_i(void)
{ 
char *lpszTmp;
DWORD dwWaitResult;	
int write_i,read_i,result;

	result=-1;
	dwWaitResult = WaitForSingleObject( 
            ghMutex,    // handle to mutex
            500);  // no time-out interval
 
	switch (dwWaitResult) 
    {
		// The thread got ownership of the mutex
		case WAIT_OBJECT_0: 
			__try { 

 
				// Get the address of the shared memory block
				// read both read and write pointers 
				lpszTmp = (char *)lpvMem+WRITE_PTR_ADDRESS; 
				write_i=*(int *)lpszTmp;
				lpszTmp = (char *)lpvMem+READ_PTR_ADDRESS; 
				read_i=*(int *)lpszTmp;
				result=0;
			} 

			__finally { 
				// Release ownership of the mutex object
				if (! ReleaseMutex(ghMutex)) 
				{ 
					// Handle error.
				} 
			} 
			break; 

		// The thread got ownership of an abandoned mutex
		// The database is in an indeterminate state
		case WAIT_ABANDONED: 
			result=-2;
			break;
		default:
            result=-3; 
	}
	if(result<0)
		return(result);

    return(read_i==write_i);
} 

__declspec(dllexport) double __cdecl get_high_resolution_time_s(void) 
{ 
struct __timeb64 timebuffer;
LARGE_INTEGER HPC_Frequency,HPC_Counter;
double time_s;
	

	if(QueryPerformanceFrequency(&HPC_Frequency)==0)
	{
		_ftime64_s( &timebuffer );
		time_s=(double)timebuffer.time + timebuffer.millitm/1000.0;
	}
	else
	{
		QueryPerformanceCounter(&HPC_Counter);
		time_s=((double)HPC_Counter.QuadPart)/HPC_Frequency.QuadPart;
	};
	return(time_s);
}


__declspec(dllexport) FILE * __cdecl InitializeRecording(char *filename){
char *lpszTmp;
int SHM_Buffer_NumOfPackets;
DWORD dwWaitResult;	
FILE * fp;
double start_time;

	start_time=get_high_resolution_time_s();


	fp=fopen(filename,"wb+");
	fseek(fp,0,SEEK_SET);



	write_matlab_attr(fp,"Data",0,0,MATLB_TDOUBLE,1,0,MATLB_PC);

	dwWaitResult = WaitForSingleObject( 
            ghMutex,    // handle to mutex
            INFINITE);  // no time-out interval
 
	switch (dwWaitResult) 
    {
		// The thread got ownership of the mutex
		case WAIT_OBJECT_0: 
			__try { 

				lpszTmp = (char *)lpvMem+START_TIME_PTR_ADDRESS;
				*(double *)lpszTmp=start_time;

				lpszTmp = (char *)lpvMem+PACKET_CNT_PTR_ADDRESS;
				*(unsigned long *)lpszTmp=0;

				lpszTmp = (char *)lpvMem+WRITE_CNT_PTR_ADDRESS;
				*(unsigned long *)lpszTmp=0;

				SHM_Buffer_NumOfPackets=CBUFF_N_PACKETS;

				lpszTmp = (char *)lpvMem+WRITE_PTR_ADDRESS; 
				*(int *)lpszTmp=SHM_Buffer_NumOfPackets-1;
				lpszTmp = (char *)lpvMem+READ_PTR_ADDRESS; 
				*(int *)lpszTmp=SHM_Buffer_NumOfPackets-1;


				lpszTmp = (char *)lpvMem+UPDATE_CB_PTR_ADDRESS;
				*(int *)lpszTmp=1;

				lpszTmp = (char *)lpvMem+FP_PTR_ADDRESS;
				*(FILE **)lpszTmp=fp;

				lpszTmp = (char *)lpvMem+HOLDMEM_PTR_ADDRESS;
				memset(lpszTmp,0,PACKET_BSIZE);
			} 

			__finally { 
				// Release ownership of the mutex object
				if (! ReleaseMutex(ghMutex)) 
				{ 
					// Handle error.
				} 
			} 
			break; 

		// The thread got ownership of an abandoned mutex
		// The database is in an indeterminate state
		case WAIT_ABANDONED: 
                return(fp); 
	}

	return(fp);
}



__declspec(dllexport) unsigned long __cdecl getPacketCnt(void)
{ 
char *lpszTmp;
DWORD dwWaitResult;	
unsigned long PacketCnt;

	dwWaitResult = WaitForSingleObject( 
            ghMutex,    // handle to mutex
            INFINITE);  // no time-out interval
 
	switch (dwWaitResult) 
    {
		// The thread got ownership of the mutex
		case WAIT_OBJECT_0: 
			__try { 

 
				// Get the address of the shared memory block
				lpszTmp = (char *)lpvMem+PACKET_CNT_PTR_ADDRESS;
				PacketCnt=*(unsigned long *)lpszTmp;
			} 

			__finally { 
				// Release ownership of the mutex object
				if (! ReleaseMutex(ghMutex)) 
				{ 
					// Handle error.
				} 
			} 
			break; 

		// The thread got ownership of an abandoned mutex
		// The database is in an indeterminate state
		case WAIT_ABANDONED: 
                return(0); 
	}


    return(PacketCnt);
} 


__declspec(dllexport) int __cdecl SHM_getWriteIndex(void) 
{ 
char *lpszTmp;
DWORD dwWaitResult;	
int WriteIndex;

	dwWaitResult = WaitForSingleObject( 
            ghMutex,    // handle to mutex
            INFINITE);  // no time-out interval
 
	switch (dwWaitResult) 
    {
		// The thread got ownership of the mutex
		case WAIT_OBJECT_0: 
			__try { 

 
				// Get the address of the shared memory block
			    lpszTmp = (char *)lpvMem+WRITE_PTR_ADDRESS; 
				WriteIndex=*(int *)lpszTmp;
			} 

			__finally { 
				// Release ownership of the mutex object
				if (! ReleaseMutex(ghMutex)) 
				{ 
					// Handle error.
				} 
			} 
			break; 

		// The thread got ownership of an abandoned mutex
		// The database is in an indeterminate state
		case WAIT_ABANDONED: 
                return(0); 
	}


    return(WriteIndex);
} 



__declspec(dllexport) unsigned long __cdecl getWriteCnt(void)
{ 
char *lpszTmp;
DWORD dwWaitResult;	
unsigned long WriteCnt;

	dwWaitResult = WaitForSingleObject( 
            ghMutex,    // handle to mutex
            INFINITE);  // no time-out interval
 
	switch (dwWaitResult) 
    {
		// The thread got ownership of the mutex
		case WAIT_OBJECT_0: 
			__try { 

 
				// Get the address of the shared memory block
				lpszTmp = (char *)lpvMem+WRITE_CNT_PTR_ADDRESS;
				WriteCnt=*(unsigned long *)lpszTmp;
			} 

			__finally { 
				// Release ownership of the mutex object
				if (! ReleaseMutex(ghMutex)) 
				{ 
					// Handle error.
				} 
			} 
			break; 

		// The thread got ownership of an abandoned mutex
		// The database is in an indeterminate state
		case WAIT_ABANDONED: 
                return(0); 
	}


    return(WriteCnt);
} 

__declspec(dllexport) void __cdecl SHM_getFilename(char *lpszBuf, int cchSize)
{ 
char *lpszTmp;
DWORD dwWaitResult;	

	dwWaitResult = WaitForSingleObject( 
            ghMutex,    // handle to mutex
            INFINITE);  // no time-out interval
 
	switch (dwWaitResult) 
    {
		// The thread got ownership of the mutex
		case WAIT_OBJECT_0: 
			__try { 

 
				if(cchSize>MAX_FNAME_LENGTH+1)
					cchSize=MAX_FNAME_LENGTH+1;
				// Get the address of the shared memory block
				lpszTmp = (char *)lpvMem+FILENAME_PTR_ADDRESS;
				while (*lpszTmp && --cchSize) 
					*lpszBuf++ = *lpszTmp++; 
				*lpszBuf = '\0'; 

			} 

			__finally { 
				// Release ownership of the mutex object
				if (! ReleaseMutex(ghMutex)) 
				{ 
					// Handle error.
				} 
			} 
			break; 

		// The thread got ownership of an abandoned mutex
		// The database is in an indeterminate state
		case WAIT_ABANDONED: 
                return; 
	}


    return;
} 


__declspec(dllexport) void __cdecl SHM_setFilename(char *lpszBuf)
{ 
char *lpszTmp;
DWORD dwWaitResult,dwCount;	


	
	dwWaitResult = WaitForSingleObject( 
            ghMutex,    // handle to mutex
            INFINITE);  // no time-out interval
 
	switch (dwWaitResult) 
    {
		// The thread got ownership of the mutex
		case WAIT_OBJECT_0: 
			__try { 

 
				dwCount=1;
				// Get the address of the shared memory block
				lpszTmp = (char *)lpvMem+FILENAME_PTR_ADDRESS;
				while (*lpszBuf && dwCount<MAX_FNAME_LENGTH+1) 
				{
					*lpszTmp++ = *lpszBuf++; 
					dwCount++;
				}
				*lpszTmp = '\0'; 

			} 

			__finally { 
				// Release ownership of the mutex object
				if (! ReleaseMutex(ghMutex)) 
				{ 
					// Handle error.
				} 
			} 
			break; 

		// The thread got ownership of an abandoned mutex
		// The database is in an indeterminate state
		case WAIT_ABANDONED: 
                return; 
	}


    return;
} 

__declspec(dllexport) void __cdecl ResetFilePointer(void)
{ 
char *lpszTmp;
DWORD dwWaitResult;	

	dwWaitResult = WaitForSingleObject( 
            ghMutex,    // handle to mutex
            INFINITE);  // no time-out interval
 
	switch (dwWaitResult) 
    {
		// The thread got ownership of the mutex
		case WAIT_OBJECT_0: 
			__try { 

 
				// Get the address of the shared memory block
				lpszTmp = (char *)lpvMem+FP_PTR_ADDRESS;
				*(FILE **)lpszTmp=NULL;
			} 

			__finally { 
				// Release ownership of the mutex object
				if (! ReleaseMutex(ghMutex)) 
				{ 
					// Handle error.
				} 
			} 
			break; 

		// The thread got ownership of an abandoned mutex
		// The database is in an indeterminate state
		case WAIT_ABANDONED: 
                return; 
	}


    return;
} 

__declspec(dllexport) FILE * __cdecl getFilePointer(void)
{ 
char *lpszTmp;
DWORD dwWaitResult;	
FILE * fp;

	dwWaitResult = WaitForSingleObject( 
            ghMutex,    // handle to mutex
            INFINITE);  // no time-out interval
 
	switch (dwWaitResult) 
    {
		// The thread got ownership of the mutex
		case WAIT_OBJECT_0: 
			__try { 

 
				// Get the address of the shared memory block
				lpszTmp = (char *)lpvMem+FP_PTR_ADDRESS;
				fp=*(FILE **)lpszTmp;
			} 

			__finally { 
				// Release ownership of the mutex object
				if (! ReleaseMutex(ghMutex)) 
				{ 
					// Handle error.
				} 
			} 
			break; 

		// The thread got ownership of an abandoned mutex
		// The database is in an indeterminate state
		case WAIT_ABANDONED: 
                return(NULL); 
	}


    return(fp);
} 


__declspec(dllexport) int __cdecl StopRecording(void){
char *lpszTmp;
DWORD dwWaitResult;
int fwrite_finished;
unsigned long h,w;
FILE * fp;
int result;

	result=1;
    fp=NULL;
	dwWaitResult = WaitForSingleObject( 
            ghMutex,    // handle to mutex
            500);  
 
	switch (dwWaitResult) 
    {
		// The thread got ownership of the mutex
		case WAIT_OBJECT_0: 
			__try { 

				lpszTmp = (char *)lpvMem+FP_PTR_ADDRESS;
				fp=*(FILE **)lpszTmp;
				if(fp!=NULL)
				{
					lpszTmp = (char *)lpvMem+UPDATE_CB_PTR_ADDRESS;
					*(int *)lpszTmp=0;
				}
				result=0;
			} 

			__finally { 
				// Release ownership of the mutex object
				if (! ReleaseMutex(ghMutex)) 
				{ 
					// Handle error.
				} 
			} 
			break; 

		// The thread got ownership of an abandoned mutex
		// The database is in an indeterminate state
		case WAIT_ABANDONED: 
			result=2;
            break; 
		case WAIT_TIMEOUT:
			result=3;
			break;
		default:
			result=4;
	}


	if(fp==NULL || result!=0)
		return(result);

	fwrite_finished=read_i_equals_write_i();
	if(fwrite_finished<0)
		return(5);
	if(!fwrite_finished)
	{
		Sleep(100);
		fwrite_finished=read_i_equals_write_i();
		if(fwrite_finished<0)
			return(6);
	};
	Sleep(50);
	Trigger_WriteEvent();
	Sleep(50);
	fwrite_finished=read_i_equals_write_i();
	result=0;
	while(!fwrite_finished && result<10)
	{
		Sleep(100);
		fwrite_finished=read_i_equals_write_i();
		if(fwrite_finished<0)
			return(7);
		result++;
	};
	if(result>10)
		return(8);


	w=PACKET_N_DOUBLE;
	h=getWriteCnt();
	fseek(fp,0,SEEK_SET);
	write_matlab_attr(fp,"Data",h,w,MATLB_TDOUBLE,1,0,MATLB_PC);
	fseek(fp,0,SEEK_END);
	fflush(fp);
	fclose(fp);

	ResetFilePointer();

	return(0);
}


__declspec(dllexport) int __cdecl write_to_circular_buffer(double *data,int num_of_values,int channel_index) 
{ 
char *lpszTmp;
double *dp1,*dp2;
unsigned short i;
DWORD dwWaitResult;	
unsigned long PacketCnt;
int update_buffer;
int result;
double store_time;


    store_time=get_high_resolution_time_s();

	dwWaitResult = WaitForSingleObject( 
            ghMutex,    // handle to mutex
            5000);  // no time-out interval
 
	switch (dwWaitResult) 
    {
		// The thread got ownership of the mutex
		case WAIT_OBJECT_0: 
			__try { 


				// write data to the hold buffer
				result=1;
				lpszTmp = (char *)lpvMem+UPDATE_CB_PTR_ADDRESS;
				update_buffer=*(int *)lpszTmp;
				if(update_buffer)
				{
					result=2;
					if(channel_index>=PACKET_N_DOUBLE)
						channel_index=PACKET_N_DOUBLE-1;
					if(channel_index+num_of_values>PACKET_N_DOUBLE)
						num_of_values=PACKET_N_DOUBLE-channel_index;

					//** subtract the start_time
					lpszTmp = (char *)lpvMem+START_TIME_PTR_ADDRESS;
					store_time-=*(double *)lpszTmp;



					lpszTmp = (char *)lpvMem+HOLDMEM_PTR_ADDRESS;
					dp1=(double *)(lpszTmp+channel_index*ITEM_BSIZE);
					dp2=data;
					for(i=0;i<num_of_values;i++) 
						*dp1++ = *dp2++;

					//** write the store time in the last channel
					dp1=(double *)(lpszTmp+(PACKET_N_DOUBLE-1)*ITEM_BSIZE);
					*dp1=store_time;

					// write the hold buffer to the circular buffer
					setNextDataPacket((double *)lpszTmp);
	
					/** increment the packet count  */
					lpszTmp = (char *)lpvMem+PACKET_CNT_PTR_ADDRESS;
					PacketCnt=*(unsigned long *)lpszTmp;
					PacketCnt+=1;
					*(unsigned long *)lpszTmp=PacketCnt;
					PacketCnt=*(unsigned long *)lpszTmp;
					result=3;
					if((PacketCnt % 100)==0)
					{
						Trigger_WriteEvent();
					};
				};  //end if(update_buffer)
			} 

			__finally { 
				// Release ownership of the mutex object
				if (! ReleaseMutex(ghMutex)) 
				{ 
					// Handle error.
					lpszTmp = (char *)lpvMem+ERROR_PTR_ADDRESS;
					if(*(int *)lpszTmp==0)
						*(int *)lpszTmp=1;

				} 
			} 
			break; 

		// The thread got ownership of an abandoned mutex
		// The database is in an indeterminate state
		case WAIT_ABANDONED: 
			lpszTmp = (char *)lpvMem+ERROR_PTR_ADDRESS;
			if(*(int *)lpszTmp==0)
				*(int *)lpszTmp=2;
			result=4;
			break;
		case WAIT_TIMEOUT:
			result=5;
			lpszTmp = (char *)lpvMem+ERROR_PTR_ADDRESS;
			if(*(int *)lpszTmp==0)
				*(int *)lpszTmp=3;
			break;
		default:
			lpszTmp = (char *)lpvMem+ERROR_PTR_ADDRESS;
			if(*(int *)lpszTmp==0)
				*(int *)lpszTmp=4;
			result=6;

	}


return(result);
}




#ifdef __cplusplus
}
#endif


DWORD WINAPI ThreadProc(LPVOID lpParam) 
{
int thread_running=1;
char *lpszTmp;
int read_i,write_i,SHM_Buffer_NumOfPackets;
double darray1[PACKET_N_DOUBLE];
unsigned long WriteCnt,wcnt;
FILE *fp;

    // lpParam not used in this example.
UNREFERENCED_PARAMETER(lpParam);

DWORD dwWaitResult;
DWORD dwWaitResultMutex;

    //printf("Thread %d waiting for write event...\n", GetCurrentThreadId());
    while(1)
	{
		dwWaitResult = WaitForSingleObject( 
		    ghWriteEvent, // event handle
		    INFINITE);    // indefinite wait

		switch (dwWaitResult) 
		{
			// Event object was signaled
			case WAIT_OBJECT_0: 



				fp=NULL;
				read_i=-1;
				write_i=-1;


				dwWaitResultMutex = WaitForSingleObject( 
						ghMutex,    // handle to mutex
						INFINITE);  // no time-out interval
 
				switch (dwWaitResultMutex) 
				{
					// The thread got ownership of the mutex
					case WAIT_OBJECT_0: 
						__try { 


							// see whether thread is going to continue:
							lpszTmp = (char *)lpvMem+THREADRUNNING_PTR_ADDRESS; 
							thread_running=*(int *)lpszTmp;
					
							//printf("Thread %d reading from buffer\n", 
							//	GetCurrentThreadId());

							lpszTmp = (char *)lpvMem+FP_PTR_ADDRESS;
							fp=*(FILE **)lpszTmp;
							SHM_Buffer_NumOfPackets=getNumOfPacketsInBuffer();

							// read both read and write pointers 
							lpszTmp = (char *)lpvMem+WRITE_PTR_ADDRESS; 
							write_i=*(int *)lpszTmp;
							lpszTmp = (char *)lpvMem+READ_PTR_ADDRESS; 
							read_i=*(int *)lpszTmp;

						} 

						__finally { 
							// Release ownership of the mutex object
							if (! ReleaseMutex(ghMutex)) 
							{ 
								// Handle error.
							} 
						} 
						break; 

		// The thread got ownership of an abandoned mutex
		// The database is in an indeterminate state
					//case WAIT_ABANDONED: 
                 
				};   // end switch(dwWaitResultMutex)




				wcnt=0;
				if(fp!=NULL && read_i>-1 && thread_running)
				{
					while(read_i!=write_i)
					{
						read_i=(read_i+1) % SHM_Buffer_NumOfPackets;
						getIndexedDataPacket(darray1,read_i);
						fwrite(darray1,PACKET_N_DOUBLE*sizeof(double),1,fp);
						wcnt++;
					}
				}
				else
					read_i=write_i;

				if(read_i>-1 && thread_running)
				{
					dwWaitResultMutex = WaitForSingleObject( 
							ghMutex,    // handle to mutex
							INFINITE);  // no time-out interval
 
					switch (dwWaitResultMutex) 
					{
						// The thread got ownership of the mutex
						case WAIT_OBJECT_0: 
							__try { 
								// update the read pointer
								lpszTmp = (char *)lpvMem+READ_PTR_ADDRESS; 
								*(int *)lpszTmp=read_i;
								// update the written counter
								lpszTmp = (char *)lpvMem+WRITE_CNT_PTR_ADDRESS;
								WriteCnt=*(unsigned long *)lpszTmp;
								WriteCnt+=wcnt;
								*(unsigned long *)lpszTmp=WriteCnt;
							} 

							__finally { 
								// Release ownership of the mutex object
								if (! ReleaseMutex(ghMutex)) 
								{ 
									// Handle error.
								} 
							} 
							break; 

							// The thread got ownership of an abandoned mutex
							// The database is in an indeterminate state
						//case WAIT_ABANDONED: 
                 
					};   // end switch(dwWaitResultMutex)
				
				};  // end if(read_i>-1



				break; 

			// An error occurred
			//default: 
				//printf("Wait error (%d)\n", GetLastError()); 
	            //return 0; 
		};
		ResetEvent(ghWriteEvent);

		if(!thread_running)
			break;
	};  // end while(1)
    return 1;
}

BOOL ProcAttach(void)
{
    BOOL  fInit; 
	HANDLE ghThread;
	DWORD dwThreadID;
	char *lpszTmp; 
	DWORD dwCount;
	int pcnt;
	char *lpszBuf; 
	char name[300]="c:\\temp\\sharedMemdllDebug.txt";
	FILE *dfh;

		hMapObject = CreateFileMapping( 
                INVALID_HANDLE_VALUE,   // use paging file
                NULL,                   // default security attributes
                PAGE_READWRITE,         // read/write access
                0,                      // size: high 32-bits
                SHMEMSIZE,              // size: low 32-bits
                TEXT("dllmemfilemap")); // name of map object
        if (hMapObject == NULL) 
                return FALSE; 
 
        // The first process to attach initializes memory
 
        fInit = (GetLastError() != ERROR_ALREADY_EXISTS); 
 
        // Get a pointer to the file-mapped shared memory
 
        lpvMem = MapViewOfFile( 
                hMapObject,     // object to map view of
                FILE_MAP_WRITE, // read/write access
                0,              // high offset:  map from
                0,              // low offset:   beginning
                0);             // default: map entire file
        if (lpvMem == NULL) 
                return FALSE;
        if (fInit)
		{
			// Fill shared memory block with zeros
           memset(lpvMem, '\0', SHMEMSIZE); 

			// Create a mutex with no initial owner
			ghMutex = CreateMutex( 
			NULL,              // default security attributes
			FALSE,             // initially not owned
			TEXT("SharedMemMutex")  // object name
			);   
			if (ghMutex == NULL) 
			{
				printf("CreateMutex error: %d\n", GetLastError());
				return FALSE;
			};


			// Create the write event handler
			ghWriteEvent = CreateEvent( 
			NULL,               // default security attributes
			TRUE,               // manual-reset event
			FALSE,              // initial state is nonsignaled
			TEXT("WriteEvent")  // object name
			); 

			if (ghWriteEvent == NULL) 
			{ 
				printf("CreateEvent failed (%d)\n", GetLastError());
				return FALSE;
			};


			//** set the flag signaling that writing thread continues running
			lpszTmp = (char *)lpvMem+THREADRUNNING_PTR_ADDRESS; 
			*(int *)lpszTmp=1;


			//** create the writing thread:

			ghThread = CreateThread(
						NULL,              // default security
						0,                 // default stack size
						ThreadProc,        // name of the thread function
						NULL,              // no thread parameters
						0,                 // default startup flags
						&dwThreadID);		

			if (ghThread == NULL) 
			{
				printf("CreateThread failed (%d)\n", GetLastError());
				return FALSE;
			};

			//** set the Thread priority
			SetThreadPriority(
			ghThread,
			THREAD_PRIORITY_LOWEST   //THREAD_PRIORITY_TIME_CRITICAL
			);


			//** store the debug filename in the shared memory
			lpszTmp = (char *)lpvMem+DEBUG_FNM_PTR_ADDRESS; 
			lpszBuf=name;
			dwCount=1;
			while (*lpszBuf && dwCount<MAX_FNAME_LENGTH+1) 
			{
				*lpszTmp++ = *lpszBuf++; 
				dwCount++;
			}
			*lpszTmp = '\0'; 

			dfh=fopen(name,"wt+");
			fprintf(dfh,"Memory initialized\n");
			fclose(dfh);

		}
		else
		{
			ghMutex = OpenMutex( 
						MUTEX_ALL_ACCESS,            // request full access
						FALSE,                       // handle not inheritable
						TEXT("SharedMemMutex"));    // object name
			ghWriteEvent = OpenEvent( 
						EVENT_ALL_ACCESS,            // request full access
						FALSE,                       // handle not inheritable
						TEXT("WriteEvent"));    // object name
		};   //  end if (fInit)
 
		//** increment the process counter
		lpszTmp = (char *)lpvMem+PROCESS_CNT_PTR_ADDRESS; 
		pcnt=*(int *)lpszTmp;
		pcnt+=1;
		*(int *)lpszTmp=pcnt;
		
		dfh=fopen(name,"at+");
		fprintf(dfh,"Process Counter incremented\n");
		fclose(dfh);

return TRUE;
}

void ProcDetach(void)
{
FILE *dfh;
char *name;
char *lpszTmp; 
int pcnt;
BOOL fIgnore;

		name=(char *)lpvMem+DEBUG_FNM_PTR_ADDRESS;
		dfh=fopen(name,"at+");
		fprintf(dfh,"Process Counter decremented\n");
		fclose(dfh);
		//** decrement the process counter
		lpszTmp = (char *)lpvMem+PROCESS_CNT_PTR_ADDRESS; 
		pcnt=*(int *)lpszTmp;
		pcnt-=1;
		*(int *)lpszTmp=pcnt;
		if(pcnt==0)
		{
			dfh=fopen(name,"at+");
			fprintf(dfh,"Shared Memory released\n");
			fclose(dfh);
			//** reset the flag signaling that writing thread continues running
			lpszTmp = (char *)lpvMem+THREADRUNNING_PTR_ADDRESS; 
			*(int *)lpszTmp=0;

			/** terminate the thread and
			 ** and close the event
			 **/
			SetEvent(ghWriteEvent);
			Sleep(100);

			dfh=fopen(name,"at+");
			fprintf(dfh,"dbg 1\n");
			fclose(dfh);

 
		};

		CloseHandle(ghWriteEvent);
				
		/** and close the mutex
		 **/
		CloseHandle(ghMutex);

        // Unmap shared memory from the process's address space
 
        fIgnore = UnmapViewOfFile(lpvMem); 
 
        // Close the process's handle to the file-mapping object
 
        fIgnore = CloseHandle(hMapObject); 

return;
}
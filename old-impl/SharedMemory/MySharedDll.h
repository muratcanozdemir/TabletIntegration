// Low level routines not involving the MUTEX:

__declspec(dllexport) void __cdecl SetSharedMem(char *lpszBuf);
__declspec(dllexport) void __cdecl GetSharedMem(char *lpszBuf, int cchSize);
__declspec(dllexport) void __cdecl SHM_getDebugFilename(char *lpszBuf, int cchSize);
__declspec(dllexport) void __cdecl setWriteIndex(int index);
__declspec(dllexport) int __cdecl getWriteIndex(void);
__declspec(dllexport) void __cdecl setReadIndex(int index);
__declspec(dllexport) int __cdecl getReadIndex(void);
__declspec(dllexport) void __cdecl setReadIndex_to_testvalue(void);

__declspec(dllexport) void __cdecl setIndexedDataPacket(double *Packet,int index);
__declspec(dllexport) double * __cdecl getIndexedDataPacket(double *Packet,int index);
__declspec(dllexport) double * __cdecl getLastWrittenDataPacket(double *Packet);
__declspec(dllexport) int __cdecl setNextDataPacket(double *Packet);
__declspec(dllexport) double * __cdecl getIndexedDataBlock(double *Packets,int start_index,int end_index);

__declspec(dllexport) void __cdecl Trigger_WriteEvent(void);
__declspec(dllexport) int __cdecl getPacketWidth(void);
__declspec(dllexport) int __cdecl getNumOfPacketsInBuffer(void);
__declspec(dllexport) int __cdecl getPacketBSize(void);
__declspec(dllexport) int __cdecl getMutexError(void);
__declspec(dllexport) int __cdecl getProcessCnt(void);

__declspec(dllexport) double __cdecl get_high_resolution_time_s(void);


// High level routines involving the MUTEX:
__declspec(dllexport) int __cdecl read_i_equals_write_i(void);
__declspec(dllexport) FILE * __cdecl InitializeRecording(char *filename);
__declspec(dllexport) unsigned long __cdecl getPacketCnt(void);
__declspec(dllexport) int __cdecl SHM_getWriteIndex(void) ;
__declspec(dllexport) unsigned long __cdecl getWriteCnt(void);
__declspec(dllexport) void __cdecl SHM_getFilename(char *lpszBuf, int cchSize);
__declspec(dllexport) void __cdecl SHM_setFilename(char *lpszBuf);
__declspec(dllexport) void __cdecl ResetFilePointer(void);
__declspec(dllexport) FILE * __cdecl getFilePointer(void);
__declspec(dllexport) int __cdecl StopRecording(void);
__declspec(dllexport) int __cdecl write_to_circular_buffer(double *data,int num_of_values,int channel_index);

/*------------------------------------------------------------------------------
WacomRecord - a simple WinTab program: test of spec. addenda
RICO 10/17/91
------------------------------------------------------------------------------*/
#define USE_MySharedDLL
#define USE_MySharedDLL_Recording
#define WRITE_ON_FILE
#undef MATLAB_ENGINE_CALL
#define DO_FORK

#define MOUSE_OR_WACOM	1	// 0: use mouse signals instead of WACOM signals
							// 1: use WACOM signals



#include <string.h>
#include <windows.h>
#if MOUSE_OR_WACOM==1
	#include "msgpack.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <direct.h>

#if MOUSE_OR_WACOM==1
	#include "wintab.h"

	#define USE_X_LIB
	#ifdef USE_X_LIB
		#include "wintabx.h"
		#include <windowsx.h>
	#endif

	#define PACKETDATA	(PK_X | PK_Y | PK_BUTTONS | PK_TIME | PK_NORMAL_PRESSURE)
		#define PACKETMODE	0
//		#define PACKETMODE	PK_BUTTONS

#include "pktdef.h"

#endif


#include <sys/timeb.h>
#include <time.h>


#include <windowsx.h>


#include <conio.h>






#include "resource.h"
#include "WacomRecord.h"

#ifndef XXXXXXX
#include "std_lib.h"
#include "str_lib.h"
#include "matlb.h"
#else
#include "compile.h"
#endif




#ifdef USE_MySharedDLL
	#include "MySharedDll.h"
#ifdef MATLAB_ENGINE_CALL
	#include <engine.h>
#endif
#else
	#undef USE_MySharedDLL_Recording
#endif


#if MOUSE_OR_WACOM==1
/* -------------------------------------------------------------------------- */
#define Inch2Cm	CASTFIX32(2.54)
#define Cm2Inch	CASTFIX32(1.0/2.54)
/* -------------------------------------------------------------------------- */

static LOGCONTEXT lcMine;
static FIX32 scale[2];
static HCTX hTab = NULL;
static DWORD bmap=(TBN_UP << 2);

#define RES_LEFT_HOR	1024
#define RES_LEFT_VER	 768

#define RES_RIGHT_HOR	1680.0
#define RES_RIGHT_VER	1050.0

#define TABLET_HEIGHT_CM	32.3440
#define TABLET_WIDTH_CM		43.4080
/** this calibration holds if the window AND the WacomTablet belongs to the left screen **/
void calibrate_packet(PACKET *pkt)
{
	pkt->pkX=(LONG)((double)pkt->pkX *(RES_LEFT_HOR+RES_RIGHT_HOR)/RES_LEFT_HOR)*30.0/30.4;
	pkt->pkY=(LONG)(((double)pkt->pkY +TABLET_HEIGHT_CM/RES_RIGHT_VER*1000.0*(RES_LEFT_VER-RES_RIGHT_VER)) *RES_RIGHT_VER/RES_LEFT_VER);
}

HCTX static NEAR TabletInit(HWND hWnd, FIX32 scale[])
{

	/* get default region */
	WTInfo(WTI_DEFCONTEXT, 0, &lcMine);

	/* modify the digitizing region */
	strcpy(lcMine.lcName, "WacomRecord Digitizing");
	lcMine.lcOptions |= CXO_MESSAGES;
	lcMine.lcPktData = PACKETDATA;
	lcMine.lcPktMode = PACKETMODE;
	lcMine.lcMoveMask = PACKETDATA;
	lcMine.lcBtnUpMask = lcMine.lcBtnDnMask;

	/* output in 1000ths of cm */
	lcMine.lcOutOrgX = lcMine.lcOutOrgY = 0;
	lcMine.lcOutExtX = INT(scale[0] * lcMine.lcInExtX);
	lcMine.lcOutExtY = INT(scale[1] * lcMine.lcInExtY);

	/* open the region */
	return WTOpen(hWnd, &lcMine, TRUE);

}
/* -------------------------------------------------------------------------- */
/* return scaling factors in thousandths of cm per axis unit */
static void TabletScaling(FIX32 scale[])
{
	AXIS aXY[2];
	int i;
	UINT wDevice;




	/* get the data */
	WTInfo(WTI_DEFCONTEXT, CTX_DEVICE, &wDevice);
	WTInfo(WTI_DEVICES+wDevice, DVC_X, &aXY[0]);
	WTInfo(WTI_DEVICES+wDevice, DVC_Y, &aXY[1]);

	/*
	FILE *fddbg;
	fddbg=fopen("C:\\Temp\\SharedMem_dbg.txt","wt");
	fprintf(fddbg,"aXY[0].axResolution=%d; aXY[1].axResolution=%d\n",aXY[0].axResolution,aXY[1].axResolution);
	fclose(fddbg);
	*/

	/* calculate the scaling factors */
	for (i = 0; i < 2; i++) {
		FIX_DIV(scale[i], CASTFIX32(1000), aXY[i].axResolution);
		if (aXY[i].axUnits == TU_INCHES) {
			FIX_MUL(scale[i], scale[i], Inch2Cm);
		}
	}
}
/* -------------------------------------------------------------------------- */

#else
#define RES_LEFT_HOR	1022
#define RES_LEFT_VER	 766
#define TABLET_WIDTH_CM		43.4080  // WACOM
#define TABLET_HEIGHT_CM	32.3440

void calibrate_mouse(LPARAM *lParam)
{
unsigned short mouse_x,mouse_y;
	mouse_x=(unsigned short)(TABLET_WIDTH_CM*1000.0/RES_LEFT_HOR*LOWORD(*lParam));
	mouse_y=(unsigned short)(TABLET_HEIGHT_CM*1000.0-TABLET_HEIGHT_CM*1000.0/RES_LEFT_VER*HIWORD(*lParam));
	*lParam=(((DWORD)mouse_y)<<16)+mouse_x;
}
#endif //  MOUSE_OR_WACOM==1

char _szAppName[] = "WacomRecord";
HINSTANCE __hInstance = NULL;  /* Our instance handle */
HWND hWnd_;                    /* main window handle */
char _CmdLine[500];

static char mat_basedir_name[2][255]={"D:\\progs\\Matlab\\TabletRecord\\Matlab_StateMachine",
									  "D:\\progs\\Matlab\\StateMachine"};

static char dat_basedir_name[255]="D:\\Data\\Exp1";

static char filename_without_path[25]="WaRecord.mat";
static char filename[255]="";

static char datadir_mat_filename_without_path[25]="DataDirName.mat";
static char datadir_mat_filename[255]="";
static char subdirname[100]="dummy";

static char prog_m_filename_without_path[25]="Paradigm_Name.m";
static char prog_m_filename[255]="";

#define NumOfProgs	3
static char progname[NumOfProgs][255]={"Exp1",
                                       "Exp2",
									   "Exp3"};

static int ProgIndex=0;



static unsigned long LastWacomTimestamp=0;
#define MIN_WACOM_STORE_INTERVAL_MS   10

LRESULT FAR PASCAL RuleAppWndProc(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);
BOOL NEAR PASCAL RegisterAppWndClass (HINSTANCE hInstance);




/* -------------------------------------------------------------------------- */
int PASCAL WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
					LPSTR lpszCmdLine, int nCmdShow)
{
	MSG msg;

	int i;
	char dirbuffer[500];

	__hInstance = hInstance;


	//------ detect the actual matlab run directory:  -----
	_getcwd(dirbuffer,500);
	i=-1;
	while(i<2)
	{
		i+=1;
		if(i==2)
			break;
		if(!_chdir(mat_basedir_name[i]))
			break;
	};
	if(i>1)
		return(0);
	if(i>0)
		strcpy(mat_basedir_name[0],mat_basedir_name[i]);
	_chdir(dirbuffer);
	//--------------------------------------------------

	strcpy(_CmdLine,lpszCmdLine);
	strcpy(filename,dat_basedir_name);
	strcat(filename,"\\");
	strcat(filename,subdirname);
	strcat(filename,"\\");
	strcat(filename,progname[0]);
	strcat(filename,".mat");

	strcpy(prog_m_filename,mat_basedir_name[0]);
	strcat(prog_m_filename,"\\simulate_EyeSeeParadigms\\");
	strcat(prog_m_filename,prog_m_filename_without_path);

	strcpy(datadir_mat_filename,mat_basedir_name[0]);
	strcat(datadir_mat_filename,"\\simulate_EyeSeeParadigms\\");
	strcat(datadir_mat_filename,datadir_mat_filename_without_path);

#ifdef COPY_DATA_TO_NETWORK_SERVER
	strcpy(message,lpszCmdLine);
#endif

	if (hPrevInstance == NULL){
		if (!RegisterAppWndClass(hInstance))
			return(0);
#ifdef USE_GL_WINDOW
		if (!RegisterOGLWndClass(hInstance,RuleAppWndProc))
			return(0);
#endif
	}

	hWnd_ = CreateDialog( hInstance, _szAppName, NULL, NULL);
	/****
	hWnd_ = CreateWindowEx(
    0,                      // no extended styles
    _szAppName,           // class name
    "Main Window",          // window name
    WS_OVERLAPPEDWINDOW |   // overlapped window
             WS_HSCROLL |   // horizontal scroll bar
             WS_VSCROLL,    // vertical scroll bar
    CW_USEDEFAULT,          // default horizontal position
    CW_USEDEFAULT,          // default vertical position
    CW_USEDEFAULT,          // default width
    CW_USEDEFAULT,          // default height
    (HWND) NULL,            // no parent or owner window
    (HMENU) NULL,           // class menu used
    hInstance,              // instance handle
    NULL);                  // no window creation data
 *****/



	if (hWnd_ == NULL)
		return(0);

	ShowWindow(hWnd_, nCmdShow);

	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}



	return(0);
}
/* -------------------------------------------------------------------------- */
BOOL NEAR APIENTRY RegisterAppWndClass (HINSTANCE hInstance)
{
	WNDCLASS WndClass;

	WndClass.style			= 0;
	WndClass.lpfnWndProc	= RuleAppWndProc;
	WndClass.cbClsExtra	 	= 0;
	WndClass.cbWndExtra	 	= DLGWINDOWEXTRA;
	WndClass.hInstance	  	= hInstance;
	WndClass.hIcon			= LoadIcon(hInstance, _szAppName);
	WndClass.hCursor		= LoadCursor(NULL, IDC_ARROW);
	WndClass.hbrBackground 	= (HBRUSH)(COLOR_WINDOW-4); /*(HBRUSH)(COLOR_WINDOW + 1);*/
	WndClass.lpszMenuName  	= NULL;
	WndClass.lpszClassName 	= _szAppName;
	return(RegisterClass(&WndClass));
}
/* -------------------------------------------------------------------------- */


int create_sharedMemory(void);
int destroy_sharedMemmory(void);

DWORD nsqrt(DWORD x)
{
	/* integer square root via Newton's method. */
	DWORD guess, oguess;

	if (x <= 1)
		return x;

	guess = 1;
	do
	{
		oguess = guess;
		guess = (guess + x/guess)/2;
	}
	while (labs(guess - oguess) > 1);

	if (guess == oguess)
		guess++;

	if (labs((guess * guess) - x) > labs((oguess * oguess) - x))
		guess = oguess;

	return guess;
}

#ifndef USE_MySharedDLL_Recording
#define MAX_REC_N 50
#endif

#define N_ITEMS 7
static double darray[N_ITEMS];
static double start_time;


void retreive_subdirname(char *subdirname,int MaxN)
{
	FILE *fp;
	int i;
	long pstart;
	long pend;
	long w;
	int vartyp;
	int is_string;
	S_PTR *strarr=NULL;
	char *p;

	fp=fopen(datadir_mat_filename,"rb");
	get_matlab_attr(fp,"DataDirname",&pstart,&pend,&w,&vartyp,&is_string);
	fseek(fp,pstart-12-sizeof(mathead),SEEK_SET);
	readmatlabstrarr(fp,&strarr);

	fclose(fp);
	i=0;
	p=subdirname;
	while((i<MaxN-1) && (strarr->sa[i]!=(char)0))
		*(p++)=strarr->sa[i++];
	*p=(char)0;
	delstree(&strarr);
	return;
}

void write_paradigm_name_File(char *ParadigmName)
{
FILE *tfp;
char strbuf[500];
	strcpy(strbuf,"ParadigmName='");
	strcat(strbuf,ParadigmName);
	strcat(strbuf,"';");

	tfp=fopen(prog_m_filename,"wt+");
	fseek(tfp,0,SEEK_END);
	fprintf(tfp,"%s\n\n",strbuf);
	fflush(tfp);
	fclose(tfp);
	return;
}

void start_recording(FILE **fp,unsigned long *PacketCount){
	*PacketCount=0;
	if(*fp!=NULL)
		return;
#ifdef WRITE_ON_FILE
#ifdef DO_FORK
	retreive_subdirname(filename,255);
#else
	strcpy(filename,dat_basedir_name);
	strcat(filename,"\\");
	strcat(filename,subdirname);
#endif
	strcat(filename,"\\");
	strcat(filename,progname[ProgIndex]);
	strcat(filename,".mat");

	strcpy(filename,modify_file_name(filename,0,"mat",""));
	if(strlen(fext(filename))==0)
		strcat(filename,"mat");
	SHM_setFilename(filename);

#ifndef USE_MySharedDLL_Recording
	*fp=fopen(filename,"wb+");
	fseek(*fp,0,SEEK_SET);



	write_matlab_attr(*fp,"Data",0,0,MATLB_TDOUBLE,1,0,MATLB_PC);
#else
	*fp=InitializeRecording(filename);
#endif
#endif

#ifdef DO_FORK
	write_paradigm_name_File(progname[ProgIndex]);
#endif
	return;
}

#if MOUSE_OR_WACOM==1
void set_darray_from_WacomPacket(double *dest_array,int *num_of_values,int *channel_index,void *PacketPtr)
{
PACKET *pkt;

	pkt=(PACKET *)PacketPtr;
	if(pkt!=NULL)
	{
		dest_array[0]=(double)(pkt->pkX)/1000.0-TABLET_WIDTH_CM/2.0;    // cm
		dest_array[1]=(double)(pkt->pkY)/1000.0-TABLET_HEIGHT_CM/2.0;    // cm
		dest_array[2]=(double)(pkt->pkNormalPressure);
		dest_array[3]=(double)(HIWORD(pkt->pkButtons));
		dest_array[4]=(double)(LOWORD(pkt->pkButtons));
		dest_array[5]=(double)pkt->pkTime;
	};
	*num_of_values=6;
	*channel_index=0;
	return;
}


#else

void set_darray_from_mouse(double *dest_array,int *num_of_values,int *channel_index,void *mouseLParams)
{
long *mouse_position;

	mouse_position=(long *)mouseLParams;
	if(mouse_position!=NULL)
	{
		dest_array[0]=(double)(LOWORD(*mouse_position))/1000.0-TABLET_WIDTH_CM/2.0;
		dest_array[1]=(double)(HIWORD(*mouse_position))/1000.0-TABLET_HEIGHT_CM/2.0;
	};
	*num_of_values=2;
	*channel_index=0;
	return;
}
#endif


int retrieve_data_from_packet(FILE **fp,void *src_ptr,unsigned long *PacketCnt){
#ifndef USE_MySharedDLL_Recording
int SHM_Buffer_NumOfPackets,read_i;
double darray1[N_ITEMS];
unsigned short i;
#else
int write_error;
#endif
#ifdef USE_GL_WINDOW
STATEMACHINE_MESSAGE_BUF msg;
#endif
int num_of_values,channel_index;


#if MOUSE_OR_WACOM==1
	set_darray_from_WacomPacket(darray,&num_of_values,&channel_index,src_ptr);
#else
	set_darray_from_mouse(darray,&num_of_values,&channel_index,src_ptr);
#endif

#ifdef USE_MySharedDLL_Recording
	if(((PACKET *)src_ptr)->pkTime>LastWacomTimestamp+MIN_WACOM_STORE_INTERVAL_MS)
	{
		write_error=write_to_circular_buffer(darray,num_of_values,channel_index);
		if(write_error!=3 && write_error!=1)
			return(0);
		LastWacomTimestamp=((PACKET *)src_ptr)->pkTime;
	};
#else
	setNextDataPacket(darray);
#endif

#ifdef USE_GL_WINDOW
	msg.code=ST_M_PENPOS;
	msg.n=ST_M_PENPOS_N;
	msg.data[0]=darray[4]*1000.0;  // x [cm]  *1000
	msg.data[1]=darray[5]*1000.0;  // x [cm]  *1000
	send_statemachine_event(&msg);
#endif
#ifdef COPY_DATA_TO_NETWORK_SERVER
	if(clnt!=NULL){
		set_shortintarray_1_arg.value[0]=(short int)(((double)(pkt->pkX)-lcMine.lcOutExtX/2.0)*80.0/1000.0)-40;
		set_shortintarray_1_arg.value[1]=(short int)(((double)(pkt->pkY)-lcMine.lcOutExtY/2.0)*80.0/1000.0)+5;
		set_shortintarray_1_arg.value[2]=(short int)(pkt->pkNormalPressure);
		set_shortintarray_1_arg.valid_count=3;
		result_1 = set_shortintarray_1(&set_shortintarray_1_arg, clnt);
		if (result_1 == NULL) {
			clnt_perror(clnt, "call to set_shortintarray_1_arg failed:");
			return(0);
		};
	};
#endif

	if(*fp!=NULL)
		(*PacketCnt)++;
	Sleep(1);
#ifndef USE_MySharedDLL_Recording
#ifdef WRITE_ON_FILE
	if(((*PacketCnt) % MAX_REC_N) ==0){
		SHM_Buffer_NumOfPackets=getNumOfPacketsInBuffer();
		read_i=getWriteIndex();
		read_i+=SHM_Buffer_NumOfPackets-MAX_REC_N;
		read_i=read_i % SHM_Buffer_NumOfPackets;
		for(i=0;i<MAX_REC_N;i++){
			read_i=(read_i+1) % SHM_Buffer_NumOfPackets;
			getIndexedDataPacket(darray1,read_i);
			fwrite(darray1,N_ITEMS*sizeof(double),1,*fp);
		};
	}
#endif
#endif
	return(1);
}

void stop_recording(FILE **fp,unsigned long *PacketCnt){
unsigned long PacketCount,h,w;
unsigned short i;
#ifndef USE_MySharedDLL_Recording
double darray1[N_ITEMS];
int SHM_Buffer_NumOfPackets,read_i;
#endif
FILE *tfp;


#ifndef USE_MySharedDLL_Recording
	if(*fp==NULL)
		return;
	PacketCount=(*PacketCnt) % MAX_REC_N;
	if(PacketCount>0){


		SHM_Buffer_NumOfPackets=getNumOfPacketsInBuffer();
		read_i=getWriteIndex();
		read_i+=SHM_Buffer_NumOfPackets-PacketCount;
		read_i=read_i % SHM_Buffer_NumOfPackets;
		for(i=0;i<PacketCount;i++){
			read_i=(read_i+1) % SHM_Buffer_NumOfPackets;
			getIndexedDataPacket(darray1,read_i);
			fwrite(darray1,N_ITEMS*sizeof(double),1,*fp);
		};
	};

	h=*PacketCnt;
	w=N_ITEMS;

	fseek(*fp,0,SEEK_SET);
	write_matlab_attr(*fp,"Data",h,w,MATLB_TDOUBLE,1,0,MATLB_PC);
	fseek(*fp,0,SEEK_END);
	fflush(*fp);
	fclose(*fp);
#else
    *fp=getFilePointer();
	if(*fp==NULL)
		return;
	StopRecording();
	h=getWriteCnt();
#endif
	*fp=NULL;

	tfp=fopen("c:\\temp\\WaRecord.txt","wt+");
	fseek(tfp,0,SEEK_END);
	fprintf(tfp,"%s: load count: %u\n",filename,getPacketCnt());
	fprintf(tfp,"%s: write count: %u\n",filename,h);
	fprintf(tfp,"Windows Packed Count: %u\n",*PacketCnt);
	i=getMutexError();
	fprintf(tfp,"Mutex error: %d\n",i);
	fclose(tfp);


	return;
}



static unsigned long PacketCount=0;
static unsigned long PenDownCount=0;
static unsigned long PenUpCount=0;
static unsigned long TimerState=0;

static FILE *fp=NULL;


#define TIMER_EVENT		0x0001
static UINT myTimer=0;


static unsigned short mouse_x,mouse_y;
static int inMode = ID_CLICK;

#ifdef MATLAB_ENGINE_CALL
static Engine *ep;
#endif

#ifdef DO_FORK
STARTUPINFO si;
PROCESS_INFORMATION pi;
#endif

void exit_procedure(HWND hWnd){
	stop_recording(&fp,&PacketCount);

#if MOUSE_OR_WACOM==1
	if (hTab)
		WTClose(hTab);
	hTab=NULL;
#endif

	if(myTimer)
		KillTimer(hWnd,myTimer);

#ifndef USE_MySharedDLL
	destroy_sharedMemmory();
#endif

#ifdef MATLAB_ENGINE_CALL
	engClose(ep);
#endif

#ifdef DO_FORK
#ifdef DO_FORK
	write_paradigm_name_File("exit");
#endif
	Sleep(200);

	CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );
#endif

	myTimer=0;
	PostQuitMessage(0);
	return;
}




LRESULT FAR APIENTRY RuleAppWndProc (HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{




#if MOUSE_OR_WACOM==1
	DWORD bit;
	PACKET pkt;
#endif

	PAINTSTRUCT psPaint;
	HDC hDC;
	LONG delta[3];	/* horz/vert/diag */
	unsigned long i;

	char buf[255];







	switch (wMsg) {

		case WM_CREATE:


#if MOUSE_OR_WACOM==1
			TabletScaling(scale);
			hTab = TabletInit(hWnd, scale);
			WTEnable(hTab, FALSE);
#endif

			myTimer=SetTimer(hWnd,TIMER_EVENT,40,NULL);
#ifdef USE_MySharedDLL
			SetSharedMem("XvC");
			i=getNumOfPacketsInBuffer();
			setWriteIndex(i-1);
#else
			if(create_sharedMemory())
				exit_procedure(hWnd);
#endif

			SetFocus(hWnd);
#ifdef DO_FORK

			strcpy(buf,"matlab -r run_eyesee('");
			strcat(buf,progname[ProgIndex]);
			strcat(buf,"')");
			// Start the child process.
			if( !CreateProcess( NULL,   // No module name (use command line)
							"matlab -nodesktop -r run_eyesee",        // Command line
							NULL,           // Process handle not inheritable
							NULL,           // Thread handle not inheritable
							FALSE,          // Set handle inheritance to FALSE
							0,              // No creation flags
							NULL,           // Use parent's environment block
							NULL,           // Use parent's starting directory
							&si,            // Pointer to STARTUPINFO structure
							&pi )           // Pointer to PROCESS_INFORMATION structure
			)
			{
				printf( "CreateProcess failed (%d).\n", GetLastError() );
				return (LRESULT)0;
			};
#endif

#ifdef MATLAB_ENGINE_CALL
			if (!(ep = engOpen(NULL)))
			{
				MessageBox ((HWND)NULL, (LPSTR)"Can't start MATLAB engine",
							(LPSTR) "Engwindemo.c", MB_OK);
				exit_procedure(hWnd);
			};
			engEvalString(ep, "cd D:\\progs\\SharedMemory");
			engEvalString(ep, "run_gap");
#endif
			break;


		case WM_PAINT:
			hDC = BeginPaint(hWnd, &psPaint);
			ShowWindow(GetDlgItem(hWnd, ID_CLICK), inMode == ID_CLICK);
			ShowWindow(GetDlgItem(hWnd, ID_PRESS), inMode == ID_PRESS);
			ShowWindow(GetDlgItem(hWnd, ID_RELEASE), inMode == ID_RELEASE);
			ShowWindow(GetDlgItem(hWnd, IDC_CHECK2), 1);


			//SetBkColor(hDC,0x00404040); /*  0x00bbggrr */
			if (inMode == ID_CLICK || 1) {
				/*
				unsigned long DialogBaseUnits;
				DialogBaseUnits=GetDialogBaseUnits();
				*/
				delta[0] = labs(mouse_x);
				delta[1] = labs(mouse_y);
				delta[2] = nsqrt(delta[0] * delta[0] + delta[1] * delta[1]);

				for (i = 0; i < 3; i++) {	 	/* direction */

					/* print result in cm */
					wsprintf(buf, "%d.%3.3d", (UINT)delta[i]/1000,
						(UINT)delta[i]%1000);
					SetWindowText(GetDlgItem(hWnd, ID_HC + i), buf);

					/* convert to inches */
					delta[i] = (INT)(delta[i]/2.54);

					/* print result in inches */
					wsprintf(buf, "%d.%3.3d", (UINT)delta[i]/1000,
						(UINT)delta[i]%1000);
					SetWindowText(GetDlgItem(hWnd, ID_HI + i), buf);

				}
				/* print Packet Count */
				wsprintf(buf, "%d", (UINT)PacketCount);
				SetWindowText(GetDlgItem(hWnd, ID_PACKETCOUNT), buf);
				/* print PenUp Count */
				wsprintf(buf, "%d", (UINT)PenUpCount);
//				wsprintf(buf, "%d", (UINT)LOWORD(DialogBaseUnits));

				SetWindowText(GetDlgItem(hWnd, ID_PENUPCOUNT), buf);
				/* print PenDOWN Count */
				wsprintf(buf, "%d", (UINT)PenUpCount);
//				wsprintf(buf, "%d", (UINT)HIWORD(DialogBaseUnits));
				SetWindowText(GetDlgItem(hWnd, ID_PENDOWNCOUNT), buf);
				/* print Filename */
				if(inMode==ID_CLICK)
					wsprintf(buf, "%s", " ");
				else
					wsprintf(buf, "%s",filename);

				SetWindowText(GetDlgItem(hWnd, ID_FILENAME), buf);
			}
			EndPaint(hWnd, &psPaint);
			break;


		case WM_ERASEBKGND:
			if(1)
			{
			RECT rcClient;

			hDC = (HDC)wParam;
			if(SetBkColor(hDC,RGB(128,128,128))!=CLR_INVALID) //  0x00bbggrr
				SetBkMode(hDC,OPAQUE);
			GetClientRect(hWnd, &rcClient);
			FillRect(hDC, &rcClient, (HBRUSH) (COLOR_WINDOW+2));
			return (LRESULT)1;
			}
			break;

		case WM_TIMER:
			switch(wParam){
				case TIMER_EVENT:
					TimerState++;
					wsprintf(buf, "%s", progname[ProgIndex]);
					SetWindowText(GetDlgItem(hWnd, IDC_CHECK2), buf);
					InvalidateRect(hWnd, NULL, TRUE);
					if(myTimer)
						KillTimer(hWnd,myTimer);
					myTimer=0;

					/**
					myTimer=SetTimer(hWnd,TIMER_EVENT,1000,NULL);
					Trigger_WriteEvent();
					**/
					break;
			};
			break;
		case WM_DESTROY:
			exit_procedure(hWnd);
			break;
			/**
		case WM_KEYDOWN:
			if (GetAsyncKeyState(VK_ESCAPE))
			{
				exit_procedure(hWnd);
				exit(0);
			}
			break;
		case WM_KEYUP:
			break;
		**/
		case WM_CHAR:
			if (LOWORD(wParam)==(UINT)'S')
			{
				exit_procedure(hWnd);
				exit(0);
			}
			break;


#if MOUSE_OR_WACOM==1
		case WT_PACKET:
			if (WTPacket((HCTX)lParam, wParam, &pkt)) {
				calibrate_packet(&pkt);
				bit = (1 << LOWORD(pkt.pkButtons));
				if(bit==TBN_DOWN)
					bit=bit;

				bmap &= ~0x00000003;  /** last value is stored in 0x000C **/
				if(((bmap & 0x0000000C) & (bit << 2))==0){  /** change occured **/
					if(bit & TBN_DOWN)
						bmap|=(TBN_DOWN);
					else
						bmap|=TBN_UP;
				};
				bmap&=~0x0000000C;
				bmap|=(bit <<2);


				if(bmap & TBN_DOWN){
					if(inMode!=ID_CLICK){
						PenDownCount++;
						InvalidateRect(hWnd, NULL, TRUE);
					};
					if (inMode == ID_PRESS) {
						inMode = ID_RELEASE;
					};

				};


				//if(inMode==ID_RELEASE){
					if(!retrieve_data_from_packet(&fp,&pkt,&PacketCount)){
						exit_procedure(hWnd);
						exit(0);
					};
				//};

				if ((inMode == ID_RELEASE) && (bmap & TBN_UP)) {

					PenUpCount++;
					InvalidateRect(hWnd, NULL, TRUE);
				};
					mouse_x = pkt.pkX;
					mouse_y = pkt.pkY;

			};  /** if Packet retrieved **/
			break;
		case WM_MOUSEMOVE:
			SetFocus(hWnd_);
			break;
#else
		case WM_MOUSEMOVE:
			calibrate_mouse(&lParam);
			mouse_x=LOWORD(lParam);
			mouse_y=HIWORD(lParam);
			InvalidateRect(hWnd, NULL, TRUE);
			if(!retrieve_data_from_packet(&fp,&lParam,&PacketCount))
			{
				exit_procedure(hWnd);
				exit(0);
			};
			SetFocus(hWnd_);
			break;
#endif

		case WM_ACTIVATE:
			break;

		case WM_COMMAND:
			if((IDC_CHECK1 ==LOWORD(wParam)) &&         // item, control, or accelerator identifier
			   (GetDlgItem(hWnd, IDC_CHECK1)==(HWND) lParam) &&      // handle of control
			   (BN_CLICKED==HIWORD(wParam)) ){
					char buf[20];

				if(Button_GetCheck((HWND) lParam)){

					if (inMode == ID_CLICK) {
#if MOUSE_OR_WACOM==1
						inMode = ID_RELEASE; //ID_PRESS;
						WTEnable(hTab, TRUE);
						WTOverlap(hTab, TRUE);
#else
						inMode = ID_RELEASE;
#endif
						PenDownCount=0;
						PenUpCount=0;
						start_recording(&fp,&PacketCount);  // <- alternate between release and click here
						wsprintf(buf, "%s", "STOP");
					}
					else{
						wsprintf(buf, "%s", "START");
						Button_SetCheck((HWND) lParam,0);
					};
				}
				else{
					if (inMode == ID_RELEASE) {
						inMode = ID_CLICK;
#if MOUSE_OR_WACOM==1
						WTEnable(hTab, FALSE);
#endif

						stop_recording(&fp,&PacketCount);  // <- alternate between release and click here

						wsprintf(buf, "%s", "START");
					}
					else{
						wsprintf(buf, "%s", "STOP");
						Button_SetCheck((HWND) lParam,0);
					};
				};
				InvalidateRect(hWnd, NULL, TRUE);
				SetWindowText((HWND) lParam, buf);

			}   // notification code IDC_CHECK1
			else if((IDC_CHECK2 ==LOWORD(wParam)) &&         // item, control, or accelerator identifier
			   (GetDlgItem(hWnd, IDC_CHECK2)==(HWND) lParam) &&      // handle of control
			   (BN_CLICKED==HIWORD(wParam)))
			{
				ProgIndex=(ProgIndex+1) % NumOfProgs;
				wsprintf(buf, "%s", progname[ProgIndex]);
				InvalidateRect(hWnd, NULL, TRUE);
				SetWindowText((HWND) lParam, buf);



			}   // notification code IDC_CHECK2
			else if((IDC_CHECK3 ==LOWORD(wParam)) &&         // item, control, or accelerator identifier
			   (GetDlgItem(hWnd, IDC_CHECK3)==(HWND) lParam) &&      // handle of control
			   (BN_CLICKED==HIWORD(wParam)))
			{
					write_paradigm_name_File("SetDataDirectory");


			};
			break;

		default:

			return DefWindowProc(hWnd, wMsg, wParam, lParam);
	}
	return (LRESULT)0;
}
/* -------------------------------------------------------------------------- */



#define BUF_SIZE 256
char szName[]=TEXT("Global\\MyFileMappingObject");
char szMsg[]=TEXT("Message from first process.");

HANDLE hMapFile;
LPCTSTR pBuf;

int create_sharedMemory(void)
{

   hMapFile = CreateFileMapping(
                 INVALID_HANDLE_VALUE,    // use paging file
                 NULL,                    // default security
                 PAGE_READWRITE,          // read/write access
                 0,                       // maximum object size (high-order DWORD)
                 BUF_SIZE,                // maximum object size (low-order DWORD)
                 szName);                 // name of mapping object

   if (hMapFile == NULL)
   {
      printf(TEXT("Could not create file mapping object (%d).\n"),
             GetLastError());
      return 1;
   }
   pBuf = (LPTSTR) MapViewOfFile(hMapFile,   // handle to map object
                        FILE_MAP_ALL_ACCESS, // read/write permission
                        0,
                        0,
                        BUF_SIZE);

   if (pBuf == NULL)
   {
      printf(TEXT("Could not map view of file (%d).\n"),
             GetLastError());

	   CloseHandle(hMapFile);

      return(1);
   }


   memcpy((PVOID)pBuf, szMsg, (strlen(szMsg) * sizeof(char)));
   return(0);
}

int destroy_sharedMemmory(void)
{
   UnmapViewOfFile(pBuf);

   CloseHandle(hMapFile);

   return 0;
}

#ifndef __ShellH__
#define __ShellH__
//-----------------------------------------------------------------------------

#if !defined(__KERNEL_MODE__) && !defined(__LINUX__)
	#include "BaseType.h"
//	#include "FrameHeader.h"
	#include "JPEG.h"
	#include "Scale.h"
	#include "FaceTracking.h"
#endif

//-----------------------------------------------------------------------------

enum
{
    shpMakeLowLightDark,    //0
    shpHMirror,
    shpVMirror,
    shpColorSpace,
	shpLowLightDarkThd,		
	shpFilterThd,			//5
	shpFilterMode,
	shpEdgeThd,
	shpEdgeGain,
	shpBW,
	shpDefectMode,			//10
	shpDefectYThd,
	shpDefectCThd,			//12
	shpTemporalNR,          // [2008/05/08, Albert]
	shpEdgeDtn,					//2008/10/16 03:17下午
	shpTempStrgLumThd,
	shpTempStrgChrThd,
	shpTempNzPwLumThd,
	shpTempNzPwChrThd,      //17
	shpMeanDiffYThd,
	shpMeanDiffCThd,
	shpLowLightThd,			//20	
    shpCount
};
 
enum ColorSpace {csRGB24, csI420, csYUY2, csCount};
enum RotateState {rsNone, rs0, rs90, rs180, rs270, rsCount};    //Rotate function not support on Non-Windows plateform
//2010/10/21 03:47下午
enum DataInput {diYUY2, diRaw, diMJPG, diYUYV_S420, diCount};

typedef struct
{
	BOOL		bMakeLowLightDark;
	BOOL		bHMirror;
	BOOL		bVMirror;
	BOOL		bBW;
	ColorSpace	eColorSpace;
	RotateState	eRotateState;
	UInt32		iLowLightDarkThd;
	UInt32		iFilterThd;
	UInt32		iEdgeThd;
	UInt32		iFilterMode;
	UInt32		iEdgeGain;
	UInt32		iDefectMode;
	UInt32		iDefectYThd;
	UInt32		iDefectCThd;
	// [2008/05/08, Albert]
	BOOL        bTemporalNR;
	BOOL				bEdgeDtn;//2008/10/16 02:55下午
	UInt32		iTempStrgLumThd;
	SInt32 		iTempStrgChrThd;
	UInt32		iTempNzPwLumThd;
  UInt32      iTempNzPwChrThd;
	UInt32		iLowLightThd;
	// [2008/05/30, Albert]
	UInt32		iMeanDiffYThd;
	UInt32		iMeanDiffCThd;    
} ShellParam;

typedef struct
{
	UInt32 iFullFrameCnt, iJPEGErrorCnt, iLenErrCnt;
	UInt32 iHeaderYSum;
} ShellReport;
//-----------------------------------------------------------------------------

typedef struct _CShell 
{
	ShellParam  Param;
	ShellReport Report;

	BOOL bWaitSync;
	int iSyncCountDown;
	int iTempStrmHSize, iTempStrmVSize;
	int iTempBufHSize, iTempBufVSize;
	int iTempHiStart, iTempViStart;
	int iTempHiSize,  iTempViSize;
	int iTempHoSize,  iTempVoSize;
	//2010/10/21 03:47下午
	//int iTempSWSubSample, iTempSrcBlkSize;
	int a32TempSWSubSample[2], a32TempSrcBlkSize[2];

	UInt32 iLoadDumpLen;
	UInt32 iStreamLen;
	BOOL   bLoadStream;
	BOOL   bDumpStream;
	UInt8  *pLoadBuf;
	UInt8  *pDumpBuf;
	UInt8  *pStreamBuf, *pStream, *pDisBuf, *pProcBuf, *pRefnBuf;

	RotateState eLastRotateState;

	CJPEG  *pJPEG;
	CScale *pScale;
	CFace  *pFACE;

	UInt8 *pLocProcBuf1, *pLocProcBuf2, *pStoreMeanBuf;
	//2010/10/21 03:48下午
	UInt8 *TempBuffer, *TempBuffer2;

} CShell;
//-------------------------------------------------------------------------


// Function Prototype
//-----------------------------------------------------------------------------
void CShell_YUVRotate(CShell *mp);
void CShell_YUV2RGB24(CShell *mp);
void CShell_YUV2I420(CShell *mp);
void CShell_YUV2YUY2(CShell *mp);
void CShell_YUV2YUV(CShell *mp);
void CShell_RawProcess(CShell *mp);

//-----------------------------------------------------------------------------
CShell* CShell_CShell(/*Header *pHeader*/);
void CShell__CShell(CShell *mp);

//Parameter Management---------------------------------------------------------
void CShell_Reset(CShell *mp);
void CShell_ReadShellParamFromReg(CShell *mp, DWORD *pShellParams);
void CShell_WriteShellParamToReg(CShell *mp, DWORD *pShellParams);
void CShell_ReadScaleParamFromReg(CShell *mp, DWORD *pScaleParams);
void CShell_WriteScaleParamToReg(CShell *mp, DWORD *pScaleParams);
void CShell_ReadFaceTrackParamFromReg(CShell *mp, DWORD *pFaceTrackParams);
void CShell_WriteFaceTrackParamToReg(CShell *mp, DWORD *pFaceTrackParams);

ShellParam  *CShell_GetParam(CShell *mp);
ShellReport *CShell_GetReport(CShell *mp);
JPEGParam   *CShell_GetJPEGParam(CShell *mp);
JPEGReport  *CShell_GetJPEGReport(CShell *mp);
ScaleParam  *CShell_GetScaleParam(CShell *mp);
ScaleReport *CShell_GetScaleReport(CShell *mp);
FaceParam   *CShell_GetFaceParam(CShell *mp);
FaceReport  *CShell_GetFaceReport(CShell *mp);

//TestAP UI Interface----------------------------------------------------------
void CShell_LoadStream(CShell *mp, BOOL bLoadStream, UInt8 *pLoadBuf, int iLoadLen);
void CShell_DumpStream(CShell *mp, BOOL bDumpStream, UInt8 *pDumpBuf);

void CShell_CalColorInfo(CShell *mp);

void CShell_SetProcessInfo(CShell *mp, UInt32 iCapturedLen, UInt8 *pStream, /*UInt8 *pProcBuf,*/ UInt8 *pDisBuf, QTableInfo *pQTableInfo);

//2010/10/21 03:48下午
//void CShell_Process(CShell *mp, BOOL bDropLastFrame, BOOL bIsJPG, ColorSpace eColorSpace,UInt8 iEffectParam);
//BOOL CShell_SetSize(CShell *mp, int iStrmHSize,   int iStrmVSize,   int iBufHSize,    int iBufVSize,    int iHiStart,   int iViStart,    int iHiSize,   int iViSize,   int iHoSize,    int iVoSize,   int iSWSubSample,   int iSrcBlkSize, BOOL bSyncChange);
void CShell_Process(CShell *mp, BOOL bDropLastFrame, DataInput Input_Mode, ColorSpace eColorSpace);
BOOL CShell_SetSize(CShell *mp, int iStrmHSize,   int iStrmVSize,   int iBufHSize,    int iBufVSize,    int iHiStart,   int iViStart,    int iHiSize,   int iViSize,   int iHoSize,    int iVoSize,   int a32SWSubSample[2],   int a32SrcBlkSize[2], BOOL bSyncChange);

void CShell_GetSize(CShell *mp, int *piStrmHSize, int *piStrmVSize, int *piBufHSize,  int *piBufVSize,  int *piHiStart, int *piViStart,  int *piHiSize, int *piViSize, int *piHoSize,  int *piVoSize, int *piSWSubSample, int *piSrcBlkSize);
BOOL CShell_SetHMirror(CShell *mp, BOOL bHMirror);
BOOL CShell_SetVMirror(CShell *mp, BOOL bVMirror);
BOOL CShell_SetColorSpace(CShell *mp, ColorSpace eColorSpace);
BOOL CShell_SetRotateState(CShell *mp, RotateState eRotateState);
void CShell_SetFilterThd(CShell *mp, UInt32 iFilterThd, UInt32 iFilterMode);
void CShell_SetEdgeThd(CShell *mp, UInt32 iEdgeThd, UInt32 iEdgeGain);
void CShell_SetBW(CShell *mp, BOOL bBW);
void CShell_YUVDefectCompensation(CShell *mp);
void CShell_BlackWhite(CShell *mp);

void CShell_I420toRGB24(UInt8 *pSrc, UInt8 *pDst, int iHSize, int iVSize);
void CShell_YUV422Process(CShell *mp);
void CShell_YUV420Process(CShell *mp);
//void CShell_FilterProcess(CShell *mp); //[2008/04/08 by Albert]
//void CShell_EdgeProcess(CShell *mp); //[2008/04/08 by Albert]
//void CShell_DeNoiseProcess(CShell *mp); //[2008/04/08 by Albert]
void CShell_YUYVProcess(CShell *mp);
//2010/10/21 03:48下午
void CShell_YUYV_S420_Process(CShell *mp);

//[2008/03/10 by Albert]
void CShell_DenoiseTech(CShell *mp);
//-----------------------------------------------------------------------------
#endif

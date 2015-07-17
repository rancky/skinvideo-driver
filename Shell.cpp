#if defined(__LINUX__)
// --------defined LINUX-------------
    #include <snxcam.h>
// ----------------------------------
#elif defined(__KERNEL_MODE__)
    #include <sncam.h>
#else
    #include <math.h>
    #include "Shell.h"
	#include ".\effect\effect.h"
	#include ".\effect\mng.h"
	#include ".\effect\imgFunc.h"
	#include "VideoCap.h"


#endif

//-------------------------------------------------------------------------
CShell* CShell_CShell()
{
	CShell *mp;
	#if defined(__LINUX__)
	{
		mp = (CShell *)rvmalloc(sizeof(CShell));
		memset(&mp->Param,  0, sizeof(mp->Param));
		memset(&mp->Report, 0, sizeof(mp->Report));

		// [2008/04/07, Albert]
		mp->pLocProcBuf1 = (UInt8 *)rvmalloc(sizeof(UInt8)*(ImgWidList[isCount-1]+16)*(ImgHgtList[isCount-1]*5/3+16)*3/2);
		mp->pLocProcBuf2 = (UInt8 *)rvmalloc(sizeof(UInt8)*(ImgWidList[isCount-1]+16)*(ImgHgtList[isCount-1]*5/3+16)*3/2);
		// [2008/05/30, Albert]
		mp->pStoreMeanBuf = (UInt8 *)rvmalloc(sizeof(UInt8)*(ImgWidList[isCount-1]+16)*(ImgHgtList[isCount-1]*5/3+16)*3/2/16);
	}
	#elif defined(__KERNEL_MODE__)
	{
		if((mp = (CShell *)ExAllocatePoolWithTag(NonPagedPool, sizeof(CShell),'SNX')) == NULL)
			return NULL;
		RtlZeroMemory(&mp->Param,  sizeof(mp->Param));
		RtlZeroMemory(&mp->Report, sizeof(mp->Report));

		// [2008/04/07, Albert]
		if((mp->pLocProcBuf1 = (UInt8 *)ExAllocatePoolWithTag(NonPagedPool, sizeof(UInt8)*(ImgWidList[isCount-1]+32)*(ImgHgtList[isCount-1]*5/3+16)*3/2,'SNX')) == NULL)		
			return NULL;
		if((mp->pLocProcBuf2 = (UInt8 *)ExAllocatePoolWithTag(NonPagedPool, sizeof(UInt8)*(ImgWidList[isCount-1]+32)*(ImgHgtList[isCount-1]*5/3+16)*3/2,'SNX')) == NULL)		
			return NULL;
		// [2008/05/30, Albert]
		if((mp->pStoreMeanBuf = (UInt8 *)ExAllocatePoolWithTag(NonPagedPool, sizeof(UInt8)*(ImgWidList[isCount-1]+32)*(ImgHgtList[isCount-1]*5/3+16)*3/2/16,'SNX')) == NULL)
			return NULL;

	}
	#else
	{
		if((mp = (CShell *)malloc(sizeof(CShell)))==NULL)
			return NULL;
		memset(&mp->Param,  0, sizeof(mp->Param));
		memset(&mp->Report, 0, sizeof(mp->Report));

		// [2008/04/07, Albert]
		if((mp->pLocProcBuf1 = (UInt8 *)malloc(sizeof(UInt8)*(ImgWidList[isCount-1]+16)*(ImgHgtList[isCount-1]*5/3+16)*3/2))==NULL)
		  return NULL;
		if((mp->pLocProcBuf2 = (UInt8 *)malloc(sizeof(UInt8)*(ImgWidList[isCount-1]+16)*(ImgHgtList[isCount-1]*5/3+16)*3/2))==NULL)		
			return NULL;
		// [2008/05/30, Albert]
		if((mp->pStoreMeanBuf = (UInt8 *)malloc(sizeof(UInt8)*(ImgWidList[isCount-1]+16)*(ImgHgtList[isCount-1]*5/3+16)*3/2/16))==NULL)
			return NULL;
		
	}
	#endif

	mp->pJPEG  = CJPEG_CJPEG();
	mp->pScale = CScale_CScale();
	mp->bWaitSync = FALSE;
	mp->pFACE     = CFace_CFace(); 

	mp->pProcBuf = mp->pLocProcBuf1; // [2008/04/07, Albert]
	mp->pRefnBuf = mp->pLocProcBuf2; // [2008/04/07, Albert]

	if((mp->pJPEG == NULL) || (mp->pScale == NULL) /*|| (mp->pHeader == NULL)*/||(mp->pFACE == NULL))
		return NULL;

	//SSE SSE2 feature detection
	#if defined(__MMX__)
	{
		UInt32 CPU_FEATURE = 0;	// initial CPU_FEATURE to avoid prefast warning (ymwu, 2005/11/11)
		#define _SSE_FEATURE_BIT    0x02000000
		#define _SSE2_FEATURE_BIT   0x04000000
		__asm
		{
			mov             eax,                1;
			cpuid;
			mov             CPU_FEATURE,        edx;
			emms;
		}
		if (CPU_FEATURE & _SSE_FEATURE_BIT)
			gbSSE = TRUE;
		if (CPU_FEATURE & _SSE2_FEATURE_BIT)
			gbSSE2 = TRUE;
	}
	#endif

	return mp;
}
//---------------------------------------------------------------------------

void CShell__CShell(CShell *mp)
{
	if(mp==NULL)
		return;

	CJPEG__CJPEG(mp->pJPEG);
	mp->pJPEG = NULL;

	CScale__CScale(mp->pScale);
	mp->pScale = NULL;

	CFace__CFace(mp->pFACE);
	mp->pFACE = NULL;


#if defined(__LINUX__)

	// [2008/04/07, Albert]
	rvfree(mp->pLocProcBuf1, sizeof(UInt8)*(ImgWidList[isCount-1]+16)*(ImgHgtList[isCount-1]*5/3+16)*3/2));
	rvfree(mp->pLocProcBuf2, sizeof(UInt8)*(ImgWidList[isCount-1]+16)*(ImgHgtList[isCount-1]*5/3+16)*3/2));	
	// [2008/05/30, Albert]
	rvfree(mp->pStoreMeanBuf, sizeof(UInt8)*(ImgWidList[isCount-1]+16)*(ImgHgtList[isCount-1]*5/3+16)*3/2/16));	
	rvfree(mp, sizeof(CShell));

#elif defined(__KERNEL_MODE__)

	// [2008/04/07, Albert]
	if(mp->pLocProcBuf1!=NULL)
		ExFreePoolWithTag(mp->pLocProcBuf1,'SNX');
	if(mp->pLocProcBuf2!=NULL)
		ExFreePoolWithTag(mp->pLocProcBuf2,'SNX');

	// [2008/05/30, Albert]
	if(mp->pStoreMeanBuf!=NULL)
		ExFreePoolWithTag(mp->pStoreMeanBuf,'SNX');
		//Michael 	2008/7/10 10:05上午
	ExFreePoolWithTag(mp,'SNX');
	    
#else
	// [2008/04/07, Albert]
	free(mp->pLocProcBuf1);
	free(mp->pLocProcBuf2);
	// [2008/05/30, Albert]
	free(mp->pStoreMeanBuf);

	free(mp);
#endif
}
//---------------------------------------------------------------------------

void CShell_ReadShellParamFromReg(CShell *mp, DWORD *pShellParams)
{
	ShellParam *pShellParam = &mp->Param;

	if (pShellParams == NULL)
		return;

	pShellParam->bMakeLowLightDark = (BOOL)pShellParams[shpMakeLowLightDark];
	pShellParam->bHMirror = (BOOL)pShellParams[shpHMirror];
	pShellParam->bVMirror = (BOOL)pShellParams[shpVMirror];
	pShellParam->eColorSpace = (ColorSpace)pShellParams[shpColorSpace];
	pShellParam->iLowLightDarkThd = (BOOL)pShellParams[shpLowLightDarkThd];
	pShellParam->iFilterThd = (UInt32)pShellParams[shpFilterThd];
	pShellParam->iFilterMode = (UInt32)pShellParams[shpFilterMode];
	pShellParam->iEdgeThd = (UInt32)pShellParams[shpEdgeThd];
	pShellParam->iEdgeGain = (UInt32)pShellParams[shpEdgeGain];

	pShellParam->iDefectMode = (UInt32)pShellParams[shpDefectMode];
	pShellParam->iDefectYThd = (UInt32)pShellParams[shpDefectYThd];
	pShellParam->iDefectCThd = (UInt32)pShellParams[shpDefectCThd];

	// [2008/06/03, Albert]
	pShellParam->bTemporalNR	 = (BOOL)pShellParams[shpTemporalNR];
	pShellParam->bEdgeDtn		 = (BOOL)pShellParams[shpEdgeDtn];//2008/10/16 02:54下午
	pShellParam->iTempStrgLumThd = (UInt32)pShellParams[shpTempStrgLumThd];
	pShellParam->iTempStrgChrThd = (UInt32)pShellParams[shpTempStrgChrThd];
	pShellParam->iTempNzPwLumThd = (UInt32)pShellParams[shpTempNzPwLumThd];
	pShellParam->iTempNzPwChrThd = (UInt32)pShellParams[shpTempNzPwChrThd];
	pShellParam->iMeanDiffYThd	 = (UInt32)pShellParams[shpMeanDiffYThd];
	pShellParam->iMeanDiffCThd   = (UInt32)pShellParams[shpMeanDiffCThd];
	pShellParam->iLowLightThd	 = (UInt32)pShellParams[shpLowLightThd];

}
//---------------------------------------------------------------------------

void CShell_Reset(CShell *mp)
{
	ShellParam *pShellParam = &mp->Param;

	CJPEG_Reset(mp->pJPEG);
	CScale_Reset(mp->pScale);
	//Steven 2005/10/25
	CFace_Reset(mp->pFACE);

	mp->bWaitSync = FALSE;
	mp->bLoadStream = FALSE;
	mp->bDumpStream = FALSE;
	mp->pLoadBuf = NULL;
	mp->pDumpBuf = NULL;
	CShell_SetHMirror(mp, pShellParam->bHMirror);
	CShell_SetVMirror(mp, pShellParam->bVMirror);
	CShell_SetColorSpace(mp, pShellParam->eColorSpace);
	CShell_SetRotateState(mp, pShellParam->eRotateState);
	mp->eLastRotateState = (RotateState)((pShellParam->eRotateState +1) %rsCount);
}
//---------------------------------------------------------------------------

void CShell_WriteShellParamToReg(CShell *mp, DWORD *pShellParams)
{
	ShellParam *pShellParam = &mp->Param;

	if (pShellParams == NULL)
		return;
	pShellParams[shpMakeLowLightDark] = (DWORD)pShellParam->bMakeLowLightDark;
	pShellParams[shpHMirror] = (DWORD)pShellParam->bHMirror;
	pShellParams[shpVMirror] = (DWORD)pShellParam->bVMirror;
	pShellParams[shpColorSpace] = (DWORD)pShellParam->eColorSpace;
	pShellParams[shpLowLightDarkThd] = (DWORD)pShellParam->iLowLightDarkThd;
	pShellParams[shpFilterThd] = (DWORD)pShellParam->iFilterThd;
	pShellParams[shpFilterMode] = (DWORD)pShellParam->iFilterMode;
	pShellParams[shpEdgeThd] = (DWORD)pShellParam->iEdgeThd;
	pShellParams[shpEdgeGain] = (DWORD)pShellParam->iEdgeGain;
}
//---------------------------------------------------------------------------

void CShell_ReadScaleParamFromReg(CShell *mp, DWORD *pScaleParams)
{
	CScale_ReadScaleParamFromReg(mp->pScale, pScaleParams);
}
//---------------------------------------------------------------------------

void CShell_WriteScaleParamToReg(CShell *mp, DWORD *pScaleParams)
{
	CScale_WriteScaleParamToReg(mp->pScale, pScaleParams);
}
//---------------------------------------------------------------------------

void CShell_ReadFaceTrackParamFromReg(CShell *mp, DWORD *pFaceTrackParams)
{
	CFace_ReadFaceTrackParamFromReg(mp->pFACE, pFaceTrackParams);
}
//---------------------------------------------------------------------------
void CShell_WriteFaceTrackParamToReg(CShell *mp, DWORD *pFaceTrackParams)
{
	CFace_WriteFaceTrackParamToReg(mp->pFACE, pFaceTrackParams);
}
//---------------------------------------------------------------------------
void CShell_SetProcessInfo(CShell *mp, UInt32 iCapturedLen, UInt8 *pStreamBuf,
    UInt8 *pDisBuf, QTableInfo *pQTableInfo)
{
	mp->iStreamLen  = iCapturedLen;
	mp->pStreamBuf  = pStreamBuf;
	mp->pStream     = pStreamBuf;
	mp->pDisBuf     = pDisBuf;
}
//---------------------------------------------------------------------------

void CShell_GetSize(CShell *mp, int *piStrmHSize, int *piStrmVSize, int *piBufHSize,
					int *piBufVSize, int *piHiStart, int *piViStart, int *piHiSize, int *piViSize,
					int *piHoSize,  int *piVoSize, int *piSWSubSample, int *piSrcBlkSize)
{
	if (mp->bWaitSync)
	{
		*piStrmHSize = mp->iTempStrmHSize;
		*piStrmVSize = mp->iTempStrmVSize;
		*piBufHSize = mp->iTempBufHSize;
		*piBufVSize = mp->iTempBufVSize;
		*piHiStart = mp->iTempHiStart;
		*piViStart = mp->iTempViStart;
		*piHiSize = mp->iTempHiSize;
		*piViSize = mp->iTempViSize;
		*piHoSize = mp->iTempHoSize;
		*piVoSize = mp->iTempVoSize;
		piSWSubSample = mp->a32TempSWSubSample;
		piSrcBlkSize = mp->a32TempSrcBlkSize;
	}
	else
	{
		JPEGParam  *pJPEGParam  = CJPEG_GetParam(mp->pJPEG);
		ScaleParam *pScaleParam = CScale_GetParam(mp->pScale);
		*piStrmHSize = pJPEGParam->iStrmHSize;
		*piStrmVSize = pJPEGParam->iStrmVSize;
		*piBufHSize = pJPEGParam->iOutBufHSize;
		*piBufVSize = pJPEGParam->iOutBufVSize;
		
		*piHiStart = pScaleParam->iHStart;
		*piViStart = pScaleParam->iVStart;
		*piHiSize = pScaleParam->iHiSize;
		*piViSize = pScaleParam->iViSize;
		*piHoSize = pScaleParam->iHoSize;
		*piVoSize = pScaleParam->iVoSize;
		
		piSWSubSample = pScaleParam->a32SWSubSample;
		piSrcBlkSize = pScaleParam->a32SrcBlkSize;
	}
}
//---------------------------------------------------------------------------

BOOL CShell_SetSize(CShell *mp, int iStrmHSize, int iStrmVSize, int iBufHSize, int iBufVSize, 
					int iHiStart, int iViStart, int iHiSize, int iViSize, int iHoSize,  int iVoSize,
					int a32SWSubSample[2], int a32SrcBlkSize[2], BOOL bSyncChange)
{
	if (bSyncChange)
	{    	
		mp->bWaitSync = TRUE;
		mp->iSyncCountDown = 1;
		mp->iTempStrmHSize = iStrmHSize;
		mp->iTempStrmVSize = iStrmVSize;
		mp->iTempBufHSize = iBufHSize;
		mp->iTempBufVSize = iBufVSize;
		mp->iTempHiStart = iHiStart;
		mp->iTempViStart = iViStart;
		mp->iTempHiSize  = iHiSize;
		mp->iTempViSize  = iViSize;
		mp->iTempHoSize  = iHoSize;
		mp->iTempVoSize  = iVoSize;
		mp->a32TempSWSubSample[0] = a32SWSubSample[0];
		mp->a32TempSWSubSample[1] = a32SWSubSample[1];
		mp->a32TempSrcBlkSize[0]  = a32SrcBlkSize[0];
		mp->a32TempSrcBlkSize[1]  = a32SrcBlkSize[1];
	}
	else
	{
		CJPEG_SetJPEGSize(mp->pJPEG, iStrmHSize, iStrmVSize,
			iBufHSize, iBufVSize, iHiStart, iViStart, iHiSize, iViSize);
		CScale_SetScaleSize(mp->pScale, iBufHSize, iBufVSize, iHiStart, iViStart, 
			iHiSize, iViSize, iHoSize, iVoSize, a32SWSubSample, a32SrcBlkSize);
	}
	return TRUE;
}
//---------------------------------------------------------------------------

BOOL CShell_SetHMirror(CShell *mp, BOOL bHMirror)
{
	mp->Param.bHMirror = bHMirror;
	return TRUE;
}
//---------------------------------------------------------------------------

BOOL CShell_SetVMirror(CShell *mp, BOOL bVMirror)
{
	mp->Param.bVMirror = bVMirror;
	return TRUE;
}
//---------------------------------------------------------------------------

BOOL CShell_SetColorSpace(CShell *mp, ColorSpace eColorSpace)
{
	mp->Param.eColorSpace = eColorSpace;
	return TRUE;
}
//---------------------------------------------------------------------------

BOOL CShell_SetRotateState(CShell *mp, RotateState eRotateState)
{
	mp->Param.eRotateState = eRotateState;
	return TRUE;
}
//---------------------------------------------------------------------------

void CShell_LoadStream(CShell *mp, BOOL bLoadStream, UInt8 *pLoadBuf, int iLoadLen)
{
	mp->bLoadStream = bLoadStream;
	mp->pLoadBuf = (bLoadStream)? pLoadBuf: NULL;
	mp->iLoadDumpLen = (bLoadStream)? iLoadLen: 0;
}
//---------------------------------------------------------------------------

void CShell_DumpStream(CShell *mp, BOOL bDumpStream, UInt8 *pDumpBuf)
{
	if (pDumpBuf && mp->pDumpBuf)  //last dump not finished yet
		return;
	mp->bDumpStream = bDumpStream;
	mp->pDumpBuf = (bDumpStream)? pDumpBuf: NULL;
}
//---------------------------------------------------------------------------

void CShell_Process(CShell *mp, BOOL bDropLastFrame, DataInput Input_Mode, ColorSpace eColorSpace)
{
	CJPEG  *pJPEG   = mp->pJPEG;
	CScale *pScale  = mp->pScale;

	ShellParam *pShellParam = &mp->Param;
	JPEGReport *pJPEGReport = CShell_GetJPEGReport(mp);

	if (mp->bWaitSync)
	{
		mp->iSyncCountDown--;
		if (mp->iSyncCountDown==0)
		{
			mp->bWaitSync = FALSE;
			CJPEG_SetJPEGSize(pJPEG, mp->iTempStrmHSize, mp->iTempStrmVSize,
				mp->iTempBufHSize, mp->iTempBufVSize, 
				mp->iTempHiStart, mp->iTempViStart, mp->iTempHiSize, mp->iTempViSize);
			CScale_SetScaleSize(pScale ,mp->iTempBufHSize, mp->iTempBufVSize, 
				mp->iTempHiStart, mp->iTempViStart, mp->iTempHiSize, mp->iTempViSize,
				mp->iTempHoSize, mp->iTempVoSize, mp->a32TempSWSubSample, mp->a32TempSrcBlkSize);
		}
	}

	{					
		if (Input_Mode == diYUY2)
		{
			DBGU_TRACE("CShell_YUYVProcess\n");
			CShell_YUYVProcess(mp);
		}
		else if (Input_Mode == diYUYV_S420)
		{
		  DBGU_TRACE("CShell_YUYV_S420_Process\n");
			CShell_YUYV_S420_Process(mp);
		}
		else if(Input_Mode == diMJPG)
		{
			JPEGParam *pJPEGParam = CJPEG_GetParam(pJPEG);
			int iJFIFLen = mp->iStreamLen;
			CJPEG_SetJPEGSize(pJPEG, pJPEGParam->iStrmHSize, pJPEGParam->iStrmVSize,
				pJPEGParam->iOutBufHSize, pJPEGParam->iOutBufVSize,
				pJPEGParam->iDecHStart, pJPEGParam->iDecVStart,
				pJPEGParam->iDecHSize, pJPEGParam->iDecVSize);
			CJPEG_JPEGDecode(pJPEG, mp->pStreamBuf, mp->pProcBuf, iJFIFLen);

		}
	}

	if (Input_Mode == diMJPG)
	{
		if (pJPEGReport->bJPEGError == TRUE)
		{
			mp->Report.iJPEGErrorCnt++;
			return;
		}
	}

	//Face Tracking
	CFace_FaceTrack(mp->pFACE, mp->pProcBuf);


	{
		CShell_YUVDefectCompensation(mp);
	}

	CShell_BlackWhite(mp);

	//padding image borders
	{
		JPEGParam  *pJPEGParam  = CShell_GetJPEGParam(mp);
		ScaleParam *pScaleParam = CShell_GetScaleParam(mp);
		if ((pScaleParam->a32SrcBlkSize[0] != iDstBlkSize) || (pScaleParam->a32SrcBlkSize[1] != iDstBlkSize))
		{
			SInt32 iYBufHSize = pJPEGParam->iOutBufHSize;
			SInt32 iYBufVSize = pJPEGParam->iOutBufVSize;
			SInt32 iYHStart = pJPEGParam->iDecHStart;
			SInt32 iYVStart = pJPEGParam->iDecVStart;
			SInt32 iYHSize  = pJPEGParam->iDecHSize;
			SInt32 iYVSize  = pJPEGParam->iDecVSize;

			SInt32 iCBufHSize = iYBufHSize/2;
			SInt32 iCBufVSize = iYBufVSize/2;
			SInt32 iCHStart = iYHStart/2;
			SInt32 iCVStart = iYVStart/2;
			SInt32 iCHSize  = iYHSize/2;
			SInt32 iCVSize  = iYVSize/2;

			UInt8 *pY  = mp->pProcBuf;
			UInt8 *pCb = pY  + iYBufHSize*iYBufVSize;
			UInt8 *pCr = pCb + iCBufHSize*iCBufVSize;

			pY  = pY  +iYVStart*iYBufHSize +iYHStart +iYHSize;
			pCb = pCb +iCVStart*iCBufHSize +iCHStart +iCHSize;
			pCr = pCr +iCVStart*iCBufHSize +iCHStart +iCHSize;

			//pad right border
			for (int i = 0; i < iYVSize; i++, pY+=iYBufHSize)
				*pY = *(pY-1);

			for (int j = 0; j < iCVSize; j++, pCb+=iCBufHSize, pCr+=iCBufHSize)
			{
				*pCb = *(pCb-1);
				*pCr = *(pCr-1);
			}

			//pad bottom border
			pY  = pY  -iYHSize;
			pCb = pCb -iCHSize;
			pCr = pCr -iCHSize;
			memcpy(pY,  pY- iYBufHSize, iYHSize+1);
			memcpy(pCb, pCb-iCBufHSize, iCHSize+1);
			memcpy(pCr, pCr-iCBufHSize, iCHSize+1);
		}
	}

	CScale_UpSample(pScale,  mp->pProcBuf, mp->pProcBuf);

	CShell_YUVRotate(mp);

	//DeNoise Process, [2009/02/19, Albert Modified]
	CShell_DenoiseTech(mp);

	switch (eColorSpace)
	{
	case csRGB24:
		if ((mp->Param.eRotateState == rs90) || (mp->Param.eRotateState == rs270))
		{
			int iDispBufSize = pScale->Param.iHoSize *pScale->Param.iVoSize*3;
			memset(mp->pDisBuf, 0, iDispBufSize);
		}
		CShell_YUV2RGB24(mp);
		break;
	case csI420:
		if ((mp->Param.eRotateState == rs90) || (mp->Param.eRotateState == rs270))
		{
			int iYBufSize = pScale->Param.iHoSize * pScale->Param.iVoSize;
			int iCBufSize = pScale->Param.iHoSize * pScale->Param.iVoSize / 2;
			memset(mp->pDisBuf, 0, iYBufSize);
			memset(mp->pDisBuf + iYBufSize, 127, iCBufSize);
		}
		CShell_YUV2I420(mp);
		break;
	case csYUY2:
		CShell_YUV2YUY2(mp);		
		break;
	}
	mp->eLastRotateState = mp->Param.eRotateState;
	//2008/5/21 04:26下午
    //Memory Swap, [2008/05/13, Albert]
	UInt8 *pTemp;
	pTemp = mp->pRefnBuf;
	mp->pRefnBuf = mp->pProcBuf;
	mp->pProcBuf = pTemp;

}
//---------------------------------------------------------------------------
void CShell_YUV2RGB24(CShell *mp)
{
	ShellParam *pShellParam = &mp->Param;
	ScaleParam *pScaleParam = CShell_GetScaleParam(mp);

	BOOL bHMirror = pShellParam->bHMirror;
	BOOL bVMirror = pShellParam->bVMirror;
	RotateState eRotateState = pShellParam->eRotateState;
	if (eRotateState == rs270)          //-90
		bVMirror = 1-bVMirror;
	else if(eRotateState == rs180)      //180
	{
		bVMirror = 1-bVMirror;
		bHMirror = 1-bHMirror;
	}
	else if(eRotateState == rs90)      //90
		bHMirror = 1-bHMirror;

	int iBufHSize  = pScaleParam->iBufHSize;
	int iBufVSize  = pScaleParam->iBufVSize;
	int iHiStart = pScaleParam->iHStart;
	int iViStart = pScaleParam->iVStart;
	int iHoSize = pScaleParam->iHoSize;
	int iVoSize = pScaleParam->iVoSize;

	int iYBufSize = iBufHSize *iBufVSize;
	int iCBufSize = iYBufSize /4;

	int iYLineSize   = iBufHSize;
	int iBGRLineSize = iHoSize*3;

	int	a32SWSubSample[2];
	a32SWSubSample[0]	= pScaleParam->a32SWSubSample[0];
	a32SWSubSample[1]	= pScaleParam->a32SWSubSample[1];
	int	s32StepH	= 1 << a32SWSubSample[0];
	int	s32StepV	= 1 << a32SWSubSample[1];
	int iYPixStep = s32StepH << 1;
	int iCPixStep = s32StepH;
	int iYLineStep = iBufHSize * (s32StepV << 1);
	int iCLineStep = iBufHSize/2 *s32StepV;
	int iBGRLineStep = (bVMirror)? (-iHoSize)*6: iHoSize*6;
	int iBGRPixStep  = (bHMirror)? -6: 6;
	
	UInt8 *pYFrameStart = mp->pProcBuf +iViStart*iBufHSize +iHiStart;
	UInt8 *pYFrameEnd   = pYFrameStart + iBufHSize*iVoSize* s32StepV;
	UInt8 *pCFrameStart = mp->pProcBuf +iYBufSize +(iViStart/2)*(iBufHSize/2) +(iHiStart/2);
	UInt8 *pBGRLineStart = mp->pDisBuf;
	
	int iYMiddleOffset = (iHoSize-iVoSize) *s32StepH /2;
	int iCMiddleOffset = (iHoSize-iVoSize) *s32StepH /2 /2;
	int iBGRMiddleOffset = (iHoSize-iVoSize) *3 /2;
	if ((pShellParam->eRotateState == rs90) || (pShellParam->eRotateState == rs270))
	{
		pYFrameStart  += iYMiddleOffset;
		pCFrameStart  += iCMiddleOffset;
		if (!bHMirror)
			pBGRLineStart += iBGRMiddleOffset;
		else
			pBGRLineStart -= iBGRMiddleOffset;
	}

	UInt8 *pYLineStart, *pCLineStart;
	int iOff0, iOff1, iOff2, iOff3;
	if ((a32SWSubSample[0] == 0) && (a32SWSubSample[1] == 0))
	{
		#if defined(__MMX__)
		{
			pBGRLineStart += ((bVMirror)? (iVoSize-2)*iHoSize*3: 0);
			pBGRLineStart += ((bHMirror)? (iHoSize-8)*3: 0);
			if (!bVMirror && !bHMirror)
			{
				iOff0 = 0;
				iOff2 = iYLineSize;
			}
			else if (!bVMirror && bHMirror)
			{
				iOff0 = 4;
				iOff2 = iYLineSize+4;
			}
			else if (bVMirror && !bHMirror)
			{
				iOff0 = iYLineSize;
				iOff2 = 0;
			}
			else if (bVMirror && bHMirror)
			{
				iOff0 = iYLineSize+4;
				iOff2 = 4;
			}
		}
		#else
		{
			pBGRLineStart += ((bVMirror)? (iVoSize-2)*iHoSize*3: 0);
			pBGRLineStart += ((bHMirror)? (iHoSize-2)*3: 0);

			if (!bVMirror && !bHMirror)
			{
				iOff0 = 0;
				iOff1 = 1;
				iOff2 = iYLineSize;
				iOff3 = iYLineSize+1;
			}
			else if (!bVMirror && bHMirror)
			{
				iOff0 = 1;
				iOff1 = 0;
				iOff2 = iYLineSize+1;
				iOff3 = iYLineSize;
			}
			else if (bVMirror && !bHMirror)
			{
				iOff0 = iYLineSize;
				iOff1 = iYLineSize+1;
				iOff2 = 0;
				iOff3 = 1;
			}
			else if (bVMirror && bHMirror)
			{
				iOff0 = iYLineSize+1;
				iOff1 = iYLineSize;
				iOff2 = 1;
				iOff3 = 0;
			}
		}
		#endif
	}
	else
	{
		pBGRLineStart += ((bVMirror)? (iVoSize-2)*iHoSize*3: 0);
		pBGRLineStart += ((bHMirror)? (iHoSize-2)*3: 0);

		if (!bVMirror && !bHMirror)
		{
			iOff0 = 0;
			iOff1 = s32StepH;
			iOff2 = iYLineSize*s32StepV;
			iOff3 = iYLineSize*s32StepV+s32StepH;
		}
		else if (!bVMirror && bHMirror)
		{
			iOff0 = s32StepH;
			iOff1 = 0;
			iOff2 = iYLineSize*s32StepV+s32StepH;
			iOff3 = iYLineSize*s32StepV;
		}
		else if (bVMirror && !bHMirror)
		{
			iOff0 = iYLineSize*s32StepV;
			iOff1 = iYLineSize*s32StepV+s32StepH;
			iOff2 = 0;
			iOff3 = s32StepH;
		}
		else if (bVMirror && bHMirror)
		{
			iOff0 = iYLineSize*s32StepV+s32StepH;
			iOff1 = iYLineSize*s32StepV;
			iOff2 = s32StepH;
			iOff3 = 0;
		}
	}

	#if defined(__MMX__)
	{
		if ((a32SWSubSample[0] == 0) && (a32SWSubSample[1] == 0))
		{
			//B = 1.00000000000000*Y + 1.77200006607382*U + 0.00000040629806*V
			//G = 1.00000000000000*Y - 0.34413567816534*U - 0.71413615558181*V
			//R = 1.00000000000000*Y - 0.00000121889419*U + 1.40199958865734*V
			//B = Y + (454U)/256
			//G = Y - (88U +183V)/256
			//R = Y + (359V)/256

			const UInt64 U2B = 0x01c601c601c601c6;
			const UInt64 U2G = 0xffa8ffa8ffa8ffa8;
			const UInt64 V2G = 0xff49ff49ff49ff49;
			const UInt64 V2R = 0x0167016701670167;

			int iTempHoSize = iHoSize;

			if ((pShellParam->eRotateState == rs90) || (pShellParam->eRotateState == rs270))
				iTempHoSize = iVoSize;
			if (!bHMirror)
			{
				for (pYLineStart=pYFrameStart, pCLineStart=pCFrameStart;
					pYLineStart<pYFrameEnd;
					pYLineStart+=iYLineStep, pCLineStart+=iCLineStep, pBGRLineStart+=iBGRLineStep)
				{
					__asm
					{
						mov         ecx,            iTempHoSize;
						mov         eax,            pCLineStart;
						mov         esi,            pYLineStart;
						mov         edi,            pBGRLineStart;
						shr         ecx,            3;
					YUV2RGB24_M_:
						push        ecx;
						//-------------------------------------------------------------
						mov         edx,            iCBufSize;
						pcmpeqw     mm7,            mm7;
						punpcklbw   mm5,            [eax];
						psllw       mm7,            15;
						punpcklbw   mm6,            [eax+edx];
						paddw       mm5,            mm7;
						paddw       mm6,            mm7;
						movq        mm4,            mm5;            //mm4 = mm5 = (U3 U2 U1 U0) *256
						movq        mm7,            mm6;            //mm6 = mm7 = (V3 V2 V1 V0) *256
						pmulhw      mm4,            U2G;
						pmulhw      mm6,            V2G;
						pmulhw      mm5,            U2B;            //mm5 = (454/256)*(U3 U2 U1 U0)
						pmulhw      mm7,            V2R;            //mm7 = (359/256)*(V3 V2 V1 V0)
						paddsw      mm6,            mm4;            //mm6 = -(88/256)*(U3 U2 U1 U0)-(183/256)*(V3 V2 V1 V0)
						//-------------------------------------------------------------
						mov         ebx,            iOff0;
						mov         ecx,            iOff2;
						mov         edx,            iBGRLineSize;
						punpckldq   mm2,            mm5;
						punpckldq   mm3,            mm6;
						punpckldq   mm4,            mm7;
						punpckhwd   mm2,            mm2;            //mm2 = (454/256)*(U1 U1 U0 U0)
						punpckhwd   mm3,            mm3;            //mm3 = -(88/256)*(U1 U1 U0 U0)-(183/256)*(V1 V1 V0 V0)
						punpckhwd   mm4,            mm4;            //mm4 = (359/256)*(V1 V1 V0 V0)
						//-------------------------------------------------------------
						punpcklbw   mm1,            [esi+ebx];
						psrlw       mm1,            8;
						movq        mm0,            mm1;
						paddsw      mm2,            mm0;            //B
						paddsw      mm1,            mm3;            //G
						paddsw      mm0,            mm4;            //R
						packuswb    mm2,            mm1;            //mm2 = G3 G2 G1 G0 B3 B2 B1 B0
						packuswb    mm1,            mm0;            //mm1 = R3 R2 R1 R0 G3 G2 G1 G0
						punpcklbw   mm2,            mm1;            //mm2 = G3 B3 G2 B2 G1 B1 G0 B0
						punpckhbw   mm1,            mm1;            //mm1 = R3 R3 R2 R2 R1 R1 R0 R0
						movq        mm0,            mm2;            //mm0 = G3 B3 G2 B2 G1 B1 G0 B0
						punpcklwd   mm2,            mm1;            //mm2 = R1 R1 G1 B1 R0 R0 G0 B0
						punpckhwd   mm0,            mm1;            //mm0 = R3 R3 G3 B3 R2 R2 G2 B2
						movd        [edi],          mm2;
						movd        [edi+6],        mm0;
						psrlq       mm2,            24;
						psrlq       mm0,            24;
						movd        [edi+2],        mm2;
						movd        [edi+8],        mm0;
						//-------------------------------------------------------------
						punpcklbw   mm1,            [esi+ecx];
						punpckldq   mm2,            mm5;
						psrlw       mm1,            8;
						punpckhwd   mm2,            mm2;            //mm2 = (454/256)*(U1 U1 U0 U0)
						paddsw      mm2,            mm1
						paddsw      mm3,            mm1
						paddsw      mm4,            mm1;
						packuswb    mm2,            mm3;            //mm2 = G3 G2 G1 G0 B3 B2 B1 B0
						packuswb    mm3,            mm4;            //mm3 = R3 R2 R1 R0 G3 G2 G1 G0
						punpcklbw   mm2,            mm3;            //mm2 = G3 B3 G2 B2 G1 B1 G0 B0
						punpckhbw   mm3,            mm3;            //mm3 = R3 R3 R2 R2 R1 R1 R0 R0
						movq        mm4,            mm2;            //mm4 = G3 B3 G2 B2 G1 B1 G0 B0
						punpcklwd   mm2,            mm3;            //mm2 = R1 R1 G1 B1 R0 R0 G0 B0
						punpckhwd   mm4,            mm3;            //mm4 = R3 R3 G3 B3 R2 R2 G2 B2
						movd        [edi+edx],      mm2;
						movd        [edi+edx+6],    mm4;
						psrlq       mm2,            24;
						psrlq       mm4,            24;
						movd        [edi+edx+2],    mm2;
						movd        [edi+edx+8],    mm4;
						//-------------------------------------------------------------
						punpckhwd   mm5,            mm5;            //mm5 = (454/256)*(U3 U3 U2 U2)
						punpckhwd   mm6,            mm6;            //mm6 = -(88/256)*(U3 U3 U2 U2)-(183/256)*(V3 V3 V2 V2)
						punpckhwd   mm7,            mm7;            //mm7 = (359/256)*(V3 V3 V2 V2)
						punpcklbw   mm0,            [esi+ebx+4];
						psrlw       mm0,            8;
						movq        mm1,            mm0;
						movq        mm2,            mm0;            //mm0 = mm1 = mm2 = Y7 Y6 Y5 Y4
						paddsw      mm0,            mm5;
						paddsw      mm1,            mm6;
						paddsw      mm2,            mm7;
						packuswb    mm0,            mm1;            //mm0 = G7 G6 G5 G4 B7 B6 B5 B4
						packuswb    mm1,            mm2;            //mm1 = R7 R6 R5 R4 G7 G6 G5 G4
						punpcklbw   mm0,            mm1;            //mm0 = G7 B7 G6 B6 G5 B5 G4 B4
						punpckhbw   mm1,            mm1;            //mm1 = R7 R7 R6 R6 R5 R5 R4 R4
						movq        mm2,            mm0;            //mm2 = G7 B7 G6 B6 G5 B5 G4 B4
						punpcklwd   mm0,            mm1;            //mm0 = R5 R5 G5 B5 R4 R4 G4 B4
						punpckhwd   mm2,            mm1;            //mm2 = R7 R7 G7 B7 R6 R6 G6 B6
						movd        [edi+12],       mm0;
						movd        [edi+18],       mm2;
						psrlq       mm0,            24;
						psrlq       mm2,            24;
						movd        [edi+14],       mm0;
						movd        [edi+20],       mm2;
						//-------------------------------------------------------------
						punpcklbw   mm0,            [esi+ecx+4];
						psrlw       mm0,            8;
						paddsw      mm5,            mm0
						paddsw      mm6,            mm0
						paddsw      mm7,            mm0;
						packuswb    mm5,            mm6;            //mm5 = G7 G6 G5 G4 B7 B6 B5 B4
						packuswb    mm6,            mm7;            //mm6 = R7 R6 R5 R4 G7 G6 G5 G4
						punpcklbw   mm5,            mm6;            //mm5 = G7 B7 G6 B6 G5 B5 G4 B4
						punpckhbw   mm6,            mm6;            //mm6 = R7 R7 R6 R6 R5 R5 R4 R4
						movq        mm7,            mm5;            //mm7 = G7 B7 G6 B6 G5 B5 G4 B4
						punpcklwd   mm5,            mm6;            //mm5 = R5 R5 G5 B5 R4 R4 G4 B4
						punpckhwd   mm7,            mm6;            //mm7 = R7 R7 G7 B7 R6 R6 G6 B6
						movd        [edi+edx+12],   mm5;
						movd        [edi+edx+18],   mm7;
						psrlq       mm5,            24;
						psrlq       mm7,            24;
						movd        [edi+edx+14],   mm5;
						movd        [edi+edx+20],   mm7;
						//-------------------------------------------------------------
						pop         ecx;
						lea         esi,            [esi+8];
						lea         eax,            [eax+4];
						lea         edi,            [edi+24];
						dec         ecx;
						jne         YUV2RGB24_M_;
						emms;
					}
				}
			}
			else
			{
				for (pYLineStart=pYFrameStart, pCLineStart=pCFrameStart;
					pYLineStart<pYFrameEnd;
					pYLineStart+=iYLineStep, pCLineStart+=iCLineStep, pBGRLineStart+=iBGRLineStep)
				{        
					__asm
					{
						mov         ecx,            iTempHoSize;
						mov         eax,            pCLineStart;
						mov         esi,            pYLineStart;
						mov         edi,            pBGRLineStart;
						shr         ecx,            3;
					YUV2RGB24_M:
						push        ecx;
						push        eax;
						//-------------------------------------------------------------
						mov         edx,            iCBufSize;
						pcmpeqw     mm7,            mm7;
						mov         ebx,            [eax];
						psllw       mm7,            15;
						mov         ecx,            [eax+edx];
						bswap       ebx;
						bswap       ecx;
						movd        mm5,            ebx;
						movd        mm6,            ecx;
						punpcklbw   mm5,            mm5;
						punpcklbw   mm6,            mm6;
						paddw       mm5,            mm7;
						paddw       mm6,            mm7;
						movq        mm4,            mm5;            //mm4 = mm5 = (U3 U2 U1 U0) *256
						movq        mm7,            mm6;            //mm6 = mm7 = (V3 V2 V1 V0) *256
						pmulhw      mm4,            U2G;
						pmulhw      mm6,            V2G;
						pmulhw      mm5,            U2B;            //mm5 = (454/256)*(U3 U2 U1 U0)
						pmulhw      mm7,            V2R;            //mm7 = (359/256)*(V3 V2 V1 V0)
						paddsw      mm6,            mm4;            //mm6 = -(88/256)*(U3 U2 U1 U0)-(183/256)*(V3 V2 V1 V0)
						//-------------------------------------------------------------
						mov         ebx,            iOff0;
						mov         ecx,            iOff2;
						mov         edx,            iBGRLineSize;
						punpckldq   mm2,            mm5;
						punpckldq   mm3,            mm6;
						punpckldq   mm4,            mm7;
						punpckhwd   mm2,            mm2;            //mm2 = (454/256)*(U1 U1 U0 U0)
						punpckhwd   mm3,            mm3;            //mm3 = -(88/256)*(U1 U1 U0 U0)-(183/256)*(V1 V1 V0 V0)
						punpckhwd   mm4,            mm4;            //mm4 = (359/256)*(V1 V1 V0 V0)
						//-------------------------------------------------------------
						mov         eax,            [esi+ebx];
						bswap       eax;
						movd        mm1,            eax;
						punpcklbw   mm1,            mm1;
						psrlw       mm1,            8;
						movq        mm0,            mm1;
						paddsw      mm2,            mm0;            //B
						paddsw      mm1,            mm3;            //G
						paddsw      mm0,            mm4;            //R
						packuswb    mm2,            mm1;            //mm2 = G3 G2 G1 G0 B3 B2 B1 B0
						packuswb    mm1,            mm0;            //mm1 = R3 R2 R1 R0 G3 G2 G1 G0
						punpcklbw   mm2,            mm1;            //mm2 = G3 B3 G2 B2 G1 B1 G0 B0
						punpckhbw   mm1,            mm1;            //mm1 = R3 R3 R2 R2 R1 R1 R0 R0
						movq        mm0,            mm2;            //mm0 = G3 B3 G2 B2 G1 B1 G0 B0
						punpcklwd   mm2,            mm1;            //mm2 = R1 R1 G1 B1 R0 R0 G0 B0
						punpckhwd   mm0,            mm1;            //mm0 = R3 R3 G3 B3 R2 R2 G2 B2
						movd        [edi],          mm2;
						movd        [edi+6],        mm0;
						psrlq       mm2,            24;
						psrlq       mm0,            24;
						movd        [edi+2],        mm2;
						movd        [edi+8],        mm0;
						//-------------------------------------------------------------
						mov         eax,            [esi+ecx];
						bswap       eax;
						movd        mm1,            eax;
						punpcklbw   mm1,            mm1;
						punpckldq   mm2,            mm5;
						psrlw       mm1,            8;
						punpckhwd   mm2,            mm2;            //mm2 = (454/256)*(U1 U1 U0 U0)
						paddsw      mm2,            mm1
						paddsw      mm3,            mm1
						paddsw      mm4,            mm1;
						packuswb    mm2,            mm3;            //mm2 = G3 G2 G1 G0 B3 B2 B1 B0
						packuswb    mm3,            mm4;            //mm3 = R3 R2 R1 R0 G3 G2 G1 G0
						punpcklbw   mm2,            mm3;            //mm2 = G3 B3 G2 B2 G1 B1 G0 B0
						punpckhbw   mm3,            mm3;            //mm3 = R3 R3 R2 R2 R1 R1 R0 R0
						movq        mm4,            mm2;            //mm4 = G3 B3 G2 B2 G1 B1 G0 B0
						punpcklwd   mm2,            mm3;            //mm2 = R1 R1 G1 B1 R0 R0 G0 B0
						punpckhwd   mm4,            mm3;            //mm4 = R3 R3 G3 B3 R2 R2 G2 B2
						movd        [edi+edx],      mm2;
						movd        [edi+edx+6],    mm4;
						psrlq       mm2,            24;
						psrlq       mm4,            24;
						movd        [edi+edx+2],    mm2;
						movd        [edi+edx+8],    mm4;
						//-------------------------------------------------------------
						punpckhwd   mm5,            mm5;            //mm5 = (454/256)*(U3 U3 U2 U2)
						punpckhwd   mm6,            mm6;            //mm6 = -(88/256)*(U3 U3 U2 U2)-(183/256)*(V3 V3 V2 V2)
						punpckhwd   mm7,            mm7;            //mm7 = (359/256)*(V3 V3 V2 V2)
						mov         eax,            [esi+ebx-4];
						bswap       eax;
						movd        mm0,            eax;
						punpcklbw   mm0,            mm0;
						psrlw       mm0,            8;
						movq        mm1,            mm0;
						movq        mm2,            mm0;            //mm0 = mm1 = mm2 = Y7 Y6 Y5 Y4
						paddsw      mm0,            mm5;
						paddsw      mm1,            mm6;
						paddsw      mm2,            mm7;
						packuswb    mm0,            mm1;            //mm0 = G7 G6 G5 G4 B7 B6 B5 B4
						packuswb    mm1,            mm2;            //mm1 = R7 R6 R5 R4 G7 G6 G5 G4
						punpcklbw   mm0,            mm1;            //mm0 = G7 B7 G6 B6 G5 B5 G4 B4
						punpckhbw   mm1,            mm1;            //mm1 = R7 R7 R6 R6 R5 R5 R4 R4
						movq        mm2,            mm0;            //mm2 = G7 B7 G6 B6 G5 B5 G4 B4
						punpcklwd   mm0,            mm1;            //mm0 = R5 R5 G5 B5 R4 R4 G4 B4
						punpckhwd   mm2,            mm1;            //mm2 = R7 R7 G7 B7 R6 R6 G6 B6
						movd        [edi+12],       mm0;
						movd        [edi+18],       mm2;
						psrlq       mm0,            24;
						psrlq       mm2,            24;
						movd        [edi+14],       mm0;
						movd        [edi+20],       mm2;
						//-------------------------------------------------------------
						mov         eax,            [esi+ecx-4];
						bswap       eax;
						movd        mm0,            eax;
						punpcklbw   mm0,            mm0;
						psrlw       mm0,            8;
						paddsw      mm5,            mm0
						paddsw      mm6,            mm0
						paddsw      mm7,            mm0;
						packuswb    mm5,            mm6;            //mm5 = G7 G6 G5 G4 B7 B6 B5 B4
						packuswb    mm6,            mm7;            //mm6 = R7 R6 R5 R4 G7 G6 G5 G4
						punpcklbw   mm5,            mm6;            //mm5 = G7 B7 G6 B6 G5 B5 G4 B4
						punpckhbw   mm6,            mm6;            //mm6 = R7 R7 R6 R6 R5 R5 R4 R4
						movq        mm7,            mm5;            //mm7 = G7 B7 G6 B6 G5 B5 G4 B4
						punpcklwd   mm5,            mm6;            //mm5 = R5 R5 G5 B5 R4 R4 G4 B4
						punpckhwd   mm7,            mm6;            //mm7 = R7 R7 G7 B7 R6 R6 G6 B6
						movd        [edi+edx+12],   mm5;
						movd        [edi+edx+18],   mm7;
						psrlq       mm5,            24;
						psrlq       mm7,            24;
						movd        [edi+edx+14],   mm5;
						movd        [edi+edx+20],   mm7;
						//-------------------------------------------------------------
						pop         eax;
						pop         ecx;
						lea         esi,            [esi+8];
						lea         eax,            [eax+4];
						lea         edi,            [edi-24];
						dec         ecx;
						jne         YUV2RGB24_M;
						emms;
					}
				}
			}
		}
		else
		{
			const UInt64 BG_  = 0xff49ffa8000001c6;
			const UInt64 R_   = 0x0000000001670000;
			__asm
			{
				movd        mm2,            iOff0;
				movd        mm3,            iOff1;
				movd        mm4,            iOff2;
				movd        mm5,            iOff3;
				emms;
			}

			for (pYLineStart=pYFrameStart, pCLineStart=pCFrameStart;
				pYLineStart<pYFrameEnd;
				pYLineStart+=iYLineStep, pCLineStart+=iCLineStep, pBGRLineStart+=iBGRLineStep)
			{
				UInt8 *pBGR0 = pBGRLineStart;
				UInt8 *pYLineEnd = pYLineStart +iHoSize*s32StepH;

				if ((pShellParam->eRotateState == rs90) || (pShellParam->eRotateState == rs270))
					pYLineEnd -= iYMiddleOffset*2;

				UInt8 *pY, *pC;
				for (pY=pYLineStart, pC=pCLineStart; pY<pYLineEnd;
					pY+=iYPixStep, pC+=iCPixStep, pBGR0+=iBGRPixStep)
				{
					__asm
					{
						mov         esi,            pC;
						mov         edx,            iCBufSize;
						mov         al,             [esi];
						mov         ah,             [esi+edx];  //eax = X X V U
						movd        mm6,            eax;
						punpcklbw   mm7,            mm6;
						pcmpeqw     mm6,            mm6;
						punpckldq   mm7,            mm7;
						psllw       mm6,            15;
						paddw       mm7,            mm6;
						movq        mm6,            mm7;        //mm6 = mm7 = V U V U
						pmaddwd     mm7,            R_;         //mm7 = X         359V 
						pmaddwd     mm6,            BG_;        //mm6 = -88U-183V 454U
						psrad       mm7,            16;
						psrad       mm6,            16;
						packssdw    mm6,            mm7;
						movq        mm7,            mm6;
						psllq       mm6,            16;         //mm6 = 89V -22U-45V 113U 0
						//-------------------------------------------------------------
						mov         esi,            pY;
						mov         edi,            pBGR0;
						movd        ebx,            mm2;
						movd        ecx,            mm3;
						mov         edx,            iBGRLineSize;
						punpcklbw   mm0,            [esi+ebx];
						punpcklbw   mm1,            [esi+ecx];
						punpcklwd   mm0,            mm0;
						punpcklwd   mm1,            mm1;
						punpckldq   mm0,            mm0;        //mm0 = Y0 Y0 Y0 Y0
						punpckldq   mm1,            mm1;        //mm1 = Y1 Y1 Y1 Y1
						psrlw       mm0,            8;
						psrlw       mm1,            8;
						paddsw      mm0,            mm6;
						paddsw      mm1,            mm7;
						packuswb    mm0,            mm1;
						psrlq       mm0,            8;
						movd        [edi],          mm0;
						psrlq       mm0,            16;
						movd        [edi+2],        mm0;
						//-------------------------------------------------------------
						movd        ebx,            mm4;
						movd        ecx,            mm5;
						punpcklbw   mm0,            [esi+ebx];
						punpcklbw   mm1,            [esi+ecx];
						punpcklwd   mm0,            mm0;
						punpcklwd   mm1,            mm1;
						punpckldq   mm0,            mm0;        //mm0 = Y2 Y2 Y2 Y2
						punpckldq   mm1,            mm1;        //mm1 = Y3 Y3 Y3 Y3
						psrlw       mm0,            8;
						psrlw       mm1,            8;
						paddsw      mm0,            mm6;
						paddsw      mm1,            mm7;
						packuswb    mm0,            mm1;
						psrlq       mm0,            8;
						movd        [edi+edx],      mm0;
						psrlq       mm0,            16;
						movd        [edi+edx+2],    mm0;
						emms;
					}
				}
			}
		}
	}
	#else
	{
		for (pYLineStart=pYFrameStart, pCLineStart=pCFrameStart;
			pYLineStart<pYFrameEnd;
			pYLineStart+=iYLineStep, pCLineStart+=iCLineStep, pBGRLineStart+=iBGRLineStep)
		{
			UInt8 *pBGR0 = pBGRLineStart;
			UInt8 *pYLineEnd = pYLineStart +iHoSize*s32StepH;

			if ((pShellParam->eRotateState == rs90) || (pShellParam->eRotateState == rs270))
				pYLineEnd -= iYMiddleOffset*2;
            
			UInt8 *pY, *pC;
			for (pY=pYLineStart, pC=pCLineStart; pY<pYLineEnd;
				pY+=iYPixStep, pC+=iCPixStep, pBGR0+=iBGRPixStep)
			{
				UInt8 *pBGR1 = pBGR0 +iBGRLineSize;
				int iB, iG, iR, iB_, iR_;
				int iG_ = (iB_=pC[0]-128)*176 + (iR_=pC[iCBufSize]-128)*365;
				iB_ *= 907;
				iR_ *= 717;

				iB = iG = iR = pY[iOff0] <<9;
				iB += iB_;
				iG -= iG_;
				iR += iR_;
				pBGR0[0] = (UInt8)((iB>130560)? 255: ((iB<0)? 0: iB>>9));
				pBGR0[1] = (UInt8)((iG>130560)? 255: ((iG<0)? 0: iG>>9));
				pBGR0[2] = (UInt8)((iR>130560)? 255: ((iR<0)? 0: iR>>9));

				iB = iG = iR = pY[iOff1] <<9;
				iB += iB_;
				iG -= iG_;
				iR += iR_;
				pBGR0[3] = (UInt8)((iB>130560)? 255: ((iB<0)? 0: iB>>9));
				pBGR0[4] = (UInt8)((iG>130560)? 255: ((iG<0)? 0: iG>>9));
				pBGR0[5] = (UInt8)((iR>130560)? 255: ((iR<0)? 0: iR>>9));

				iB = iG = iR = pY[iOff2] <<9;
				iB += iB_;
				iG -= iG_;
				iR += iR_;
				pBGR1[0] = (UInt8)((iB>130560)? 255: ((iB<0)? 0: iB>>9));
				pBGR1[1] = (UInt8)((iG>130560)? 255: ((iG<0)? 0: iG>>9));
				pBGR1[2] = (UInt8)((iR>130560)? 255: ((iR<0)? 0: iR>>9));

				iB = iG = iR = pY[iOff3] <<9;
				iB += iB_;
				iG -= iG_;
				iR += iR_;
				pBGR1[3] = (UInt8)((iB>130560)? 255: ((iB<0)? 0: iB>>9));
				pBGR1[4] = (UInt8)((iG>130560)? 255: ((iG<0)? 0: iG>>9));
				pBGR1[5] = (UInt8)((iR>130560)? 255: ((iR<0)? 0: iR>>9));
			}
		}
	}
	#endif
}
//---------------------------------------------------------------------------

void CShell_YUV2I420(CShell *mp)
{
	ShellParam *pShellParam = &mp->Param;
	ScaleParam *pScaleParam = CShell_GetScaleParam(mp);

	BOOL bHMirror = pShellParam->bHMirror;
	BOOL bVMirror = pShellParam->bVMirror;
	RotateState eRotateState = pShellParam->eRotateState;
	if (eRotateState == rs270)          //-90
		bVMirror = 1-bVMirror;
	else if(eRotateState == rs180)      //180
	{
		bVMirror = 1-bVMirror;
		bHMirror = 1-bHMirror;
	}
	else if(eRotateState == rs90)      //90
		bHMirror = 1-bHMirror;

	int iBufHSize  = pScaleParam->iBufHSize;
	int iBufVSize  = pScaleParam->iBufVSize;
	int iHiStart = pScaleParam->iHStart;
	int iViStart = pScaleParam->iVStart;
	int iHoSize = pScaleParam->iHoSize;
	int iVoSize = pScaleParam->iVoSize;

	int iYBufSize = iBufHSize *iBufVSize;
	int iCBufSize = iYBufSize /4;

	int iCOutSize  = iHoSize *iVoSize /4;
	int iYLineSize = iBufHSize;
	int iILineSize = iHoSize;

	int	a32SWSubSample[2];
	a32SWSubSample[0]	= pScaleParam->a32SWSubSample[0];
	a32SWSubSample[1]	= pScaleParam->a32SWSubSample[1];
	int	s32StepH	= 1 << a32SWSubSample[0];
	int	s32StepV	= 1 << a32SWSubSample[1];
	int iYPixStep = s32StepH << 1;
	int iCPixStep = s32StepH;
	int iYLineStep = iBufHSize * (s32StepV << 1);
	int iCLineStep = iBufHSize/2 *s32StepV;
	int iILineStep = (bVMirror)? (-(iHoSize<<1)): (iHoSize<<1);
	int iULineStep = (bVMirror)? (-(iHoSize>>1)): (iHoSize>>1);
	int iIPixStep = (bHMirror)? -2: 2;
	int iUPixStep = (bHMirror)? -1: 1;
	
	UInt8 *pYFrameStart = mp->pProcBuf +iViStart*iBufHSize +iHiStart;
	UInt8 *pYFrameEnd   = pYFrameStart +iBufHSize*iVoSize* s32StepV;
	UInt8 *pCFrameStart = mp->pProcBuf +iYBufSize +(iViStart>>1)*(iBufHSize>>1) +(iHiStart>>1);
	UInt8 *pILineStart = mp->pDisBuf;
	UInt8 *pULineStart = pILineStart +iHoSize*iVoSize;
	
	int iYMiddleOffset = (iHoSize-iVoSize) * s32StepH /2;
	int iCMiddleOffset = (iHoSize-iVoSize) * s32StepH /2 /2;
	int iIMiddleOffset = (iHoSize-iVoSize) /2;
	int iUMiddleOffset = (iHoSize-iVoSize) /2 /2;

	if ((pShellParam->eRotateState == rs90) || (pShellParam->eRotateState == rs270))
	{
		pYFrameStart  += iYMiddleOffset;
		pCFrameStart  += iCMiddleOffset;
		if (!bHMirror)
		{
			pILineStart += iIMiddleOffset;
			pULineStart += iUMiddleOffset;
		}
		else
		{
			pILineStart -= iIMiddleOffset;
			pULineStart -= iUMiddleOffset;
		}
	}

	UInt8 *pYLineStart, *pCLineStart;
	int iOff0, iOff1, iOff2, iOff3;
	if ((a32SWSubSample[0] == 0) && (a32SWSubSample[1] == 0))
	{
		#if defined(__MMX__)
		{
			pILineStart += ((bVMirror)? (iVoSize-2)*iHoSize: 0);
			pULineStart += ((bVMirror)? (iVoSize/2-1)*(iHoSize/2): 0);
			pILineStart += ((bHMirror)? (iHoSize-8): 0);
			pULineStart += ((bHMirror)? (iHoSize/2-4): 0);

			if (!bVMirror && !bHMirror)
			{
				iOff0 = 0;
				iOff2 = iYLineSize;
			}
			else if (!bVMirror && bHMirror)
			{
				iOff0 = 4;
				iOff2 = iYLineSize+4;
			}
			else if (bVMirror && !bHMirror)
			{
				iOff0 = iYLineSize;
				iOff2 = 0;
			}
			else if (bVMirror && bHMirror)
			{
				iOff0 = iYLineSize+4;
				iOff2 = 4;
			}
		}
		#else
		{
			pILineStart += ((bVMirror)? (iVoSize-2)*iHoSize: 0);
			pULineStart += ((bVMirror)? (iVoSize/2-1)*(iHoSize/2): 0);
			pILineStart += ((bHMirror)? (iHoSize-2): 0);
			pULineStart += ((bHMirror)? (iHoSize/2-1): 0);

			if (!bVMirror && !bHMirror)
			{
				iOff0 = 0;
				iOff1 = 1;
				iOff2 = iYLineSize;
				iOff3 = iYLineSize+1;
			}
			else if (!bVMirror && bHMirror)
			{
				iOff0 = 1;
				iOff1 = 0;
				iOff2 = iYLineSize+1;
				iOff3 = iYLineSize;
			}
			else if (bVMirror && !bHMirror)
			{
				iOff0 = iYLineSize;
				iOff1 = iYLineSize+1;
				iOff2 = 0;
				iOff3 = 1;
			}
			else if (bVMirror && bHMirror)
			{
				iOff0 = iYLineSize+1;
				iOff1 = iYLineSize;
				iOff2 = 1;
				iOff3 = 0;
			}
		}
		#endif
	}
	else
	{
		pILineStart += ((bVMirror)? (iVoSize-2)*iHoSize: 0);
		pULineStart += ((bVMirror)? (iVoSize/2-1)*(iHoSize/2): 0);
		pILineStart += ((bHMirror)? (iHoSize-2): 0);
		pULineStart += ((bHMirror)? (iHoSize/2-1): 0);
		if (!bVMirror && !bHMirror)
		{
			iOff0 = 0;
			iOff1 = s32StepH;
			iOff2 = iYLineSize*s32StepV;
			iOff3 = iYLineSize*s32StepV+s32StepH;
		}
		else if (!bVMirror && bHMirror)
		{
			iOff0 = s32StepH;
			iOff1 = 0;
			iOff2 = iYLineSize*s32StepV+s32StepH;
			iOff3 = iYLineSize*s32StepV;
		}
		else if (bVMirror && !bHMirror)
		{
			iOff0 = iYLineSize*s32StepV;
			iOff1 = iYLineSize*s32StepV+s32StepH;
			iOff2 = 0;
			iOff3 = s32StepH;
		}
		else if (bVMirror && bHMirror)
		{
			iOff0 = iYLineSize*s32StepV+s32StepH;
			iOff1 = iYLineSize*s32StepV;
			iOff2 = s32StepH;
			iOff3 = 0;
		}
	}

	#if defined(__MMX__)
	{
		if ((a32SWSubSample[0] == 0) && (a32SWSubSample[1] == 0))
		{
			__asm
			{
				movd        mm3,            iOff0;
				movd        mm4,            iOff2;
				movd        mm5,            iILineSize;
				movd        mm6,            iCBufSize;
				movd        mm7,            iCOutSize;
				emms;
			}

			int iTempHoSize = iHoSize;

			if ((pShellParam->eRotateState == rs90) || (pShellParam->eRotateState == rs270))
				iTempHoSize = iVoSize;
			if (!bHMirror)
			{
				for (pYLineStart=pYFrameStart, pCLineStart=pCFrameStart;
					pYLineStart<pYFrameEnd;
					pYLineStart+=iYLineStep, pCLineStart+=iCLineStep,
					pILineStart+=iILineStep, pULineStart+=iULineStep)
				{
					__asm
					{
						mov         ecx,            iTempHoSize;
						mov         esi,            pYLineStart;
						mov         edi,            pILineStart;
						shr         ecx,            3;
						mov         eax,            pCLineStart;
						mov         ebx,            pULineStart;
					YUV2I420_M_:
						movd        mm2,            ecx;
						//-------------------------------------------------------------
						movd        edx,            mm3;
						movd        ecx,            mm4;
						movq        mm0,            [esi+edx];
						movq        mm1,            [esi+ecx];
						movd        edx,            mm5;
						movq        [edi],          mm0;
						movq        [edi+edx],      mm1;
						movd        ecx,            mm6;
						movd        edx,            mm7;
						movd        mm0,            [eax];
						movd        mm1,            [eax+ecx];
						movd        [ebx],          mm0;
						movd        [ebx+edx],      mm1;
						//-------------------------------------------------------------
						lea         esi,            [esi+8];
						lea         edi,            [edi+8];
						lea         eax,            [eax+4];
						lea         ebx,            [ebx+4];
						movd        ecx,            mm2;
						dec         ecx;
						jne         YUV2I420_M_;
						emms;
					}
				}
			}
			else
			{
				for (pYLineStart=pYFrameStart, pCLineStart=pCFrameStart;
					pYLineStart<pYFrameEnd;
					pYLineStart+=iYLineStep, pCLineStart+=iCLineStep,
					pILineStart+=iILineStep, pULineStart+=iULineStep)
				{
					__asm
					{
						mov         ecx,            iTempHoSize;
						mov         esi,            pYLineStart;
						mov         edi,            pILineStart;
						shr         ecx,            3;
						mov         eax,            pCLineStart;
						mov         ebx,            pULineStart;
					YUV2I420_M:
						movd        mm2,            ecx;
						//-------------------------------------------------------------
						movd        edx,            mm3;
						mov         ecx,            [esi+edx-4];
						mov         edx,            [esi+edx];
						bswap       ecx;
						bswap       edx;
						mov         [edi+4],        ecx;
						mov         [edi],          edx;
						//-------------------------------------------------------------
						movd        edx,            mm4;
						mov         ecx,            [esi+edx-4];
						mov         edx,            [esi+edx];
						bswap       ecx;
						bswap       edx;
						movd        mm1,            ecx;
						movd        ecx,            mm5;
						mov         [edi+ecx],      edx;
						movd        [edi+ecx+4],    mm1;
						//-------------------------------------------------------------
						movd        edx,            mm6;
						mov         ecx,            [eax];
						mov         edx,            [eax+edx];
						bswap       ecx;
						bswap       edx;
						mov         [ebx],          ecx;
						movd        ecx,            mm7;
						mov         [ebx+ecx],      edx;
						//-------------------------------------------------------------
						lea         esi,            [esi+8];
						lea         edi,            [edi-8];
						lea         eax,            [eax+4];
						lea         ebx,            [ebx-4];
						movd        ecx,            mm2;
						dec         ecx;
						jne         YUV2I420_M;
						emms;
					}
				}
			}
		}
		else
		{
			for (pYLineStart=pYFrameStart, pCLineStart=pCFrameStart;
				pYLineStart<pYFrameEnd;
				pYLineStart+=iYLineStep, pCLineStart+=iCLineStep,
				pILineStart+=iILineStep, pULineStart+=iULineStep)
			{
				UInt8 *pI0 = pILineStart;
				UInt8 *pU  = pULineStart;
				UInt8 *pYLineEnd = pYLineStart +iHoSize*s32StepH;

				if ((pShellParam->eRotateState == rs90) || (pShellParam->eRotateState == rs270))
					pYLineEnd -= iYMiddleOffset*2;

				UInt8 *pY, *pC;
				for (pY=pYLineStart, pC=pCLineStart; pY<pYLineEnd;
					pY+=iYPixStep, pC+=iCPixStep, pI0+=iIPixStep, pU+=iUPixStep)
				{
					UInt8 *pI1 = pI0 + iILineSize;
					pI0[0] = pY[iOff0];
					pI0[1] = pY[iOff1];
					pI1[0] = pY[iOff2];
					pI1[1] = pY[iOff3];
					pU[0]         = pC[0];
					pU[iCOutSize] = pC[iCBufSize];
				}
			}
		}
	}
	#else//*/
	{
		for (pYLineStart=pYFrameStart, pCLineStart=pCFrameStart;
			pYLineStart<pYFrameEnd;
			pYLineStart+=iYLineStep, pCLineStart+=iCLineStep,
			pILineStart+=iILineStep, pULineStart+=iULineStep)
		{
			UInt8 *pI0 = pILineStart;
			UInt8 *pU  = pULineStart;
			//2010/10/27 04:23下午
			UInt8 *pYLineEnd = pYLineStart +iHoSize*s32StepH;
			//UInt8 *pYLineEnd = pYLineStart +iHoSize*iPixStep;

			if ((pShellParam->eRotateState == rs90) || (pShellParam->eRotateState == rs270))
				pYLineEnd -= iYMiddleOffset*2;
            
			UInt8 *pY, *pC;
			for (pY=pYLineStart, pC=pCLineStart; pY<pYLineEnd;
				pY+=iYPixStep, pC+=iCPixStep, pI0+=iIPixStep, pU+=iUPixStep)
			{
				UInt8 *pI1 = pI0 + iILineSize;
				pI0[0] = pY[iOff0];
				pI0[1] = pY[iOff1];
				pI1[0] = pY[iOff2];
				pI1[1] = pY[iOff3];
				pU[0]         = pC[0];
				pU[iCOutSize] = pC[iCBufSize];
			}
		}
	}
	#endif
}
//---------------------------------------------------------------------------

// shawn 2011/07/12 use like driver version +++++
void CShell_YUV2YUY2(CShell *mp)
{
	ShellParam *pShellParam = &mp->Param;
	ScaleParam *pScaleParam = CShell_GetScaleParam(mp);

	int	a32SWSubSample[2];
	a32SWSubSample[0]	= pScaleParam->a32SWSubSample[0];
	a32SWSubSample[1]	= pScaleParam->a32SWSubSample[1];
	int	s32StepH	= 1 << a32SWSubSample[0];
	int	s32StepV	= 1 << a32SWSubSample[1];
	BOOL bHMirror = pShellParam->bHMirror;
	BOOL bVMirror = pShellParam->bVMirror;
	RotateState eRotateState = pShellParam->eRotateState;
	if (eRotateState == rs270)          //-90
		bVMirror = 1-bVMirror;
	else if(eRotateState == rs180)      //180
	{
		bVMirror = 1-bVMirror;
		bHMirror = 1-bHMirror;
	}
	else if(eRotateState == rs90)      //90
		bHMirror = 1-bHMirror;

	int iBufHSize  = pScaleParam->iBufHSize;
	int iBufVSize  = pScaleParam->iBufVSize;
	int iCBufHSize  = iBufHSize>>1;
	int iHiStart = pScaleParam->iHStart;
	int iViStart = pScaleParam->iVStart;
	int iHoSize = pScaleParam->iHoSize;
	int iVoSize = pScaleParam->iVoSize;

	int iYBufSize = iBufHSize *iBufVSize;
	int iCBufSize = iYBufSize /4;

	UInt8 *pYStart = mp->pProcBuf +iViStart*iBufHSize +iHiStart;
	UInt8 *pCbStart = mp->pProcBuf +iYBufSize +(iViStart/2)*(iBufHSize/2) +(iHiStart/2);
	UInt8 *pCrStart = mp->pProcBuf +iYBufSize +iCBufSize +(iViStart/2)*(iBufHSize/2) +(iHiStart/2);
	UInt8 *pYUY2Start = mp->pDisBuf;


	int iHoStart = 0;
	int iHoEnd = iHoSize - 1;
	if ((pShellParam->eRotateState == rs90) || (pShellParam->eRotateState == rs270))
	{
		iHoStart = (iHoSize - iVoSize) / 2;
		iHoEnd = iHoStart + iVoSize - 1;
	}

	if(!bHMirror & !bVMirror)
	{
		for (int j=0; j<iVoSize; j++)
		{
			for (int i=0; i<iHoSize; i+=2)
			{
				if ((i < iHoStart) || (i > iHoEnd))
				{
					*pYUY2Start++ = 0;
					*pYUY2Start++ = 127;
					*pYUY2Start++ = 0;
					*pYUY2Start++ = 127;
				}
				else
				{
					*pYUY2Start++ = pYStart[j*iBufHSize * s32StepV + i * s32StepH];
					*pYUY2Start++ = pCbStart[(j>>1)*iCBufHSize * s32StepV + (i>>1) * s32StepH];
					*pYUY2Start++ = pYStart[(j*iBufHSize* s32StepV + i * s32StepH)+s32StepH];
					*pYUY2Start++ = pCrStart[(j>>1)*iCBufHSize * s32StepV + (i>>1) * s32StepH];
				}
			}
		}
	}
	else if(bHMirror & !bVMirror)
	{
		for (int j=0; j<iVoSize; j++)
		{
			for (int i=iHoSize-2; i>=0; i-=2)
			{
				if ((i < iHoStart) || (i > iHoEnd))
				{
					*pYUY2Start++ = 0;
					*pYUY2Start++ = 127;
					*pYUY2Start++ = 0;
					*pYUY2Start++ = 127;
				}
				else
				{
					*pYUY2Start++ = pYStart[(j*iBufHSize * s32StepV + i * s32StepH)+s32StepH];
					*pYUY2Start++ = pCbStart[(j>>1)*iCBufHSize * s32StepV + (i>>1) * s32StepH];
					*pYUY2Start++ = pYStart[j*iBufHSize * s32StepV + i * s32StepH];
					*pYUY2Start++ = pCrStart[(j>>1)*iCBufHSize * s32StepV + (i>>1) * s32StepH];
				}
			}
		}
	}
	else if(!bHMirror & bVMirror)
	{
		for (int j=iVoSize-1; j>=0; j--)
		{
			for (int i=0; i<iHoSize; i+=2)
			{
				if ((i < iHoStart) || (i > iHoEnd))
				{
					*pYUY2Start++ = 0;
					*pYUY2Start++ = 127;
					*pYUY2Start++ = 0;
					*pYUY2Start++ = 127;
				}
				else
				{
					*pYUY2Start++ = pYStart[j*iBufHSize * s32StepV + i * s32StepH];
					*pYUY2Start++ = pCbStart[(j>>1)*iCBufHSize * s32StepV + (i>>1) * s32StepH];
					*pYUY2Start++ = pYStart[(j*iBufHSize * s32StepV + i * s32StepH)+s32StepH];
					*pYUY2Start++ = pCrStart[(j>>1)*iCBufHSize * s32StepV + (i>>1) * s32StepH];
				}
			}
		}
	}
	else
	{
		for (int j=iVoSize-1; j>=0; j--)
		{
			for (int i=iHoSize-2; i>=0; i-=2)
			{
				if ((i < iHoStart) || (i > iHoEnd))
				{
					*pYUY2Start++ = 0;
					*pYUY2Start++ = 127;
					*pYUY2Start++ = 0;
					*pYUY2Start++ = 127;
				}
				else
				{
					*pYUY2Start++ = pYStart[(j*iBufHSize * s32StepV + i * s32StepH)+s32StepH];
					*pYUY2Start++ = pCbStart[(j>>1)*iCBufHSize * s32StepV + (i>>1) * s32StepH];
					*pYUY2Start++ = pYStart[j*iBufHSize * s32StepV + i * s32StepH];
					*pYUY2Start++ = pCrStart[(j>>1)*iCBufHSize * s32StepV + (i>>1) * s32StepH];
				}
			}
		}
	}
}

//void CShell_YUV2YUY2(CShell *mp)
//{	
//	ShellParam *pShellParam = &mp->Param;
//	ScaleParam *pScaleParam = CShell_GetScaleParam(mp);
//	
//	int iSWSubSample = pScaleParam->a32SWSubSample[0];
//
//	BOOL bHMirror = pShellParam->bHMirror;
//	BOOL bVMirror = pShellParam->bVMirror;
//	RotateState eRotateState = pShellParam->eRotateState;
//	if (eRotateState == rs270)          //-90
//		bVMirror = 1-bVMirror;
//	else if(eRotateState == rs180)      //180
//	{
//		bVMirror = 1-bVMirror;
//		bHMirror = 1-bHMirror;
//	}
//	else if(eRotateState == rs90)      //90
//		bHMirror = 1-bHMirror;
//	
//	int iBufHSize  = pScaleParam->iBufHSize;
//	int iBufVSize  = pScaleParam->iBufVSize;
//	int iCBufHSize  = iBufHSize>>1;
//	int iHiStart = pScaleParam->iHStart;
//	int iViStart = pScaleParam->iVStart;
//	int iHoSize = pScaleParam->iHoSize;
//	int iVoSize = pScaleParam->iVoSize;
//	
//	int iHeight = iBufVSize;//Shelley Add
//	int iWidth = iBufHSize;
//	int OriWidth = (iHoSize<<iSWSubSample);
//	int OriHeight = (iVoSize<<iSWSubSample);
//	int iWidthLeft = iBufHSize-OriWidth;
//	int iYBufSize = iBufHSize *iBufVSize;	
//	int iCBufSize = iYBufSize /4;
//	int iPixStep = 1<<iSWSubSample;
//	
//	UInt8 *pYStart = mp->pProcBuf +iViStart*iBufHSize +iHiStart;
//	UInt8 *pCbStart = mp->pProcBuf +iYBufSize +(iViStart/2)*(iBufHSize/2) +(iHiStart/2);
//	UInt8 *pCrStart = mp->pProcBuf +iYBufSize +iCBufSize +(iViStart/2)*(iBufHSize/2) +(iHiStart/2);
//	UInt8 *pYUY2Start = mp->pDisBuf;
//	UInt32 AddrY  = (UInt32)pYStart;
//	UInt32 AddrCb = (UInt32)pCbStart;
//	UInt32 AddrCr = (UInt32)pCrStart;
//	UInt32 AddrYUY2 = (UInt32)pYUY2Start;
//	
//	int iHoStart = 0;
//	int iHoEnd = iHoSize - 1;
//	if ((pShellParam->eRotateState == rs90) || (pShellParam->eRotateState == rs270))
//	{
//		iHoStart = (iHoSize - iVoSize) / 2;
//		iHoEnd = iHoStart + iVoSize - 1;
//	}
//	
//	#if defined(__MMX__)//1:SSE2 or MMX 0:C
//	{
//		int iYUY2LineStep = iHoSize<<2;
//		SInt32 iVCnt = (iVoSize>>1);
//		
//		if (!iSWSubSample)//iSWSubSample is 0
//		{
//			int iYLineStart = ((bVMirror)? (OriHeight-1)*iWidth : 0);
//			int iCLineStart = ((bVMirror)? ((OriHeight>>1)-1)*(iWidth>>1) : 0);
//			if (!bHMirror)
//			{
//				int iYLineStep = ( (bVMirror)?   (-(iWidth+iHoEnd-iHoStart+1)) : iWidthLeft ) ;	
//				int iCLineStep = ( (bVMirror)?   (-((iWidth+iHoEnd-iHoStart+1)>>1)) : (iWidthLeft>>1) ) ;	
//			
//				AddrY += iYLineStart;
//				AddrCb += iCLineStart;					
//				AddrY += iHoStart;
//				AddrCb += (iHoStart>>1);
//				
//				int iCbCrWidth = (-((iHoEnd-iHoStart+1)>>1));
//				
//				if (gbSSE2 && (!(AddrYUY2&15)) && (!(iHoStart&31)) && (!(AddrY&15)) && (!(AddrCb&15)) )//1:SSE2 0:MMX
//				{
// 					SInt32 iHCnt = ((iHoEnd-iHoStart+1)>>5);
// 					SInt32 iHLoop = iHCnt;
//					SInt32 iHCnt_B = (iHoStart>>3);
// 					SInt32 iHLoop_B = iHCnt_B;
//					SInt32 iHCnt_B1 = iHCnt_B<<1;
// 					SInt32 iHLoop_B1 = iHCnt_B1;
//					
//					__asm
//					{
//						mov         esi,            AddrY;
//						mov         edx,            AddrCb;
//						mov         edi,            AddrYUY2;
//						mov         eax,            iCBufSize;
//I420toYUY2_V_SSE:
//						mov         ecx,            iHCnt_B;
//						test		ecx,			ecx;
//						je			I420toYUY2_H_E_SSE;						
//						mov         iHLoop_B,       ecx;
//I420toYUY2_H_L_SSE2:					
//						pcmpeqw     xmm0,           xmm0;			//xmm0:FFFFFFFF FFFFFFF FFFFFFFF FFFFFFFF	
//						psllw       xmm0,           15;				//xmm0:80008000 80008000 80008000 80008000
//						movdqa		[edi],			xmm0;
//						lea         edi,            [edi+0x10];
//						dec         iHLoop_B;
//						jne         I420toYUY2_H_L_SSE2;						
//
//I420toYUY2_H_E_SSE:							
//						movdqa		xmm0,			[esi];
//						movdqa		xmm1,			[esi+0x10];
//						movdqa		xmm2,			[edx];
//						movdqa		xmm3,			[edx+eax];				
//						
//						movdqa		xmm4,			xmm2;
//						movdqa		xmm5,			xmm0;
//						punpcklbw	xmm2,			xmm3;	
//						punpckhbw	xmm4,			xmm3;
//						punpcklbw	xmm0,			xmm2;
//						punpckhbw	xmm5,			xmm2;
//						movdqa		xmm6,			xmm1;
//						punpcklbw	xmm1,			xmm4;
//						punpckhbw	xmm6,			xmm4;
//						
//						movdqa		[edi],			xmm0;
//						movdqa		[edi+0x10],		xmm5;
//						movdqa		[edi+0x20],		xmm1;
//						movdqa		[edi+0x30],		xmm6;
//						
//						lea         esi,            [esi+0x20];
//						lea         edx,            [edx+0x10];
//						lea         edi,            [edi+0x40];
//						dec         iHLoop;
//						jne         I420toYUY2_H_E_SSE;
//
//						mov         ecx,            iHCnt_B1;
//						test		ecx,			ecx;
//						je			NextLine;
//						mov         iHLoop_B1,      ecx;
//I420toYUY2_H_R_SSE2:					
//						pcmpeqw     xmm0,           xmm0;			//xmm0:FFFFFFFF FFFFFFF FFFFFFFF FFFFFFFF	
//						psllw       xmm0,           15;				//xmm0:80008000 80008000 80008000 80008000
//						movdqa		[edi],			xmm0;
//						lea         edi,            [edi+0x10];					
//						dec         iHLoop_B1;
//						jne         I420toYUY2_H_R_SSE2;
//						//-----------------------------------------------------------------
//NextLine:				mov         ecx,            iHCnt;
//						mov         ebx,            iCbCrWidth;
//						lea         edx,            [edx+ebx];
//						mov         iHLoop,         ecx;
//						mov         ebx,            iYLineStep;
//						lea         esi,            [esi+ebx];
//						//-----------------------------------------------------------------
//I420toYUY2_H_O_SSE:
//						movdqa		xmm0,			[esi];
//						movdqa		xmm1,			[esi+0x10];
//						movdqa		xmm2,			[edx];
//						movdqa		xmm3,			[edx+eax];				
//						
//						movdqa		xmm4,			xmm2;
//						movdqa		xmm5,			xmm0;
//						punpcklbw	xmm2,			xmm3;	
//						punpckhbw	xmm4,			xmm3;
//						punpcklbw	xmm0,			xmm2;
//						punpckhbw	xmm5,			xmm2;
//						movdqa		xmm6,			xmm1;
//						punpcklbw	xmm1,			xmm4;
//						punpckhbw	xmm6,			xmm4;
//						
//						movdqa		[edi],			xmm0;
//						movdqa		[edi+0x10],		xmm5;
//						movdqa		[edi+0x20],		xmm1;
//						movdqa		[edi+0x30],		xmm6;
//						
//						lea         esi,            [esi+0x20];
//						lea         edx,            [edx+0x10];
//						lea         edi,            [edi+0x40];
//						dec         iHLoop;
//						jne         I420toYUY2_H_O_SSE;
//
//						mov         ecx,            iHCnt_B;
//						test		ecx,			ecx;
//						je			NextLine1;
//						mov         iHLoop_B,		ecx;
//I420toYUY2_H_OR_SSE2:					
//						pcmpeqw     xmm0,           xmm0;			//xmm0:FFFFFFFF FFFFFFF FFFFFFFF FFFFFFFF	
//						psllw       xmm0,           15;				//xmm0:80008000 80008000 80008000 80008000
//						movdqa		[edi],			xmm0;
//						lea         edi,            [edi+0x10];					
//						dec         iHLoop_B;
//						jne         I420toYUY2_H_OR_SSE2;
//						//-----------------------------------------------------------------							
//NextLine1:				mov         ebx,            iYLineStep;
//						lea         esi,            [esi+ebx];
//						mov         ebx,            iCLineStep;
//						lea         edx,            [edx+ebx];
//						mov         ecx,            iHCnt;
//						mov         iHLoop,         ecx;
//						dec         iVCnt;
//						jne         I420toYUY2_V_SSE;
//						
//						emms;
//					}
//				}//SSE2 END
//				else
//				{
//					SInt32 iHCnt = ((iHoEnd-iHoStart+1)>>4);
//					SInt32 iHLoop = iHCnt;
//					SInt32 iHCnt_B = (iHoStart>>2);
//					SInt32 iHLoop_B = iHCnt_B;
//					SInt32 iHCnt_B1 = iHCnt_B<<1;
//					SInt32 iHLoop_B1 = iHCnt_B1;
//
//					__asm
//					{
//						mov         esi,            AddrY;
//						mov         edx,            AddrCb;
//						mov         edi,            AddrYUY2;
//						mov         eax,            iCBufSize;
//I420toYUY2_V_MMX:
//						mov         ecx,            iHCnt_B;
//						test		ecx,			ecx;
//						je			I420toYUY2_H_E_MMX;
//						mov         iHLoop_B,       ecx;
//I420toYUY2_H_L_MMX:	//Rotate						
//						pcmpeqw     mm0,           mm0;			//xmm0:FFFFFFFF FFFFFFF	
//						psllw       mm0,           15;				//xmm0:80008000 80008000						
//						movq		[edi],			mm0;
//						lea         edi,            [edi+0x8];
//						dec         iHLoop_B;
//						jne         I420toYUY2_H_L_MMX;
//
//I420toYUY2_H_E_MMX:							
//						movq		mm0,			[esi];			//Y:0~7
//						movq		mm1,			[esi+0x08];		//Y:8~15
//						movq		mm2,			[edx];			//Cb:0~7
//						movq		mm3,			[edx+eax];		//Cr:0~7
//						
//						movq		mm4,			mm2;			//Cb:0~7
//						movq		mm5,			mm0;			//Y:0~7
//						punpcklbw	mm2,			mm3;			//Cr3Cb3 Cr2Cb2 Cr1Cb1 Cr0Cb0
//						punpckhbw	mm4,			mm3;			//Cr7Cb7 Cr6Cb6 Cr5Cb5 Cr4Cb4
//						punpcklbw	mm0,			mm2;			//Cr1Y3 Cb1Y2 Cr0Y1 Cb0Y0
//						punpckhbw	mm5,			mm2;			//Cr3Y7 Cb3Y6 Cr2Y5 Cb2Y4
//						movq		mm6,			mm1;			//Y:8~15
//						punpcklbw	mm1,			mm4;
//						punpckhbw	mm6,			mm4;
//						
//						movq		[edi],			mm0;
//						movq		[edi+0x08],		mm5;
//						movq		[edi+0x10],		mm1;
//						movq		[edi+0x18],		mm6;
//						
//						lea         esi,            [esi+0x10];
//						lea         edx,            [edx+0x08];
//						lea         edi,            [edi+0x20];
//						dec         iHLoop;
//						jne         I420toYUY2_H_E_MMX;
//
//						mov         ecx,            iHCnt_B1;
//						test		ecx,			ecx;
//						jna			NextLine2;
//						mov         iHLoop_B1,      ecx;
//I420toYUY2_H_R_MMX:	
//						pcmpeqw     mm0,           mm0;			//xmm0:FFFFFFFF FFFFFFF	
//						psllw       mm0,           15;				//xmm0:80008000 80008000						
//						movq		[edi],			mm0;
//						lea         edi,            [edi+0x8];			
//						dec         iHLoop_B1;
//						jne         I420toYUY2_H_R_MMX;
//
//						//-----------------------------------------------------------------
//NextLine2:				mov         ecx,            iHCnt;
//						mov         ebx,            iCbCrWidth;
//						lea         edx,            [edx+ebx];
//						mov         iHLoop,         ecx;
//						mov         ebx,            iYLineStep;
//						lea         esi,            [esi+ebx];
//						//-----------------------------------------------------------------
//I420toYUY2_H_O_MMX:
//						movq		mm0,			[esi];
//						movq		mm1,			[esi+0x08];
//						movq		mm2,			[edx];
//						movq		mm3,			[edx+eax];				
//						
//						movq		mm4,			mm2;
//						movq		mm5,			mm0;
//						punpcklbw	mm2,			mm3;	
//						punpckhbw	mm4,			mm3;
//						punpcklbw	mm0,			mm2;
//						punpckhbw	mm5,			mm2;
//						movq		mm6,			mm1;
//						punpcklbw	mm1,			mm4;
//						punpckhbw	mm6,			mm4;
//						
//						movq		[edi],			mm0;
//						movq		[edi+0x08],		mm5;
//						movq		[edi+0x10],		mm1;
//						movq		[edi+0x18],		mm6;
//						
//						lea         esi,            [esi+0x10];
//						lea         edx,            [edx+0x08];
//						lea         edi,            [edi+0x20];
//						dec         iHLoop;
//						jne         I420toYUY2_H_O_MMX;
//
//						mov         ecx,            iHCnt_B;
//						test		ecx,			ecx;
//						jna			NextLine3;
//						mov         iHLoop_B,		ecx;
//I420toYUY2_H_OR_MMX:
//						pcmpeqw     mm0,           mm0;			//xmm0:FFFFFFFF FFFFFFF	
//						psllw       mm0,           15;				//xmm0:80008000 80008000						
//						movq		[edi],			mm0;
//						lea         edi,            [edi+0x8];			
//						dec         iHLoop_B;
//						jne         I420toYUY2_H_OR_MMX;
//						//-----------------------------------------------------------------							
//NextLine3:				mov         ebx,            iYLineStep;
//						lea         esi,            [esi+ebx];
//						mov         ebx,            iCLineStep;
//						lea         edx,            [edx+ebx];
//						mov         ecx,            iHCnt;
//						mov         iHLoop,         ecx;
//						dec         iVCnt;
//						jne         I420toYUY2_V_MMX;
//						
//						emms;
//					}
//				}//MMX END
//			}
//			else//H Mirror
//			{
//				int iYLineStep = ( (bVMirror)?   (-(iWidthLeft+(iHoStart<<1))) : (iWidth+iHoEnd-iHoStart+1) ) ;
//				int iCLineStep = ( (bVMirror)?   (-((iWidthLeft>>1)+iHoStart)) : ((iWidth+iHoEnd-iHoStart+1)>>1) ) ;
//					
//				AddrY += (OriWidth);
//				AddrCb += ((OriWidth>>1));					
//				AddrY += iYLineStart;
//				AddrCb += iCLineStart;					
//				AddrY -= iHoStart;
//				AddrCb -= (iHoStart>>1);
//				
//				int iCbCrWidth = ((iHoEnd-iHoStart+1)>>1);
//
//				if (gbSSE2 && (!(AddrYUY2&15)) && (!(iHoStart&31)) && (!(AddrY&15)) && (!(AddrCb&15)) )//1:SSE2 0:MMX
//				{
//					SInt32 iHCnt = ((iHoEnd-iHoStart+1)>>5);
//					SInt32 iHLoop = iHCnt;
//					SInt32 iHCnt_B = (iHoStart>>3);
//					SInt32 iHLoop_B = iHCnt_B;
//					SInt32 iHCnt_B1 = (iHCnt_B<<1);
//					SInt32 iHLoop_B1 = iHCnt_B1;
//				
//					__asm
//					{
//						mov         esi,            AddrY;
//						mov         edx,            AddrCb;
//						mov         edi,            AddrYUY2;
//						mov         eax,            iCBufSize;
//I420toYUY2_HM_V_SSE2:
//						mov         ecx,            iHCnt_B;
//						test		ecx,			ecx;
//						je			I420toYUY2_HM_E_SSE2;
//						mov         iHLoop_B,       ecx;
//I420toYUY2_HM_L_SSE2:					
//						pcmpeqw     xmm0,           xmm0;			//xmm0:FFFFFFFF FFFFFFF FFFFFFFF FFFFFFFF	
//						psllw       xmm0,           15;				//xmm0:80008000 80008000 80008000 80008000
//						movdqa		[edi],			xmm0;
//						lea         edi,            [edi+0x10];
//						dec         iHLoop_B;
//						jne         I420toYUY2_HM_L_SSE2;		
//
//I420toYUY2_HM_E_SSE2:
//						movdqa		xmm0,			[edx+eax-0x10];			//Cr:0~15
//						movdqa		xmm1,			[edx-0x10];				//Cb:0~15
//						movdqa		xmm5,			[esi-0x10];				//Y:16~31
//						movdqa		xmm2,			[esi-0x20];				//Y:0~15
//						movdqa		xmm3,			xmm0;					//Cr:0~15
//						punpcklbw	xmm0,			xmm1;					//CbCr:0~7
//						punpckhbw	xmm3,			xmm1;					//CbCr:8~15
//						
//						movdqa		xmm4,			xmm2;					//Y:0~15
//						punpcklbw	xmm2,			xmm0;					//Cb3Y7Cr3Y6   Cb2Y5Cr2Y4   Cb1Y3Cr1Y2   Cb0Y1Cr0Y0
//						punpckhbw	xmm4,			xmm0;					//Cb7Y15Cr7Y14 Cb6Y13Cr6Y12 Cb5Y11Cr5Y10 Cb4Y9Cr4Y8
//						pshuflw		xmm2,			xmm2,		00011011b;	//Cb3Y7Cr3Y6   Cb2Y5Cr2Y4   Cr0Y0Cb0Y1   Cr1Y2Cb1Y3
//						pshuflw		xmm4,			xmm4,		00011011b;	//Cb7Y15Cr7Y14 Cb6Y13Cr6Y12 Cr4Y8Cb4Y9   Cr5Y10Cb5Y11 
//						pshufhw		xmm2,			xmm2,		00011011b;	//Cr2Y4Cb2Y5   Cr3Y6Cb3Y7   Cr0Y0Cb0Y1   Cr1Y2Cb1Y3
//						pshufhw		xmm4,			xmm4,		00011011b;	//Cr6Y12Cb6Y13 Cr7Y14Cb7Y15 Cr4Y8Cb4Y9   Cr5Y10Cb5Y11 
//						pshufd		xmm2,			xmm2,		01001110b;	//Cr0Y0Cb0Y1   Cr1Y2Cb1Y3   Cr2Y4Cb2Y5   Cr3Y6Cb3Y7   
//						pshufd		xmm4,			xmm4,		01001110b;	//Cr4Y8Cb4Y9   Cr5Y10Cb5Y11 Cr6Y12Cb6Y13 Cr7Y14Cb7Y15 
//						
//						movdqa		xmm6,			xmm5;					//Y:16~31
//						punpcklbw	xmm5,			xmm3;					//Cb11Y23Cr11Y22   Cb10Y21Cr10Y20   Cb9Y19Cr9Y18   Cb8Y17Cr8Y16
//						punpckhbw	xmm6,			xmm3;					//Cb15Y31Cr15Y30   Cb14Y29Cr14Y28   Cb13Y27Cr13Y26 Cb12Y25Cr12Y24
//						pshuflw		xmm5,			xmm5,		00011011b;	//Cb11Y23Cr11Y22   Cb10Y21Cr10Y20   Cr8Y16Cb8Y17   Cr9Y18Cb9Y19   
//						pshuflw		xmm6,			xmm6,		00011011b;	//Cb15Y31Cr15Y30   Cb14Y29Cr14Y28   Cr12Y24Cb12Y25 Cr13Y26Cb13Y27  
//						pshufhw		xmm5,			xmm5,		00011011b;	//Cr10Y20Cb10Y21   Cr11Y22Cb11Y23   Cr8Y16Cb8Y17   Cr9Y18Cb9Y19
//						pshufhw		xmm6,			xmm6,		00011011b;	//Cr14Y28Cb14Y29   Cr15Y30Cb15Y31   Cr12Y24Cb12Y25 Cr13Y26Cb13Y27 
//						pshufd		xmm5,			xmm5,		01001110b;	//Cr8Y16Cb8Y17     Cr9Y18Cb9Y19     Cr10Y20Cb10Y21 Cr11Y22Cb11Y23     
//						pshufd		xmm6,			xmm6,		01001110b;	//Cr12Y24Cb12Y25   Cr13Y26Cb13Y27   Cr14Y28Cb14Y29 Cr15Y30Cb15Y31
//						
//						movdqa		[edi],			xmm6;
//						movdqa		[edi+0x10],		xmm5;
//						movdqa		[edi+0x20],		xmm4;
//						movdqa		[edi+0x30],		xmm2;
//						
//						lea         esi,            [esi-0x20];
//						lea         edx,            [edx-0x10];
//						lea         edi,            [edi+0x40];
//						dec         iHLoop;
//						jne         I420toYUY2_HM_E_SSE2;
//
//						mov         ecx,            iHCnt_B1;
//						test		ecx,			ecx;
//						jna			NextLine4;
//						mov         iHLoop_B1,      ecx;
//I420toYUY2_HM_R_SSE2:					
//						pcmpeqw     xmm0,           xmm0;			//xmm7:FFFFFFFF FFFFFFF FFFFFFFF FFFFFFFF	
//						psllw       xmm0,           15;				//xmm7:80008000 80008000 80008000 80008000		
//						movdqa		[edi],			xmm0;
//						lea         edi,            [edi+0x10];					
//						dec         iHLoop_B1;
//						jne         I420toYUY2_HM_R_SSE2;
//						//-------------------------------------------------------------
//NextLine4:				mov         ecx,            iHCnt;
//						mov         ebx,            iCbCrWidth;
//						lea         edx,            [edx+ebx];
//						mov         iHLoop,         ecx;
//						mov         ebx,            iYLineStep;
//						lea         esi,            [esi+ebx];
//I420toYUY2_HM_O_SSE2:
//						movdqa		xmm0,			[edx+eax-0x10];			//Cr:0~15
//						movdqa		xmm1,			[edx-0x10];				//Cb:0~15
//						movdqa		xmm5,			[esi-0x10];				//Y:16~31
//						movdqa		xmm2,			[esi-0x20];				//Y:0~15
//						movdqa		xmm3,			xmm0;					//Cr:0~15
//						punpcklbw	xmm0,			xmm1;					//CbCr:0~7
//						punpckhbw	xmm3,			xmm1;					//CbCr:8~15
//						
//						movdqa		xmm4,			xmm2;					//Y:0~15
//						punpcklbw	xmm2,			xmm0;					//Cb3Y7Cr3Y6   Cb2Y5Cr2Y4   Cb1Y3Cr1Y2   Cb0Y1Cr0Y0
//						punpckhbw	xmm4,			xmm0;					//Cb7Y15Cr7Y14 Cb6Y13Cr6Y12 Cb5Y11Cr5Y10 Cb4Y9Cr4Y8
//						pshuflw		xmm2,			xmm2,		00011011b;	//Cb3Y7Cr3Y6   Cb2Y5Cr2Y4   Cr0Y0Cb0Y1   Cr1Y2Cb1Y3
//						pshuflw		xmm4,			xmm4,		00011011b;	//Cb7Y15Cr7Y14 Cb6Y13Cr6Y12 Cr4Y8Cb4Y9   Cr5Y10Cb5Y11 
//						pshufhw		xmm2,			xmm2,		00011011b;	//Cr2Y4Cb2Y5   Cr3Y6Cb3Y7   Cr0Y0Cb0Y1   Cr1Y2Cb1Y3
//						pshufhw		xmm4,			xmm4,		00011011b;	//Cr6Y12Cb6Y13 Cr7Y14Cb7Y15 Cr4Y8Cb4Y9   Cr5Y10Cb5Y11 
//						pshufd		xmm2,			xmm2,		01001110b;	//Cr0Y0Cb0Y1   Cr1Y2Cb1Y3   Cr2Y4Cb2Y5   Cr3Y6Cb3Y7   
//						pshufd		xmm4,			xmm4,		01001110b;	//Cr4Y8Cb4Y9   Cr5Y10Cb5Y11 Cr6Y12Cb6Y13 Cr7Y14Cb7Y15 
//						
//						movdqa		xmm6,			xmm5;					//Y:16~31
//						punpcklbw	xmm5,			xmm3;					//Cb11Y23Cr11Y22   Cb10Y21Cr10Y20   Cb9Y19Cr9Y18   Cb8Y17Cr8Y16
//						punpckhbw	xmm6,			xmm3;					//Cb15Y31Cr15Y30   Cb14Y29Cr14Y28   Cb13Y27Cr13Y26 Cb12Y25Cr12Y24
//						pshuflw		xmm5,			xmm5,		00011011b;	//Cb11Y23Cr11Y22   Cb10Y21Cr10Y20   Cr8Y16Cb8Y17   Cr9Y18Cb9Y19   
//						pshuflw		xmm6,			xmm6,		00011011b;	//Cb15Y31Cr15Y30   Cb14Y29Cr14Y28   Cr12Y24Cb12Y25 Cr13Y26Cb13Y27  
//						pshufhw		xmm5,			xmm5,		00011011b;	//Cr10Y20Cb10Y21   Cr11Y22Cb11Y23   Cr8Y16Cb8Y17   Cr9Y18Cb9Y19
//						pshufhw		xmm6,			xmm6,		00011011b;	//Cr14Y28Cb14Y29   Cr15Y30Cb15Y31   Cr12Y24Cb12Y25 Cr13Y26Cb13Y27 
//						pshufd		xmm5,			xmm5,		01001110b;	//Cr8Y16Cb8Y17     Cr9Y18Cb9Y19     Cr10Y20Cb10Y21 Cr11Y22Cb11Y23     
//						pshufd		xmm6,			xmm6,		01001110b;	//Cr12Y24Cb12Y25   Cr13Y26Cb13Y27   Cr14Y28Cb14Y29 Cr15Y30Cb15Y31
//						
//						movdqa		[edi],			xmm6;
//						movdqa		[edi+0x10],		xmm5;
//						movdqa		[edi+0x20],		xmm4;
//						movdqa		[edi+0x30],		xmm2;
//						
//						lea         esi,            [esi-0x20];
//						lea         edx,            [edx-0x10];
//						lea         edi,            [edi+0x40];
//						dec         iHLoop;
//						jne         I420toYUY2_HM_O_SSE2;
//
//						mov         ecx,            iHCnt_B;
//						test		ecx,			ecx;
//						jna			NextLine5;
//						mov         iHLoop_B,		ecx;
//I420toYUY2_HM_OR_SSE2:					
//						pcmpeqw     xmm0,           xmm0;			//xmm7:FFFFFFFF FFFFFFF FFFFFFFF FFFFFFFF	
//						psllw       xmm0,           15;				//xmm7:80008000 80008000 80008000 80008000
//						movdqa		[edi],			xmm0;
//						lea         edi,            [edi+0x10];					
//						dec         iHLoop_B;
//						jne         I420toYUY2_HM_OR_SSE2;
//						//-------------------------------------------------------------
//NextLine5:				mov         ebx,            iYLineStep;
//						lea         esi,            [esi+ebx];
//						mov         ebx,            iCLineStep;
//						lea         edx,            [edx+ebx];
//						mov         ecx,            iHCnt;
//						mov         iHLoop,         ecx;
//						dec         iVCnt;
//						jne         I420toYUY2_HM_V_SSE2;
//						
//						emms;
//						}
//					}//SSE2 END
//					else
//					{
//						SInt32 iHCnt = ((iHoEnd-iHoStart+1)>>4);
//						SInt32 iHLoop = iHCnt;						
//						SInt32 iHCnt_B = (iHoStart>>2);
//						SInt32 iHLoop_B = iHCnt_B;
//						SInt32 iHCnt_B1 = iHCnt_B<<1;
//						SInt32 iHLoop_B1 = iHCnt_B1;
//						
//						__asm
//						{
//							mov         esi,            AddrY;
//							mov         edx,            AddrCb;
//							mov         edi,            AddrYUY2;
//							mov         eax,            iCBufSize;
//I420toYUY2_HM_V_MMX:
//							mov         ecx,            iHCnt_B;
//							test		ecx,			ecx;
//							je			I420toYUY2_HM_E_MMX;
//							mov         iHLoop_B,       ecx;
//I420toYUY2_HM_L_MMX:	//Rotate	
//							pcmpeqw     mm0,           mm0;			//xmm0:FFFFFFFF FFFFFFF	
//							psllw       mm0,           15;				//xmm0:80008000 80008000
//							movq		[edi],			mm0;
//							lea         edi,            [edi+0x8];
//							dec         iHLoop_B;
//							jne         I420toYUY2_HM_L_MMX;
//I420toYUY2_HM_E_MMX:
//							mov         ebx,            [edx-0x4];	//Cb172 Cb173 Cb174 Cb175
//							mov         ecx,            [edx+eax-0x4];
//							bswap       ebx;						//Cb175 Cb174 Cb173 Cb172
//							bswap       ecx;
//							movd        mm0,            ebx;
//							movd        mm1,            ecx;
//							punpcklbw	mm0,			mm1;		//Cb175 Cr175 Cb174 Cr174 Cb173 Cr173 Cb172 Cr172
//							
//							mov         ebx,            [edx-0x8];	//Cb168 Cb169 Cb170 Cb171
//							mov         ecx,            [edx+eax-0x8];
//							bswap       ebx;					
//							bswap       ecx;
//							movd        mm2,            ebx;
//							movd        mm1,            ecx;
//							punpcklbw	mm2,			mm1;
//							
//							mov         ebx,            [esi-0x4];	//Y
//							mov         ecx,            [esi-0x8];
//							bswap       ebx;					
//							bswap       ecx;					
//							movd        mm1,            ebx;					
//							movd        mm3,            ecx;				
//							punpcklbw	mm1,			mm0;		//Low: Y Cb Y Cr
//							psllq		mm3,			32;
//							punpckhbw	mm3,			mm0;		//High: Y Cb Y Cr
//							
//							mov         ebx,            [esi-0xc];	//Y
//							mov         ecx,            [esi-0x10];
//							bswap       ebx;					
//							bswap       ecx;					
//							movd        mm4,            ebx;					
//							movd        mm5,            ecx;					
//							punpcklbw	mm4,			mm2;		//Low: Y Cb Y Cr
//							psllq		mm5,			32;
//							punpckhbw	mm5,			mm2;		//High: Y Cb Y Cr
//							
//							movq		[edi],			mm1;
//							movq		[edi+0x8],		mm3;
//							movq		[edi+0x10],		mm4;
//							movq		[edi+0x18],		mm5;
//							
//							lea         esi,            [esi-0x10];
//							lea         edx,            [edx-0x08];
//							lea         edi,            [edi+0x20];
//							dec         iHLoop;
//							jne         I420toYUY2_HM_E_MMX;
//
//							mov         ecx,            iHCnt_B1;
//							test		ecx,			ecx;
//							jna			NextLine6;
//							mov         iHLoop_B1,      ecx;
//I420toYUY2_HM_R_MMX:	
//							pcmpeqw     mm0,           mm0;			//xmm0:FFFFFFFF FFFFFFF	
//							psllw       mm0,           15;				//xmm0:80008000 80008000
//							movq		[edi],			mm0;
//							lea         edi,            [edi+0x8];			
//							dec         iHLoop_B1;
//							jne         I420toYUY2_HM_R_MMX;
//							//-------------------------------------------------------------
//NextLine6:					mov         ecx,            iHCnt;
//							mov         ebx,            iCbCrWidth;
//							lea         edx,            [edx+ebx];
//							mov         iHLoop,         ecx;
//							mov         ebx,            iYLineStep;
//							lea         esi,            [esi+ebx];
//I420toYUY2_HM_O_MMX:
//							mov         ebx,            [edx-0x4];	//Cb172 Cb173 Cb174 Cb175
//							mov         ecx,            [edx+eax-0x4];
//							bswap       ebx;						//Cb175 Cb174 Cb173 Cb172
//							bswap       ecx;
//							movd        mm0,            ebx;
//							movd        mm1,            ecx;
//							punpcklbw	mm0,			mm1;		//Cb175 Cr175 Cb174 Cr174 Cb173 Cr173 Cb172 Cr172
//							
//							mov         ebx,            [edx-0x8];	//Cb168 Cb169 Cb170 Cb171
//							mov         ecx,            [edx+eax-0x8];
//							bswap       ebx;					
//							bswap       ecx;
//							movd        mm2,            ebx;
//							movd        mm1,            ecx;
//							punpcklbw	mm2,			mm1;
//							
//							mov         ebx,            [esi-0x4];	//Y
//							mov         ecx,            [esi-0x8];
//							bswap       ebx;					
//							bswap       ecx;					
//							movd        mm1,            ebx;					
//							movd        mm3,            ecx;				
//							punpcklbw	mm1,			mm0;		//Low: Y Cb Y Cr
//							psllq		mm3,			32;
//							punpckhbw	mm3,			mm0;		//High: Y Cb Y Cr				
//							
//							mov         ebx,            [esi-0xc];	//Y
//							mov         ecx,            [esi-0x10];
//							bswap       ebx;					
//							bswap       ecx;					
//							movd        mm4,            ebx;					
//							movd        mm5,            ecx;					
//							punpcklbw	mm4,			mm2;		//Low: Y Cb Y Cr
//							psllq		mm5,			32;
//							punpckhbw	mm5,			mm2;		//High: Y Cb Y Cr
//							
//							movq		[edi],			mm1;
//							movq		[edi+0x8],		mm3;
//							movq		[edi+0x10],		mm4;
//							movq		[edi+0x18],		mm5;
//							
//							lea         esi,            [esi-0x10];
//							lea         edx,            [edx-0x08];
//							lea         edi,            [edi+0x20];
//							dec         iHLoop;
//							jne         I420toYUY2_HM_O_MMX;
//
//							mov         ecx,            iHCnt_B;
//							test		ecx,			ecx;
//							jna			NextLine7;
//							mov         iHLoop_B,		ecx;
//I420toYUY2_HM_OR_MMX:
//							pcmpeqw     mm0,           mm0;			//xmm0:FFFFFFFF FFFFFFF	
//							psllw       mm0,           15;				//xmm0:80008000 80008000
//							movq		[edi],			mm0;
//							lea         edi,            [edi+0x8];			
//							dec         iHLoop_B;
//							jne         I420toYUY2_HM_OR_MMX;
//							//-------------------------------------------------------------
//NextLine7:					mov         ebx,            iYLineStep;
//							lea         esi,            [esi+ebx];
//							mov         ebx,            iCLineStep;
//							lea         edx,            [edx+ebx];
//							mov         ecx,            iHCnt;
//							mov         iHLoop,         ecx;
//							dec         iVCnt;
//							jne         I420toYUY2_HM_V_MMX;
//							
//							emms;
//						}							
//					}//MMX END		
//				} 
//			}//END iSWSubSample = 0
//			else if (iSWSubSample==1)//iSWSubSample is 1
//			{
//				int iYLineStart = ((bVMirror)? (OriHeight-2)*iWidth : 0);
//				int iCLineStart = ((bVMirror)? ((OriHeight>>1)-2)*(iWidth>>1) : 0);	
//				
//				if (!bHMirror)
//				{
//					int iYLineStep = ( (bVMirror)?   (-((iWidth+iHoEnd-iHoStart+1)<<1)) : ((iWidth-(iHoEnd-iHoStart+1))<<1) );
//					int iCLineStep = ( (bVMirror)?   (-( iWidth+iHoEnd-iHoStart+1)    ) : (iWidth-(iHoEnd-iHoStart+1))      );
//					
//					AddrY += iYLineStart;
//					AddrCb += iCLineStart;				
//					AddrY += (iHoStart<<1);
//					AddrCb += (iHoStart);
//					
//					int iCbCrWidth = (-(iHoEnd-iHoStart+1));	
//					
//					if (gbSSE2 && (!(AddrYUY2&15)) && (!(iHoStart&31)) && (!(AddrY&15)) && (!(AddrCb&15)) )//1:SSE2 0:MMX
//					{
//						SInt32 iHCnt = ((iHoEnd-iHoStart+1)>>4);								
//						SInt32 iHLoop = iHCnt;
//						SInt32 iHCnt_B = (iHoStart>>3);
//						SInt32 iHLoop_B = iHCnt_B;
//						SInt32 iHCnt_B1 = iHCnt_B<<1;
//						SInt32 iHLoop_B1 = iHCnt_B1;
//						
//						__asm
//						{
//							mov         esi,            AddrY;
//							mov         edx,            AddrCb;
//							mov         edi,            AddrYUY2;
//							mov         eax,            iCBufSize;
//I420toYUY2_V_1_SSE:
//							mov         ecx,            iHCnt_B;
//							test		ecx,			ecx;
//							je			I420toYUY2_H_E_1_SSE2;
//							mov         iHLoop_B,       ecx;
//I420toYUY2_H1_L_SSE2:					
//							pcmpeqw     xmm0,           xmm0;			//xmm0:FFFFFFFF FFFFFFF FFFFFFFF FFFFFFFF	
//							psllw       xmm0,           15;				//xmm0:80008000 80008000 80008000 80008000
//							movdqa		[edi],			xmm0;
//							lea         edi,            [edi+0x10];
//							dec         iHLoop_B;
//							jne         I420toYUY2_H1_L_SSE2;
//I420toYUY2_H_E_1_SSE2:					
//							movdqa		xmm0,			[esi];		//Y15Y14 Y13Y12 Y11Y10 Y9Y8 Y7Y6 Y5Y4 Y3Y2 Y1Y0
//							movdqa		xmm1,			[esi+0x10];
//							movdqa		xmm2,			[edx];
//							movdqa		xmm3,			[edx+eax];
//							
//							psllw		xmm0,			8;			//Y1400 Y1200 Y1000 Y800 Y600 Y400 Y200 Y000
//							psllw		xmm1,			8;			
//							psllw		xmm2,			8;			
//							psllw		xmm3,			8;					
//							psrlw		xmm0,			8;			//00Y14 00Y12 00Y10 00Y8 00Y6 00Y4 00Y2 00Y0
//							psrlw		xmm1,			8;	
//							psrlw		xmm2,			8;
//							psrlw		xmm3,			8;
//							
//							packuswb	xmm0,			xmm1;		//Y30Y28Y26Y24Y22Y20Y18Y16 Y14Y12Y10Y8Y6Y4Y2Y0
//							pxor		xmm4,			xmm4;
//							packuswb	xmm2,			xmm4;		//0000000000000000 Cb14Cb12Cb10Cb8Cb6Cb4Cb2Cb0
//							packuswb	xmm3,			xmm4;
//							
//							movdqa		xmm1,			xmm0;
//							punpcklbw	xmm2,			xmm3;		//Cr14Cb14 Cr12Cb12 Cr10Cb10 Cr8Cb8 Cr6Cb6 Cr4Cb4 Cr2Cb2 Cr0Cb0
//							punpcklbw	xmm0,			xmm2;		//Cr6Y14  Cb6Y12  Cr4Y10  Cb4Y8   Cr2Y6   Cb2Y4   Cr0Y2  Cb0Y0
//							punpckhbw	xmm1,			xmm2;		//Cr14Y30 Cb14Y28 Cr12Y26 Cb12Y24 Cr10Y22 Cb10Y20 Cr8Y18 Cb8Y16
//							
//							movdqa		[edi],			xmm0;
//							movdqa		[edi+0x10],		xmm1;
//							
//							lea         esi,            [esi+0x20];
//							lea         edx,            [edx+0x10];
//							lea         edi,            [edi+0x20];
//							dec         iHLoop;
//							jne         I420toYUY2_H_E_1_SSE2;
//
//							mov         ecx,            iHCnt_B1;
//							test		ecx,			ecx;
//							jna			NextLine8;
//							mov         iHLoop_B1,      ecx;
//I420toYUY2_H1_R_SSE2:					
//							pcmpeqw     xmm0,           xmm0;			//xmm7:FFFFFFFF FFFFFFF FFFFFFFF FFFFFFFF	
//							psllw       xmm0,           15;				//xmm7:80008000 80008000 80008000 80008000
//							movdqa		[edi],			xmm0;
//							lea         edi,            [edi+0x10];					
//							dec         iHLoop_B1;
//							jne         I420toYUY2_H1_R_SSE2;
//							//-------------------------------------------------------------
//NextLine8:					mov         ecx,            iHCnt;
//							mov         ebx,            iCbCrWidth;
//							lea         edx,            [edx+ebx];
//							mov         iHLoop,         ecx;
//							mov         ebx,            iYLineStep;
//							lea         esi,            [esi+ebx];
//							
//I420toYUY2_H_O_1_SSE2:
//							movdqa		xmm0,			[esi];		//Y15Y14 Y13Y12 Y11Y10 Y9Y8 Y7Y6 Y5Y4 Y3Y2 Y1Y0
//							movdqa		xmm1,			[esi+0x10];
//							movdqa		xmm2,			[edx];
//							movdqa		xmm3,			[edx+eax];
//							
//							psllw		xmm0,			8;			//Y1400 Y1200 Y1000 Y800 Y600 Y400 Y200 Y000
//							psllw		xmm1,			8;			
//							psllw		xmm2,			8;			
//							psllw		xmm3,			8;					
//							psrlw		xmm0,			8;			//00Y14 00Y12 00Y10 00Y8 00Y6 00Y4 00Y2 00Y0
//							psrlw		xmm1,			8;	
//							psrlw		xmm2,			8;
//							psrlw		xmm3,			8;
//							
//							packuswb	xmm0,			xmm1;		//Y30Y28Y26Y24Y22Y20Y18Y16 Y14Y12Y10Y8Y6Y4Y2Y0
//							pxor		xmm4,			xmm4;
//							packuswb	xmm2,			xmm4;		//0000000000000000 Cb14Cb12Cb10Cb8Cb6Cb4Cb2Cb0
//							packuswb	xmm3,			xmm4;
//							
//							movdqa		xmm1,			xmm0;
//							punpcklbw	xmm2,			xmm3;		//Cr14Cb14 Cr12Cb12 Cr10Cb10 Cr8Cb8 Cr6Cb6 Cr4Cb4 Cr2Cb2 Cr0Cb0					
//							punpcklbw	xmm0,			xmm2;		//Cr6Y14  Cb6Y12  Cr4Y10  Cb4Y8   Cr2Y6   Cb2Y4   Cr0Y2  Cb0Y0
//							punpckhbw	xmm1,			xmm2;		//Cr14Y30 Cb14Y28 Cr12Y26 Cb12Y24 Cr10Y22 Cb10Y20 Cr8Y18 Cb8Y16
//							
//							movdqa		[edi],			xmm0;
//							movdqa		[edi+0x10],		xmm1;
//							
//							lea         esi,            [esi+0x20];
//							lea         edx,            [edx+0x10];
//							lea         edi,            [edi+0x20];
//							dec         iHLoop;
//							jne         I420toYUY2_H_O_1_SSE2;
//
//							mov         ecx,            iHCnt_B;
//							test		ecx,			ecx;
//							jna			NextLine9;
//							mov         iHLoop_B,		ecx;
//I420toYUY2_H1_OR_SSE2:					
//							pcmpeqw     xmm0,           xmm0;			//xmm7:FFFFFFFF FFFFFFF FFFFFFFF FFFFFFFF	
//							psllw       xmm0,           15;				//xmm7:80008000 80008000 80008000 80008000
//							movdqa		[edi],			xmm0;
//							lea         edi,            [edi+0x10];					
//							dec         iHLoop_B;
//							jne         I420toYUY2_H1_OR_SSE2;
//
//							//-------------------------------------------------------------
//NextLine9:					mov         ebx,            iYLineStep;
//							lea         esi,            [esi+ebx];
//							mov         ebx,            iCLineStep;
//							lea         edx,            [edx+ebx];
//							mov         ecx,            iHCnt;
//							mov         iHLoop,         ecx;
//							dec         iVCnt;
//							jne         I420toYUY2_V_1_SSE;
//							
//							emms;
//						}
//					}//SSE2 END
//					else
//					{
//						SInt32 iHCnt = ((iHoEnd-iHoStart+1)>>3);
//						SInt32 iHLoop = iHCnt;
//						SInt32 iHCnt_B = (iHoStart>>2);
//						SInt32 iHLoop_B = iHCnt_B;
//						SInt32 iHCnt_B1 = iHCnt_B<<1;
//						SInt32 iHLoop_B1 = iHCnt_B1;
//						
//						__asm
//						{
//							mov         esi,            AddrY;
//							mov         edx,            AddrCb;
//							mov         edi,            AddrYUY2;
//							mov         eax,            iCBufSize;
//I420toYUY2_V_1_MMX:
//							mov         ecx,            iHCnt_B;
//							test		ecx,			ecx;
//							je			I420toYUY2_H_E_1;
//							mov         iHLoop_B,       ecx;
//I420toYUY2_H1_L_MMX:	//Rotate	
//							pcmpeqw     mm0,           mm0;			//xmm0:FFFFFFFF FFFFFFF	
//							psllw       mm0,           15;				//xmm0:80008000 80008000
//							movq		[edi],			mm0;
//							lea         edi,            [edi+0x8];
//							dec         iHLoop_B;
//							jne         I420toYUY2_H1_L_MMX;
//
//I420toYUY2_H_E_1:
//							mov         cl,             [edx];
//							mov         ch,             [edx+eax];  //ecx = Cr Cb
//							pinsrw		MM0,			ecx,		0;
//							mov         cl,             [edx+2];
//							mov         ch,             [edx+eax+2];//ecx = Cr Cb
//							pinsrw		MM0,			ecx,		1;
//							mov         cl,             [edx+4];
//							mov         ch,             [edx+eax+4];//ecx = Cr Cb
//							pinsrw		MM0,			ecx,		2;
//							mov         cl,             [edx+6];
//							mov         ch,             [edx+eax+6];//ecx = Cr Cb
//							pinsrw		MM0,			ecx,		3;
//							
//							mov         cl,             [esi];
//							mov         ch,             [esi+2];	//ecx = Y Y
//							pinsrw		MM1,			ecx,		0;
//							mov         cl,             [esi+4];
//							mov         ch,             [esi+6];
//							pinsrw		MM1,			ecx,		1;
//							mov         cl,             [esi+8];
//							mov         ch,             [esi+0xA]; 
//							pinsrw		MM2,			ecx,		2;
//							mov         cl,             [esi+0xC];
//							mov         ch,             [esi+0xE];
//							pinsrw		MM2,			ecx,		3;
//							
//							punpcklbw	mm1,			mm0;//Low: Y Cb Y Cr				
//							punpckhbw	mm2,			mm0;//High: Y Cb Y Cr
//							
//							movq		[edi],			mm1;
//							movq		[edi+0x8],		mm2;
//							
//							lea         esi,            [esi+0x10];
//							lea         edx,            [edx+8];
//							lea         edi,            [edi+0x10];
//							dec         iHLoop;
//							jne         I420toYUY2_H_E_1;
//
//							mov         ecx,            iHCnt_B1;
//							test		ecx,			ecx;
//							jna			NextLineA;
//							mov         iHLoop_B1,      ecx;
//I420toYUY2_H1_R_MMX:	
//							pcmpeqw     mm0,           mm0;			//xmm0:FFFFFFFF FFFFFFF	
//							psllw       mm0,           15;				//xmm0:80008000 80008000
//							movq		[edi],			mm0;
//							lea         edi,            [edi+0x8];			
//							dec         iHLoop_B1;
//							jne         I420toYUY2_H1_R_MMX;
//							//-------------------------------------------------------------
//NextLineA:					mov         ecx,            iHCnt;
//							mov         ebx,            iCbCrWidth;
//							lea         edx,            [edx+ebx];
//							mov         iHLoop,         ecx;
//							mov         ebx,            iYLineStep;
//							lea         esi,            [esi+ebx];
//							
//I420toYUY2_H_O_1:
//							mov         cl,             [edx];
//							mov         ch,             [edx+eax];  //ecx = Cr Cb
//							pinsrw		MM0,			ecx,		0;
//							mov         cl,             [edx+2];
//							mov         ch,             [edx+eax+2];//ecx = Cr Cb
//							pinsrw		MM0,			ecx,		1;
//							mov         cl,             [edx+4];
//							mov         ch,             [edx+eax+4];//ecx = Cr Cb
//							pinsrw		MM0,			ecx,		2;
//							mov         cl,             [edx+6];
//							mov         ch,             [edx+eax+6];//ecx = Cr Cb
//							pinsrw		MM0,			ecx,		3;
//							
//							mov         cl,             [esi];
//							mov         ch,             [esi+2];	//ecx = Y Y
//							pinsrw		MM1,			ecx,		0;
//							mov         cl,             [esi+4];
//							mov         ch,             [esi+6];
//							pinsrw		MM1,			ecx,		1;
//							mov         cl,             [esi+8];
//							mov         ch,             [esi+0xA]; 
//							pinsrw		MM2,			ecx,		2;
//							mov         cl,             [esi+0xC];
//							mov         ch,             [esi+0xE];
//							pinsrw		MM2,			ecx,		3;
//							
//							punpcklbw	mm1,			mm0;//Low: Y Cb Y Cr				
//							punpckhbw	mm2,			mm0;//High: Y Cb Y Cr
//							
//							movq		[edi],			mm1;
//							movq		[edi+0x8],		mm2;
//							
//							lea         esi,            [esi+0x10];
//							lea         edx,            [edx+8];
//							lea         edi,            [edi+0x10];
//							dec         iHLoop;
//							jne         I420toYUY2_H_O_1;
//
//							mov         ecx,            iHCnt_B;
//							test		ecx,			ecx;
//							jna			NextLineB;
//							mov         iHLoop_B,		ecx;
//I420toYUY2_H1_OR_MMX:
//							pcmpeqw     mm0,           mm0;			//xmm0:FFFFFFFF FFFFFFF	
//							psllw       mm0,           15;				//xmm0:80008000 80008000
//							movq		[edi],			mm0;
//							lea         edi,            [edi+0x8];			
//							dec         iHLoop_B;
//							jne         I420toYUY2_H1_OR_MMX;
//							//-------------------------------------------------------------
//NextLineB:					mov         ebx,            iYLineStep;
//							lea         esi,            [esi+ebx];
//							mov         ebx,            iCLineStep;
//							lea         edx,            [edx+ebx];
//							mov         ecx,            iHCnt;
//							mov         iHLoop,         ecx;
//							dec         iVCnt;
//							jne         I420toYUY2_V_1_MMX;
//							
//							emms;
//						}					
//					} //MMX END					
//				}
//				else//H Mirror
//				{
//					int iYLineStep = ( (bVMirror)?   (-((iWidth-(iHoEnd-iHoStart+1))<<1)): ((iWidth+iHoEnd-iHoStart+1)<<1));
//					int iCLineStep = ( (bVMirror)?   (- (iWidth-(iHoEnd-iHoStart+1))    ): (iWidth+iHoEnd-iHoStart+1) );
//					
//					AddrY += (OriWidth);
//					AddrCb += ((OriWidth>>1));			
//					AddrY += iYLineStart;
//					AddrCb += iCLineStart;			
//					AddrY -= (iHoStart<<1);
//					AddrCb -= (iHoStart);
//					
//					int iCbCrWidth = ((iHoEnd-iHoStart+1));
//					
//					if (gbSSE2 && (!(AddrYUY2&15)) && (!(iHoStart&31)) && (!(AddrY&15)) && (!(AddrCb&15)) )//1:SSE2 0:MMX
//					{
//						SInt32 iHCnt = ((iHoEnd-iHoStart+1)>>4);
//						SInt32 iHLoop = iHCnt;
//						SInt32 iHCnt_B = (iHoStart>>3);
//						SInt32 iHLoop_B = iHCnt_B;
//						SInt32 iHCnt_B1 = iHCnt_B<<1;
//						SInt32 iHLoop_B1 = iHCnt_B1;
//					
//						__asm
//						{
//							mov         esi,            AddrY;
//							mov         edx,            AddrCb;
//							mov         edi,            AddrYUY2;
//							mov         eax,            iCBufSize;
//I420toYUY2_HM_V_1_SSE2:
//							mov         ecx,            iHCnt_B;
//							test		ecx,			ecx;
//							je			I420toYUY2_HM_E_1_SSE2;
//							mov         iHLoop_B,       ecx;
//I420toYUY2_HM1_L_SSE2:					
//							pcmpeqw     xmm0,           xmm0;			//xmm0:FFFFFFFF FFFFFFF FFFFFFFF FFFFFFFF	
//							psllw       xmm0,           15;				//xmm0:80008000 80008000 80008000 80008000
//							movdqa		[edi],			xmm0;
//							lea         edi,            [edi+0x10];
//							dec         iHLoop_B;
//							jne         I420toYUY2_HM1_L_SSE2;		
//
//I420toYUY2_HM_E_1_SSE2:
//							movdqa		xmm0,			[edx+eax-0x10];			//Cr:0~15
//							movdqa		xmm1,			[edx-0x10];				//Cb:0~15
//							movdqa		xmm5,			[esi-0x10];				//Y:16~31
//							movdqa		xmm2,			[esi-0x20];				//Y:0~15
//							
//							psllw		xmm0,			8;	
//							psllw		xmm1,			8;			
//							psllw		xmm2,			8;						//Y1400 Y1200 Y1000 Y800 Y600 Y400 Y200 Y000
//							psllw		xmm5,			8;					
//							psrlw		xmm0,			8;	
//							psrlw		xmm1,			8;	
//							psrlw		xmm2,			8;						//00Y14 00Y12 00Y10 00Y8 00Y6 00Y4 00Y2 00Y0
//							psrlw		xmm5,			8;
//							
//							packuswb	xmm2,			xmm5;					//Y30Y28Y26Y24Y22Y20Y18Y16 Y14Y12Y10Y8Y6Y4Y2Y0
//							pxor		xmm4,			xmm4;
//							packuswb	xmm1,			xmm4;					//0000000000000000 Cb14Cb12Cb10Cb8Cb6Cb4Cb2Cb0
//							packuswb	xmm0,			xmm4;
//							
//							movdqa		xmm6,			xmm2;
//							punpcklbw	xmm0,			xmm1;					//Cb14Cr14 Cb12Cr12 Cb10Cr10 Cb8Cr8 Cb6Cr6 Cb4Cr4 Cb2Cr2 Cb0Cr0					
//							punpcklbw	xmm2,			xmm0;					//Cb6Y14  Cr6Y12  Cb4Y10  Cr4Y8   Cb2Y6   Cr2Y4   Cb0Y2  Cr0Y0
//							punpckhbw	xmm6,			xmm0;					//Cb14Y30 Cr14Y28 Cb12Y26 Cr12Y24 Cb10Y22 Cr10Y20 Cb8Y18 Cr8Y16
//							pshuflw		xmm2,			xmm2,		00011011b;	//Cb6Y14  Cr6Y12  Cb4Y10  Cr4Y8   Cr0Y0   Cb0Y2   Cr2Y4   Cb2Y6        
//							pshuflw		xmm6,			xmm6,		00011011b;	//Cb14Y30 Cr14Y28 Cb12Y26 Cr12Y24 Cr8Y16  Cb8Y18  Cr10Y20 Cb10Y22
//							pshufhw		xmm2,			xmm2,		00011011b;	//Cr4Y8   Cb4Y10  Cr6Y12  Cb6Y14  Cr0Y0   Cb0Y2   Cr2Y4   Cb2Y6 
//							pshufhw		xmm6,			xmm6,		00011011b;	//Cr12Y24 Cb12Y26 Cr14Y28 Cb14Y30 Cr8Y16  Cb8Y18  Cr10Y20 Cb10Y22 
//							pshufd		xmm2,			xmm2,		01001110b;	//Cr0Y0   Cb0Y2   Cr2Y4   Cb2Y6   Cr4Y8   Cb4Y10  Cr6Y12  Cb6Y14  
//							pshufd		xmm6,			xmm6,		01001110b;	//Cr8Y16  Cb8Y18  Cr10Y20 Cb10Y22 Cr12Y24 Cb12Y26 Cr14Y28 Cb14Y30 
//							
//							movdqa		[edi],			xmm6;
//							movdqa		[edi+0x10],		xmm2;
//							
//							lea         esi,            [esi-0x20];
//							lea         edx,            [edx-0x10];
//							lea         edi,            [edi+0x20];
//							dec         iHLoop;
//							jne         I420toYUY2_HM_E_1_SSE2;
//
//							mov         ecx,            iHCnt_B1;
//							test		ecx,			ecx;
//							jna			NextLineC;
//							mov         iHLoop_B1,      ecx;
//I420toYUY2_HM1_R_SSE2:					
//							pcmpeqw     xmm0,           xmm0;			//xmm0:FFFFFFFF FFFFFFF FFFFFFFF FFFFFFFF	
//							psllw       xmm0,           15;				//xmm0:80008000 80008000 80008000 80008000
//							movdqa		[edi],			xmm0;
//							lea         edi,            [edi+0x10];					
//							dec         iHLoop_B1;
//							jne         I420toYUY2_HM1_R_SSE2;
//							//-------------------------------------------------------------
//NextLineC:					mov         ecx,            iHCnt;
//							mov         ebx,            iCbCrWidth;
//							lea         edx,            [edx+ebx];					
//							mov         iHLoop,         ecx;
//							mov         ebx,            iYLineStep;
//							lea         esi,            [esi+ebx];
//I420toYUY2_HM_O_1_SSE2:
//							movdqa		xmm0,			[edx+eax-0x10];			//Cr:0~15
//							movdqa		xmm1,			[edx-0x10];				//Cb:0~15
//							movdqa		xmm5,			[esi-0x10];				//Y:16~31
//							movdqa		xmm2,			[esi-0x20];				//Y:0~15
//							
//							psllw		xmm0,			8;	
//							psllw		xmm1,			8;			
//							psllw		xmm2,			8;						//Y1400 Y1200 Y1000 Y800 Y600 Y400 Y200 Y000
//							psllw		xmm5,			8;					
//							psrlw		xmm0,			8;	
//							psrlw		xmm1,			8;	
//							psrlw		xmm2,			8;						//00Y14 00Y12 00Y10 00Y8 00Y6 00Y4 00Y2 00Y0
//							psrlw		xmm5,			8;
//							
//							packuswb	xmm2,			xmm5;					//Y30Y28Y26Y24Y22Y20Y18Y16 Y14Y12Y10Y8Y6Y4Y2Y0
//							pxor		xmm4,			xmm4;
//							packuswb	xmm1,			xmm4;					//0000000000000000 Cb14Cb12Cb10Cb8Cb6Cb4Cb2Cb0
//							packuswb	xmm0,			xmm4;
//							
//							movdqa		xmm6,			xmm2;
//							punpcklbw	xmm0,			xmm1;					//Cb14Cr14 Cb12Cr12 Cb10Cr10 Cb8Cr8 Cb6Cr6 Cb4Cr4 Cb2Cr2 Cb0Cr0					
//							punpcklbw	xmm2,			xmm0;					//Cb6Y14  Cr6Y12  Cb4Y10  Cr4Y8   Cb2Y6   Cr2Y4   Cb0Y2  Cr0Y0
//							punpckhbw	xmm6,			xmm0;					//Cb14Y30 Cr14Y28 Cb12Y26 Cr12Y24 Cb10Y22 Cr10Y20 Cb8Y18 Cr8Y16
//							pshuflw		xmm2,			xmm2,		00011011b;	//Cb6Y14  Cr6Y12  Cb4Y10  Cr4Y8   Cr0Y0   Cb0Y2   Cr2Y4   Cb2Y6        
//							pshuflw		xmm6,			xmm6,		00011011b;	//Cb14Y30 Cr14Y28 Cb12Y26 Cr12Y24 Cr8Y16  Cb8Y18  Cr10Y20 Cb10Y22
//							pshufhw		xmm2,			xmm2,		00011011b;	//Cr4Y8   Cb4Y10  Cr6Y12  Cb6Y14  Cr0Y0   Cb0Y2   Cr2Y4   Cb2Y6 
//							pshufhw		xmm6,			xmm6,		00011011b;	//Cr12Y24 Cb12Y26 Cr14Y28 Cb14Y30 Cr8Y16  Cb8Y18  Cr10Y20 Cb10Y22 
//							pshufd		xmm2,			xmm2,		01001110b;	//Cr0Y0   Cb0Y2   Cr2Y4   Cb2Y6   Cr4Y8   Cb4Y10  Cr6Y12  Cb6Y14  
//							pshufd		xmm6,			xmm6,		01001110b;	//Cr8Y16  Cb8Y18  Cr10Y20 Cb10Y22 Cr12Y24 Cb12Y26 Cr14Y28 Cb14Y30 
//							
//							movdqa		[edi],			xmm6;
//							movdqa		[edi+0x10],		xmm2;
//							
//							lea         esi,            [esi-0x20];
//							lea         edx,            [edx-0x10];
//							lea         edi,            [edi+0x20];
//							dec         iHLoop;
//							jne         I420toYUY2_HM_O_1_SSE2;
//
//							mov         ecx,            iHCnt_B;
//							test		ecx,			ecx;
//							jna			NextLineD;
//							mov         iHLoop_B,		ecx;
//I420toYUY2_HM1_OR_SSE2:					
//							pcmpeqw     xmm0,           xmm0;			//xmm0:FFFFFFFF FFFFFFF FFFFFFFF FFFFFFFF	
//							psllw       xmm0,           15;				//xmm0:80008000 80008000 80008000 80008000
//							movdqa		[edi],			xmm0;
//							lea         edi,            [edi+0x10];					
//							dec         iHLoop_B;
//							jne         I420toYUY2_HM1_OR_SSE2;
//
//							//-------------------------------------------------------------
//NextLineD:					mov         ebx,            iYLineStep;
//							lea         esi,            [esi+ebx];
//							mov         ebx,            iCLineStep;
//							lea         edx,            [edx+ebx];
//							mov         ecx,            iHCnt;
//							mov         iHLoop,         ecx;
//							dec         iVCnt;
//							jne         I420toYUY2_HM_V_1_SSE2;
//							
//							emms;
//						}
//					}//SSE2 END
//					else
//					{
//						SInt32 iHCnt = ((iHoEnd-iHoStart+1)>>3);
//						SInt32 iHLoop = iHCnt;
//						SInt32 iHCnt_B = (iHoStart>>2);
//						SInt32 iHLoop_B = iHCnt_B;
//						SInt32 iHCnt_B1 = iHCnt_B<<1;
//						SInt32 iHLoop_B1 = iHCnt_B1;
//						
//						__asm
//						{
//							mov         esi,            AddrY;
//							mov         edx,            AddrCb;
//							mov         edi,            AddrYUY2;
//							mov         eax,            iCBufSize;
//I420toYUY2_HM_V_1_MMX:
//							mov         ecx,            iHCnt_B;
//							test		ecx,			ecx;
//							je			I420toYUY2_HM_E_1;
//							mov         iHLoop_B,       ecx;
//I420toYUY2_HM1_L_MMX:	//Rotate	
//							pcmpeqw     mm0,           mm0;			//xmm0:FFFFFFFF FFFFFFF	
//							psllw       mm0,           15;				//xmm0:80008000 80008000
//							movq		[edi],			mm0;
//							lea         edi,            [edi+0x8];
//							dec         iHLoop_B;
//							jne         I420toYUY2_HM1_L_MMX;
//I420toYUY2_HM_E_1:
//							mov         cl,             [edx-2];
//							mov         ch,             [edx+eax-2];  //ecx = Cr Cb
//							pinsrw		MM0,			ecx,		0;
//							mov         cl,             [edx-4];
//							mov         ch,             [edx+eax-4];//ecx = Cr Cb
//							pinsrw		MM0,			ecx,		1;
//							mov         cl,             [edx-6];
//							mov         ch,             [edx+eax-6];//ecx = Cr Cb
//							pinsrw		MM0,			ecx,		2;
//							mov         cl,             [edx-8];
//							mov         ch,             [edx+eax-8];//ecx = Cr Cb
//							pinsrw		MM0,			ecx,		3;
//							
//							mov         cl,             [esi-2];
//							mov         ch,             [esi-4];	//ecx = Y Y
//							pinsrw		MM1,			ecx,		0;
//							mov         cl,             [esi-6];
//							mov         ch,             [esi-8];
//							pinsrw		MM1,			ecx,		1;
//							mov         cl,             [esi-0xA];
//							mov         ch,             [esi-0xC]; 
//							pinsrw		MM2,			ecx,		2;
//							mov         cl,             [esi-0xE];
//							mov         ch,             [esi-0x10];
//							pinsrw		MM2,			ecx,		3;
//							
//							punpcklbw	mm1,			mm0;//Low: Y Cb Y Cr				
//							punpckhbw	mm2,			mm0;//High: Y Cb Y Cr
//							
//							movq		[edi],			mm1;
//							movq		[edi+0x8],		mm2;
//							
//							lea         esi,            [esi-0x10];
//							lea         edx,            [edx-8];
//							lea         edi,            [edi+0x10];
//							dec         iHLoop;
//							jne         I420toYUY2_HM_E_1;
//
//							mov         ecx,            iHCnt_B1;
//							test		ecx,			ecx;
//							jna			NextLineE;
//							mov         iHLoop_B1,      ecx;
//I420toYUY2_HM1_R_MMX:	
//							pcmpeqw     mm0,           mm0;			//xmm0:FFFFFFFF FFFFFFF	
//							psllw       mm0,           15;				//xmm0:80008000 80008000
//							movq		[edi],			mm0;
//							lea         edi,            [edi+0x8];			
//							dec         iHLoop_B1;
//							jne         I420toYUY2_HM1_R_MMX;
//							//-------------------------------------------------------------
//NextLineE:					mov         ecx,            iHCnt;
//							mov         ebx,            iCbCrWidth;
//							lea         edx,            [edx+ebx];
//							mov         iHLoop,         ecx;
//							mov         ebx,            iYLineStep;
//							lea         esi,            [esi+ebx];
//I420toYUY2_HM_O_1:
//							mov         cl,             [edx-2];
//							mov         ch,             [edx+eax-2];  //ecx = Cr Cb
//							pinsrw		MM0,			ecx,		0;
//							mov         cl,             [edx-4];
//							mov         ch,             [edx+eax-4];//ecx = Cr Cb
//							pinsrw		MM0,			ecx,		1;
//							mov         cl,             [edx-6];
//							mov         ch,             [edx+eax-6];//ecx = Cr Cb
//							pinsrw		MM0,			ecx,		2;
//							mov         cl,             [edx-8];
//							mov         ch,             [edx+eax-8];//ecx = Cr Cb
//							pinsrw		MM0,			ecx,		3;
//							
//							mov         cl,             [esi-2];
//							mov         ch,             [esi-4];	//ecx = Y Y
//							pinsrw		MM1,			ecx,		0;
//							mov         cl,             [esi-6];
//							mov         ch,             [esi-8];
//							pinsrw		MM1,			ecx,		1;
//							mov         cl,             [esi-0xA];
//							mov         ch,             [esi-0xC]; 
//							pinsrw		MM2,			ecx,		2;
//							mov         cl,             [esi-0xE];
//							mov         ch,             [esi-0x10];
//							pinsrw		MM2,			ecx,		3;
//							
//							punpcklbw	mm1,			mm0;//Low: Y Cb Y Cr				
//							punpckhbw	mm2,			mm0;//High: Y Cb Y Cr
//							
//							movq		[edi],			mm1;
//							movq		[edi+0x8],		mm2;
//							
//							lea         esi,            [esi-0x10];
//							lea         edx,            [edx-8];
//							lea         edi,            [edi+0x10];
//							dec         iHLoop;
//							jne         I420toYUY2_HM_O_1;
//
//							mov         ecx,            iHCnt_B;
//							test		ecx,			ecx;
//							jna			NextLineF;
//							mov         iHLoop_B,		ecx;
//I420toYUY2_HM1_OR_MMX:
//							pcmpeqw     mm0,           mm0;			//xmm0:FFFFFFFF FFFFFFF	
//							psllw       mm0,           15;				//xmm0:80008000 80008000
//							movq		[edi],			mm0;
//							lea         edi,            [edi+0x8];			
//							dec         iHLoop_B;
//							jne         I420toYUY2_HM1_OR_MMX;
//							//-------------------------------------------------------------
//NextLineF:					mov         ebx,            iYLineStep;
//							lea         esi,            [esi+ebx];
//							mov         ebx,            iCLineStep;
//							lea         edx,            [edx+ebx];
//							mov         ecx,            iHCnt;
//							mov         iHLoop,         ecx;
//							dec         iVCnt;
//							jne         I420toYUY2_HM_V_1_MMX;
//							
//							emms;
//						}				
//					} //MMX END
//				}
//			}//END iSWSubSample=1
//			else//iSWSubSample is 2
//			{
//				int iYLineStart = ((bVMirror)? (OriHeight-4)*iWidth : 0);
//				int iCLineStart = ((bVMirror)? ((OriHeight>>1)-4)*(iWidth>>1) : 0);			
//				SInt32 iHCnt = ((iHoEnd-iHoStart+1)>>3);
//				if (!bHMirror)
//				{
//					int iYLineStep = ( (bVMirror)?   (-((iWidth+(iHoEnd-iHoStart+1))<<2)) : ((iWidth-(iHoEnd-iHoStart+1))<<2) );
//					int iCLineStep = ( (bVMirror)?   (-((iWidth+(iHoEnd-iHoStart+1))<<1)) : ((iWidth-(iHoEnd-iHoStart+1))<<1) );
//					
//					AddrY += iYLineStart;
//					AddrCb += iCLineStart;				
//					AddrY += (iHoStart<<2);
//					AddrCb += (iHoStart<<1);
//					
//					int iCbCrWidth = (-((iHoEnd-iHoStart+1)<<1));
//					
//					if (gbSSE2 && (!(AddrYUY2&15)) && (!(iHoStart&31)) && (!(AddrY&15)) && (!(AddrCb&15)) )//1:SSE2 0:MMX
//					{
//						iHCnt = ((iHoEnd-iHoStart+1)>>4);
//						SInt32 iHLoop = iHCnt;
//						SInt32 iHCnt_B = (iHoStart>>3);
//						SInt32 iHLoop_B = iHCnt_B;
//						SInt32 iHCnt_B1 = iHCnt_B<<1;
//						SInt32 iHLoop_B1 = iHCnt_B1;
//					
//						iHLoop = iHCnt;
//						__asm
//						{
//							mov         esi,            AddrY;
//							mov         edx,            AddrCb;
//							mov         edi,            AddrYUY2;
//							mov         eax,            iCBufSize;
//I420toYUY2_H_V_2_SSE2:
//							mov         ecx,            iHCnt_B;
//							test		ecx,			ecx;
//							je			I420toYUY2_H_E_2_SSE2;
//							mov         iHLoop_B,       ecx;
//I420toYUY2_H2_L_SSE2:					
//							pcmpeqw     xmm0,           xmm0;			//xmm0:FFFFFFFF FFFFFFF FFFFFFFF FFFFFFFF	
//							psllw       xmm0,           15;				//xmm0:80008000 80008000 80008000 80008000
//							movdqa		[edi],			xmm0;
//							lea         edi,            [edi+0x10];
//							dec         iHLoop_B;
//							jne         I420toYUY2_H2_L_SSE2;
//I420toYUY2_H_E_2_SSE2:					
//							movdqa		xmm0,			[esi];		//Y15Y14 Y13Y12 Y11Y10 Y9Y8 Y7Y6 Y5Y4 Y3Y2 Y1Y0
//							movdqa		xmm1,			[esi+0x10];
//							movdqa		xmm2,			[edx];
//							movdqa		xmm3,			[edx+eax];
//							
//							pslld		xmm0,			24;			//Y12000000 Y8000000 Y4000000 Y0000000
//							pslld		xmm1,			24;			//Y28000000 Y24000000 Y20000000 Y16000000
//							pslld		xmm2,			24;			
//							pslld		xmm3,			24;					
//							psrld		xmm0,			24;			//0000 00Y12 0000 00Y8  0000 00Y4  0000 00Y0
//							psrld		xmm1,			24;			//0000 00Y28 0000 00Y24 0000 00Y20 0000 00Y16
//							psrld		xmm2,			24;
//							psrld		xmm3,			24;
//							packssdw	xmm0,			xmm1;		//00Y28 00Y24 00Y20 00Y16 00Y12 00Y8 00Y4 00Y0
//							
//							movdqa		xmm4,			[esi+0x20];	//Y15Y14 Y13Y12 Y11Y10 Y9Y8 Y7Y6 Y5Y4 Y3Y2 Y1Y0
//							movdqa		xmm5,			[esi+0x30];
//							pslld		xmm4,			24;			//Y44000000 Y40000000 Y36000000 Y32000000
//							pslld		xmm5,			24;			//Y60000000 Y56000000 Y52000000 Y48000000
//							psrld		xmm4,			24;			//0000 00Y44 0000 00Y40 0000 00Y36 0000 00Y32
//							psrld		xmm5,			24;			//0000 00Y60 0000 00Y56 0000 00Y52 0000 00Y48
//							packssdw	xmm4,			xmm5;		//00Y60 00Y56 00Y52 00Y48 00Y44 00Y40 00Y36 00Y32
//							packuswb	xmm0,			xmm4;		//Y60Y56Y52Y48 Y44Y40Y36Y32 Y28Y24Y20Y16 Y12Y8Y4Y0
//							
//							movdqa		xmm6,			[edx+0x10];
//							movdqa		xmm7,			[edx+eax+0x10];
//							pslld		xmm6,			24;			
//							pslld		xmm7,			24;			
//							psrld		xmm6,			24;			
//							psrld		xmm7,			24;	
//							packssdw	xmm2,			xmm6;		//00Cb28 00Cb24 00Cb20 00Cb16 00Cb12 00Cb8 00Cb4 00Cb0
//							packssdw	xmm3,			xmm7;		//00Cr28 00Cr24 00Cr20 00Cr16 00Cr12 00Cr8 00Cr4 00Cr0
//							pxor		xmm1,			xmm1;		//0
//							packuswb	xmm2,			xmm1;		//0000000000000000  Cb28Cb24Cb20Cb16Cb12Cb8Cb4Cb0
//							packuswb	xmm3,			xmm1;		//0000000000000000  Cr28Cr24Cr20Cr16Cr12Cr8Cr4Cr0
//							
//							movdqa		xmm1,			xmm0;
//							punpcklbw	xmm2,			xmm3;		//Cr28Cb28 Cr24Cb24 Cr20Cb20 Cr16Cb16 Cr12Cb12 Cr8Cb8 Cr4Cb4 Cr0Cb0
//							punpcklbw	xmm0,			xmm2;		//Cr12Y28  Cb12Y24  Cr8Y20  Cb8Y16   Cr4Y12   Cb4Y8   Cr0Y4  Cb0Y0
//							punpckhbw	xmm1,			xmm2;		//Cr28Y60 Cb28Y56 Cr24Y52 Cb24Y48 Cr20Y44 Cb20Y40 Cr16Y36 Cb16Y32
//							
//							movdqa		[edi],			xmm0;
//							movdqa		[edi+0x10],		xmm1;
//							
//							lea         esi,            [esi+0x40];
//							lea         edx,            [edx+0x20];
//							lea         edi,            [edi+0x20];
//							dec         iHLoop;
//							jne         I420toYUY2_H_E_2_SSE2;
//
//							mov         ecx,            iHCnt_B1;
//							test		ecx,			ecx;
//							jna			NextLine10;
//							mov         iHLoop_B1,      ecx;
//I420toYUY2_H2_R_SSE2:					
//							pcmpeqw     xmm0,           xmm0;			//xmm7:FFFFFFFF FFFFFFF FFFFFFFF FFFFFFFF	
//							psllw       xmm0,           15;				//xmm7:80008000 80008000 80008000 80008000
//							movdqa		[edi],			xmm0;
//							lea         edi,            [edi+0x10];					
//							dec         iHLoop_B1;
//							jne         I420toYUY2_H2_R_SSE2;
//							//-------------------------------------------------------------
//NextLine10:					mov         ecx,            iHCnt;
//							mov         ebx,            iCbCrWidth;
//							lea         edx,            [edx+ebx];
//							mov         iHLoop,         ecx;
//							mov         ebx,            iYLineStep;
//							lea         esi,            [esi+ebx];
//							
//I420toYUY2_H_O_2_SSE2:
//							movdqa		xmm0,			[esi];		//Y15Y14 Y13Y12 Y11Y10 Y9Y8 Y7Y6 Y5Y4 Y3Y2 Y1Y0
//							movdqa		xmm1,			[esi+0x10];
//							movdqa		xmm2,			[edx];
//							movdqa		xmm3,			[edx+eax];
//							
//							pslld		xmm0,			24;			//Y12000000 Y8000000 Y4000000 Y0000000
//							pslld		xmm1,			24;			//Y28000000 Y24000000 Y20000000 Y16000000
//							pslld		xmm2,			24;			
//							pslld		xmm3,			24;					
//							psrld		xmm0,			24;			//0000 00Y12 0000 00Y8  0000 00Y4  0000 00Y0
//							psrld		xmm1,			24;			//0000 00Y28 0000 00Y24 0000 00Y20 0000 00Y16
//							psrld		xmm2,			24;
//							psrld		xmm3,			24;
//							packssdw	xmm0,			xmm1;		//00Y28 00Y24 00Y20 00Y16 00Y12 00Y8 00Y4 00Y0
//							
//							movdqa		xmm4,			[esi+0x20];	//Y15Y14 Y13Y12 Y11Y10 Y9Y8 Y7Y6 Y5Y4 Y3Y2 Y1Y0
//							movdqa		xmm5,			[esi+0x30];
//							pslld		xmm4,			24;			//Y44000000 Y40000000 Y36000000 Y32000000
//							pslld		xmm5,			24;			//Y60000000 Y56000000 Y52000000 Y48000000
//							psrld		xmm4,			24;			//0000 00Y44 0000 00Y40 0000 00Y36 0000 00Y32
//							psrld		xmm5,			24;			//0000 00Y60 0000 00Y56 0000 00Y52 0000 00Y48
//							packssdw	xmm4,			xmm5;		//00Y60 00Y56 00Y52 00Y48 00Y44 00Y40 00Y36 00Y32
//							packuswb	xmm0,			xmm4;		//Y60Y56Y52Y48 Y44Y40Y36Y32 Y28Y24Y20Y16 Y12Y8Y4Y0
//							
//							movdqa		xmm6,			[edx+0x10];
//							movdqa		xmm7,			[edx+eax+0x10];
//							pslld		xmm6,			24;			
//							pslld		xmm7,			24;			
//							psrld		xmm6,			24;			
//							psrld		xmm7,			24;	
//							packssdw	xmm2,			xmm6;		//00Cb28 00Cb24 00Cb20 00Cb16 00Cb12 00Cb8 00Cb4 00Cb0
//							packssdw	xmm3,			xmm7;		//00Cr28 00Cr24 00Cr20 00Cr16 00Cr12 00Cr8 00Cr4 00Cr0
//							pxor		xmm1,			xmm1;		//0
//							packuswb	xmm2,			xmm1;		//0000000000000000  Cb28Cb24Cb20Cb16Cb12Cb8Cb4Cb0
//							packuswb	xmm3,			xmm1;		//0000000000000000  Cr28Cr24Cr20Cr16Cr12Cr8Cr4Cr0
//							
//							movdqa		xmm1,			xmm0;
//							punpcklbw	xmm2,			xmm3;		//Cr28Cb28 Cr24Cb24 Cr20Cb20 Cr16Cb16 Cr12Cb12 Cr8Cb8 Cr4Cb4 Cr0Cb0
//							punpcklbw	xmm0,			xmm2;		//Cr12Y28  Cb12Y24  Cr8Y20  Cb8Y16   Cr4Y12   Cb4Y8   Cr0Y4  Cb0Y0
//							punpckhbw	xmm1,			xmm2;		//Cr28Y60 Cb28Y56 Cr24Y52 Cb24Y48 Cr20Y44 Cb20Y40 Cr16Y36 Cb16Y32
//							
//							movdqa		[edi],			xmm0;
//							movdqa		[edi+0x10],		xmm1;
//							
//							lea         esi,            [esi+0x40];
//							lea         edx,            [edx+0x20];
//							lea         edi,            [edi+0x20];
//							dec         iHLoop;
//							jne         I420toYUY2_H_O_2_SSE2;
//
//							mov         ecx,            iHCnt_B;
//							test		ecx,			ecx;
//							jna			NextLine11;
//							mov         iHLoop_B,		ecx;
//I420toYUY2_H2_OR_SSE2:					
//							pcmpeqw     xmm0,           xmm0;			//xmm7:FFFFFFFF FFFFFFF FFFFFFFF FFFFFFFF	
//							psllw       xmm0,           15;				//xmm7:80008000 80008000 80008000 80008000
//							movdqa		[edi],			xmm0;
//							lea         edi,            [edi+0x10];					
//							dec         iHLoop_B;
//							jne         I420toYUY2_H2_OR_SSE2;
//
//							//-------------------------------------------------------------
//NextLine11:					mov         ebx,            iYLineStep;
//							lea         esi,            [esi+ebx];
//							mov         ebx,            iCLineStep;
//							lea         edx,            [edx+ebx];
//							mov         ecx,            iHCnt;
//							mov         iHLoop,         ecx;
//							dec         iVCnt;
//							jne         I420toYUY2_H_V_2_SSE2;
//							
//							emms;
//						}
//					}//SSE2 END
//					else
//					{
//						SInt32 iHLoop = iHCnt;
//						SInt32 iHCnt_B = (iHoStart>>2);
//						SInt32 iHLoop_B = iHCnt_B;
//						SInt32 iHCnt_B1 = iHCnt_B<<1;
//						SInt32 iHLoop_B1 = iHCnt_B1;
//						
//						__asm
//						{
//							mov         esi,            AddrY;
//							mov         edx,            AddrCb;
//							mov         edi,            AddrYUY2;
//							mov         eax,            iCBufSize;
//I420toYUY2_H_V_2_MMX:
//							mov         ecx,            iHCnt_B;
//							test		ecx,			ecx;
//							je			I420toYUY2_H_E_2;
//							mov         iHLoop_B,       ecx;
//I420toYUY2_H2_L_MMX:	//Rotate	
//							pcmpeqw     mm0,           mm0;			//xmm0:FFFFFFFF FFFFFFF	
//							psllw       mm0,           15;				//xmm0:80008000 80008000
//							movq		[edi],			mm0;
//							lea         edi,            [edi+0x8];
//							dec         iHLoop_B;
//							jne         I420toYUY2_H2_L_MMX;
//I420toYUY2_H_E_2:
//							mov         cl,             [edx];
//							mov         ch,             [edx+eax];  //ecx = Cr Cb
//							pinsrw		MM0,			ecx,		0;
//							mov         cl,             [edx+4];
//							mov         ch,             [edx+eax+4];//ecx = Cr Cb
//							pinsrw		MM0,			ecx,		1;
//							mov         cl,             [edx+8];
//							mov         ch,             [edx+eax+8];//ecx = Cr Cb
//							pinsrw		MM0,			ecx,		2;
//							mov         cl,             [edx+0xC];
//							mov         ch,             [edx+eax+0xC];//ecx = Cr Cb
//							pinsrw		MM0,			ecx,		3;
//							
//							mov         cl,             [esi];
//							mov         ch,             [esi+4];	//ecx = Y Y
//							pinsrw		MM1,			ecx,		0;
//							mov         cl,             [esi+8];
//							mov         ch,             [esi+0xC];
//							pinsrw		MM1,			ecx,		1;
//							mov         cl,             [esi+0x10];
//							mov         ch,             [esi+0x14]; 
//							pinsrw		MM2,			ecx,		2;
//							mov         cl,             [esi+0x18];
//							mov         ch,             [esi+0x1C];
//							pinsrw		MM2,			ecx,		3;
//							
//							punpcklbw	mm1,			mm0;//Low: Y Cb Y Cr				
//							punpckhbw	mm2,			mm0;//High: Y Cb Y Cr
//							
//							movq		[edi],			mm1;
//							movq		[edi+0x8],		mm2;
//							
//							lea         esi,            [esi+0x20];
//							lea         edx,            [edx+0x10];
//							lea         edi,            [edi+0x10];
//							dec         iHLoop;
//							jne         I420toYUY2_H_E_2;
//
//							mov         ecx,            iHCnt_B1;
//							test		ecx,			ecx;
//							jna			NextLine12;
//							mov         iHLoop_B1,      ecx;
//I420toYUY2_H2_R_MMX:	
//							pcmpeqw     mm0,            mm0;			//xmm0:FFFFFFFF FFFFFFF	
//							psllw       mm0,            15;				//xmm0:80008000 80008000
//							movq		[edi],			mm0;
//							lea         edi,            [edi+0x8];			
//							dec         iHLoop_B1;
//							jne         I420toYUY2_H2_R_MMX;
//							//-------------------------------------------------------------
//NextLine12:					mov         ecx,            iHCnt;
//							mov         ebx,            iCbCrWidth;
//							lea         edx,            [edx+ebx];
//							mov         iHLoop,         ecx;
//							mov         ebx,            iYLineStep;
//							lea         esi,            [esi+ebx];
//							
//I420toYUY2_H_O_2:
//							mov         eax,            iCBufSize;
//							
//							mov         cl,             [edx];
//							mov         ch,             [edx+eax];  //ecx = Cr Cb
//							pinsrw		MM0,			ecx,		0;
//							mov         cl,             [edx+4];
//							mov         ch,             [edx+eax+4];//ecx = Cr Cb
//							pinsrw		MM0,			ecx,		1;
//							mov         cl,             [edx+8];
//							mov         ch,             [edx+eax+8];//ecx = Cr Cb
//							pinsrw		MM0,			ecx,		2;
//							mov         cl,             [edx+0xC];
//							mov         ch,             [edx+eax+0xC];//ecx = Cr Cb
//							pinsrw		MM0,			ecx,		3;
//							
//							mov         cl,             [esi];
//							mov         ch,             [esi+4];	//ecx = Y Y
//							pinsrw		MM1,			ecx,		0;
//							mov         cl,             [esi+8];
//							mov         ch,             [esi+0xC];
//							pinsrw		MM1,			ecx,		1;
//							mov         cl,             [esi+0x10];
//							mov         ch,             [esi+0x14]; 
//							pinsrw		MM2,			ecx,		2;
//							mov         cl,             [esi+0x18];
//							mov         ch,             [esi+0x1C];
//							pinsrw		MM2,			ecx,		3;
//							
//							punpcklbw	mm1,			mm0;//Low: Y Cb Y Cr				
//							punpckhbw	mm2,			mm0;//High: Y Cb Y Cr
//							
//							movq		[edi],			mm1;
//							movq		[edi+0x8],		mm2;
//							
//							lea         esi,            [esi+0x20];
//							lea         edx,            [edx+0x10];
//							lea         edi,            [edi+0x10];
//							dec         iHLoop;
//							jne         I420toYUY2_H_O_2;
//
//							mov         ecx,            iHCnt_B;
//							test		ecx,			ecx;
//							jna			NextLine13;
//							mov         iHLoop_B,		ecx;
//I420toYUY2_H2_OR_MMX:
//							pcmpeqw     mm0,            mm0;			//xmm0:FFFFFFFF FFFFFFF	
//							psllw       mm0,            15;				//xmm0:80008000 80008000
//							movq		[edi],			mm0;
//							lea         edi,            [edi+0x8];			
//							dec         iHLoop_B;
//							jne         I420toYUY2_H2_OR_MMX;
//							//-------------------------------------------------------------
//NextLine13:					mov         ebx,            iYLineStep;
//							lea         esi,            [esi+ebx];
//							mov         ebx,            iCLineStep;
//							lea         edx,            [edx+ebx];
//							mov         ecx,            iHCnt;
//							mov         iHLoop,         ecx;
//							dec         iVCnt;
//							jne         I420toYUY2_H_V_2_MMX;
//							
//							emms;
//						}
//					}//MMX END 
//				}
//				else//H Mirror
//				{
//					int iYLineStep = ((bVMirror)? (-((iWidth-(iHoEnd-iHoStart+1))<<2)) : ((iWidth+(iHoEnd-iHoStart+1))<<2) );
//					int iCLineStep = ((bVMirror)? (-((iWidth-(iHoEnd-iHoStart+1))<<1)) : ((iWidth+(iHoEnd-iHoStart+1))<<1));
//					
//					AddrY += (OriWidth);
//					AddrCb += ((OriWidth>>1));			
//					AddrY += iYLineStart;
//					AddrCb += iCLineStart;			
//					AddrY -= (iHoStart<<2);
//					AddrCb -= (iHoStart<<1);
//					
//					int iCbCrWidth = ((iHoEnd-iHoStart+1)<<1);
//					SInt32 iHLoop = iHCnt;
//					
//					if (gbSSE2 && (!(AddrYUY2&15)) && (!(iHoStart&31)) && (!(AddrY&15)) && (!(AddrCb&15)) )//1:SSE2 0:MMX
//					{
//						iHCnt = ((iHoEnd-iHoStart+1)>>4);
//						iHLoop = iHCnt;
//						SInt32 iHCnt_B = (iHoStart>>3);
//						SInt32 iHLoop_B = iHCnt_B;
//						SInt32 iHCnt_B1 = iHCnt_B<<1;
//						SInt32 iHLoop_B1 = iHCnt_B1;
//
//						__asm
//						{
//							mov         esi,            AddrY;
//							mov         edx,            AddrCb;
//							mov         edi,            AddrYUY2;
//							mov         eax,            iCBufSize;
//I420toYUY2_HM_V_2_SSE2:
//							mov         ecx,            iHCnt_B;
//							test		ecx,			ecx;
//							je			I420toYUY2_HM_E_2_SSE2;
//							mov         iHLoop_B,       ecx;
//I420toYUY2_HM2_L_SSE2:					
//							pcmpeqw     xmm0,           xmm0;			//xmm0:FFFFFFFF FFFFFFF FFFFFFFF FFFFFFFF	
//							psllw       xmm0,           15;				//xmm0:80008000 80008000 80008000 80008000
//							movdqa		[edi],			xmm0;
//							lea         edi,            [edi+0x10];
//							dec         iHLoop_B;
//							jne         I420toYUY2_HM2_L_SSE2;	
//I420toYUY2_HM_E_2_SSE2:
//							movdqa		xmm0,			[edx+eax-0x20];			//Cr:0~15
//							movdqa		xmm1,			[edx-0x20];				//Cb:0~15
//							movdqa		xmm5,			[esi-0x30];				//Y:16~31
//							movdqa		xmm2,			[esi-0x40];				//Y:0~15
//							
//							pslld		xmm0,			24;	
//							pslld		xmm1,			24;	
//							pslld		xmm2,			24;			
//							pslld		xmm5,			24;					
//							psrld		xmm0,			24;			
//							psrld		xmm1,			24;			
//							psrld		xmm2,			24;			//0000 00Y12 0000 00Y8  0000 00Y4  0000 00Y0
//							psrld		xmm5,			24;			//0000 00Y28 0000 00Y24 0000 00Y20 0000 00Y16
//							packssdw	xmm2,			xmm5;		//00Y28 00Y24 00Y20 00Y16 00Y12 00Y8 00Y4 00Y0
//							
//							movdqa		xmm4,			[esi-0x10];
//							movdqa		xmm3,			[esi-0x20];
//							pslld		xmm4,			24;			//Y44000000 Y40000000 Y36000000 Y32000000
//							pslld		xmm3,			24;			//Y60000000 Y56000000 Y52000000 Y48000000
//							psrld		xmm4,			24;			//0000 00Y44 0000 00Y40 0000 00Y36 0000 00Y32
//							psrld		xmm3,			24;			//0000 00Y60 0000 00Y56 0000 00Y52 0000 00Y48
//							packssdw	xmm3,			xmm4;		//00Y60 00Y56 00Y52 00Y48 00Y44 00Y40 00Y36 00Y32
//							packuswb	xmm2,			xmm3;		//Y60Y56Y52Y48 Y44Y40Y36Y32 Y28Y24Y20Y16 Y12Y8Y4Y0
//							
//							movdqa		xmm6,			[edx-0x10];
//							movdqa		xmm7,			[edx+eax-0x10];
//							pslld		xmm6,			24;			
//							pslld		xmm7,			24;			
//							psrld		xmm6,			24;			
//							psrld		xmm7,			24;	
//							packssdw	xmm1,			xmm6;		//00Cb28 00Cb24 00Cb20 00Cb16 00Cb12 00Cb8 00Cb4 00Cb0
//							packssdw	xmm0,			xmm7;		//00Cr28 00Cr24 00Cr20 00Cr16 00Cr12 00Cr8 00Cr4 00Cr0
//							pxor		xmm3,			xmm3;		//0
//							packuswb	xmm1,			xmm3;		//0000000000000000  Cb28Cb24Cb20Cb16Cb12Cb8Cb4Cb0
//							packuswb	xmm0,			xmm3;		//0000000000000000  Cr28Cr24Cr20Cr16Cr12Cr8Cr4Cr0
//							
//							movdqa		xmm4,			xmm2;
//							punpcklbw	xmm0,			xmm1;		//Cb28Cr28 Cb24Cr24 Cb20Cr20 Cb16Cr16 Cb12Cr12 Cb8Cr8 Cb4Cr4 Cb0Cr0
//							punpcklbw	xmm2,			xmm0;		//Cb12Y28  Cr12Y24  Cb8Y20  Cr8Y16   Cb4Y12   Cr4Y8   Cb0Y4  Cr0Y0
//							punpckhbw	xmm4,			xmm0;		//Cb28Y60 Cr28Y56 Cb24Y52 Cr24Y48 Cb20Y44 Cr20Y40 Cb16Y36 Cr16Y32
//							
//							pshuflw		xmm2,			xmm2,		00011011b;	//Cb12Y28 Cr12Y24 Cb8Y20  Cr8Y16  Cr0Y0   Cb0Y4   Cr4Y8   Cb4Y12        
//							pshuflw		xmm4,			xmm4,		00011011b;	//Cb28Y60 Cr28Y56 Cb24Y52 Cr24Y48 Cr16Y32 Cb16Y36 Cr20Y40 Cb20Y44  
//							pshufhw		xmm2,			xmm2,		00011011b;	//Cr8Y16  Cb8Y20  Cr12Y24 Cb12Y28 Cr0Y0   Cb0Y4   Cr4Y8   Cb4Y12 
//							pshufhw		xmm4,			xmm4,		00011011b;	//Cr24Y48 Cb24Y52 Cr28Y56 Cb28Y60 Cr16Y32 Cb16Y36 Cr20Y40 Cb20Y44  
//							pshufd		xmm2,			xmm2,		01001110b;	//Cr0Y0   Cb0Y4   Cr4Y8   Cb4Y12  Cr8Y16  Cb8Y20  Cr12Y24 Cb12Y28   
//							pshufd		xmm4,			xmm4,		01001110b;	//Cr16Y32 Cb16Y36 Cr20Y40 Cb20Y44 Cr24Y48 Cb24Y52 Cr28Y56 Cb28Y60  
//							
//							movdqa		[edi],			xmm4;
//							movdqa		[edi+0x10],		xmm2;
//							
//							lea         esi,            [esi-0x40];
//							lea         edx,            [edx-0x20];
//							lea         edi,            [edi+0x20];
//							dec         iHLoop;
//							jne         I420toYUY2_HM_E_2_SSE2;
//
//							mov         ecx,            iHCnt_B1;
//							test		ecx,			ecx;
//							jna			NextLine14;
//							mov         iHLoop_B1,      ecx;
//I420toYUY2_HM2_R_SSE2:					
//							pcmpeqw     xmm0,           xmm0;			//xmm0:FFFFFFFF FFFFFFF FFFFFFFF FFFFFFFF	
//							psllw       xmm0,           15;				//xmm0:80008000 80008000 80008000 80008000
//							movdqa		[edi],			xmm0;
//							lea         edi,            [edi+0x10];					
//							dec         iHLoop_B1;
//							jne         I420toYUY2_HM2_R_SSE2;
//							//-------------------------------------------------------------
//NextLine14:					mov         ecx,            iHCnt;
//							mov         ebx,            iCbCrWidth;
//							lea         edx,            [edx+ebx];
//							mov         iHLoop,         ecx;
//							mov         ebx,            iYLineStep;
//							lea         esi,            [esi+ebx];
//I420toYUY2_HM_O_2_SSE2:
//							movdqa		xmm0,			[edx+eax-0x20];			//Cr:0~15
//							movdqa		xmm1,			[edx-0x20];				//Cb:0~15
//							movdqa		xmm5,			[esi-0x30];				//Y:16~31
//							movdqa		xmm2,			[esi-0x40];				//Y:0~15
//							
//							pslld		xmm0,			24;	
//							pslld		xmm1,			24;	
//							pslld		xmm2,			24;			
//							pslld		xmm5,			24;					
//							psrld		xmm0,			24;			
//							psrld		xmm1,			24;			
//							psrld		xmm2,			24;			//0000 00Y12 0000 00Y8  0000 00Y4  0000 00Y0
//							psrld		xmm5,			24;			//0000 00Y28 0000 00Y24 0000 00Y20 0000 00Y16
//							packssdw	xmm2,			xmm5;		//00Y28 00Y24 00Y20 00Y16 00Y12 00Y8 00Y4 00Y0
//							
//							movdqa		xmm4,			[esi-0x10];
//							movdqa		xmm3,			[esi-0x20];
//							pslld		xmm4,			24;			//Y44000000 Y40000000 Y36000000 Y32000000
//							pslld		xmm3,			24;			//Y60000000 Y56000000 Y52000000 Y48000000
//							psrld		xmm4,			24;			//0000 00Y44 0000 00Y40 0000 00Y36 0000 00Y32
//							psrld		xmm3,			24;			//0000 00Y60 0000 00Y56 0000 00Y52 0000 00Y48
//							packssdw	xmm3,			xmm4;		//00Y60 00Y56 00Y52 00Y48 00Y44 00Y40 00Y36 00Y32
//							packuswb	xmm2,			xmm3;		//Y60Y56Y52Y48 Y44Y40Y36Y32 Y28Y24Y20Y16 Y12Y8Y4Y0
//							
//							movdqa		xmm6,			[edx-0x10];
//							movdqa		xmm7,			[edx+eax-0x10];
//							pslld		xmm6,			24;			
//							pslld		xmm7,			24;			
//							psrld		xmm6,			24;			
//							psrld		xmm7,			24;	
//							packssdw	xmm1,			xmm6;		//00Cb28 00Cb24 00Cb20 00Cb16 00Cb12 00Cb8 00Cb4 00Cb0
//							packssdw	xmm0,			xmm7;		//00Cr28 00Cr24 00Cr20 00Cr16 00Cr12 00Cr8 00Cr4 00Cr0
//							pxor		xmm3,			xmm3;		//0
//							packuswb	xmm1,			xmm3;		//0000000000000000  Cb28Cb24Cb20Cb16Cb12Cb8Cb4Cb0
//							packuswb	xmm0,			xmm3;		//0000000000000000  Cr28Cr24Cr20Cr16Cr12Cr8Cr4Cr0
//							
//							movdqa		xmm4,			xmm2;
//							punpcklbw	xmm0,			xmm1;		//Cb28Cr28 Cb24Cr24 Cb20Cr20 Cb16Cr16 Cb12Cr12 Cb8Cr8 Cb4Cr4 Cb0Cr0
//							punpcklbw	xmm2,			xmm0;		//Cb12Y28  Cr12Y24  Cb8Y20  Cr8Y16   Cb4Y12   Cr4Y8   Cb0Y4  Cr0Y0
//							punpckhbw	xmm4,			xmm0;		//Cb28Y60 Cr28Y56 Cb24Y52 Cr24Y48 Cb20Y44 Cr20Y40 Cb16Y36 Cr16Y32
//							
//							pshuflw		xmm2,			xmm2,		00011011b;	//Cb12Y28 Cr12Y24 Cb8Y20  Cr8Y16  Cr0Y0   Cb0Y4   Cr4Y8   Cb4Y12        
//							pshuflw		xmm4,			xmm4,		00011011b;	//Cb28Y60 Cr28Y56 Cb24Y52 Cr24Y48 Cr16Y32 Cb16Y36 Cr20Y40 Cb20Y44  
//							pshufhw		xmm2,			xmm2,		00011011b;	//Cr8Y16  Cb8Y20  Cr12Y24 Cb12Y28 Cr0Y0   Cb0Y4   Cr4Y8   Cb4Y12 
//							pshufhw		xmm4,			xmm4,		00011011b;	//Cr24Y48 Cb24Y52 Cr28Y56 Cb28Y60 Cr16Y32 Cb16Y36 Cr20Y40 Cb20Y44  
//							pshufd		xmm2,			xmm2,		01001110b;	//Cr0Y0   Cb0Y4   Cr4Y8   Cb4Y12  Cr8Y16  Cb8Y20  Cr12Y24 Cb12Y28   
//							pshufd		xmm4,			xmm4,		01001110b;	//Cr16Y32 Cb16Y36 Cr20Y40 Cb20Y44 Cr24Y48 Cb24Y52 Cr28Y56 Cb28Y60  
//							
//							movdqa		[edi],			xmm4;
//							movdqa		[edi+0x10],		xmm2;
//							
//							lea         esi,            [esi-0x40];
//							lea         edx,            [edx-0x20];
//							lea         edi,            [edi+0x20];
//							dec         iHLoop;
//							jne         I420toYUY2_HM_O_2_SSE2;
//
//							mov         ecx,            iHCnt_B;
//							test		ecx,			ecx;
//							jna			NextLine15;
//							mov         iHLoop_B,		ecx;
//I420toYUY2_HM2_OR_SSE2:					
//							pcmpeqw     xmm0,           xmm0;			//xmm0:FFFFFFFF FFFFFFF FFFFFFFF FFFFFFFF	
//							psllw       xmm0,           15;				//xmm0:80008000 80008000 80008000 80008000
//							movdqa		[edi],			xmm0;
//							lea         edi,            [edi+0x10];					
//							dec         iHLoop_B;
//							jne         I420toYUY2_HM2_OR_SSE2;
//							//-------------------------------------------------------------
//NextLine15:					mov         ebx,            iYLineStep;
//							lea         esi,            [esi+ebx];
//							mov         ebx,            iCLineStep;
//							lea         edx,            [edx+ebx];
//							mov         ecx,            iHCnt;
//							mov         iHLoop,         ecx;
//							dec         iVCnt;
//							jne         I420toYUY2_HM_V_2_SSE2;
//							
//							emms;
//						}
//					}//SSE2 END
//					else
//					{
//						SInt32 iHCnt_B = (iHoStart>>2);
//						SInt32 iHLoop_B = iHCnt_B;
//						SInt32 iHCnt_B1 = iHCnt_B<<1;
//						SInt32 iHLoop_B1 = iHCnt_B1;
//						__asm
//						{
//							mov         esi,            AddrY;
//							mov         edx,            AddrCb;
//							mov         edi,            AddrYUY2;
//							mov         eax,            iCBufSize;
//I420toYUY2_HM_V_2_MMX:
//							mov         ecx,            iHCnt_B;
//							test		ecx,			ecx;
//							je			I420toYUY2_HM_E_2;
//							mov         iHLoop_B,       ecx;
//I420toYUY2_HM2_L_MMX:	//Rotate	
//							pcmpeqw     mm0,            mm0;			//xmm0:FFFFFFFF FFFFFFF	
//							psllw       mm0,            15;				//xmm0:80008000 80008000
//							movq		[edi],			mm0;
//							lea         edi,            [edi+0x8];
//							dec         iHLoop_B;
//							jne         I420toYUY2_HM2_L_MMX;
//I420toYUY2_HM_E_2:	
//							mov         cl,             [edx-4];
//							mov         ch,             [edx+eax-4];  //ecx = Cr Cb
//							pinsrw		MM0,			ecx,		0;
//							mov         cl,             [edx-8];
//							mov         ch,             [edx+eax-8];//ecx = Cr Cb
//							pinsrw		MM0,			ecx,		1;
//							mov         cl,             [edx-0xC];
//							mov         ch,             [edx+eax-0xC];//ecx = Cr Cb
//							pinsrw		MM0,			ecx,		2;
//							mov         cl,             [edx-0x10];
//							mov         ch,             [edx+eax-0x10];//ecx = Cr Cb
//							pinsrw		MM0,			ecx,		3;
//							
//							mov         cl,             [esi-4];
//							mov         ch,             [esi-8];	//ecx = Y Y
//							pinsrw		MM1,			ecx,		0;
//							mov         cl,             [esi-0xC];
//							mov         ch,             [esi-0x10];
//							pinsrw		MM1,			ecx,		1;
//							mov         cl,             [esi-0x14];
//							mov         ch,             [esi-0x18]; 
//							pinsrw		MM2,			ecx,		2;
//							mov         cl,             [esi-0x1C];
//							mov         ch,             [esi-0x20];
//							pinsrw		MM2,			ecx,		3;
//							
//							punpcklbw	mm1,			mm0;//Low: Y Cb Y Cr				
//							punpckhbw	mm2,			mm0;//High: Y Cb Y Cr
//							
//							movq		[edi],			mm1;
//							movq		[edi+0x8],		mm2;
//							
//							lea         esi,            [esi-0x20];
//							lea         edx,            [edx-0x10];
//							lea         edi,            [edi+0x10];
//							dec         iHLoop;
//							jne         I420toYUY2_HM_E_2;
//
//							mov         ecx,            iHCnt_B1;
//							test		ecx,			ecx;
//							jna			NextLine16;
//							mov         iHLoop_B1,      ecx;
//I420toYUY2_HM2_R_MMX:	
//							pcmpeqw     mm0,            mm0;			//xmm0:FFFFFFFF FFFFFFF	
//							psllw       mm0,            15;				//xmm0:80008000 80008000
//							movq		[edi],			mm0;
//							lea         edi,            [edi+0x8];			
//							dec         iHLoop_B1;
//							jne         I420toYUY2_HM2_R_MMX;
//							//-------------------------------------------------------------
//NextLine16:					mov         ecx,            iHCnt;
//							mov         ebx,            iCbCrWidth;
//							lea         edx,            [edx+ebx];
//							mov         iHLoop,         ecx;
//							mov         ebx,            iYLineStep;
//							lea         esi,            [esi+ebx];
//							
//I420toYUY2_HM_O_2:
//							mov         cl,             [edx-4];
//							mov         ch,             [edx+eax-4];  //ecx = Cr Cb
//							pinsrw		MM0,			ecx,		0;
//							mov         cl,             [edx-8];
//							mov         ch,             [edx+eax-8];//ecx = Cr Cb
//							pinsrw		MM0,			ecx,		1;
//							mov         cl,             [edx-0xC];
//							mov         ch,             [edx+eax-0xC];//ecx = Cr Cb
//							pinsrw		MM0,			ecx,		2;
//							mov         cl,             [edx-0x10];
//							mov         ch,             [edx+eax-0x10];//ecx = Cr Cb
//							pinsrw		MM0,			ecx,		3;
//							
//							mov         cl,             [esi-4];
//							mov         ch,             [esi-8];	//ecx = Y Y
//							pinsrw		MM1,			ecx,		0;
//							mov         cl,             [esi-0xC];
//							mov         ch,             [esi-0x10];
//							pinsrw		MM1,			ecx,		1;
//							mov         cl,             [esi-0x14];
//							mov         ch,             [esi-0x18]; 
//							pinsrw		MM2,			ecx,		2;
//							mov         cl,             [esi-0x1C];
//							mov         ch,             [esi-0x20];
//							pinsrw		MM2,			ecx,		3;
//							
//							punpcklbw	mm1,			mm0;//Low: Y Cb Y Cr				
//							punpckhbw	mm2,			mm0;//High: Y Cb Y Cr
//							
//							movq		[edi],			mm1;
//							movq		[edi+0x8],		mm2;
//							
//							lea         esi,            [esi-0x20];
//							lea         edx,            [edx-0x10];
//							lea         edi,            [edi+0x10];
//							dec         iHLoop;
//							jne         I420toYUY2_HM_O_2;
//
//							mov         ecx,            iHCnt_B;
//							test		ecx,			ecx;
//							jna			NextLine17;
//							mov         iHLoop_B,		ecx;
//I420toYUY2_HM2_OR_MMX:
//							pcmpeqw     mm0,            mm0;			//xmm0:FFFFFFFF FFFFFFF	
//							psllw       mm0,            15;				//xmm0:80008000 80008000
//							movq		[edi],			mm0;
//							lea         edi,            [edi+0x8];			
//							dec         iHLoop_B;
//							jne         I420toYUY2_HM2_OR_MMX;
//							//-------------------------------------------------------------
//NextLine17:					mov         ebx,            iYLineStep;
//							lea         esi,            [esi+ebx];
//							mov         ebx,            iCLineStep;
//							lea         edx,            [edx+ebx];
//							mov         ecx,            iHCnt;
//							mov         iHLoop,         ecx;
//							dec         iVCnt;
//							jne         I420toYUY2_HM_V_2_MMX;
//							
//							emms;
//						}
//					} 
//				}
//			}//END iSWSubSample=2
//		}
//#else//C
//		{
//			if(!bHMirror & !bVMirror)
//			{
//				for (int j=0; j<iVoSize; j++)
//				{
//					for (int i=0; i<iHoSize; i+=2)
//					{
//						if ((i < iHoStart) || (i > iHoEnd))
//						{
//							*pYUY2Start++ = 0;
//							*pYUY2Start++ = 127;
//							*pYUY2Start++ = 0;
//							*pYUY2Start++ = 127;
//						}
//						else
//						{
//							*pYUY2Start++ = pYStart[((j*iBufHSize + i)<<iSWSubSample)];
//							*pYUY2Start++ = pCbStart[((j>>1)*iCBufHSize + (i>>1))<<iSWSubSample];
//							*pYUY2Start++ = pYStart[((j*iBufHSize + i)<<iSWSubSample)+(1<<iSWSubSample)];
//							*pYUY2Start++ = pCrStart[((j>>1)*iCBufHSize + (i>>1))<<iSWSubSample];
//						}
//					}
//				}
//			}
//			else if(bHMirror & !bVMirror)
//			{
//				for (int j=0; j<iVoSize; j++)
//				{
//					for (int i=iHoSize-2; i>=0; i-=2)
//					{
//						if ((i < iHoStart) || (i > iHoEnd))
//						{
//							*pYUY2Start++ = 0;
//							*pYUY2Start++ = 127;
//							*pYUY2Start++ = 0;
//							*pYUY2Start++ = 127;
//						}
//						else
//						{
//							*pYUY2Start++ = pYStart[((j*iBufHSize + i)<<iSWSubSample)+(1<<iSWSubSample)];
//							*pYUY2Start++ = pCbStart[((j>>1)*iCBufHSize + (i>>1))<<iSWSubSample];
//							*pYUY2Start++ = pYStart[((j*iBufHSize + i)<<iSWSubSample)];
//							*pYUY2Start++ = pCrStart[((j>>1)*iCBufHSize + (i>>1))<<iSWSubSample];
//						}
//					}
//				}
//			}
//			else if(!bHMirror & bVMirror)
//			{
//				for (int j=iVoSize-1; j>=0; j--)
//				{
//					for (int i=0; i<iHoSize; i+=2)
//					{
//						if ((i < iHoStart) || (i > iHoEnd))
//						{
//							*pYUY2Start++ = 0;
//							*pYUY2Start++ = 127;
//							*pYUY2Start++ = 0;
//							*pYUY2Start++ = 127;
//						}
//						else
//						{
//							*pYUY2Start++ = pYStart[((j*iBufHSize + i)<<iSWSubSample)];
//							*pYUY2Start++ = pCbStart[((j>>1)*iCBufHSize + (i>>1))<<iSWSubSample];
//							*pYUY2Start++ = pYStart[((j*iBufHSize + i)<<iSWSubSample)+(1<<iSWSubSample)];
//							*pYUY2Start++ = pCrStart[((j>>1)*iCBufHSize + (i>>1))<<iSWSubSample];
//						}
//					}
//				}
//			}
//			else
//			{
//				for (int j=iVoSize-1; j>=0; j--)
//				{
//					for (int i=iHoSize-2; i>=0; i-=2)
//					{
//						if ((i < iHoStart) || (i > iHoEnd))
//						{
//							*pYUY2Start++ = 0;
//							*pYUY2Start++ = 127;
//							*pYUY2Start++ = 0;
//							*pYUY2Start++ = 127;
//						}
//						else
//						{
//							*pYUY2Start++ = pYStart[((j*iBufHSize + i)<<iSWSubSample)+(1<<iSWSubSample)];
//							*pYUY2Start++ = pCbStart[((j>>1)*iCBufHSize + (i>>1))<<iSWSubSample];
//							*pYUY2Start++ = pYStart[((j*iBufHSize + i)<<iSWSubSample)];
//							*pYUY2Start++ = pCrStart[((j>>1)*iCBufHSize + (i>>1))<<iSWSubSample];
//						}
//					}
//				}
//			}
//		}
//#endif
//}
// shawn 2011/07/12 use like driver version -----
//---------------------------------------------------------------------------
void CShell_YUVRotate(CShell *mp)
{
	ShellParam *pShellParam = &mp->Param;
	ScaleParam *pScaleParam = CShell_GetScaleParam(mp);

	RotateState eRotateState = pShellParam->eRotateState;
	if ((eRotateState == rsNone) || (eRotateState == rs0) || (eRotateState == rs180))
		return;

	int	s32StepH	= 1 << pScaleParam->a32SWSubSample[0];
	int	s32StepV	= 1 << pScaleParam->a32SWSubSample[1];
	int iBufHSize = pScaleParam->iBufHSize;
	int iBufVSize = pScaleParam->iBufVSize;
	int iHiStart = pScaleParam->iHStart;
	int iViStart = pScaleParam->iVStart;
	int iHoSize = pScaleParam->iHoSize;
	int iVoSize = pScaleParam->iVoSize;
	int iYBufSize = iBufHSize *iBufVSize;
	int iCBufSize = iYBufSize /4;
	int iYMiddleOffset = (iHoSize-iVoSize) * s32StepH /2;
	int iCMiddleOffset = (iHoSize-iVoSize) * s32StepH /2 /2;

	if (iHoSize > iVoSize)
		iHoSize = iVoSize;
	else if (iVoSize > iHoSize)
		iVoSize = iHoSize;

	int iYLineStep = iBufHSize * s32StepV;
	int iCLineStep = iBufHSize * s32StepV / 2;

	int i, j, si, di;
	UInt8 tmp;
    
	UInt8 *pY = mp->pProcBuf +(iViStart)*iBufHSize +iHiStart;
	UInt8 *pCb = mp->pProcBuf +iYBufSize +(iViStart/2)*(iBufHSize/2) +(iHiStart/2);
	UInt8 *pCr = mp->pProcBuf +iYBufSize +iCBufSize +(iViStart/2)*(iBufHSize/2) +(iHiStart/2);
	
	pY  += iYMiddleOffset;
	pCb += iCMiddleOffset;
	pCr += iCMiddleOffset;
	
	for (j = 0 ; j < iVoSize; j ++)
	{
		si = di = 0;
		
		for (i = (j + 1); i < iVoSize; i ++)
		{
			si	+= s32StepH;
			di	+= iYLineStep;
			
			tmp		= pY[di];
			pY[di]	= pY[si];
			pY[si]	= tmp;
		}
		
		pY	+= iYLineStep + s32StepH;
	}
	
	for (j = 0 ; j < iVoSize / 2; j ++)
	{
		si = di = 0;
		
		for (i = (j + 1); i < iVoSize / 2; i ++)
		{
			si	+= s32StepH;
			di	+= iCLineStep;
			
			tmp		= pCb[di];
			pCb[di]	= pCb[si];
			pCb[si]	= tmp;
			
			tmp		= pCr[di];
			pCr[di]	= pCr[si];
			pCr[si]	= tmp;
		}
		
		pCb	+= iCLineStep + s32StepH;
		pCr	+= iCLineStep + s32StepH;
	}
}
//---------------------------------------------------------------------------

ShellParam *CShell_GetParam(CShell *mp)  
{
	return &(mp->Param);
}
//---------------------------------------------------------------------------

ShellReport *CShell_GetReport(CShell *mp) 
{
	return &(mp->Report);
}
//---------------------------------------------------------------------------

JPEGParam *CShell_GetJPEGParam(CShell *mp)
{
	return CJPEG_GetParam(mp->pJPEG);
}
//---------------------------------------------------------------------------

JPEGReport *CShell_GetJPEGReport(CShell *mp)
{
	return CJPEG_GetReport(mp->pJPEG);
}
//---------------------------------------------------------------------------

ScaleParam *CShell_GetScaleParam(CShell *mp)
{
	return CScale_GetParam(mp->pScale);
}
//---------------------------------------------------------------------------

ScaleReport *CShell_GetScaleReport(CShell *mp)
{
	return CScale_GetReport(mp->pScale);
}
//---------------------------------------------------------------------------
FaceParam *CShell_GetFaceParam(CShell *mp)
{    
	return CFace_GetParam(mp->pFACE);
}
//---------------------------------------------------------------------------
FaceReport *CShell_GetFaceReport(CShell *mp)
{
	return CFace_GetReport(mp->pFACE);
}
//---------------------------------------------------------------------------
void CShell_SetBW(CShell *mp, BOOL bBW)
{
	mp->Param.bBW = bBW;
}
//---------------------------------------------------------------------------
void CShell_I420toRGB24(UInt8 *pSrc, UInt8 *pDst, int iHSize, int iVSize)
{
	SInt32 iHalfHSize = iHSize /2;
	SInt32 iQuarterBufSize = iHSize*iVSize /4;
 
	UInt8 *pY  = (UInt8*)pSrc;
	UInt8 *pCb = (UInt8*)(pY + iVSize*iHSize);
	UInt8 *pCr = (UInt8*)(pCb + (iVSize*iHSize>>2));
	UInt32 AddrY  = (UInt32)pY;
	UInt32 AddrCb = (UInt32)pCb;
	UInt32 AddrCr = (UInt32)pCr;
	UInt32 AddrDIB = (UInt32)pDst;

	#if defined(__MMX__)
	{
		//------------------------------------------------------------------------
		//R = 1.00000000000000*Y - 0.00000121889419*U + 1.40199958865734*V
		//G = 1.00000000000000*Y - 0.34413567816534*U - 0.71413615558181*V
		//B = 1.00000000000000*Y + 1.77200006607382*U + 0.00000040629806*V
		//64R = 64Y + 89V
		//64G = 64Y - 22U - 45V
		//64B = 64Y + 113U
		//0x0040*R = 0x0040*Y + 0x0071*V
		//0x0040*G = 0x0040*Y + 0xffea*U + 0xffd3*V
		//0x0040*B = 0x0040*Y + 0x0059*U
		const UInt64 U2B = 0x0071007100710071;
		const UInt64 U2G = 0xffeaffeaffeaffea;
		const UInt64 V2G = 0xffd3ffd3ffd3ffd3;
		const UInt64 V2R = 0x0059005900590059;

		SInt32 iRGBLineStep = iHSize *3;
		SInt32 iHCnt = (iHSize>>2) -1;
		SInt32 iVCnt = iVSize>>1;
		SInt32 iHLoop;
		__asm
		{
			mov         esi,            AddrY;
			mov         edx,            AddrCb;
			mov         edi,            AddrDIB;
			mov         ebx,            iRGBLineStep;
		Shell_I420toRGB24_V:
			mov         ecx,            iHCnt;
			push        edi;
			push        edx;
			push        esi;
			mov         iHLoop,         ecx;
		Shell_I420toRGB24_H:
			mov         eax,            iQuarterBufSize;
			pcmpeqw     mm7,            mm7;
			punpcklbw   mm4,            [edx];
			punpcklbw   mm6,            [edx+eax];
			psllw       mm7,            15;
			psubw       mm4,            mm7;
			psubw       mm6,            mm7;
			psraw       mm4,            8;
			psraw       mm6,            8;
			punpcklwd   mm4,            mm4;            //mm4 = U2 U2 U0 U0
			punpcklwd   mm6,            mm6;            //mm6 = V2 V2 V0 V0
			movq        mm5,            mm4;            //mm4 = mm5 = U2 U2 U0 U0
			movq        mm7,            mm6;            //mm6 = mm7 = V2 V2 V0 V0
			//-------------------------------------------------------------
			movq        mm0,            U2G;
			movq        mm1,            V2G;
			pmullw      mm4,            mm0;
			pmullw      mm6,            mm1;
			movq        mm0,            U2B;
			movq        mm1,            V2R;
			pmullw      mm5,            mm0;            //mm5 = B3' B2' B1' B0'
			pmullw      mm7,            mm1;            //mm7 = R3' R2' R1' R0'
			paddsw      mm6,            mm4;            //mm6 = G3' G2' G1' G0'
			//-------------------------------------------------------------
			//pxor        mm1,            mm1;          //error effect is less than 1
			punpcklbw   mm1,            [esi];
			psrlw       mm1,            2;
			movq        mm0,            mm1;
			movq        mm2,            mm1;            //mm0 = mm1 = mm2 = 64*(Y3 Y2 Y1 Y0)
			paddsw      mm0,            mm5;
			paddsw      mm1,            mm6;
			paddsw      mm2,            mm7;
			psraw       mm0,            6;              //mm0 = B3 B2 B1 B0
			psraw       mm1,            6;              //mm1 = G3 G2 G1 G0
			psraw       mm2,            6;              //mm2 = R3 R2 R1 R0
			//-------------------------------------------------------------
			packuswb    mm0,            mm0;            //mm0 = B3 B2 B1 B0 B3 B2 B1 B0
			packuswb    mm1,            mm1;            //mm1 = G3 G2 G1 G0 G3 G2 G1 G0
			packuswb    mm2,            mm2;            //mm2 = R3 R2 R1 R0 R3 R2 R1 R0
			punpcklbw   mm0,            mm1;
			punpcklbw   mm2,            mm2;            //mm2 = R3 R3 R2 R2 R1 R1 R0 R0
			movq        mm1,            mm0;            //mm0 = mm1 = G3 B3 G2 B2 G1 B1 G0 B0
			punpcklwd   mm0,            mm2;            //mm0 = R1 R1 G1 B1 R0 R0 G0 B0
			punpckhwd   mm1,            mm2;            //mm1 = R3 R3 G3 B3 R2 R2 G2 B2
			movq        mm2,            mm0;
			movq        mm3,            mm1;
			psllq       mm0,            40;             //mm0 = R0 G0 B0  0  0  0  0  0
			psllq       mm1,            40;             //mm1 = R2 G2 B2  0  0  0  0  0
			punpckhdq   mm0,            mm2;            //mm0 = R1 R1 G1 B1 R0 G0 B0  0
			punpckhdq   mm1,            mm3;            //mm1 = R3 R3 G3 B3 R2 G2 B2  0
			psrlq       mm0,            8;
			psrlq       mm1,            8;
			movq        [edi],          mm0;
			movq        [edi+6],        mm1;
			//-------------------------------------------------------------
			mov         eax,            iHalfHSize;
			//pxor        mm1,            mm1;            //error effect is less than 1
			punpcklbw   mm1,            [esi+eax*2];
			//-------------------------------------------------------------
			psrlw       mm1,            2;              //mm1 = 64*(Y3 Y2 Y1 Y0)
			paddsw      mm5,            mm1;
			paddsw      mm6,            mm1;
			paddsw      mm7,            mm1;
			psraw       mm5,            6;              //mm5 = B3 B2 B1 B0
			psraw       mm6,            6;              //mm6 = G3 G2 G1 G0
			psraw       mm7,            6;              //mm7 = R3 R2 R1 R0
			//-------------------------------------------------------------
			packuswb    mm5,            mm5;            //mm5 = B3 B2 B1 B0 B3 B2 B1 B0
			packuswb    mm6,            mm6;            //mm6 = G3 G2 G1 G0 G3 G2 G1 G0
			packuswb    mm7,            mm7;            //mm7 = R3 R2 R1 R0 R3 R2 R1 R0
			punpcklbw   mm5,            mm6;
			punpcklbw   mm7,            mm7;            //mm7 = R3 R3 R2 R2 R1 R1 R0 R0
			movq        mm6,            mm5;            //mm5 = mm6 = G3 B3 G2 B2 G1 B1 G0 B0
			punpcklwd   mm5,            mm7;            //mm5 = R1 R1 G1 B1 R0 R0 G0 B0
			punpckhwd   mm6,            mm7;            //mm6 = R3 R3 G3 B3 R2 R2 G2 B2
			movq        mm4,            mm5;
			movq        mm7,            mm6;
			psllq       mm4,            40;             //mm4 = R0 G0 B0  0  0  0  0  0
			psllq       mm7,            40;             //mm7 = R2 G2 B2  0  0  0  0  0
			punpckhdq   mm4,            mm5;            //mm4 = R1 R1 G1 B1 R0 G0 B0  0
			punpckhdq   mm7,            mm6;            //mm7 = R3 R3 G3 B3 R2 G2 B2  0
			psrlq       mm4,            8;
			psrlq       mm7,            8;
			movq        [edi+ebx],      mm4;
			movq        [edi+ebx+6],    mm7;
			//-------------------------------------------------------------
			lea         esi,            [esi+4];
			lea         edx,            [edx+2];
			lea         edi,            [edi+12];
			dec         iHLoop;
			jne         Shell_I420toRGB24_H;
			//-------------------------------------------------------------
		Shell_I420toRGB24_HLast:
			mov         eax,            iQuarterBufSize;
			pcmpeqw     mm7,            mm7;
			punpcklbw   mm4,            [edx];
			punpcklbw   mm6,            [edx+eax];
			psllw       mm7,            15;
			psubw       mm4,            mm7;
			psubw       mm6,            mm7;
			psraw       mm4,            8;
			psraw       mm6,            8;
			punpcklwd   mm4,            mm4;            //mm4 = U2 U2 U0 U0
			punpcklwd   mm6,            mm6;            //mm6 = V2 V2 V0 V0
			movq        mm5,            mm4;            //mm4 = mm5 = U2 U2 U0 U0
			movq        mm7,            mm6;            //mm6 = mm7 = V2 V2 V0 V0
			//-------------------------------------------------------------
			movq        mm0,            U2G;
			movq        mm1,            V2G;
			pmullw      mm4,            mm0;
			pmullw      mm6,            mm1;
			movq        mm0,            U2B;
			movq        mm1,            V2R;
			pmullw      mm5,            mm0;            //mm5 = B3' B2' B1' B0'
			pmullw      mm7,            mm1;            //mm7 = R3' R2' R1' R0'
			paddsw      mm6,            mm4;            //mm6 = G3' G2' G1' G0'
			//-------------------------------------------------------------
			//pxor        mm1,            mm1;          //error effect is less than 1
			punpcklbw   mm1,            [esi];
			psrlw       mm1,            2;
			movq        mm0,            mm1;
			movq        mm2,            mm1;            //mm0 = mm1 = mm2 = 64*(Y3 Y2 Y1 Y0)
			paddsw      mm0,            mm5;
			paddsw      mm1,            mm6;
			paddsw      mm2,            mm7;
			psraw       mm0,            6;              //mm0 = B3 B2 B1 B0
			psraw       mm1,            6;              //mm1 = G3 G2 G1 G0
			psraw       mm2,            6;              //mm2 = R3 R2 R1 R0
			//-------------------------------------------------------------
			packuswb    mm0,            mm0;            //mm0 = B3 B2 B1 B0 B3 B2 B1 B0
			packuswb    mm1,            mm1;            //mm1 = G3 G2 G1 G0 G3 G2 G1 G0
			packuswb    mm2,            mm2;            //mm2 = R3 R2 R1 R0 R3 R2 R1 R0
			punpcklbw   mm0,            mm1;
			punpcklbw   mm2,            mm2;            //mm2 = R3 R3 R2 R2 R1 R1 R0 R0
			movq        mm1,            mm0;            //mm0 = mm1 = G3 B3 G2 B2 G1 B1 G0 B0
			punpcklwd   mm0,            mm2;            //mm0 = R1 R1 G1 B1 R0 R0 G0 B0
			punpckhwd   mm1,            mm2;            //mm1 = R3 R3 G3 B3 R2 R2 G2 B2
			movq        mm2,            mm0;
			movq        mm3,            mm1;
			psllq       mm0,            40;             //mm0 = R0 G0 B0  0  0  0  0  0
			psllq       mm1,            40;             //mm1 = R2 G2 B2  0  0  0  0  0
			punpckhdq   mm0,            mm2;            //mm0 = R1 R1 G1 B1 R0 G0 B0  0
			punpckhdq   mm1,            mm3;            //mm1 = R3 R3 G3 B3 R2 G2 B2  0
			psrlq       mm0,            8;
			psrlq       mm1,            8;
			movq        [edi],          mm0;
			movd        [edi+6],        mm1;
			movd        ecx,            mm1;
			mov         [edi+10],       cx;
			//-------------------------------------------------------------
			mov         eax,            iHalfHSize;
			//pxor        mm1,            mm1;          //error effect is less than 1
			punpcklbw   mm1,            [esi+eax*2];
			//-------------------------------------------------------------
			psrlw       mm1,            2;              //mm1 = 64*(Y3 Y2 Y1 Y0)
			paddsw      mm5,            mm1;
			paddsw      mm6,            mm1;
			paddsw      mm7,            mm1;
			psraw       mm5,            6;              //mm5 = B3 B2 B1 B0
			psraw       mm6,            6;              //mm6 = G3 G2 G1 G0
			psraw       mm7,            6;              //mm7 = R3 R2 R1 R0
			//-------------------------------------------------------------
			packuswb    mm5,            mm5;            //mm5 = B3 B2 B1 B0 B3 B2 B1 B0
			packuswb    mm6,            mm6;            //mm6 = G3 G2 G1 G0 G3 G2 G1 G0
			packuswb    mm7,            mm7;            //mm7 = R3 R2 R1 R0 R3 R2 R1 R0
			punpcklbw   mm5,            mm6;
			punpcklbw   mm7,            mm7;            //mm7 = R3 R3 R2 R2 R1 R1 R0 R0
			movq        mm6,            mm5;            //mm5 = mm6 = G3 B3 G2 B2 G1 B1 G0 B0
			punpcklwd   mm5,            mm7;            //mm5 = R1 R1 G1 B1 R0 R0 G0 B0
			punpckhwd   mm6,            mm7;            //mm6 = R3 R3 G3 B3 R2 R2 G2 B2
			movq        mm4,            mm5;
			movq        mm7,            mm6;
			psllq       mm4,            40;             //mm4 = R0 G0 B0  0  0  0  0  0
			psllq       mm7,            40;             //mm7 = R2 G2 B2  0  0  0  0  0
			punpckhdq   mm4,            mm5;            //mm4 = R1 R1 G1 B1 R0 G0 B0  0
			punpckhdq   mm7,            mm6;            //mm7 = R3 R3 G3 B3 R2 G2 B2  0
			psrlq       mm4,            8;
			psrlq       mm7,            8;
			movq        [edi+ebx],      mm4;
			movd        [edi+ebx+6],    mm7;
			movd        ecx,            mm7;
			mov         [edi+ebx+10],   cx;
			//-------------------------------------------------------------
			pop         esi;
			pop         edx;
			pop         edi;
			lea         esi,            [esi+eax*4];
			lea         edx,            [edx+eax];
			lea         edi,            [edi+ebx*2];
			dec         iVCnt;
			jne         Shell_I420toRGB24_V;
			emms;
		}
	}
	#else
	/*{
		SInt32 iYOffset    = 0;
		SInt32 iCbCrOffset = 0;
		SInt32 iRGBOffset  = 0;
		for (int j = 0; j < iVSize; j++)
		{
			for (int i = 0; i < iHSize; i++)
			{
				SInt32 iYx512, iU, iV, iB, iG, iR;
				iB = iG = iR = pY[iYOffset +i] <<9;
				iB = (iB + (iU = pCb[iCbCrOffset +(i>>1)] -128) *907);
				iR = (iR + (iV = pCr[iCbCrOffset +(i>>1)] -128) *717);
				iG = (iG - iU*176 - iV*365);
				mpSwapDIBData_I420[iRGBOffset+i*3+0] = (UInt8)((iB>130560)? 255: ((iB<0)? 0: iB>>9));
				mpSwapDIBData_I420[iRGBOffset+i*3+1] = (UInt8)((iG>130560)? 255: ((iG<0)? 0: iG>>9));
				mpSwapDIBData_I420[iRGBOffset+i*3+2] = (UInt8)((iR>130560)? 255: ((iR<0)? 0: iR>>9));
			}
			iYOffset    += iHSize;
			iCbCrOffset += ((j&0x01) *(iHSize>>1));
			iRGBOffset  += (iHSize*3);
		}
	}*/
	#endif
}
//---------------------------------------------------------------------------

void CShell_YUVDefectCompensation(CShell *mp)
{
	if(!mp->Param.iDefectMode)
		return;

	JPEGParam  *pJPEGParam  = CShell_GetJPEGParam(mp);
	SInt32 iYBufHSize = pJPEGParam->iOutBufHSize;
	SInt32 iYBufVSize = pJPEGParam->iOutBufVSize;
	SInt32 iCBufHSize = iYBufHSize>>1;
	SInt32 iCBufVSize = iYBufVSize>>1;
	SInt32 iYHSize  = pJPEGParam->iDecHSize;
	SInt32 iYVSize  = pJPEGParam->iDecVSize;
	SInt32 iCHSize  = iYHSize>>1;
	SInt32 iCVSize  = iYVSize>>1;

	int i,j;

	UInt8 *pY  = mp->pProcBuf;
	UInt8 *pCb = pY  + iYBufHSize*iYBufVSize;
	UInt8 *pCr = pCb + iCBufHSize*iCBufVSize;

	SInt32 iYThd = mp->Param.iDefectYThd;
	SInt32 iCThd = mp->Param.iDefectCThd;
	SInt32 C,P0,P1,P2,P3,P4,P5,P6,P7;

	//Defect Mode : bit 1 VGA enable, bit 2 SXGA enable
	if(!(((mp->Param.iDefectMode & 1) & (iYHSize < 1280)) |
		(((mp->Param.iDefectMode>>1) & 1) & (iYHSize >= 1280))))
		return;

	for(i=2;i<iYVSize-2;i++)
	{
		for(j=2;j<iYHSize-2;j++)
		{
			C = pY[i*iYBufHSize+j];
			P3 = pY[(i)*iYBufHSize+j-2];
			
			if(C-P3 > iYThd)
			{
				P0 = pY[(i-1)*iYBufHSize+j-1];
				P1 = pY[(i-2)*iYBufHSize+j];
				P2 = pY[(i-1)*iYBufHSize+j+1];
				P4 = pY[(i)*iYBufHSize+j+2];
				P5 = pY[(i+1)*iYBufHSize+j-1];
				P6 = pY[(i+2)*iYBufHSize+j];
				P7 = pY[(i+1)*iYBufHSize+j+1];
				if(	(C-P0 > iYThd) + (C-P1 > iYThd) + (C-P2 > iYThd) +
					(C-P4 > iYThd) + (C-P5 > iYThd) +
					(C-P6 > iYThd) + (C-P7 > iYThd) > 5)
				{
					if(	(C >= pY[(i)*iYBufHSize+j+1]) &
						(C >= pY[(i)*iYBufHSize+j-1]) &
						(C >= pY[(i+1)*iYBufHSize+j]) &
						(C >= pY[(i-1)*iYBufHSize+j]) )
					{					
						if((P4 > P7) & (P6 > P7))
						{
							if(P4-P6 > iYThd)
								pY[(i+1)*iYBufHSize+j+1] = (UInt8)P6;
							else if(P6-P4 > iYThd)
								pY[(i+1)*iYBufHSize+j+1] = (UInt8)P4;
							else
								pY[(i+1)*iYBufHSize+j+1] = (UInt8)((P4+P6)>>1);
						}
						else if((P7-P4 > iYThd) & (P7-P6 > iYThd))
							pY[(i+1)*iYBufHSize+j+1] = (UInt8)((P7+max(P4,P6))>>1);

						if((P3 > P5) & (P6 > P5))
						{
							if(P3-P6 > iYThd)
								pY[(i+1)*iYBufHSize+j-1] = (UInt8)P6;
							else if(P6-P3 > iYThd)
								pY[(i+1)*iYBufHSize+j-1] = (UInt8)P3;
							else
								pY[(i+1)*iYBufHSize+j-1] = (UInt8)((P3+P6)>>1);
						}
						else if((P5 -P3 > iYThd) & (P5-P6 > iYThd))
							pY[(i+1)*iYBufHSize+j-1] = (UInt8)((P5+max(P3,P6))>>1);

						if((P4 > P2) & (P1 > P2))
						{
							if(P4-P1 > iYThd)
								pY[(i-1)*iYBufHSize+j+1] = (UInt8)P1;
							else if(P1-P4 > iYThd)
								pY[(i-1)*iYBufHSize+j+1] = (UInt8)P4;
							else
								pY[(i-1)*iYBufHSize+j+1] = (UInt8)((P4+P1)>>1);
						}
						else if((P2 -P4 > iYThd) & (P2-P1 > iYThd))
							pY[(i-1)*iYBufHSize+j+1] = (UInt8)((P2+max(P4,P1))>>1);

						if((P1 > P0) & (P3 > P0))
						{
							if(P3-P1 > iYThd)
								pY[(i-1)*iYBufHSize+j-1] = (UInt8)P1;
							else if(P1-P3 > iYThd)
								pY[(i-1)*iYBufHSize+j-1] = (UInt8)P3;
							else
								pY[(i-1)*iYBufHSize+j-1] = (UInt8)((P1+P3)>>1);
						}
						else if((P0 -P1 > iYThd) & (P0-P3 > iYThd))
							pY[(i-1)*iYBufHSize+j-1] = (UInt8)((P0+max(P1,P3))>>1);

						P1 = max(max(min(P1,P4),min(P3,P6)),min(max(P1,P4),max(P3,P6)));
						P2 = max(max(min(P0,P2),min(P5,P7)),min(max(P0,P2),max(P5,P7)));
						C = max(P1,P2);
						pY[i*iYBufHSize+j] = (UInt8)C;

						if(pY[(i)*iYBufHSize+j-1] > C)
							pY[(i)*iYBufHSize+j-1] = (P3-C > iYThd)?(UInt8)C:(UInt8)((P3+C)>>1);
						if(pY[(i)*iYBufHSize+j+1] > C)
							pY[(i)*iYBufHSize+j+1] = (P4-C > iYThd)?(UInt8)C:(UInt8)((P4+C)>>1);
						if(pY[(i-1)*iYBufHSize+j] > C)
							pY[(i-1)*iYBufHSize+j] = (P1-C > iYThd)?(UInt8)C:(UInt8)((P1+C)>>1);
						if(pY[(i+1)*iYBufHSize+j] > C)
							pY[(i+1)*iYBufHSize+j] = (P6-C > iYThd)?(UInt8)C:(UInt8)((P6+C)>>1);
					}
				}
			}
		}
	}

	for(i=2;i<iCVSize-2;i++)
	{
		for(j=2;j<iCHSize-2;j++)
		{
			C = pCr[i*iCBufHSize+j];
			P3 = pCr[(i)*iCBufHSize+j-2];
			if(C-P3 > iCThd)
			{
				P0 = pCr[(i-2)*iCBufHSize+j-2];
				P1 = pCr[(i-2)*iCBufHSize+j];
				P2 = pCr[(i-2)*iCBufHSize+j+2];
				P4 = pCr[(i)*iCBufHSize+j+2];
				P5 = pCr[(i+2)*iCBufHSize+j-2];
				P6 = pCr[(i+2)*iCBufHSize+j];
				P7 = pCr[(i+2)*iCBufHSize+j+2];
				if(	((C-P0) > iCThd) + ((C-P1) > iCThd) + ((C-P2) > iCThd) +
					((C-P4) > iCThd) + ((C-P5) > iCThd) +
					((C-P6) > iCThd) + ((C-P7) > iCThd) > 5)
				{
					pCr[i*iCBufHSize+j] = (UInt8)min(max(P3,P4),max(P1,P6));
				}
			}
			else if(P3-C > iCThd)
			{
				P0 = pCr[(i-2)*iCBufHSize+j-2];
				P1 = pCr[(i-2)*iCBufHSize+j];
				P2 = pCr[(i-2)*iCBufHSize+j+2];
				P4 = pCr[(i)*iCBufHSize+j+2];
				P5 = pCr[(i+2)*iCBufHSize+j-2];
				P6 = pCr[(i+2)*iCBufHSize+j];
				P7 = pCr[(i+2)*iCBufHSize+j+2];
				if(	((P0-C) > iCThd) + ((P1-C) > iCThd) + ((P2-C) > iCThd) +
					((P4-C) > iCThd) + ((P5-C) > iCThd) +
					((P6-C) > iCThd) + ((P7-C) > iCThd) > 5)
				{
					pCr[i*iCBufHSize+j] = (UInt8)max(min(P3,P4),min(P1,P6));
				}
			}
			C = pCb[i*iCBufHSize+j];
			P3 = pCb[(i)*iCBufHSize+j-2];
			if(C-P3 > iCThd)
			{
				P0 = pCb[(i-1)*iCBufHSize+j-1];
				P1 = pCb[(i-2)*iCBufHSize+j];
				P2 = pCb[(i-1)*iCBufHSize+j+1];
				P4 = pCb[(i)*iCBufHSize+j+2];
				P5 = pCb[(i+1)*iCBufHSize+j-1];
				P6 = pCb[(i+2)*iCBufHSize+j];
				P7 = pCb[(i+1)*iCBufHSize+j+1];
				if(	(C-P0 > iCThd) + (C-P1 > iCThd) + (C-P2 > iCThd) +
					(C-P4 > iCThd) + (C-P5 > iCThd) +
					(C-P6 > iCThd) + (C-P7 > iCThd) > 5)
				{
					pCb[i*iCBufHSize+j] = (UInt8)min(max(P3,P4),max(P1,P6));
				}
			}
			else if(P3-C > iCThd)
			{
				P0 = pCb[(i-1)*iCBufHSize+j-1];
				P1 = pCb[(i-2)*iCBufHSize+j];
				P2 = pCb[(i-1)*iCBufHSize+j+1];
				P4 = pCb[(i)*iCBufHSize+j+2];
				P5 = pCb[(i+1)*iCBufHSize+j-1];
				P6 = pCb[(i+2)*iCBufHSize+j];
				P7 = pCb[(i+1)*iCBufHSize+j+1];
				if(	(P0-C > iCThd) + (P1-C > iCThd) + (P2-C > iCThd) +
					(P4-C > iCThd) + (P5-C > iCThd) +
					(P6-C > iCThd) + (P7-C > iCThd) > 5)
				{
					pCb[i*iCBufHSize+j] = (UInt8)max(min(P3,P4),min(P1,P6));
				}
			}
		}
	}
}
//---------------------------------------------------------------------------
void CShell_BlackWhite(CShell *mp)
{
	if(!mp->Param.bBW)
		return;

	JPEGParam  *pJPEGParam  = CShell_GetJPEGParam(mp);
	SInt32 iYBufHSize = pJPEGParam->iOutBufHSize;
	SInt32 iYBufVSize = pJPEGParam->iOutBufVSize;
	SInt32 iCBufHSize = iYBufHSize>>1;
	SInt32 iCBufVSize = iYBufVSize>>1;
	SInt32 iYHSize  = pJPEGParam->iDecHSize;
	SInt32 iYVSize  = pJPEGParam->iDecVSize;
	SInt32 iCHSize  = iYHSize>>1;
	SInt32 iCVSize  = iYVSize>>1;
	SInt32 iCHStart	= pJPEGParam->iDecHStart>>1;
	SInt32 iCVStart	= pJPEGParam->iDecVStart>>1;

	int i,j;

	UInt8 *pCb = mp->pProcBuf  + iYBufHSize*iYBufVSize;
	UInt8 *pCr = pCb + iCBufHSize*iCBufVSize;

	for(i=iCVStart;i<iCVSize+iCVStart;i++)
	{
		memset(pCb+i*iCBufHSize+iCHStart,128, iCHSize);
		memset(pCr+i*iCBufHSize+iCHStart,128, iCHSize);
	}
}
//---------------------------------------------------------------------------
void CShell_YUYVProcess(CShell *mp)
{
	ShellParam *pShellParam = &mp->Param;
	const JPEGParam  *pJPEGParam  = CShell_GetJPEGParam(mp);

	int iYBufHSize = pJPEGParam->iOutBufHSize;
	int iYBufVSize = pJPEGParam->iOutBufVSize;

	int iCBufHSize = iYBufHSize>>1;
	int iCBufVSize = iYBufVSize>>1;

	int iDecHSize = pJPEGParam->iStrmHSize;
	int iDecVSize = pJPEGParam->iStrmVSize;
	int iYBufSize = iYBufHSize *iYBufVSize;
	int iCBufSize = iYBufSize >> 2;
	UInt8 *pStream = mp->pStream;
	UInt8 *pYBuf = mp->pProcBuf;
	UInt8 *pCBuf = mp->pProcBuf + iYBufSize;
	UInt8 *pCbBuf = (UInt8*)(pCBuf);
	UInt8 *pCrBuf = (UInt8*)(pCBuf + iCBufSize);

	int iSLineStep = iDecHSize;
	int iYLineStep = iYBufHSize;
	int iCLineStep = iYBufHSize>>1;

	UInt8 *pSLine = pStream;
	UInt8 *pYLine = pYBuf;
	UInt8 *pCLine = pCBuf;
	
#if defined(__MMX__)
	{
		int iWidthGap = iYBufHSize - (iDecHSize>>1);
		__int64 ZERO_FF_16	= 0x00ff00ff00ff00ff;
		__int64 ZERO_FF_32	= 0x000000ff000000ff;
		__asm
		{
			mov			ecx,			iDecVSize;
			mov			esi,			pStream;
			mov			edi,			pYBuf;
			mov			eax,			pCbBuf;
			mov			ebx,			pCrBuf;
			shr			ecx,			1;
			mov			edx,			iWidthGap;
			movq		mm7,			ZERO_FF_16;
			movq		mm6,			ZERO_FF_32;
			shr			edx,			1;
YUYV_V_LOOP:
			push		ecx;
			mov			ecx,			iDecHSize;
			shr			ecx,			4;
YUYV_H_LOOP1:											//		LSB                 MSB
			movq        mm0,			[esi];			//mm0 = Y0 U0 Y1 V0 Y2 U1 Y3 V1
			movq        mm1,			[esi+8];		//mm1 = Y4 U2 Y5 V2 Y6 U3 Y7 V3
			movq        mm2,			mm0;            
			movq		mm3,			mm1;
			pand		mm2,			mm7;			//mm2 = Y0 00 Y1 00 Y2 00 Y3 00
			pand		mm3,			mm7;			//mm3 = Y4 00 Y5 00 Y6 00 Y7 00
			psrlq		mm0,			8;				//mm0 = U0 Y1 V0 Y2 U1 Y3 V1 00
			psrlq		mm1,			8;				//mm1 = U2 Y5 V2 Y6 U3 Y7 V3 00
			packuswb	mm2,			mm3;			//mm3 = Y0 Y1 Y2 Y3 Y4 Y5 Y6 Y7
			movq		mm4,			mm0;
			movq		mm5,			mm1;
			movq		[edi],			mm2;			//save to Y buffer
			pand		mm0,			mm6;			//mm0 = U0 00 00 00 U1 00 00 00
			pand		mm1,			mm6;			//mm1 = U2 00 00 00 U3 00 00 00
			psrlq		mm4,			16;
			psrlq		mm5,			16;
			packssdw	mm0,			mm1;			//mm0 = U0 00 U1 00 U2 00 U3 00
			pand		mm4,			mm6;			//mm0 = V0 00 00 00 V1 00 00 00
			pand		mm5,			mm6;			//mm1 = V2 00 00 00 V3 00 00 00
			packssdw	mm4,			mm5;			//mm2 = V0 00 V1 00 V2 00 V3 00
			packuswb	mm0,			mm0;			//mm0 = U0 U1 U2 U3 U0 U1 U2 U3
			packuswb	mm4,			mm4;			//mm2 = V0 V1 V2 V3 V0 U1 V2 V3
			movd		[eax],			mm0;			//save to Cb buffer
			movd		[ebx],			mm4;			//save to Cr buffer
			add			esi,			16;
			add			edi,			8;
			add			eax,			4;
			add			ebx,			4;
			dec         ecx;
			jnz         YUYV_H_LOOP1;
			mov			ecx,			iDecHSize;
			lea			edi,			[edi + edx*2];
			lea			eax,			[eax + edx];
			lea			ebx,			[ebx + edx];
			shr			ecx,			4;
YUYV_H_LOOP2:											//		LSB                 MSB
			movq        mm0,			[esi];			//mm0 = Y0 U0 Y1 V0 Y2 U1 Y3 V1
			movq        mm1,			[esi+8];		//mm1 = Y4 U2 Y5 V2 Y6 U3 Y7 V3
			movq        mm2,			mm0;            
			movq		mm3,			mm1;
			pand		mm2,			mm7;			//mm2 = Y0 00 Y1 00 Y2 00 Y3 00
			pand		mm3,			mm7;			//mm3 = Y4 00 Y5 00 Y6 00 Y7 00
			packuswb	mm2,			mm3;			//mm3 = Y0 Y1 Y2 Y3 Y4 Y5 Y6 Y7
			movq		[edi],			mm2;			//save to Y buffer
			add			esi,			16;
			add			edi,			8;
			dec         ecx;
			jnz         YUYV_H_LOOP2;
			pop			ecx;
			lea			edi,			[edi + edx*2];
			dec			ecx;
			jnz			YUYV_V_LOOP;
			emms;
		}
	}
#else
	int i,j;
	for (j=0; j<iDecVSize; j+=2)
	{
		for (i=0; i<iDecHSize; i+=4)
		{			
			UInt8 *pCb			= pCLine;
			UInt8 *pCr			= pCLine+iCBufSize;
			pYLine[(i>>1)]		= pSLine[i+0];
			pYLine[(i>>1)+1]	= pSLine[i+2];
			pCb[(i>>2)]			= pSLine[i+1];
			pCr[(i>>2)]			= pSLine[i+3];
		}
		pYLine+=iYLineStep;
		pCLine+=iCLineStep;
		pSLine+=iSLineStep;
		for (i=0; i<iDecHSize; i+=4)
		{
			pYLine[(i>>1)]		= pSLine[i+0];
			pYLine[(i>>1)+1]	= pSLine[i+2];
		}
		pYLine+=iYLineStep;
		pSLine+=iSLineStep;
	}
#endif
}
//---------------------------------------------------------------------------
void CShell_YUYV_S420_Process(CShell *mp)
{
	ShellParam *pShellParam = &mp->Param;
	const JPEGParam  *pJPEGParam  = CShell_GetJPEGParam(mp);

	int iYBufHSize = pJPEGParam->iOutBufHSize;
	int iYBufVSize = pJPEGParam->iOutBufVSize;

	int iCBufHSize = iYBufHSize>>1;
	int iCBufVSize = iYBufVSize>>1;

	int iDecHSize = pJPEGParam->iStrmHSize;
	int iDecVSize = pJPEGParam->iStrmVSize;
	int iYBufSize = iYBufHSize *iYBufVSize;
	int iCBufSize = iYBufSize >> 2;
	UInt8 *pStream = mp->pStream;
	UInt8 *pYBuf = mp->pProcBuf;
	UInt8 *pCBuf = mp->pProcBuf + iYBufSize;
	UInt8 *pCbBuf = (UInt8*)(pCBuf);
	UInt8 *pCrBuf = (UInt8*)(pCBuf + iCBufSize);
	//2010/11/10 04:03下午
//	int iSLineStep = iDecHSize;
	int iSLineStep = (iDecHSize*3)>>2;
	int iYLineStep = iYBufHSize;
	int iCLineStep = iYBufHSize>>1;

	UInt8 *pSLine = pStream;
	UInt8 *pYLine = pYBuf;
	UInt8 *pCLine = pCBuf;
	
	//YUYV S420 -> YUV
	UInt8 *pCb			= pCLine;
	UInt8 *pCr			= pCLine+iCBufSize;	
	UInt16 *pSLine16	= (UInt16 *)pSLine;
	UInt16 *pYLine16	= (UInt16 *)pYLine;
	UInt16 *pCb16		= (UInt16 *)pCb;
	UInt16 *pCr16		= (UInt16 *)pCr;
	
	int i,j;

	for (j=0; j<iDecVSize; j+=2)
	{
		for (i=0; i<iDecHSize>>3; i++)
		{
			
			pYLine16[j*(iYLineStep>>1) + ((i<<1)  )] = pSLine16[j*(iSLineStep>>1) + (3*i  )];
			pYLine16[j*(iYLineStep>>1) + ((i<<1)+1)] = pSLine16[j*(iSLineStep>>1) + (3*i+1)];
			pCb16[(j>>1)*(iCLineStep>>1) + ((i)  )] = pSLine16[(j)*(iSLineStep>>1) + (3*i+2)];
		}
		
		for (i=0; i<iDecHSize>>3; i++)
		{
			pYLine16[(j+1)*(iYLineStep>>1) + ((i<<1)  )] = pSLine16[(j+1)*(iSLineStep>>1) + (3*i )];
			pYLine16[(j+1)*(iYLineStep>>1) + ((i<<1)+1)] = pSLine16[(j+1)*(iSLineStep>>1) + (3*i+1)];
			pCr16[(j>>1)*(iCLineStep>>1) + ((i)  )] = pSLine16[(j+1)*(iSLineStep>>1) + (3*i+2)];									 
		}			
	}
}
//---------------------------------------------------------------------------

void CShell_DenoiseTech(CShell *mp) //[2008/04/23, by Albert]
{
	ShellParam *pShellParam = &mp->Param;

	if(!pShellParam->bTemporalNR)
		return;

	ScaleParam *pScaleParam = CShell_GetScaleParam(mp);
	
	int iSWSubSample = pScaleParam->iSWSubSample;

	int iPixStep=1<<iSWSubSample;//Dajiang
	if (iPixStep>2)
	{
		return;
	}

	SInt32 iYBufHSize = pScaleParam->iBufHSize;
	SInt32 iYBufVSize = pScaleParam->iBufVSize;
	SInt32 iCBufHSize = iYBufHSize>>1;
	SInt32 iCBufVSize = iYBufVSize>>1;
	SInt32 iYHSize    = (pScaleParam->iHoSize)<<iSWSubSample;
	SInt32 iYVSize    = (pScaleParam->iVoSize)<<iSWSubSample;
	SInt32 iCHSize    = iYHSize>>1;
	SInt32 iCVSize    = iYVSize>>1;
	SInt32 iYHStart   = pScaleParam->iHStart;
	SInt32 iYVStart   = pScaleParam->iVStart;
	SInt32 iCHStart   = iYHStart>>1;
	SInt32 iCVStart   = iYVStart>>1;

	UInt8 *pY  = mp->pProcBuf;
	UInt8 *pCb = pY + iYBufHSize*iYBufVSize;
	UInt8 *pCr = pCb + iCBufHSize*iCBufVSize;
		
	pY  = pY  + iYVStart*iYBufHSize + iYHStart;
	pCb = pCb + iCVStart*iCBufHSize + iCHStart;
	pCr = pCr + iCVStart*iCBufHSize + iCHStart;

	UInt8 *pKeptBufY  = mp->pRefnBuf;
	UInt8 *pKeptBufCb = pKeptBufY + iYBufHSize*iYBufVSize;
	UInt8 *pKeptBufCr = pKeptBufCb + iCBufHSize*iCBufVSize;
		
	pKeptBufY  = pKeptBufY  + iYVStart*iYBufHSize + iYHStart;
	pKeptBufCb = pKeptBufCb + iCVStart*iCBufHSize + iCHStart;
	pKeptBufCr = pKeptBufCr + iCVStart*iCBufHSize + iCHStart;

	int i = 0, j = 0, x = 0, y = 0;
	int  curPxY[16] = {0},  prePxY[16] = {0};
	int curPxCb[16] = {0}, prePxCb[16] = {0};
	int curPxCr[16] = {0}, prePxCr[16] = {0};
	int NegCountY = 0, NegCountCb = 0, NegCountCr = 0;
	int jCount = 0, iCount = 0, a = 0, b = 0;
	bool bChroState = false;
	int MotionThY  = 0, Ymean  = 0, diffYstd  = 0;
	int MotionThCb = 0, Cbmean = 0, diffCbstd = 0;
	int MotionThCr = 0, Crmean = 0, diffCrstd = 0;
	int MtnDiffSumY = 0, MtnDiffSumCb = 0, MtnDiffSumCr = 0;

	int MotionRangeY  = pShellParam->iTempStrgLumThd;
	int MotionRangeCb = pShellParam->iTempStrgLumThd + pShellParam->iTempStrgChrThd;
	int MotionRangeCr = pShellParam->iTempStrgLumThd + pShellParam->iTempStrgChrThd;
	int NozPwrMarginY  = pShellParam->iTempNzPwLumThd;
	int NozPwrMarginCb = pShellParam->iTempNzPwLumThd + pShellParam->iTempNzPwChrThd;
	int NozPwrMarginCr = pShellParam->iTempNzPwLumThd + pShellParam->iTempNzPwChrThd;

	// JND Threshold
	static UInt8 JND[256] = {19, 18, 17, 17, 16, 16, 16, 15, 15, 15, 15, 14, 14, 14, 14, 14,
							 13, 13, 13, 13, 13, 12, 12, 12, 12, 12, 12, 12, 11, 11, 11, 11,
							 11, 11, 11, 11, 10, 10, 10, 10, 10, 10, 10, 10, 10,  9,  9,  9,
							  9,  9,  9,  9,  9,  9,  9,  8,  8,  8,  8,  8,  8,  8,  8,  8,
							  8,  8,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  6,  6,
							  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  5,  5,  5,  5,  5,
							  5,  5,  5,  5,  5,  5,  5,  5,  5,  4,  4,  4,  4,  4,  4,  4,
							  4,  4,  4,  4,  4,  4,  4,  4,  3,  3,  3,  3,  3,  3,  3,  3,
							  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  4,
							  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
							  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  5,
							  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,
							  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  6,
							  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
							  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  7, 
							  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7};

	// [2008/05/29, by Albert]
	UInt32 CountCurBlockY  = 0, CountProcBlockY  = 0;
	UInt32 CountCurBlockCb = 0, CountProcBlockCb = 0;
	UInt32 CountCurBlockCr = 0, CountProcBlockCr = 0;
	UInt32 LowLightBlkCount = 0;
	int LowLightThd = pShellParam->iLowLightThd;
	bool DoFilterProcCndt = false, LowLightMode = false;
	// [2008/05/30, by Albert]
	UInt8 *pMeanKeptBufY  = mp->pStoreMeanBuf;
	UInt8 *pMeanKeptBufCb = pMeanKeptBufY  + (iYHSize*iYVSize/16);
	UInt8 *pMeanKeptBufCr = pMeanKeptBufCb + (iCHSize*iCVSize/16);

	int MeanLmDiffThd = pShellParam->iMeanDiffYThd;
	int MeanCmDiffThd = pShellParam->iMeanDiffCThd;
	int preYmean = 0, preCbmean = 0, preCrmean = 0;
	// [2008/06/02, by Albert]
	// For Y part
	bool MotionStateY1[1600/4] = {0}, MotionStateY2[1600/4] = {0};
	bool *pMoStYCurPtr = MotionStateY1;
	bool *pMoStYPrePtr = MotionStateY2;
	// For Cb part
	bool MotionStateCb1[1600/2/4] = {0}, MotionStateCb2[1600/2/4] = {0};
	bool *pMoStCbCurPtr = MotionStateCb1;
	bool *pMoStCbPrePtr = MotionStateCb2;
	// For Cr part
	bool MotionStateCr1[1600/2/4] = {0}, MotionStateCr2[1600/2/4] = {0};
	bool *pMoStCrCurPtr = MotionStateCr1;
	bool *pMoStCrPrePtr = MotionStateCr2;
	
	int MotionCal = 0;

	int HardMtnDiffCnt;//Kathy add begin

	UInt8 YCurEdgeBuf[36] = {0};
	int  YCurSmoothCnt = 0;
	bool SmoothBlockMark1[1600/4] = {0}, SmoothBlockMark2[1600/4] = {0};
	bool *CurSmoothBlockMark = SmoothBlockMark1;
	bool *PreSmoothBlockMark = SmoothBlockMark2;
	
	int YBlockCurDC[1600/4] = {0}, YBlockPreDC[1600/4] = {0};
	int *YBlockCurDCValue = YBlockCurDC;
	int *YBlockPreDCValue = YBlockPreDC;//Kathy add end
	bool EdgeBlock = false;
	bool OddBlock = false;
	
	for(j = 0; j<iYVSize-3*iPixStep; j+=4*iPixStep) // block-based filtering
	{
		jCount++;
		b = (j>>1);
			
		for(i = 0; i<iYHSize-3*iPixStep; i+=4*iPixStep) 
		{
			OddBlock = false;	
	   		iCount++;
	  		a = (i>>1);
			if(pShellParam->bEdgeDtn)
			{
				if( ((i/iPixStep%8) ==0) && (j>0))
				{
					OddBlock = true;
					for (int p=0; p<4; p++)
					{	
						YCurEdgeBuf[p*9+0] = YCurEdgeBuf[p*9+8];
						#if defined(__MMX__)
						{
							const UInt64 TH = 0x2828282828282828;
							UInt8 *pYBlkStart = (pY+(p*iPixStep+j-1*iPixStep)*iYBufHSize+i); 
							UInt8 *YEdgeBuf = YCurEdgeBuf+p*9+1;   
							_asm
							{
								mov				ecx,               pYBlkStart;
								mov				edi,			   YEdgeBuf;
								mov				ebx,			   iPixStep;

								cmp				ebx,			   1;
								jna				Pix_Step_1_Edg;
								//pixstep=2--
								movq            mm0,               [ecx];       // mm0 = Y7, Y6, Y5, Y4, Y3, Y2, Y1, Y0
								psllw			mm0,			   8;			// mm0 = Y6, 0, Y4, 0, Y2, 0, Y0, 0
								psrlq			mm0,			   8;			// mm0 = 0, Y6, 0, Y4, 0, Y2, 0, Y0
								movq            mm1,               [ecx+8];     // mm1 = Y15, Y14, Y13, Y12, Y11, Y10, Y9, Y8
								psllw			mm1,			   8;			// mm1 = Y14, 0, Y12, 0, Y10, 0, Y8, 0
								psrlq			mm1,			   8;			// mm1 = 0, Y14, 0, Y12, 0, Y10, 0, Y8
								packuswb        mm0,               mm1;			// mm0 = Y14, Y12, Y10, Y8, Y6, Y4, Y2, Y0
								movq            mm1,               [ecx+12];    // mm1 = Y11, Y10, Y9, Y8, Y7, Y6, Y5, Y4
								psllw			mm1,			   8;			// mm1 = Y10, 0, Y8, 0, Y6, 0, Y4, 0
								psrlq			mm1,			   8;			// mm1 = 0, Y10, 0, Y8, 0, Y6, 0, Y4
								movq            mm2,               [ecx+20];    // mm2 = Y19, Y18, Y17, Y16, Y15, Y14, Y13, Y12
								psllw			mm2,			   8;			// mm2 = Y18, 0, Y16, 0, Y14, 0, Y12, 0
								psrlq			mm2,			   8;			// mm2 = 0, Y18, 0, Y16, 0, Y14, 0, Y12
								packuswb        mm1,               mm2;			// mm1 = Y18, Y16, Y14, Y12, Y10, Y8, Y6, Y4
								add             ecx,               iYBufHSize;
								add             ecx,               iYBufHSize;
								add             ecx,               iYBufHSize;
								add             ecx,               iYBufHSize;
								movq            mm2,               [ecx];       // mm2 = Y7, Y6, Y5, Y4, Y3, Y2, Y1, Y0
								psllw			mm2,			   8;			// mm2 = Y6, 0, Y4, 0, Y2, 0, Y0, 0
								psrlq			mm2,			   8;			// mm2 = 0, Y6, 0, Y4, 0, Y2, 0, Y0
								movq            mm3,               [ecx+8];     // mm3 = Y15, Y14, Y13, Y12, Y11, Y10, Y9, Y8
								psllw			mm3,			   8;			// mm3 = Y14, 0, Y12, 0, Y10, 0, Y8, 0
								psrlq			mm3,			   8;			// mm3 = 0, Y14, 0, Y12, 0, Y10, 0, Y8
								packuswb        mm2,               mm3;			// mm2 = Y14, Y12, Y10, Y8, Y6, Y4, Y2, Y0
								movq            mm3,               [ecx+12];    // mm1 = Y11, Y10, Y9, Y8, Y7, Y6, Y5, Y4
								psllw			mm3,			   8;			// mm1 = Y10, 0, Y8, 0, Y6, 0, Y4, 0
								psrlq			mm3,			   8;			// mm1 = 0, Y10, 0, Y8, 0, Y6, 0, Y4
								movq            mm4,               [ecx+20];    // mm2 = Y19, Y18, Y17, Y16, Y15, Y14, Y13, Y12
								psllw			mm4,			   8;			// mm2 = Y18, 0, Y16, 0, Y14, 0, Y12, 0
								psrlq			mm4,			   8;			// mm2 = 0, Y18, 0, Y16, 0, Y14, 0, Y12
								packuswb        mm3,               mm4;			// mm1 = Y18, Y16, Y14, Y12, Y10, Y8, Y6, Y4
								jmp				End_Pix_step_Edg;

							Pix_Step_1_Edg:
								movq            mm0,               [ecx]; //Y7 Y6 Y5 Y4 Y3 Y2 Y1 Y0
								movq			mm1,			   [ecx+2];//Y9 Y8 Y7 Y6 Y5 Y4 Y3 Y2
								add             ecx,               iYBufHSize;
								add             ecx,               iYBufHSize;
								movq			mm2,			   [ecx];//Y23 Y22 Y21 Y20 Y19 Y18 Y17 Y16
								movq            mm3,               [ecx+2];//Y25 Y24 Y23 Y22 Y21 Y20 Y19 Y18

							End_Pix_step_Edg:
								movq            mm6,               mm0;//mm6=mm0
								movq			mm4,               mm1;
								psubusb         mm6,               mm1;//mm6=mm6-mm1
								psubusb			mm1,			   mm0;
								por				mm6,			   mm1;//mm6=|mm0-mm1|
								
								movq			mm1,               mm0;
								movq			mm5,			   mm2;
								psubusb         mm0,			   mm2;//mm0=mm0-mm2
								psubusb			mm2,			   mm1;
								por				mm0,               mm2;//mm0=|mm0-mm2|;
								
								movq			mm1,			   mm3;
								movq            mm2,               mm5;
								psubusb         mm5,               mm3;//mm2=mm2-mm3
								psubusb         mm1,               mm2;
								por				mm1,               mm5;///mm1=|mm2-mm3|;
								
								movq            mm2,               mm3;
								psubusb         mm3,               mm4;
								psubusb         mm4,               mm2;
								por				mm3,               mm4;//mm3=|mm1-mm3|;
								
								paddusb			mm1,               mm6;
								paddusb         mm0,               mm3;
								
								pcmpgtb         mm1,               TH;
								pcmpgtb         mm0,               TH;
								
								por             mm1,               mm0;
								
								movq			[edi],             mm1;
								emms;
							}					
						}
						#else
						{
							for(int q=0; q<8; q++)
							{
								int temp1 = abs(pY[iYBufHSize*(p*iPixStep+j-1*iPixStep)+q*iPixStep] - pY[iYBufHSize*(p*iPixStep+j-1*iPixStep)+q*iPixStep+2*iPixStep])+
									abs(pY[iYBufHSize*(p*iPixStep+j+1*iPixStep)+q*iPixStep] - pY[iYBufHSize*(p*iPixStep+j+1*iPixStep)+q*iPixStep+2*iPixStep]);
								int temp2 = abs(pY[iYBufHSize*(p*iPixStep+j-1*iPixStep)+q*iPixStep] - pY[iYBufHSize*(p*iPixStep+j+1*iPixStep)+q*iPixStep])+
									abs(pY[iYBufHSize*(p*iPixStep+j-1*iPixStep)+q*iPixStep+2*iPixStep] - pY[iYBufHSize*(p*iPixStep+j+1*iPixStep)+q*iPixStep+2*iPixStep]);
								
								if(temp1>40 || temp2>40 )		
								{
									YCurEdgeBuf[9*p+q+1] = 1;
								}
							}
		
						}
						#endif					
					}
				}
			}
			
			CountCurBlockY++;

			HardMtnDiffCnt = 0;//Kathy add

			const UInt64 Zeros64 = 0x0000000000000000;
			const UInt32 Zeros32 = 0x00000000;
			const UInt64 ConstVal = 0x0002000200020002;

			// For Luma Part - Start
			#if defined(__MMX__)
			{
				UInt8 *pYBlkStart = (pY+j*iYBufHSize+i); // notice!!!
				__asm
				{
					mov             eax,               0x00000004;
					movq            mm0,               Zeros64;
					movq            mm1,               Zeros64;
					movq            mm2,               Zeros64;
					mov				ecx,               pYBlkStart;
					mov				ebx,			   iPixStep;

				Line_Add_Loop_Y:
					//------------------------------------------------------------------					
					cmp				ebx,			   1;
					jna				Pix_Step_1_Y_Add;
					//pixstep=2--		
					movq            mm1,               [ecx];       // mm1 = Y7, Y6, Y5, Y4, Y3, Y2, Y1, Y0
					psllw			mm1,			   8;			// mm1 = Y6, 0, Y4, 0, Y2, 0, Y0, 0
					psrlq			mm1,			   8;			// mm1 = 0, Y6, 0, Y4, 0, Y2, 0, Y0
					paddusw         mm2,               mm1;         // mm2 = (Y6+Y14+Y22+Y30), (Y4+Y12+Y20+Y28), (Y2+Y10+Y18+Y26), (Y0+Y8+Y16+Y24)
 					add             ecx,               iYBufHSize;
					jmp				End_Pix_step_Y_Add;

				Pix_Step_1_Y_Add:
					movd            mm1,               [ecx];       // mm1 = 0, 0, 0, 0, Y3, Y2, Y1, Y0
					punpcklbw       mm1,               mm0;         // mm1 = 0 Y3, 0 Y2, 0 Y1, 0 Y0
					paddusw         mm2,               mm1;         // mm2 = (Y3+Y7+Y11+Y15), (Y2+Y6+Y10+Y14), (Y1+Y5+Y9+Y13), (Y0+Y4+Y8+Y12)				

				End_Pix_step_Y_Add:
 					add             ecx,               iYBufHSize;
 					dec             eax;
					jnz             Line_Add_Loop_Y;
					//-------------------------------------------------------------------------------
					movd            ebx,               mm2;         // ebx = (Y1+Y5+Y9+Y13), (Y0+Y4+Y8+Y12)
					psrlq           mm2,               32;          
					movd            edx,               mm2;         // edx = (Y3+Y7+Y11+Y15), (Y2+Y6+Y10+Y14)
					add             dx,                bx;          // dx = (Y2+Y6+Y10+Y14) + (Y0+Y4+Y8+Y12)
					mov             ax,                dx;          // ax = dx
					shr             ebx,               16;
					shr             edx,               16;
					add             ax,                bx;          // ax = (Y2+Y6+Y10+Y14) + (Y0+Y4+Y8+Y12) + (Y1+Y5+Y9+Y13)
					add             ax,                dx;          // ax = (Y2+Y6+Y10+Y14) + (Y0+Y4+Y8+Y12) + (Y1+Y5+Y9+Y13)+ (Y3+Y7+Y11+Y15)
					add             ax,                8;
					shr             ax,                4;
					mov             Ymean,             eax;

					emms;
				}
			}
			#else
			{
			    // Current Y Frame
				curPxY[0]  = pY[(j+0*iPixStep)*iYBufHSize+(i+0*iPixStep)]; // C00
				curPxY[1]  = pY[(j+0*iPixStep)*iYBufHSize+(i+1*iPixStep)]; // C01
				curPxY[2]  = pY[(j+0*iPixStep)*iYBufHSize+(i+2*iPixStep)]; // C02
				curPxY[3]  = pY[(j+0*iPixStep)*iYBufHSize+(i+3*iPixStep)]; // C03
  				curPxY[4]  = pY[(j+1*iPixStep)*iYBufHSize+(i+0*iPixStep)]; // C10
				curPxY[5]  = pY[(j+1*iPixStep)*iYBufHSize+(i+1*iPixStep)]; // C11
				curPxY[6]  = pY[(j+1*iPixStep)*iYBufHSize+(i+2*iPixStep)]; // C12
				curPxY[7]  = pY[(j+1*iPixStep)*iYBufHSize+(i+3*iPixStep)]; // C13
				curPxY[8]  = pY[(j+2*iPixStep)*iYBufHSize+(i+0*iPixStep)]; // C20
				curPxY[9]  = pY[(j+2*iPixStep)*iYBufHSize+(i+1*iPixStep)]; // C21
				curPxY[10] = pY[(j+2*iPixStep)*iYBufHSize+(i+2*iPixStep)]; // C22
				curPxY[11] = pY[(j+2*iPixStep)*iYBufHSize+(i+3*iPixStep)]; // C23
				curPxY[12] = pY[(j+3*iPixStep)*iYBufHSize+(i+0*iPixStep)]; // C30
				curPxY[13] = pY[(j+3*iPixStep)*iYBufHSize+(i+1*iPixStep)]; // C31
				curPxY[14] = pY[(j+3*iPixStep)*iYBufHSize+(i+2*iPixStep)]; // C32
				curPxY[15] = pY[(j+3*iPixStep)*iYBufHSize+(i+3*iPixStep)]; // C33
				
			    Ymean = ( curPxY[0]+curPxY[1] +curPxY[2] +curPxY[3] +curPxY[4] +curPxY[5] +curPxY[6] +curPxY[7]+
		    		      curPxY[8]+curPxY[9]+curPxY[10]+curPxY[11]+curPxY[12]+curPxY[13]+curPxY[14]+curPxY[15]+8 )>>4;
			}
			#endif
		
			LowLightBlkCount += (Ymean<LowLightThd)? 1: 0;
			LowLightMode = ( LowLightBlkCount>=(CountCurBlockY>>2) );

			MotionThY = JND[Ymean]*MotionRangeY;
		
			MtnDiffSumY = 0; NegCountY = 0;

			preYmean = pMeanKeptBufY[CountCurBlockY-1];
			pMeanKeptBufY[CountCurBlockY-1] = (UInt8) Ymean;
			DoFilterProcCndt = false;

			if ( abs(Ymean-preYmean)<MeanLmDiffThd )
			{
				EdgeBlock = false;
				#if defined(__MMX__)
				{
					UInt8 *pYBlkStart = (pY+j*iYBufHSize+i); // notice!!!
					UInt8 *pKeptYBlkStart = (pKeptBufY+j*iYBufHSize+i); // notice!!!
					UInt8 *pEdgeStart = OddBlock?YCurEdgeBuf:YCurEdgeBuf+4;
					const UInt64 EdgeCnt = 0x0003000300030003;
					const UInt64 temp = 0xffffffffffffffff;

					__asm
					{
						mov             eax,               0x00000004;
						movq            mm0,               Zeros64;
						movq            mm1,               Zeros64;
						movq            mm2,               Zeros64;
						movq            mm3,               Zeros64;
						movq            mm4,               Zeros64;
						movq            mm5,               Zeros64;
						movq            mm6,               Zeros64;
						movq            mm7,               Zeros64;
						mov		        esi,               pYBlkStart;
						mov				edi,               pKeptYBlkStart;
						mov             ecx,               pEdgeStart;
						mov				ebx,			   iPixStep;

					Line_Abs_Sub_Loop_Y:
						//------------------------------------------------------------------
						cmp				ebx,			   1;
						jna				Pix_Step_1_Y_Abs_Sub;
						//pixstep=2--
						movq            mm1,               [esi];       // mm1 = cY7, cY6, cY5, cY4, cY3, cY2, cY1, cY0
						psllw			mm1,			   8;			// mm1 = cY6, 0, cY4, 0, cY2, 0, cY0, 0
						psrlq			mm1,			   8;			// mm1 = 0, cY6, 0, cY4, 0, cY2, 0, cY0
						movq            mm2,               [edi];       // mm2 = pY7, pY6, pY5, pY4, pY3, pY2, pY1, pY0
						psllw			mm2,			   8;			// mm2 = pY6, 0, pY4, 0, pY2, 0, pY0, 0
						psrlq			mm2,			   8;			// mm2 = 0, pY6, 0, pY4, 0, pY2, 0, pY0
						add             esi,               iYBufHSize;
						add             edi,               iYBufHSize;
						jmp				End_Pix_step_Y_Abs_Sub;

					Pix_Step_1_Y_Abs_Sub:
						movd            mm1,               [esi];       // mm1 = 0, 0, 0, 0, cY3, cY2, cY1, cY0
						punpcklbw       mm1,               mm0;         // mm1 = 0 cY3, 0 cY2, 0 cY1, 0 cY0
						movd            mm2,               [edi];       // mm2 = 0, 0, 0, 0, pY3, pY2, pY1, pY0
						punpcklbw       mm2,               mm0;         // mm2 = 0 pY3, 0 pY2, 0 pY1, 0 pY0

					End_Pix_step_Y_Abs_Sub:						
						// Sum of Motion Difference
						movq            mm3,               mm1;         // mm3 = mm1
						movq            mm4,               mm2;         // mm4 = mm2
						psubusw         mm3,               mm2;         // mm3 = mm1 - mm2
						psubusw         mm4,               mm1;         // mm4 = mm2 - mm1
						paddusw         mm3,               mm4;         // mm3 = |mm1 - mm2|
						paddusw         mm5,               mm3;         // mm5 = mm3 + mmx;
						
						movd            mm4,               [ecx];      
						punpcklbw       mm4,               mm0; 
						
						pcmpgtw         mm3,			   EdgeCnt;
						pxor            mm3,               temp;
						psrlw           mm3,               15;
						paddusw         mm6,               mm3;

						pcmpgtw         mm4,			   Zeros64;
						psrlw           mm4,               15;
						paddusw         mm7,               mm4;

						add             esi,               iYBufHSize;
						add             edi,               iYBufHSize;
						add             ecx,               9;
						
						dec             eax;
						jnz             Line_Abs_Sub_Loop_Y;
						//-------------------------------------------------------------------------------
						// Sum of Motion Difference
						mov             eax,               Zeros32;
						mov             ebx,               Zeros32;
						movd            ecx,               mm5;         // ecx = (D1+D5+D9+D13), (D0+D4+D8+D12)
						psrlq           mm5,               32;          
						movd            edx,               mm5;         // edx = (D3+D7+D11+D15), (D2+D6+D10+D14)
						add             dx,                cx;          // dx = (D2+D6+D10+D14) + (D0+D4+D8+D12)
						mov             ax,                dx;          // ax = dx
						shr             ecx,               16;
						shr             edx,               16;
						add             dx,                cx;          // dx = (Y2+Y6+Y10+Y14) + (Y0+Y4+Y8+Y12) + (Y1+Y5+Y9+Y13)
						add             ax,                dx;          // ax = (Y2+Y6+Y10+Y14) + (Y0+Y4+Y8+Y12) + (Y1+Y5+Y9+Y13)+ (Y3+Y7+Y11+Y15)
						mov             MtnDiffSumY,       eax;         // MtnDiffSumY
						// Standard deviation of absolution difference
						add             ax,                8;
						shr             ax,                4;
						mov             bx,                ax;
						add             bx,                2;
						shr             bx,                2;
						add             ax,                bx;
						mov             diffYstd,          eax;         // DiffYstd
						// Calculation of HardMtnDiffCnt
						mov             eax,               Zeros32;
						mov             ebx,               Zeros32;
						movd            ecx,               mm6;
						psrlq           mm6,               32;
						movd            edx,               mm6;
						add             dx,                cx;
						mov             ax,                dx;
						shr             ecx,               16;
						shr             edx,               16;
						add             dx,                cx;
						add             ax,                dx;
						mov             HardMtnDiffCnt,         eax;         // HardMtnDiffCnt
						
						mov             eax,               Zeros32;
						mov             ebx,               Zeros32;
						movd            ecx,               mm7;
						psrlq           mm7,               32;
						movd            edx,               mm7;
						add             dx,                cx;
						mov             ax,                dx;
						shr             ecx,               16;
						shr             edx,               16;
						add             dx,                cx;
						add             ax,                dx;
						mov             YCurSmoothCnt,     eax;   

						emms;
					}
					CurSmoothBlockMark[i>>2] = (YCurSmoothCnt<2)?1:0;
					EdgeBlock = (YCurSmoothCnt>7)?1:0;
				}
				#else
				{
					// Previous Y Frame
					prePxY[0]  = pKeptBufY[(j+0*iPixStep)*iYBufHSize+(i+0*iPixStep)]; // P00
					prePxY[1]  = pKeptBufY[(j+0*iPixStep)*iYBufHSize+(i+1*iPixStep)]; // P01
					prePxY[2]  = pKeptBufY[(j+0*iPixStep)*iYBufHSize+(i+2*iPixStep)]; // P02
					prePxY[3]  = pKeptBufY[(j+0*iPixStep)*iYBufHSize+(i+3*iPixStep)]; // P03
  					prePxY[4]  = pKeptBufY[(j+1*iPixStep)*iYBufHSize+(i+0*iPixStep)]; // P10
					prePxY[5]  = pKeptBufY[(j+1*iPixStep)*iYBufHSize+(i+1*iPixStep)]; // P11
					prePxY[6]  = pKeptBufY[(j+1*iPixStep)*iYBufHSize+(i+2*iPixStep)]; // P12
					prePxY[7]  = pKeptBufY[(j+1*iPixStep)*iYBufHSize+(i+3*iPixStep)]; // P13
					prePxY[8]  = pKeptBufY[(j+2*iPixStep)*iYBufHSize+(i+0*iPixStep)]; // P20
					prePxY[9]  = pKeptBufY[(j+2*iPixStep)*iYBufHSize+(i+1*iPixStep)]; // P21
					prePxY[10] = pKeptBufY[(j+2*iPixStep)*iYBufHSize+(i+2*iPixStep)]; // P22
					prePxY[11] = pKeptBufY[(j+2*iPixStep)*iYBufHSize+(i+3*iPixStep)]; // P23
					prePxY[12] = pKeptBufY[(j+3*iPixStep)*iYBufHSize+(i+0*iPixStep)]; // P30
					prePxY[13] = pKeptBufY[(j+3*iPixStep)*iYBufHSize+(i+1*iPixStep)]; // P31
					prePxY[14] = pKeptBufY[(j+3*iPixStep)*iYBufHSize+(i+2*iPixStep)]; // P32
					prePxY[15] = pKeptBufY[(j+3*iPixStep)*iYBufHSize+(i+3*iPixStep)]; // P33

					YCurSmoothCnt = 0;
					HardMtnDiffCnt = 0;

					for (y = 0; y < 4; y++) 
					{
						for (x = 0; x < 4; x++) 
						{
							int temp = curPxY[y*4+x]-prePxY[y*4+x];
							MtnDiffSumY += abs(temp);															
							HardMtnDiffCnt += (abs(temp)<4)? 1:0;
							if(pShellParam->bEdgeDtn)
							{
								if(OddBlock)
								{
									YCurSmoothCnt += (YCurEdgeBuf[y*9+x]?1:0);
								}
								else
								{
									YCurSmoothCnt += (YCurEdgeBuf[y*9+x+4]?1:0);
								}														
							}
																			
						}
					}

					CurSmoothBlockMark[i>>2] = (YCurSmoothCnt<2)?1:0;
					EdgeBlock = (YCurSmoothCnt>7)?1:0;
					diffYstd = ((MtnDiffSumY+8)>>4);
					diffYstd += ((diffYstd+2)>>2);
				}
				#endif
				
				bool SmoothBlock = false;
				bool smNoFilter = false;
				if(pShellParam->bEdgeDtn)
				{
					if ( (j>0) & (j<iYVSize-4*iPixStep) & (i>0) & (i<iYHSize-4*iPixStep) )
					{			
						SmoothBlock = CurSmoothBlockMark[i>>2] & CurSmoothBlockMark[(i>>2)-1*iPixStep] &
							PreSmoothBlockMark[i>>2] & PreSmoothBlockMark[(i>>2)-1*iPixStep] & PreSmoothBlockMark[(i>>2)+1*iPixStep];
					}
				}			
				
				
				DoFilterProcCndt = ( (MtnDiffSumY<MotionThY)  & (diffYstd<(JND[Ymean]+NozPwrMarginY)) );
				DoFilterProcCndt = (LowLightMode)? ( DoFilterProcCndt & (CountProcBlockY>=(CountCurBlockY>>1)) ): DoFilterProcCndt;
				
				MotionCal = 0;
				// Neighborhood motion check
				if ( (j>0) & (j<iYVSize-3*iPixStep) & (i>0) & (i<iYHSize-4*iPixStep) )
				{
					MotionCal = 0;
					if ( DoFilterProcCndt )
					{
						MotionCal += (pMoStYPrePtr[(i>>2)-1*iPixStep])? 1: 0;
						MotionCal += (pMoStYPrePtr[(i>>2)+0*iPixStep])? 1: 0;
						MotionCal += (pMoStYPrePtr[(i>>2)+1*iPixStep])? 1: 0;
						MotionCal += (pMoStYCurPtr[(i>>2)-1*iPixStep])? 1: 0;
						
						DoFilterProcCndt = DoFilterProcCndt & (MotionCal<3);
						if(pShellParam->bEdgeDtn)
						{
							if(DoFilterProcCndt && SmoothBlock)
							{
								if( (Ymean-preYmean)<JND[Ymean] && HardMtnDiffCnt>=8)
									;
								else
									smNoFilter = true;	
							}
							if (EdgeBlock)	
							{
								DoFilterProcCndt = DoFilterProcCndt & (MtnDiffSumY< JND[Ymean]*20);
							}
						}
					}
				}			

				// Temporal Average Filtering - Y
				if(pShellParam->bEdgeDtn)
				{
					int YDc2 = Ymean;
					int YDc1 = Ymean;
					YBlockCurDCValue[(i>>2)] = Ymean;
					if ( DoFilterProcCndt ) 
					{ 
						YDc2 = (Ymean+3*preYmean+2)>>2;
						CountProcBlockY++;
						if (smNoFilter)
						{	
							int offset=0;
								
							int k=1;
							if (abs(YBlockCurDCValue[(i>>2)-1*iPixStep] - YDc1)<4)
							{
								k++;
								offset += YBlockCurDCValue[(i>>2)-1*iPixStep];
							}
							if (abs(YBlockPreDCValue[(i>>2)-1*iPixStep] - YDc1)<4)
							{
								k++;
								offset += YBlockPreDCValue[(i>>2)-1*iPixStep];
							}
							if (abs(YBlockPreDCValue[(i>>2)] - YDc1)<4)
							{
								k++;
								offset += YBlockPreDCValue[(i>>2)];
							}		
							
							offset = (short)(offset+YDc1-YDc2*k)/k;
					
							YBlockCurDCValue[(i>>2)] = min(255,max(0,YDc2+offset));
						

							#if defined(__MMX__)
							{
								UInt8 *pYBlkStart = (pY+j*iYBufHSize+i); // notice!!!
								UInt8 *pKeptYBlkStart = (pKeptBufY+j*iYBufHSize+i); // notice!!!	

								short  offsetbuf[4]={0};
								offsetbuf[0]=offset;
								offsetbuf[1]=offset;
								offsetbuf[2]=offset;
								offsetbuf[3]=offset;
								
								__asm
								{
									mov             eax,               0x00000004;
									movq            mm0,               Zeros64;
									movq            mm1,               Zeros64;
									movq            mm2,               Zeros64;
									movq            mm3,               ConstVal;
									mov		        esi,               pYBlkStart;
									mov				edi,               pKeptYBlkStart;
									mov				ebx,			   iPixStep;
									
								Average_Filter_Loop_Y:
									//------------------------------------------------------------------
									cmp				ebx,			   1;
									jna				Pix_Step_1_Y_Average_Filter;
									//pixstep=2--
									movq            mm1,               [esi];       // mm1 = cY7, cY6, cY5, cY4, cY3, cY2, cY1, cY0
									psllw			mm1,			   8;			// mm1 = cY6, 0, cY4, 0, cY2, 0, cY0, 0
									psrlq			mm1,			   8;			// mm1 = 0, cY6, 0, cY4, 0, cY2, 0, cY0
									movq            mm2,               [edi];       // mm2 = pY7, pY6, pY5, pY4, pY3, pY2, pY1, pY0
									psllw			mm2,			   8;			// mm2 = pY6, 0, pY4, 0, pY2, 0, pY0, 0
									psrlq			mm2,			   8;			// mm2 = 0, pY6, 0, pY4, 0, pY2, 0, pY0
									
									paddusw         mm1,               mm2;
									paddusw         mm1,               mm2;
									paddusw         mm1,               mm2;
									paddusw         mm1,               mm3;
									psrlw           mm1,               2;

									paddsw			mm1,			   offsetbuf;
									packuswb        mm1,               mm0;
									punpcklbw       mm1,               mm0;

									movq            [esi],             mm1;
									add             esi,               iYBufHSize;
									add             edi,               iYBufHSize;
									jmp				End_Pix_step_Y_Average_Filter;

								Pix_Step_1_Y_Average_Filter:
									movd            mm1,               [esi];       // mm1 = 0, 0, 0, 0, cY3, cY2, cY1, cY0
									punpcklbw       mm1,               mm0;         // mm1 = 0 cY3, 0 cY2, 0 cY1, 0 cY0
									movd            mm2,               [edi];       // mm2 = 0, 0, 0, 0, pY3, pY2, pY1, pY0
									punpcklbw       mm2,               mm0;         // mm2 = 0 pY3, 0 pY2, 0 pY1, 0 pY0
									
									paddusw         mm1,               mm2;
									paddusw         mm1,               mm2;
									paddusw         mm1,               mm2;
									paddusw         mm1,               mm3;
									psrlw           mm1,               2;

									paddsw			mm1,			   offsetbuf;
									packuswb        mm1,               mm0;									
									movd            [esi],             mm1;

								End_Pix_step_Y_Average_Filter:									
									add             esi,               iYBufHSize;
									add             edi,               iYBufHSize;									
										
									dec             eax;
									jnz             Average_Filter_Loop_Y;
									//------------------------------------------------------------------
									emms;
								}
							}
							#else					
							{	
								for (y = 0; y < 4; y++) 
								{
									for (x = 0; x < 4; x++) 
									{									
										pY[(j+y*iPixStep)*iYBufHSize+(i+x*iPixStep)] = (UInt8) min(255,(max(0,( (pY[(j+y*iPixStep)*iYBufHSize+(i+x*iPixStep)]+pKeptBufY[(j+y*iPixStep)*iYBufHSize+(i+x*iPixStep)]*3+2)>>2 )+offset)));								
									}
								}		
							}
							#endif							
						}		
						else
						{
							#if defined(__MMX__)
							{
								UInt8 *pYBlkStart = (pY+j*iYBufHSize+i); // notice!!!
								UInt8 *pKeptYBlkStart = (pKeptBufY+j*iYBufHSize+i); // notice!!!
								__asm
								{
									mov             eax,               0x00000004;
									movq            mm0,               Zeros64;
									movq            mm1,               Zeros64;
									movq            mm2,               Zeros64;
									movq            mm3,               ConstVal;
									mov		        esi,               pYBlkStart;
									mov				edi,               pKeptYBlkStart;
									mov				ebx,			   iPixStep;

								Average_Filter_Loop_Y2:
									//------------------------------------------------------------------
									cmp				ebx,			   1;
									jna				Pix_Step_1_Y2_Average_Filter;
									//pixstep=2--
									movq            mm1,               [esi];       // mm1 = cY7, cY6, cY5, cY4, cY3, cY2, cY1, cY0
									psllw			mm1,			   8;			// mm1 = cY6, 0, cY4, 0, cY2, 0, cY0, 0
									psrlq			mm1,			   8;			// mm1 = 0, cY6, 0, cY4, 0, cY2, 0, cY0
									movq            mm2,               [edi];       // mm2 = pY7, pY6, pY5, pY4, pY3, pY2, pY1, pY0
									psllw			mm2,			   8;			// mm2 = pY6, 0, pY4, 0, pY2, 0, pY0, 0
									psrlq			mm2,			   8;			// mm2 = 0, pY6, 0, pY4, 0, pY2, 0, pY0
									
									paddusw         mm1,               mm2;
									paddusw         mm1,               mm2;
									paddusw         mm1,               mm2;
									paddusw         mm1,               mm3;
									psrlw           mm1,               2;

									movq            [esi],             mm1;
									add             esi,               iYBufHSize;
									add             edi,               iYBufHSize;
									jmp				End_Pix_step_Y2_Average_Filter;

								Pix_Step_1_Y2_Average_Filter:
									movd            mm1,               [esi];       // mm1 = 0, 0, 0, 0, cY3, cY2, cY1, cY0
									punpcklbw       mm1,               mm0;         // mm1 = 0 cY3, 0 cY2, 0 cY1, 0 cY0
									movd            mm2,               [edi];       // mm2 = 0, 0, 0, 0, pY3, pY2, pY1, pY0
									punpcklbw       mm2,               mm0;         // mm2 = 0 pY3, 0 pY2, 0 pY1, 0 pY0

									paddusw         mm1,               mm2;
									paddusw         mm1,               mm2;
									paddusw         mm1,               mm2;
									paddusw         mm1,               mm3;
									psrlw           mm1,               2;
									
									packuswb        mm1,               mm0;
									movd            [esi],             mm1;

								End_Pix_step_Y2_Average_Filter:
									add             esi,               iYBufHSize;
									add             edi,               iYBufHSize;										
										
									dec             eax;
									jnz             Average_Filter_Loop_Y2;
									//------------------------------------------------------------------
									emms;
								}
							}
							#else
							{
								for (y = 0; y < 4; y++) 
								{
									for (x = 0; x < 4; x++) 
									{
											pY[(j+y*iPixStep)*iYBufHSize+(i+x*iPixStep)] = (UInt8) ( (pY[(j+y*iPixStep)*iYBufHSize+(i+x*iPixStep)]+pKeptBufY[(j+y*iPixStep)*iYBufHSize+(i+x*iPixStep)]*3+2)>>2 );
									}
								}
							}
							#endif
							YBlockCurDCValue[(i>>2)] = YDc2;
						}									
					}
				}
				else
				{
					if ( DoFilterProcCndt ) 
					{ 
						CountProcBlockY++;
						
						#if defined(__MMX__)
						{
							UInt8 *pYBlkStart = (pY+j*iYBufHSize+i); // notice!!!
							UInt8 *pKeptYBlkStart = (pKeptBufY+j*iYBufHSize+i); // notice!!!
							__asm
							{
								mov             eax,               0x00000004;
								movq            mm0,               Zeros64;
								movq            mm1,               Zeros64;
								movq            mm2,               Zeros64;
								movq            mm3,               ConstVal;
								mov		        esi,               pYBlkStart;
								mov				edi,               pKeptYBlkStart;
								mov				ebx,			   iPixStep;

							Average_Filter_Loop_Y3:
								//------------------------------------------------------------------
								cmp				ebx,			   1;
								jna				Pix_Step_1_Y3_Average_Filter;
								//pixstep=2--
								movq            mm1,               [esi];       // mm1 = cY7, cY6, cY5, cY4, cY3, cY2, cY1, cY0
								psllw			mm1,			   8;			// mm1 = cY6, 0, cY4, 0, cY2, 0, cY0, 0
								psrlq			mm1,			   8;			// mm1 = 0, cY6, 0, cY4, 0, cY2, 0, cY0
								movq            mm2,               [edi];       // mm2 = pY7, pY6, pY5, pY4, pY3, pY2, pY1, pY0
								psllw			mm2,			   8;			// mm2 = pY6, 0, pY4, 0, pY2, 0, pY0, 0
								psrlq			mm2,			   8;			// mm2 = 0, pY6, 0, pY4, 0, pY2, 0, pY0
								
								paddusw         mm1,               mm2;
								paddusw         mm1,               mm2;
								paddusw         mm1,               mm2;
								paddusw         mm1,               mm3;
								psrlw           mm1,               2;

								movq            [esi],             mm1;
								add             esi,               iYBufHSize;
								add             edi,               iYBufHSize;
								jmp				End_Pix_step_Y3_Average_Filter;

							Pix_Step_1_Y3_Average_Filter:
								movd            mm1,               [esi];       // mm1 = 0, 0, 0, 0, cY3, cY2, cY1, cY0
								punpcklbw       mm1,               mm0;         // mm1 = 0 cY3, 0 cY2, 0 cY1, 0 cY0
								movd            mm2,               [edi];       // mm2 = 0, 0, 0, 0, pY3, pY2, pY1, pY0
								punpcklbw       mm2,               mm0;         // mm2 = 0 pY3, 0 pY2, 0 pY1, 0 pY0

								paddusw         mm1,               mm2;
								paddusw         mm1,               mm2;
								paddusw         mm1,               mm2;
								paddusw         mm1,               mm3;
								psrlw           mm1,               2;
								packuswb        mm1,               mm0;
								movd            [esi],             mm1;

							End_Pix_step_Y3_Average_Filter:
								add             esi,               iYBufHSize;
								add             edi,               iYBufHSize;
								dec             eax;
								jnz             Average_Filter_Loop_Y3;
								//------------------------------------------------------------------
								emms;
							}
						}
						#else
						{
							for (y = 0; y < 4; y++) 
							{
								for (x = 0; x < 4; x++) 
								{
									pY[(j+y*iPixStep)*iYBufHSize+(i+x*iPixStep)] = (UInt8) ( (pY[(j+y*iPixStep)*iYBufHSize+(i+x*iPixStep)]+pKeptBufY[(j+y*iPixStep)*iYBufHSize+(i+x*iPixStep)]*3+2)>>2 );
								}
							}
						}
						#endif
					}
				}
			} // For mean check of motion block

			pMoStYCurPtr[i>>2] = (!DoFilterProcCndt);
			// For Luma Part - End

			// For Chroma Part - Start
			bChroState = ( (jCount & 1) & (iCount & 1) & (b < (iCVSize-3*iPixStep)) & (a < (iCHSize-3*iPixStep)) );
				
			if ( bChroState ) 
			{	
				// For Cb Part - Start
				CountCurBlockCb++;

				#if defined(__MMX__)
				{
					UInt8 *pCbBlkStart = (pCb+b*iCBufHSize+a); // notice!!!
					__asm
					{
						mov             eax,               0x00000004;
						movq            mm0,               Zeros64;
						movq            mm1,               Zeros64;
						movq            mm2,               Zeros64;
						mov				ecx,               pCbBlkStart;
						mov				ebx,			   iPixStep;

					Line_Add_Loop_Cb:
						//------------------------------------------------------------------
						cmp				ebx,			   1;
						jna				Pix_Step_1_Cb_Add;
						//pixstep=2--
						movq            mm1,               [ecx];       // mm1 = Cb7, Cb6, Cb5, Cb4, Cb3, Cb2, Cb1, Cb0
						psllw			mm1,			   8;			// mm1 = Cb6, 0, Cb4, 0, Cb2, 0, Cb0, 0
						psrlq			mm1,			   8;			// mm1 = 0, Cb6, 0, Cb4, 0, Cb2, 0, Cb0
						add             ecx,               iCBufHSize;
						jmp				End_Pix_step_Cb_Add;

					Pix_Step_1_Cb_Add:
						movd            mm1,               [ecx];       // mm1 = 0, 0, 0, 0, Cb3, Cb2, Cb1, Cb0
						punpcklbw       mm1,               mm0;         // mm1 = 0 Cb3, 0 Cb2, 0 Cb1, 0 Cb0

					End_Pix_step_Cb_Add:
						paddusw         mm2,               mm1;         // mm2 = (Cb3+Cb7+Cb11+Cb15), (Cb2+Cb6+Cb10+Cb14), (Cb1+Cb5+Cb9+Cb13), (Cb0+Cb4+Cb8+Cb12)
												
						add             ecx,               iCBufHSize;
						dec             eax;
						jnz             Line_Add_Loop_Cb;
						//-------------------------------------------------------------------------------
						movd            ebx,               mm2;         // ebx = (Cb1+Cb5+Cb9+Cb13), (Cb0+Cb4+Cb8+Cb12)
						psrlq           mm2,               32;          
						movd            edx,               mm2;         // edx = (Cb3+Cb7+Cb11+Cb15), (Cb2+Cb6+Cb10+Cb14)
						add             dx,                bx;          // dx = (Cb2+Cb6+Cb10+Cb14) + (Cb0+Cb4+Cb8+Cb12)
						mov             ax,                dx;          // ax = dx
						shr             ebx,               16;
						shr             edx,               16;
						add             ax,                bx;          // ax = (Cb2+Cb6+Cb10+Cb14) + (Cb0+Cb4+Cb8+Cb12) + (Cb1+Cb5+Cb9+Cb13)
						add             ax,                dx;          // ax = (Cb2+Cb6+Cb10+Cb14) + (Cb0+Cb4+Cb8+Cb12) + (Cb1+Cb5+Cb9+Cb13)+ (Cb3+Cb7+Cb11+Cb15)
						add             ax,                8;
						shr             ax,                4;
						mov             Cbmean,            eax;

						emms;
					}
				}
				#else
				{
					// Current Cb Frame
					curPxCb[0]  = pCb[(b+0*iPixStep)*iCBufHSize+(a+0*iPixStep)]; // C00
					curPxCb[1]  = pCb[(b+0*iPixStep)*iCBufHSize+(a+1*iPixStep)]; // C01
					curPxCb[2]  = pCb[(b+0*iPixStep)*iCBufHSize+(a+2*iPixStep)]; // C02
					curPxCb[3]  = pCb[(b+0*iPixStep)*iCBufHSize+(a+3*iPixStep)]; // C03
					curPxCb[4]  = pCb[(b+1*iPixStep)*iCBufHSize+(a+0*iPixStep)]; // C10
					curPxCb[5]  = pCb[(b+1*iPixStep)*iCBufHSize+(a+1*iPixStep)]; // C11
					curPxCb[6]  = pCb[(b+1*iPixStep)*iCBufHSize+(a+2*iPixStep)]; // C12
					curPxCb[7]  = pCb[(b+1*iPixStep)*iCBufHSize+(a+3*iPixStep)]; // C13
					curPxCb[8]  = pCb[(b+2*iPixStep)*iCBufHSize+(a+0*iPixStep)]; // C20
					curPxCb[9]  = pCb[(b+2*iPixStep)*iCBufHSize+(a+1*iPixStep)]; // C21
					curPxCb[10] = pCb[(b+2*iPixStep)*iCBufHSize+(a+2*iPixStep)]; // C22
					curPxCb[11] = pCb[(b+2*iPixStep)*iCBufHSize+(a+3*iPixStep)]; // C23
					curPxCb[12] = pCb[(b+3*iPixStep)*iCBufHSize+(a+0*iPixStep)]; // C30
					curPxCb[13] = pCb[(b+3*iPixStep)*iCBufHSize+(a+1*iPixStep)]; // C31
					curPxCb[14] = pCb[(b+3*iPixStep)*iCBufHSize+(a+2*iPixStep)]; // C32
					curPxCb[15] = pCb[(b+3*iPixStep)*iCBufHSize+(a+3*iPixStep)]; // C33

					Cbmean = ( curPxCb[0]+curPxCb[1] +curPxCb[2] +curPxCb[3] +curPxCb[4] +curPxCb[5] +curPxCb[6] +curPxCb[7]+
		     	 			   curPxCb[8]+curPxCb[9]+curPxCb[10]+curPxCb[11]+curPxCb[12]+curPxCb[13]+curPxCb[14]+curPxCb[15]+8 )>>4;
				}
				#endif

				MotionThCb = JND[Cbmean]*MotionRangeCb;
						
				MtnDiffSumCb = 0; NegCountCb = 0;

				preCbmean = pMeanKeptBufCb[CountCurBlockCb-1];
				pMeanKeptBufCb[CountCurBlockCb-1] = (UInt8) Cbmean;
				DoFilterProcCndt = false;

				if ( abs(Cbmean-preCbmean)<MeanCmDiffThd )
				{
					#if defined(__MMX__)
					{
						UInt8 *pCbBlkStart = (pCb+b*iCBufHSize+a); // notice!!!
						UInt8 *pKeptCbBlkStart = (pKeptBufCb+b*iCBufHSize+a); // notice!!!
						__asm
						{
							mov             eax,               0x00000004;
							movq            mm0,               Zeros64;
							movq            mm1,               Zeros64;
							movq            mm2,               Zeros64;
							movq            mm3,               Zeros64;
							movq            mm4,               Zeros64;
							movq            mm5,               Zeros64;
							movq            mm6,               Zeros64;
							mov		        esi,               pCbBlkStart;
							mov				edi,               pKeptCbBlkStart;
							mov				ebx,			   iPixStep;

						Line_Abs_Sub_Loop_Cb:
							//------------------------------------------------------------------
							cmp				ebx,			   1;
							jna				Pix_Step_1_Cb_Abs_Sub;
							//pixstep=2--
							movq            mm1,               [esi];       // mm1 = cCb7, cCb6, cCb5, cCb4, cCb3, cCb2, cCb1, cCb0
							psllw			mm1,			   8;			// mm1 = cCb6, 0, cCb4, 0, cCb2, 0, cCb0, 0
							psrlq			mm1,			   8;			// mm1 = 0, cCb6, 0, cCb4, 0, cCb2, 0, cCb0
							movq            mm2,               [edi];       // mm2 = pCb7, pCb6, pCb5, pCb4, pCb3, pCb2, pCb1, pCb0
							psllw			mm2,			   8;			// mm2 = pCb6, 0, pCb4, 0, pCb2, 0, pCb0, 0
							psrlq			mm2,			   8;			// mm2 = 0, pCb6, 0, pCb4, 0, pCb2, 0, pCb0
							add             esi,               iCBufHSize;
							add             edi,               iCBufHSize;
							jmp				End_Pix_step_Cb_Abs_Sub;

						Pix_Step_1_Cb_Abs_Sub:
							movd            mm1,               [esi];       // mm1 = 0, 0, 0, 0, cCb3, cCb2, cCb1, cCb0
							punpcklbw       mm1,               mm0;         // mm1 = 0 cCb3, 0 cCb2, 0 cCb1, 0 cCb0
							movd            mm2,               [edi];       // mm2 = 0, 0, 0, 0, pCb3, pCb2, pCb1, pCb0
							punpcklbw       mm2,               mm0;         // mm2 = 0 pCb3, 0 pCb2, 0 pCb1, 0 pCb0

						End_Pix_step_Cb_Abs_Sub:
							// Sum of Motion Difference
							movq            mm3,               mm1;         // mm3 = mm1
							movq            mm4,               mm2;         // mm4 = mm2
							psubusw         mm3,               mm2;         // mm3 = mm1 - mm2
							psubusw         mm4,               mm1;         // mm4 = mm2 - mm1
							paddusw         mm3,               mm4;         // mm3 = |mm1 - mm2|
							paddusw         mm5,               mm3;         // mm5 = mm3 + mmx;
							
							// Calculation of Negative Sign
							movq            mm4,               mm2;         // mm4 = mm2
							pcmpgtw         mm4,               mm1;
							psrlw           mm4,               15;
							paddusw         mm6,               mm4;         // mm6 = mm4 + mmx;

							add             esi,               iCBufHSize;
							add             edi,               iCBufHSize;
							
							dec             eax;
							jnz             Line_Abs_Sub_Loop_Cb;
							//-------------------------------------------------------------------------------
							// Sum of Motion Difference
							mov             eax,               Zeros32;
							mov             ebx,               Zeros32;
							movd            ecx,               mm5;         // ecx = (D1+D5+D9+D13), (D0+D4+D8+D12)
							psrlq           mm5,               32;          
							movd            edx,               mm5;         // edx = (D3+D7+D11+D15), (D2+D6+D10+D14)
							add             dx,                cx;          // dx = (D2+D6+D10+D14) + (D0+D4+D8+D12)
							mov             ax,                dx;          // ax = dx
							shr             ecx,               16;
							shr             edx,               16;
							add             dx,                cx;          // dx = (Cb2+Cb6+Cb10+Cb14) + (Cb0+Cb4+Cb8+Cb12) + (Cb1+Cb5+Cb9+Cb13)
							add             ax,                dx;          // ax = (Cb2+Cb6+Cb10+Cb14) + (Cb0+Cb4+Cb8+Cb12) + (Cb1+Cb5+Cb9+Cb13)+ (Cb3+Cb7+Cb11+Cb15)
							mov             MtnDiffSumCb,      eax;         // MtnDiffSumCb
							// Standard deviation of absolution difference
							add             ax,                8;
							shr             ax,                4;
							mov             bx,                ax;
							add             bx,                2;
							shr             bx,                2;
							add             ax,                bx;
							mov             diffCbstd,         eax;         // DiffCbstd
							// Calculation of Negative Sign
							mov             eax,               Zeros32;
							mov             ebx,               Zeros32;
							movd            ecx,               mm6;
							psrlq           mm6,               32;
							movd            edx,               mm6;
							add             dx,                cx;
							mov             ax,                dx;
							shr             ecx,               16;
							shr             edx,               16;
							add             dx,                cx;
							add             ax,                dx;
							mov             NegCountCb,        eax;         // NegCountCb
			
							emms;
						}
					}
					#else
					{
						// Previous Cb Frame
						prePxCb[0]  = pKeptBufCb[(b+0*iPixStep)*iCBufHSize+(a+0*iPixStep)]; // P00
						prePxCb[1]  = pKeptBufCb[(b+0*iPixStep)*iCBufHSize+(a+1*iPixStep)]; // P01
						prePxCb[2]  = pKeptBufCb[(b+0*iPixStep)*iCBufHSize+(a+2*iPixStep)]; // P02
						prePxCb[3]  = pKeptBufCb[(b+0*iPixStep)*iCBufHSize+(a+3*iPixStep)]; // P03
						prePxCb[4]  = pKeptBufCb[(b+1*iPixStep)*iCBufHSize+(a+0*iPixStep)]; // P10
						prePxCb[5]  = pKeptBufCb[(b+1*iPixStep)*iCBufHSize+(a+1*iPixStep)]; // P11
						prePxCb[6]  = pKeptBufCb[(b+1*iPixStep)*iCBufHSize+(a+2*iPixStep)]; // P12
						prePxCb[7]  = pKeptBufCb[(b+1*iPixStep)*iCBufHSize+(a+3*iPixStep)]; // P13
						prePxCb[8]  = pKeptBufCb[(b+2*iPixStep)*iCBufHSize+(a+0*iPixStep)]; // P20
						prePxCb[9]  = pKeptBufCb[(b+2*iPixStep)*iCBufHSize+(a+1*iPixStep)]; // P21
						prePxCb[10] = pKeptBufCb[(b+2*iPixStep)*iCBufHSize+(a+2*iPixStep)]; // P22
						prePxCb[11] = pKeptBufCb[(b+2*iPixStep)*iCBufHSize+(a+3*iPixStep)]; // P23
						prePxCb[12] = pKeptBufCb[(b+3*iPixStep)*iCBufHSize+(a+0*iPixStep)]; // P30
						prePxCb[13] = pKeptBufCb[(b+3*iPixStep)*iCBufHSize+(a+1*iPixStep)]; // P31
						prePxCb[14] = pKeptBufCb[(b+3*iPixStep)*iCBufHSize+(a+2*iPixStep)]; // P32
						prePxCb[15] = pKeptBufCb[(b+3*iPixStep)*iCBufHSize+(a+3*iPixStep)]; // P33

						for (y = 0; y < 4; y++) 
						{
							for (x = 0; x < 4; x++) 
							{
								MtnDiffSumCb += abs(curPxCb[y*4+x]-prePxCb[y*4+x]); // For Cb
								NegCountCb += (curPxCb[y*4+x]>=prePxCb[y*4+x])? 0: 1;
							}
						}

						diffCbstd = ((MtnDiffSumCb+8)>>4);
						diffCbstd += ((diffCbstd+2)>>2);
					}
					#endif
									
					// Normal motion check
					DoFilterProcCndt = ( MtnDiffSumCb<MotionThCb & (NegCountCb>=4 | NegCountCb<=11) & (diffCbstd<(JND[Cbmean]+NozPwrMarginCb)) );

					// Low light motion check
					DoFilterProcCndt = (LowLightMode)? ( DoFilterProcCndt & (CountProcBlockCb>=(CountCurBlockCb>>1)) ): DoFilterProcCndt;
					
					// Neighborhood motion check
					if ( (b>0) & (b<iCVSize-3*iPixStep) & (a>0) & (a<iCHSize-4*iPixStep) )
					{
						MotionCal = 0;
						if ( DoFilterProcCndt )
						{
							MotionCal += (pMoStCbPrePtr[(a>>2)-1*iPixStep])? 1: 0;
							MotionCal += (pMoStCbPrePtr[(a>>2)+0*iPixStep])? 1: 0;
							MotionCal += (pMoStCbPrePtr[(a>>2)+1*iPixStep])? 1: 0;
							MotionCal += (pMoStCbCurPtr[(a>>2)-1*iPixStep])? 1: 0;
							
							DoFilterProcCndt = DoFilterProcCndt & (MotionCal<3);
						}
					}

					// Temporal Average Filtering - Cb
					if ( DoFilterProcCndt ) 
					{
						CountProcBlockCb++;

						#if defined(__MMX__)
						{
							UInt8 *pCbBlkStart = (pCb+b*iCBufHSize+a); // notice!!!
							UInt8 *pKeptCbBlkStart = (pKeptBufCb+b*iCBufHSize+a); // notice!!!
							__asm
							{
								mov             eax,               0x00000004;
								movq            mm0,               Zeros64;
								movq            mm1,               Zeros64;
								movq            mm2,               Zeros64;
								movq            mm3,               ConstVal;
								mov		        esi,               pCbBlkStart;
								mov				edi,               pKeptCbBlkStart;
								mov				ebx,			   iPixStep;

							Average_Filter_Loop_Cb:
								//------------------------------------------------------------------
								cmp				ebx,			   1;
								jna				Pix_Step_1_Cb_Average_Filter;
								//pixstep=2--
								movq            mm1,               [esi];       // mm1 = cCb7, cCb6, cCb5, cCb4, cCb3, cCb2, cCb1, cCb0
								psllw			mm1,			   8;			// mm1 = cCb6, 0, cCb4, 0, cCb2, 0, cCb0, 0
								psrlq			mm1,			   8;			// mm1 = 0, cCb6, 0, cCb4, 0, cCb2, 0, cCb0
								movq            mm2,               [edi];       // mm2 = pCb7, pCb6, pCb5, pCb4, pCb3, pCb2, pCb1, pCb0
								psllw			mm2,			   8;			// mm2 = pCb6, 0, pCb4, 0, pCb2, 0, pCb0, 0
								psrlq			mm2,			   8;			// mm2 = 0, pCb6, 0, pCb4, 0, pCb2, 0, pCb0
								
								paddusw         mm1,               mm2;
								paddusw         mm1,               mm2;
								paddusw         mm1,               mm2;
								paddusw         mm1,               mm3;
								psrlw           mm1,               2;

								movq            [esi],             mm1;
								add             esi,               iCBufHSize;
								add             edi,               iCBufHSize;
								jmp				End_Pix_step_Cb_Average_Filter;

							Pix_Step_1_Cb_Average_Filter:
								movd            mm1,               [esi];       // mm1 = 0, 0, 0, 0, cCb3, cCb2, cCb1, cCb0
								punpcklbw       mm1,               mm0;         // mm1 = 0 cCb3, 0 cCb2, 0 cCb1, 0 cCb0
								movd            mm2,               [edi];       // mm2 = 0, 0, 0, 0, pCb3, pCb2, pCb1, pCb0
								punpcklbw       mm2,               mm0;         // mm2 = 0 pCb3, 0 pCb2, 0 pCb1, 0 pCb0

								paddusw         mm1,               mm2;
								paddusw         mm1,               mm2;
								paddusw         mm1,               mm2;
								paddusw         mm1,               mm3;
								psrlw           mm1,               2;
								packuswb        mm1,               mm0;

								movd            [esi],             mm1;

							End_Pix_step_Cb_Average_Filter:
								add             esi,               iCBufHSize;
								add             edi,               iCBufHSize;

								dec             eax;
								jnz             Average_Filter_Loop_Cb;
								//------------------------------------------------------------------
								emms;
							}
						}
						#else
						{
							for (y = 0; y < 4; y++) 
							{
								for (x = 0; x < 4; x++) 
								{
									pCb[(b+y*iPixStep)*iCBufHSize+(a+x*iPixStep)] = (UInt8) ( (pCb[(b+y*iPixStep)*iCBufHSize+(a+x*iPixStep)]+pKeptBufCb[(b+y*iPixStep)*iCBufHSize+(a+x*iPixStep)]*3+2)>>2 );
								}
							}
						}
						#endif
					}
				} // For mean check of motion block

				pMoStCbCurPtr[a>>2] = (!DoFilterProcCndt);
				// For Cb Part - End
							
				// For Cr Part - Start
				CountCurBlockCr++;

				#if defined(__MMX__)
				{
					UInt8 *pCrBlkStart = (pCr+b*iCBufHSize+a); // notice!!!
					__asm
					{
						mov             eax,               0x00000004;
						movq            mm0,               Zeros64;
						movq            mm1,               Zeros64;
						movq            mm2,               Zeros64;
						mov				ecx,               pCrBlkStart;
						mov				ebx,			   iPixStep;

					Line_Add_Loop_Cr:
						//------------------------------------------------------------------
						cmp				ebx,			   1;
						jna				Pix_Step_1_Cr_Add;
						//pixstep=2--
						movq            mm1,               [ecx];       // mm1 = Cr7, Cr6, Cr5, Cr4, Cr3, Cr2, Cr1, Cr0
						psllw			mm1,			   8;			// mm1 = Cr6, 0, Cr4, 0, Cr2, 0, Cr0, 0
						psrlq			mm1,			   8;			// mm1 = 0, Cr6, 0, Cr4, 0, Cr2, 0, Cr0
						add             ecx,               iCBufHSize;
						jmp				End_Pix_step_Cr_Add;

					Pix_Step_1_Cr_Add:
						movd            mm1,               [ecx];       // mm1 = 0, 0, 0, 0, Cr3, Cr2, Cr1, Cr0
						punpcklbw       mm1,               mm0;         // mm1 = 0 Cr3, 0 Cr2, 0 Cr1, 0 Cr0

					End_Pix_step_Cr_Add:
						paddusw         mm2,               mm1;         // mm2 = (Cr3+Cr7+Cr11+Cr15), (Cr2+Cr6+Cr10+Cr14), (Cr1+Cr5+Cr9+Cr13), (Cr0+Cr4+Cr8+Cr12)
						add             ecx,               iCBufHSize;
						dec             eax;
						jnz             Line_Add_Loop_Cr;
						//-------------------------------------------------------------------------------
						movd            ebx,               mm2;         // ebx = (Cr1+Cr5+Cr9+Cr13), (Cr0+Cr4+Cr8+Cr12)
						psrlq           mm2,               32;          
						movd            edx,               mm2;         // edx = (Cr3+Cr7+Cr11+Cr15), (Cr2+Cr6+Cr10+Cr14)
						add             dx,                bx;          // dx = (Cr2+Cr6+Cr10+Cr14) + (Cr0+Cr4+Cr8+Cr12)
						mov             ax,                dx;          // ax = dx
						shr             ebx,               16;
						shr             edx,               16;
						add             ax,                bx;          // ax = (Cr2+Cr6+Cr10+Cr14) + (Cr0+Cr4+Cr8+Cr12) + (Cr1+Cr5+Cr9+Cr13)
						add             ax,                dx;          // ax = (Cr2+Cr6+Cr10+Cr14) + (Cr0+Cr4+Cr8+Cr12) + (Cr1+Cr5+Cr9+Cr13)+ (Cr3+Cr7+Cr11+Cr15)
						add             ax,                8;
						shr             ax,                4;
						mov             Crmean,            eax;

						emms;
					}
				}
				#else
				{
					// Current Cr Frame
					curPxCr[0]  = pCr[(b+0*iPixStep)*iCBufHSize+(a+0*iPixStep)]; // C00
					curPxCr[1]  = pCr[(b+0*iPixStep)*iCBufHSize+(a+1*iPixStep)]; // C01
					curPxCr[2]  = pCr[(b+0*iPixStep)*iCBufHSize+(a+2*iPixStep)]; // C02
					curPxCr[3]  = pCr[(b+0*iPixStep)*iCBufHSize+(a+3*iPixStep)]; // C03
		  			curPxCr[4]  = pCr[(b+1*iPixStep)*iCBufHSize+(a+0*iPixStep)]; // C10
					curPxCr[5]  = pCr[(b+1*iPixStep)*iCBufHSize+(a+1*iPixStep)]; // C11
					curPxCr[6]  = pCr[(b+1*iPixStep)*iCBufHSize+(a+2*iPixStep)]; // C12
					curPxCr[7]  = pCr[(b+1*iPixStep)*iCBufHSize+(a+3*iPixStep)]; // C13
					curPxCr[8]  = pCr[(b+2*iPixStep)*iCBufHSize+(a+0*iPixStep)]; // C20
					curPxCr[9]  = pCr[(b+2*iPixStep)*iCBufHSize+(a+1*iPixStep)]; // C21
					curPxCr[10] = pCr[(b+2*iPixStep)*iCBufHSize+(a+2*iPixStep)]; // C22
					curPxCr[11] = pCr[(b+2*iPixStep)*iCBufHSize+(a+3*iPixStep)]; // C23
					curPxCr[12] = pCr[(b+3*iPixStep)*iCBufHSize+(a+0*iPixStep)]; // C30
					curPxCr[13] = pCr[(b+3*iPixStep)*iCBufHSize+(a+1*iPixStep)]; // C31
					curPxCr[14] = pCr[(b+3*iPixStep)*iCBufHSize+(a+2*iPixStep)]; // C32
					curPxCr[15] = pCr[(b+3*iPixStep)*iCBufHSize+(a+3*iPixStep)]; // C33

					Crmean = ( curPxCr[0]+curPxCr[1] +curPxCr[2] +curPxCr[3] +curPxCr[4] +curPxCr[5] +curPxCr[6] +curPxCr[7]+
		     	 			   curPxCr[8]+curPxCr[9]+curPxCr[10]+curPxCr[11]+curPxCr[12]+curPxCr[13]+curPxCr[14]+curPxCr[15]+8 )>>4;
				}
				#endif

				MotionThCr = JND[Crmean]*MotionRangeCr;
								
				MtnDiffSumCr = 0; NegCountCr = 0;

				preCrmean = pMeanKeptBufCr[CountCurBlockCr-1];
				pMeanKeptBufCr[CountCurBlockCr-1] = (UInt8) Crmean;
				DoFilterProcCndt = false;

				if ( abs(Crmean-preCrmean)<MeanCmDiffThd )
				{
					#if defined(__MMX__)
					{
						UInt8 *pCrBlkStart = (pCr+b*iCBufHSize+a); // notice!!!
						UInt8 *pKeptCrBlkStart = (pKeptBufCr+b*iCBufHSize+a); // notice!!!
						__asm
						{
							mov             eax,               0x00000004;
							movq            mm0,               Zeros64;
							movq            mm1,               Zeros64;
							movq            mm2,               Zeros64;
							movq            mm3,               Zeros64;
							movq            mm4,               Zeros64;
							movq            mm5,               Zeros64;
							movq            mm6,               Zeros64;
							mov		        esi,               pCrBlkStart;
							mov				edi,               pKeptCrBlkStart;
							mov				ebx,			   iPixStep;

						Line_Abs_Sub_Loop_Cr:
							//------------------------------------------------------------------
							cmp				ebx,			   1;
							jna				Pix_Step_1_Cr_Abs_Sub;
							//pixstep=2--
							movq            mm1,               [esi];       // mm1 = cCr7, cCr6, cCr5, cCr4, cCr3, cCr2, cCr1, cCr0
							psllw			mm1,			   8;			// mm1 = cCr6, 0, cCr4, 0, cCr2, 0, cCr0, 0
							psrlq			mm1,			   8;			// mm1 = 0, cCr6, 0, cCr4, 0, cCr2, 0, cCr0
							movq            mm2,               [edi];       // mm2 = pCr7, pCr6, pCr5, pCr4, pCr3, pCr2, pCr1, pCr0
							psllw			mm2,			   8;			// mm2 = pCr6, 0, pCr4, 0, pCr2, 0, pCr0, 0
							psrlq			mm2,			   8;			// mm2 = 0, pCr6, 0, pCr4, 0, pCr2, 0, pCr0
							add             esi,               iCBufHSize;
							add             edi,               iCBufHSize;
							jmp				End_Pix_step_Cr_Abs_Sub;

						Pix_Step_1_Cr_Abs_Sub:
							movd            mm1,               [esi];       // mm1 = 0, 0, 0, 0, cCr3, cCr2, cCr1, cCr0
							punpcklbw       mm1,               mm0;         // mm1 = 0 cCr3, 0 cCr2, 0 cCr1, 0 cCr0
							movd            mm2,               [edi];       // mm2 = 0, 0, 0, 0, pCr3, pCr2, pCr1, pCr0
							punpcklbw       mm2,               mm0;         // mm2 = 0 pCr3, 0 pCr2, 0 pCr1, 0 pCr0

						End_Pix_step_Cr_Abs_Sub:
							// Sum of Motion Difference
							movq            mm3,               mm1;         // mm3 = mm1
							movq            mm4,               mm2;         // mm4 = mm2
							psubusw         mm3,               mm2;         // mm3 = mm1 - mm2
							psubusw         mm4,               mm1;         // mm4 = mm2 - mm1
							paddusw         mm3,               mm4;         // mm3 = |mm1 - mm2|
							paddusw         mm5,               mm3;         // mm5 = mm3 + mmx;
							
							// Calculation of Negative Sign
							movq            mm4,               mm2;         // mm4 = mm2
							pcmpgtw         mm4,               mm1;
							psrlw           mm4,               15;
							paddusw         mm6,               mm4;         // mm6 = mm4 + mmx;

							add             esi,               iCBufHSize;
							add             edi,               iCBufHSize;
							
							dec             eax;
							jnz             Line_Abs_Sub_Loop_Cr;
							//-------------------------------------------------------------------------------
							// Sum of Motion Difference
							mov             eax,               Zeros32;
							mov             ebx,               Zeros32;
							movd            ecx,               mm5;         // ecx = (D1+D5+D9+D13), (D0+D4+D8+D12)
							psrlq           mm5,               32;          
							movd            edx,               mm5;         // edx = (D3+D7+D11+D15), (D2+D6+D10+D14)
							add             dx,                cx;          // dx = (D2+D6+D10+D14) + (D0+D4+D8+D12)
							mov             ax,                dx;          // ax = dx
							shr             ecx,               16;
							shr             edx,               16;
							add             dx,                cx;          // dx = (Cr2+Cr6+Cr10+Cr14) + (Cr0+Cr4+Cr8+Cr12) + (Cr1+Cr5+Cr9+Cr13)
							add             ax,                dx;          // ax = (Cr2+Cr6+Cr10+Cr14) + (Cr0+Cr4+Cr8+Cr12) + (Cr1+Cr5+Cr9+Cr13)+ (Cr3+Cr7+Cr11+Cr15)
							mov             MtnDiffSumCr,      eax;         // MtnDiffSumCr
							// Standard deviation of absolution difference
							add             ax,                8;
							shr             ax,                4;
							mov             bx,                ax;
							add             bx,                2;
							shr             bx,                2;
							add             ax,                bx;
							mov             diffCrstd,         eax;         // DiffCrstd
							// Calculation of Negative Sign
							mov             eax,               Zeros32;
							mov             ebx,               Zeros32;
							movd            ecx,               mm6;
							psrlq           mm6,               32;
							movd            edx,               mm6;
							add             dx,                cx;
							mov             ax,                dx;
							shr             ecx,               16;
							shr             edx,               16;
							add             dx,                cx;
							add             ax,                dx;
							mov             NegCountCr,        eax;         // NegCountCr
			
							emms;
						}
					}
					#else
					{
						// Previous Cr Frame
						prePxCr[0]  = pKeptBufCr[(b+0*iPixStep)*iCBufHSize+(a+0*iPixStep)]; // P00
						prePxCr[1]  = pKeptBufCr[(b+0*iPixStep)*iCBufHSize+(a+1*iPixStep)]; // P01
						prePxCr[2]  = pKeptBufCr[(b+0*iPixStep)*iCBufHSize+(a+2*iPixStep)]; // P02
						prePxCr[3]  = pKeptBufCr[(b+0*iPixStep)*iCBufHSize+(a+3*iPixStep)]; // P03
		  				prePxCr[4]  = pKeptBufCr[(b+1*iPixStep)*iCBufHSize+(a+0*iPixStep)]; // P10
						prePxCr[5]  = pKeptBufCr[(b+1*iPixStep)*iCBufHSize+(a+1*iPixStep)]; // P11
						prePxCr[6]  = pKeptBufCr[(b+1*iPixStep)*iCBufHSize+(a+2*iPixStep)]; // P12
						prePxCr[7]  = pKeptBufCr[(b+1*iPixStep)*iCBufHSize+(a+3*iPixStep)]; // P13
						prePxCr[8]  = pKeptBufCr[(b+2*iPixStep)*iCBufHSize+(a+0*iPixStep)]; // P20
						prePxCr[9]  = pKeptBufCr[(b+2*iPixStep)*iCBufHSize+(a+1*iPixStep)]; // P21
						prePxCr[10] = pKeptBufCr[(b+2*iPixStep)*iCBufHSize+(a+2*iPixStep)]; // P22
						prePxCr[11] = pKeptBufCr[(b+2*iPixStep)*iCBufHSize+(a+3*iPixStep)]; // P23
						prePxCr[12] = pKeptBufCr[(b+3*iPixStep)*iCBufHSize+(a+0*iPixStep)]; // P30
						prePxCr[13] = pKeptBufCr[(b+3*iPixStep)*iCBufHSize+(a+1*iPixStep)]; // P31
						prePxCr[14] = pKeptBufCr[(b+3*iPixStep)*iCBufHSize+(a+2*iPixStep)]; // P32
						prePxCr[15] = pKeptBufCr[(b+3*iPixStep)*iCBufHSize+(a+3*iPixStep)]; // P33

						for (y = 0; y < 4; y++) 
						{
							for (x = 0; x < 4; x++) 
							{
								MtnDiffSumCr += abs(curPxCr[y*4+x]-prePxCr[y*4+x]); // For Cr
								NegCountCr += (curPxCr[y*4+x]>=prePxCr[y*4+x])? 0: 1;
							}
						}
						diffCrstd = ((MtnDiffSumCr+8)>>4);
						diffCrstd += ((diffCrstd+2)>>2);
					}
					#endif
					
					// Normal motion check
					DoFilterProcCndt = ( MtnDiffSumCr<MotionThCr & (NegCountCr>=4 | NegCountCr<=11) & (diffCrstd<(JND[Crmean]+NozPwrMarginCr)) );

					// Low light motion check
					DoFilterProcCndt = (LowLightMode)? ( DoFilterProcCndt & (CountProcBlockCr>=(CountCurBlockCr>>1)) ): DoFilterProcCndt;
					
					// Neighborhood motion check
					if ( (b>0) & (b<iCVSize-3*iPixStep) & (a>0) & (a<iCHSize-4*iPixStep) )
					{
						MotionCal = 0;
						if ( DoFilterProcCndt )
						{
							MotionCal += (pMoStCrPrePtr[(a>>2)-1*iPixStep])? 1: 0;
							MotionCal += (pMoStCrPrePtr[(a>>2)+0*iPixStep])? 1: 0;
							MotionCal += (pMoStCrPrePtr[(a>>2)+1*iPixStep])? 1: 0;
							MotionCal += (pMoStCrCurPtr[(a>>2)-1*iPixStep])? 1: 0;
							
							DoFilterProcCndt = DoFilterProcCndt & (MotionCal<3);
						}
					}
					
					// Temporal Average Filtering - Cr
					if ( DoFilterProcCndt ) 
					{
						CountProcBlockCr++;

						#if defined(__MMX__)
						{
							UInt8 *pCrBlkStart = (pCr+b*iCBufHSize+a); // notice!!!
							UInt8 *pKeptCrBlkStart = (pKeptBufCr+b*iCBufHSize+a); // notice!!!
							__asm
							{
								mov             eax,               0x00000004;
								movq            mm0,               Zeros64;
								movq            mm1,               Zeros64;
								movq            mm2,               Zeros64;
								movq            mm3,               ConstVal;
								mov		        esi,               pCrBlkStart;
								mov				edi,               pKeptCrBlkStart;
								mov				ebx,			   iPixStep;

							Average_Filter_Loop_Cr:
								//------------------------------------------------------------------
								cmp				ebx,			   1;
								jna				Pix_Step_1_Cr_Average_Filter;
								//pixstep=2--
								movq            mm1,               [esi];       // mm1 = cCr7, cCr6, cCr5, cCr4, cCr3, cCr2, cCr1, cCr0
								psllw			mm1,			   8;			// mm1 = cCr6, 0, cCr4, 0, cCr2, 0, cCr0, 0
								psrlq			mm1,			   8;			// mm1 = 0, cCr6, 0, cCr4, 0, cCr2, 0, cCr0
								movq            mm2,               [edi];       // mm2 = pCr7, pCr6, pCr5, pCr4, pCr3, pCr2, pCr1, pCr0
								psllw			mm2,			   8;			// mm2 = pCr6, 0, pCr4, 0, pCr2, 0, pCr0, 0
								psrlq			mm2,			   8;			// mm2 = 0, pCr6, 0, pCr4, 0, pCr2, 0, pCr0
								
								paddusw         mm1,               mm2;
								paddusw         mm1,               mm2;
								paddusw         mm1,               mm2;
								paddusw         mm1,               mm3;
								psrlw           mm1,               2;

								movq            [esi],             mm1;
								add             esi,               iCBufHSize;
								add             edi,               iCBufHSize;
								jmp				End_Pix_step_Cr_Average_Filter;

							Pix_Step_1_Cr_Average_Filter:
								movd            mm1,               [esi];       // mm1 = 0, 0, 0, 0, cCr3, cCr2, cCr1, cCr0
								punpcklbw       mm1,               mm0;         // mm1 = 0 cCr3, 0 cCr2, 0 cCr1, 0 cCr0
								movd            mm2,               [edi];       // mm2 = 0, 0, 0, 0, pCr3, pCr2, pCr1, pCr0
								punpcklbw       mm2,               mm0;         // mm2 = 0 pCr3, 0 pCr2, 0 pCr1, 0 pCr0
								
								paddusw         mm1,               mm2;
								paddusw         mm1,               mm2;
								paddusw         mm1,               mm2;
								paddusw         mm1,               mm3;
								psrlw           mm1,               2;
								packuswb        mm1,               mm0;
								movd            [esi],             mm1;

							End_Pix_step_Cr_Average_Filter:
								add             esi,               iCBufHSize;
								add             edi,               iCBufHSize;

								dec             eax;
								jnz             Average_Filter_Loop_Cr;

								emms;
							}
						}
						#else
						{
							for (y = 0; y < 4; y++) 
							{
								for (x = 0; x < 4; x++) 
								{
									pCr[(b+y*iPixStep)*iCBufHSize+(a+x*iPixStep)] = (UInt8) ( (pCr[(b+y*iPixStep)*iCBufHSize+(a+x*iPixStep)]+pKeptBufCr[(b+y*iPixStep)*iCBufHSize+(a+x*iPixStep)]*3+2)>>2 );
								}
							}
						}		  					
						#endif
					}
				} // For mean check of motion block

				pMoStCrCurPtr[a>>2] = (!DoFilterProcCndt);
				// For Cr Part - End
				
			} // For Chroma Part - End
			
		}	// for i

		memset(pMoStYPrePtr, 0, 1600/4);
		bool *pMoStTemp;
		pMoStTemp = pMoStYCurPtr;
		pMoStYCurPtr = pMoStYPrePtr;
		pMoStYPrePtr = pMoStTemp;

		//Kathy
		if(pShellParam->bEdgeDtn)
		{
			pMoStTemp = CurSmoothBlockMark;
			CurSmoothBlockMark = PreSmoothBlockMark;
			PreSmoothBlockMark = pMoStTemp;


			int *pTemp;
			pTemp = YBlockCurDCValue;
			YBlockCurDCValue = YBlockPreDCValue;
			YBlockPreDCValue = pTemp;
		}
		bChroState = ( (jCount & 1) & (b < (iCVSize-3*iPixStep)) );
		if ( bChroState )
		{
			// Cb part
			memset(pMoStCbPrePtr, 0, 1600/2/4);
			pMoStTemp = pMoStCbCurPtr;
			pMoStCbCurPtr = pMoStCbPrePtr;
			pMoStCbPrePtr = pMoStTemp;
			// Cr part
			memset(pMoStCrPrePtr, 0, 1600/2/4);
			pMoStTemp = pMoStCrCurPtr;
			pMoStCrCurPtr = pMoStCrPrePtr;
			pMoStCrPrePtr = pMoStTemp;
		}

	} // for j
}
//---------------------------------------------------------------------------

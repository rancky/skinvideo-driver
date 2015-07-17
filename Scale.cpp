#if defined(__LINUX__)
// --------defined LINUX-------------
    #include <snxcam.h>
// ----------------------------------
#elif defined(__KERNEL_MODE__)
    #include <sncam.h>
#else
    #include "Scale.h"
#endif
//---------------------------------------------------------------------------

CScale* CScale_CScale()
{
    CScale *mp;
    #if defined(__LINUX__)
    // --------defined LINUX-------------
    {
        mp = (CScale *)rvmalloc(sizeof(CScale));
        memset(&mp->Param,  0, sizeof(mp->Param));
        memset(&mp->Report, 0, sizeof(mp->Report));
    }
    // ----------------------------------
    #elif defined(__KERNEL_MODE__)
    {
        if((mp = (CScale *)ExAllocatePoolWithTag(NonPagedPool, sizeof(CScale),'SNX')) == NULL)
			return mp;
        RtlZeroMemory(&mp->Param,  sizeof(mp->Param));
        RtlZeroMemory(&mp->Report, sizeof(mp->Report));
    }
    #else
    {
		if((mp = (CScale *)malloc(sizeof(CScale)))==NULL)
			return mp;
        memset(&mp->Param,  0, sizeof(mp->Param));
        memset(&mp->Report, 0, sizeof(mp->Report));
    }
    #endif

    return mp;
}
//---------------------------------------------------------------------------

void CScale__CScale(CScale *mp)
{
#if defined(__LINUX__)
	// --------defined LINUX-------------
	rvfree(mp, sizeof(CScale));
	// ----------------------------------
#elif defined(__KERNEL_MODE__)
	if(mp!=NULL)
		ExFreePoolWithTag(mp,'SNX');
#else
	free(mp);
#endif
}
//---------------------------------------------------------------------------
    
void CScale_ReadScaleParamFromReg(CScale *mp,DWORD *pScaleParams)
{
    if (pScaleParams == NULL)
        return;

#if defined(PARAMETER_FROM_TESTAP)
#else
#endif
}
//---------------------------------------------------------------------------

void CScale_Reset(CScale *mp)
{
}
//---------------------------------------------------------------------------

void CScale_WriteScaleParamToReg(CScale *mp,DWORD *pScaleParams)
{
    if (pScaleParams == NULL)
        return;
}
//---------------------------------------------------------------------------

BOOL CScale_SetScaleSize(CScale *mp, int iBufHSize, int iBufVSize, int iHStart, int iVStart,
    int iHiSize, int iViSize, int iHoSize, int iVoSize, int a32SWSubSample[2], int a32SrcBlkSize[2])
{
    ScaleParam  *pParam  = &mp->Param;

    if ((a32SWSubSample[0] < 0) || (a32SWSubSample[0] > 2))
	{
		//TRACE("This version only support scale ratio: 1:1/1:2/1:4 !");
		
        return FALSE;
	}
    if ((a32SWSubSample[1] < 0) || (a32SWSubSample[1] > 2))
	{
		//TRACE("This version only support scale ratio: 1:1/1:2/1:4 !");
		
        return FALSE;
	}
    // Steven : 2005/11/02 for 3X scale
    if ((a32SrcBlkSize[0]<iDstBlkSize/3) || (a32SrcBlkSize[0]>iDstBlkSize))
        return FALSE;
    if ((a32SrcBlkSize[1]<iDstBlkSize/3) || (a32SrcBlkSize[1]>iDstBlkSize))
        return FALSE;
    if ((iHiSize>iBufHSize) || (iViSize>iBufVSize))
        return FALSE;
    if ((iHoSize>iBufHSize) || (iVoSize>iBufVSize))
        return FALSE;

    pParam->iBufHSize = iBufHSize;
    pParam->iBufVSize = iBufVSize;
    pParam->iHStart = iHStart;
    pParam->iVStart = iVStart;
    pParam->iHiSize  = iHiSize;
    pParam->iViSize  = iViSize;
    pParam->iHoSize  = iHoSize;
    pParam->iVoSize  = iVoSize;
    pParam->a32SWSubSample[0] = a32SWSubSample[0];
    pParam->a32SWSubSample[1] = a32SWSubSample[1];
    pParam->a32SrcBlkSize[0]  = a32SrcBlkSize[0];
    pParam->a32SrcBlkSize[1]  = a32SrcBlkSize[1];

    CScale_CreateScalingTap(mp);
    return TRUE;
}
//---------------------------------------------------------------------------

void CScale_CreateScalingTap(CScale *mp)
{
	int		i;
    ScaleParam	*pParam		= &mp->Param;
	
	
	if ((iDstBlkSize == pParam->a32SrcBlkSize[0]) && (iDstBlkSize == pParam->a32SrcBlkSize[1]))
	{
		return;
	}
	
	for (i = 0; i < iDstBlkSize; i ++)
	{
		mp->aa32ScaleIdx[0][i]	= (i * pParam->a32SrcBlkSize[0]) / iDstBlkSize;	//++ 各点在水平方向的映射位置
		mp->aa32ScaleIdx[1][i]	= (i * pParam->a32SrcBlkSize[1]) / iDstBlkSize;	//++ 各点在垂直方向的映射位置
		
		mp->iScaleLineIdx[i]	= mp->aa32ScaleIdx[1][i] * pParam->iBufHSize;	//++ 各点在内存中的行偏移距离
		
		mp->aa32ScaleTap[0][i]	= iDstBlkSize - (i * pParam->a32SrcBlkSize[0]) % iDstBlkSize;	//++ 各参考点在水平方向的权重
		mp->aa32ScaleTap[1][i]	= iDstBlkSize - (i * pParam->a32SrcBlkSize[1]) % iDstBlkSize;	//++ 各参考点在垂直方向的权重
		
#if defined(__MMX__)
		int		s32WeightA;
		int		s32WeightB;
		
		s32WeightA	= mp->aa32ScaleTap[1][i];
		s32WeightB	= iDstBlkSize - mp->aa32ScaleTap[1][i];
		mp->iVScaleTap64[i][0] = ((UInt64)(s32WeightA & 0xffff)<<48) | ((UInt64)(s32WeightA & 0xffff)<<32) | ((UInt64)(s32WeightA & 0xffff)<<16) | (UInt64)(s32WeightA & 0xffff);
		mp->iVScaleTap64[i][1] = ((UInt64)(s32WeightB & 0xffff)<<48) | ((UInt64)(s32WeightB & 0xffff)<<32) | ((UInt64)(s32WeightB & 0xffff)<<16) | (UInt64)(s32WeightB & 0xffff);
		s32WeightA	= mp->aa32ScaleTap[0][i];
		s32WeightB	= iDstBlkSize - mp->aa32ScaleTap[0][i];
		mp->iHScaleTap16[2*i]   = (UInt16)s32WeightA;
		mp->iHScaleTap16[2*i+1] = (UInt16)s32WeightB;
#endif
	}
}
//---------------------------------------------------------------------------

void CScale_SubSample(CScale *mp,UInt8 *pSrc, UInt8 *pDst)
{
}
//-----------------------------------------------------------------------------

void CScale_UpSample(CScale *mp, UInt8 *pSrc, UInt8 *pDst)
{
	int		s32StepH, s32StepV;
	int		a32SrcBlkSize[2];
    ScaleParam	*pParam  = &mp->Param;
	static int	s32Count	= 0;


	a32SrcBlkSize[0]	= pParam->a32SrcBlkSize[0];
	a32SrcBlkSize[1]	= pParam->a32SrcBlkSize[1];
	if ((a32SrcBlkSize[0] == iDstBlkSize) && (a32SrcBlkSize[1] == iDstBlkSize))
	{
		return;
	}

	s32StepH	= 1 << pParam->a32SWSubSample[0];
	s32StepV	= 1 << pParam->a32SWSubSample[1];

    int iYBufHSize = pParam->iBufHSize;
    int iYBufVSize = pParam->iBufVSize;
    int iCBufHSize = iYBufHSize /2;
    int iCBufVSize = iYBufVSize /2;
    int iYBufSize = iYBufHSize *iYBufVSize;
    int iCBufSize = iCBufHSize *iCBufVSize;

    int iYHStart = pParam->iHStart;
    int iYVStart = pParam->iVStart;
    int iCHStart = iYHStart /2;
    int iCVStart = iYVStart /2;

    int iYHoSize = pParam->iHoSize * s32StepH;
    int iYVoSize = pParam->iVoSize * s32StepV;
    int iCHoSize = iYHoSize /2;
    int iCVoSize = iYVoSize /2;
    
    UInt8 *pYSrc = pSrc +iYVStart*iYBufHSize +iYHStart;
    UInt8 *pYDst = pDst +iYVStart*iYBufHSize +iYHStart;
    UInt8 *pCSrc = pSrc +iYBufSize +iCVStart*iCBufHSize +iCHStart;
    UInt8 *pCDst = pDst +iYBufSize +iCVStart*iCBufHSize +iCHStart;

    //Y part
    {
        int iHBlkCnt = iYHoSize/iDstBlkSize +((iYHoSize%iDstBlkSize)? 1: 0);
        int iVBlkCnt = iYVoSize/iDstBlkSize +((iYVoSize%iDstBlkSize)? 1: 0);

        for (int j = iVBlkCnt-1; j >= 0; j--)
        {
            for (int i = iHBlkCnt-1; i >= 0; i--)
            {
                UInt8 *pYBlkSrc = pYSrc +j*a32SrcBlkSize[1]*iYBufHSize +i*a32SrcBlkSize[0];
                UInt8 *pYBlkDst = pYDst +j*iDstBlkSize*iYBufHSize +i*iDstBlkSize;
                UInt32 AddrYSrc = (UInt32)pYBlkSrc;
                UInt32 AddrYDst = (UInt32)pYBlkDst;

                #if defined(__MMX__)
                {
                    int umax = (j==iVBlkCnt-1)? (iYVoSize -iDstBlkSize*j-s32StepV): iDstBlkSize-s32StepV;
                    int vmax = (i==iHBlkCnt-1)? (iYHoSize -iDstBlkSize*i-s32StepH): iDstBlkSize-s32StepH;
                    int iDstLineOff = umax*iYBufHSize;
					
                    for (int u = umax; u >= 0; u-=s32StepV, iDstLineOff-=(iYBufHSize*s32StepV))
                    {
                        int iOffU = mp->iScaleLineIdx[u];
                        int iOffD = iOffU +iYBufHSize;
                        {
                            //Tap
                            //      a a'  b b'
                            //  A
                            //  A'
                            UInt64 iUTap64 = mp->iVScaleTap64[u][0];
                            UInt64 iDTap64 = mp->iVScaleTap64[u][1];
                            UInt32 AddrScaleIdx = (UInt32)(&mp->aa32ScaleIdx[0][0]);
                            UInt32 AddrScaleTap = (UInt32)(&mp->iHScaleTap16[0]);
                            UInt32 AddrYDstPix = AddrYDst +iDstLineOff;

                            //....................................................
                            //...................Y25   Y24   Y23   Y22   Y21   Y20
                            //                             -----------------------
                            //...................Y15   Y14 | Y13   Y12   Y11   Y10
                            //
                            //...................Y05   Y04 | Y03   Y02   Y01   Y00
                            __asm
                            {
                                mov             eax,            s32StepH;
                                neg             eax;
                                movd            mm4,            eax;
                                movd            mm5,            iYBufHSize;
                                movq            mm6,            iUTap64;
                                movq            mm7,            iDTap64;
                                mov             ecx,            vmax;
                                mov             esi,            AddrYSrc;
                                mov             edi,            AddrYDstPix;
                                add             esi,            iOffU;
                                lea             edi,            [edi+ecx];
                                mov             eax,            AddrScaleTap;
                                mov             ebx,            AddrScaleIdx;
                                lea             eax,            [eax+ecx*4];
                                lea             ebx,            [ebx+ecx*4];
                            UpSampleY:
                                push            ecx;
                                //-----------------------------------------------------
                                movd            mm0,            [eax];
                                movq            mm1,            mm7;
                                pmullw          mm1,            mm0;                //mm1 = A'*a A'*a'
                                pmullw          mm0,            mm6;                //mm0 = A *a A* a'
                                //-----------------------------------------------------
                                mov             ecx,            [ebx];
                                movd            edx,            mm5;
                                lea             ecx,            [esi+ecx];
                                //-----------------------------------------------------
                                punpcklbw       mm2,            [ecx];
                                punpcklbw       mm3,            [ecx+edx];
                                psrlw           mm2,            8;                  //mm2 = ... Y11 Y10
                                psrlw           mm3,            8;                  //mm3 = ... Y01 Y00
                                pmaddwd         mm0,            mm2;
                                pmaddwd         mm1,            mm3;
                                paddd           mm0,            mm1;                //mm0 = Yn00
                                //-----------------------------------------------------
                                movd            ecx,            mm0;
                                movd            edx,            mm4;
                                shr             ecx,            iDstBlkSizePowerx2;
                                lea             eax,            [eax+edx*4];
                                mov             [edi],          cl;
                                lea             ebx,            [ebx+edx*4];
                                pop             ecx;
                                lea             edi,            [edi+edx];
                                add             ecx,            edx;
                                jge             UpSampleY;
                                emms;
                            }
                        }
                    }
                }
                #else
                {
                    int umax = (j==iVBlkCnt-1)? (iYVoSize -iDstBlkSize*j-s32StepV): iDstBlkSize-s32StepV;
                    int vmax = (i==iHBlkCnt-1)? (iYHoSize -iDstBlkSize*i-s32StepH): iDstBlkSize-s32StepH;
                    int iDstLineOff = umax*iYBufHSize;

                    for (int u = umax; u >= 0; u-=s32StepV, iDstLineOff-=(iYBufHSize*s32StepV))
                    {
                        int iOffU = mp->iScaleLineIdx[u];
                        int iOffD = iOffU +iYBufHSize;
                        int iUTap = mp->aa32ScaleTap[1][u];
                        int iDTap = iDstBlkSize -iUTap;

                        for (int v = vmax; v >= 0; v-=s32StepH)
                        {
                            SInt32 iDstOff = iDstLineOff+v;

                            SInt32 iOffL;
                            SInt32 iOffLU = iOffU + (iOffL=mp->aa32ScaleIdx[0][v]);
                            SInt32 iOffLD = iOffD + iOffL;

                            SInt32 iLTap, iRTap;
                            SInt32 iLUTap = iUTap *(iLTap=mp->aa32ScaleTap[0][v]);
                            SInt32 iRUTap = iUTap *(iRTap=iDstBlkSize-iLTap);
                            SInt32 iLDTap = iDTap *iLTap;
                            SInt32 iRDTap = iDTap *iRTap;

                            pYBlkDst[iDstOff] = (iLUTap*pYBlkSrc[iOffLU] + iRUTap*pYBlkSrc[iOffLU+1] +
                                                 iLDTap*pYBlkSrc[iOffLD] + iRDTap*pYBlkSrc[iOffLD+1]) >>iDstBlkSizePowerx2;
                        }
                    }
                }
                #endif
            }
        }
    }

    //C part
    {
        int iHBlkCnt = iCHoSize/iDstBlkSize +((iCHoSize%iDstBlkSize)? 1: 0);
        int iVBlkCnt = iCVoSize/iDstBlkSize +((iCVoSize%iDstBlkSize)? 1: 0);

        for (int j = iVBlkCnt-1; j >= 0; j--)
        {
            for (int i = iHBlkCnt-1; i >= 0; i--)
            {
                UInt8 *pCBlkSrc = pCSrc +j*a32SrcBlkSize[1]*iCBufHSize +i*a32SrcBlkSize[0];
                UInt8 *pCBlkDst = pCDst +j*iDstBlkSize*iCBufHSize +i*iDstBlkSize;
                UInt32 AddrCSrc = (UInt32)pCBlkSrc;
                UInt32 AddrCDst = (UInt32)pCBlkDst;

                #if defined(__MMX__)
                {
                    int umax = (j==iVBlkCnt-1)? (iCVoSize -iDstBlkSize*j-s32StepV): iDstBlkSize-s32StepV;
                    int vmax = (i==iHBlkCnt-1)? (iCHoSize -iDstBlkSize*i-s32StepH): iDstBlkSize-s32StepH;
                    int iDstLineOff = umax*iCBufHSize;
					
                    for (int u = umax; u >= 0; u-=s32StepV, iDstLineOff-=(iCBufHSize*s32StepV))
                    {
                        int iOffU = mp->iScaleLineIdx[u]>>1;
                        int iOffD = iOffU +iCBufHSize;
                        {
                            //Tap
                            //      a a'  b b'
                            //  A
                            //  A'
                            UInt64 iUTap64 = mp->iVScaleTap64[u][0];
                            UInt64 iDTap64 = mp->iVScaleTap64[u][1];
                            UInt32 AddrScaleIdx = (UInt32)(&mp->aa32ScaleIdx[0][0]);
                            UInt32 AddrScaleTap = (UInt32)(&mp->iHScaleTap16[0]);
                            UInt32 AddrCDstPix = AddrCDst +iDstLineOff;

                            //....................................................
                            //...................C25   C24   C23   C22   C21   C20
                            //                             -----------------------
                            //...................C15   C14 | C13   C12   C11   C10
                            //
                            //...................C05   C04 | C03   C02   C01   C00
                            int iPixStep_ = -s32StepH;
                            __asm
                            {
                                movd            mm5,            iCBufSize;
                                movq            mm6,            iUTap64;
                                movq            mm7,            iDTap64;
                                mov             ecx,            vmax;
                                mov             esi,            AddrCSrc;
                                mov             edi,            AddrCDstPix;
                                add             esi,            iOffU;
                                lea             edi,            [edi+ecx];
                                mov             eax,            AddrScaleTap;
                                mov             ebx,            AddrScaleIdx;
                                lea             eax,            [eax+ecx*4];
                                lea             ebx,            [ebx+ecx*4];
                            UpSampleC:
                                push            ecx;
                                //-----------------------------------------------------
                                movd            mm0,            [eax];
                                movq            mm1,            mm7;
                                pmullw          mm1,            mm0;                //mm1 = A'*a A'*a'
                                pmullw          mm0,            mm6;                //mm0 = A *a A* a'
                                punpckldq       mm1,            mm1;                //mm1 = A'*a A'*a' A'*a A'*a'
                                punpckldq       mm0,            mm0;                //mm0 = A *a A* a' A *a A* a'
                                //-----------------------------------------------------
                                mov             ecx,            [ebx];
                                movd            edx,            mm5;
                                lea             ecx,            [esi+ecx];
                                //-----------------------------------------------------
                                pxor            mm4,            mm4;
                                movd            mm2,            [ecx];
                                punpcklwd       mm2,            [ecx+edx];
                                add             ecx,            iCBufHSize;
                                movd            mm3,            [ecx];
                                punpcklwd       mm3,            [ecx+edx];
                                punpcklbw       mm2,            mm4;                //mm2 = V11 V10 U11 U10
                                punpcklbw       mm3,            mm4;                //mm3 = V01 V00 U01 U00
                                pmaddwd         mm2,            mm0;
                                pmaddwd         mm3,            mm1;
                                paddd           mm2,            mm3;
                                psrld           mm2,            iDstBlkSizePowerx2;
                                packssdw        mm2,            mm3;
                                packuswb        mm2,            mm2;
                                //-----------------------------------------------------
                                movd            edx,            mm5;
                                movd            ecx,            mm2;
                                mov             [edi],          cl;
                                mov             [edi+edx],      ch;
                                //-----------------------------------------------------
                                mov             edx,            iPixStep_;
                                lea             eax,            [eax+edx*4];
                                lea             ebx,            [ebx+edx*4];
                                pop             ecx;
                                lea             edi,            [edi+edx];
                                add             ecx,            edx;
                                jge             UpSampleC;
                                emms;
                            }
                        }
                    }
                }
                #else
                {
                    UInt8 *pCbBlkSrc = pCBlkSrc;
                    UInt8 *pCrBlkSrc = pCbBlkSrc +iCBufSize;

                    UInt8 *pCbBlkDst = pCBlkDst;
                    UInt8 *pCrBlkDst = pCbBlkDst +iCBufSize;
					
					// TSTU 2005/11/02 : fixed bug
                    int umax = (j==iVBlkCnt-1)? (iCVoSize -iDstBlkSize*j-s32StepV): iDstBlkSize-s32StepV;
                    int vmax = (i==iHBlkCnt-1)? (iCHoSize -iDstBlkSize*i-s32StepH): iDstBlkSize-s32StepH;
                    int iDstLineOff = umax*iCBufHSize;

                    for (int u = umax; u >= 0; u-=s32StepV, iDstLineOff-=(iCBufHSize*s32StepV))
                    {
                        int iOffU = mp->iScaleLineIdx[u]>>1;
                        int iOffD = iOffU +iCBufHSize;

                        int iUTap = mp->aa32ScaleTap[1][u];
                        int iDTap = iDstBlkSize -iUTap;

                        for (int v = vmax; v >= 0; v-=s32StepH)
                        {
                            SInt32 iDstOff = iDstLineOff+v;

                            SInt32 iOffL;
                            SInt32 iOffLU = iOffU + (iOffL=mp->aa32ScaleIdx[0][v]);
                            SInt32 iOffLD = iOffD + iOffL;

                            SInt32 iLTap, iRTap;
                            SInt32 iLUTap = iUTap *(iLTap=mp->aa32ScaleTap[0][v]);
                            SInt32 iRUTap = iUTap *(iRTap=iDstBlkSize-iLTap);
                            SInt32 iLDTap = iDTap *iLTap;
                            SInt32 iRDTap = iDTap *iRTap;

                            pCbBlkDst[iDstOff] = (iLUTap*pCbBlkSrc[iOffLU] + iRUTap*pCbBlkSrc[iOffLU+1] +
                                                  iLDTap*pCbBlkSrc[iOffLD] + iRDTap*pCbBlkSrc[iOffLD+1]) >>iDstBlkSizePowerx2;
                            pCrBlkDst[iDstOff] = (iLUTap*pCrBlkSrc[iOffLU] + iRUTap*pCrBlkSrc[iOffLU+1] +
                                                  iLDTap*pCrBlkSrc[iOffLD] + iRDTap*pCrBlkSrc[iOffLD+1]) >>iDstBlkSizePowerx2;
                        }
                    }
                }
               #endif
            }
        }
    }
}
//-----------------------------------------------------------------------------
    
ScaleParam  *CScale_GetParam(CScale *mp)
{
    return &mp->Param;
}
//-----------------------------------------------------------------------------

ScaleReport *CScale_GetReport(CScale *mp)
{
    return &mp->Report;
}
//-----------------------------------------------------------------------------

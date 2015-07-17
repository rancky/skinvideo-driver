#if defined(__LINUX__)
    #include <snxcam.h>
#elif defined(__KERNEL_MODE__)
    #include <sncam.h>
#else
    #include <math.h>
    #include "JPEG.h"
#endif

//Eric Test
static __int64 mm0_tmp = 0;
//---------------------------------------------------------------------------

CJPEG* CJPEG_CJPEG()
{
    CJPEG *mp;

    #if defined(__KERNEL_MODE__)
    {
        if((mp = (CJPEG *)ExAllocatePoolWithTag(NonPagedPool, sizeof(CJPEG),'SNX')) == NULL)
			return mp;
        RtlZeroMemory(&mp->Param,  sizeof(mp->Param));
        RtlZeroMemory(&mp->Report, sizeof(mp->Report));
    }
    #else
    {
		if((mp = (CJPEG *)malloc(sizeof(CJPEG)))==NULL)
			return mp;
        memset(&mp->Param,  0, sizeof(mp->Param));
        memset(&mp->Report, 0, sizeof(mp->Report));
    }
    #endif

    mp->pHuffTable[0] = (HuffTable*) (((INT_PTR)mp->iYDCHuff +31) & (~31));
    mp->pHuffTable[1] = (HuffTable*) (((INT_PTR)mp->iCDCHuff +31) & (~31));
    mp->pHuffTable[2] = (HuffTable*) (((INT_PTR)mp->iYACHuff +31) & (~31));
    mp->pHuffTable[3] = (HuffTable*) (((INT_PTR)mp->iCACHuff +31) & (~31));

    mp->pYQTable = (SInt16*) (((INT_PTR)mp->iQTable +31) &(~31));
    mp->pCQTable = mp->pYQTable +128;

    mp->pDeHuffBuf   = (SInt16*)(((INT_PTR)mp->iDeHuffBuf   +31) &(~31));
    mp->pIDCTProcBuf = (SInt16*)(((INT_PTR)mp->iIDCTProcBuf +31) &(~31));

    #if defined(__KERNEL_MODE__)
    {
        RtlCopyMemory(mp->pHuffTable[0]->LookUp, YDCLookUp, sizeof(YDCLookUp));
        RtlCopyMemory(mp->pHuffTable[1]->LookUp, CDCLookUp, sizeof(CDCLookUp));
        RtlCopyMemory(mp->pHuffTable[2]->LookUp, YACLookUp, sizeof(YACLookUp));
        RtlCopyMemory(mp->pHuffTable[3]->LookUp, CACLookUp, sizeof(CACLookUp));

        RtlCopyMemory(mp->pHuffTable[0]->Tree, YDCTree, sizeof(YDCTree));
        RtlCopyMemory(mp->pHuffTable[1]->Tree, CDCTree, sizeof(CDCTree));
        RtlCopyMemory(mp->pHuffTable[2]->Tree, YACTree, sizeof(YACTree));
        RtlCopyMemory(mp->pHuffTable[3]->Tree, CACTree, sizeof(CACTree));
    }
    #else
    {
        memcpy(mp->pHuffTable[0]->LookUp, YDCLookUp, sizeof(YDCLookUp));
        memcpy(mp->pHuffTable[1]->LookUp, CDCLookUp, sizeof(CDCLookUp));
        memcpy(mp->pHuffTable[2]->LookUp, YACLookUp, sizeof(YACLookUp));
        memcpy(mp->pHuffTable[3]->LookUp, CACLookUp, sizeof(CACLookUp));

        memcpy(mp->pHuffTable[0]->Tree, YDCTree, sizeof(YDCTree));
        memcpy(mp->pHuffTable[1]->Tree, CDCTree, sizeof(CDCTree));
        memcpy(mp->pHuffTable[2]->Tree, YACTree, sizeof(YACTree));
        memcpy(mp->pHuffTable[3]->Tree, CACTree, sizeof(CACTree));
    }
    #endif

	FLOATING FloatSave;
	NTSTATUS Ntstatus;
	START_FLOATING(FloatSave,Ntstatus);
	CHECK_FLOATING_VAR(Ntstatus,mp);

    {
        for (int j = 0; j < 8; j++)
        {
            for (int i = 0; i < 8; i++)
            {
                double c;
                if (j==0)
                    c = sqrt(2.0) /4;
                else
                    c = cos(j*dPI/16) /2;

                if (i==0)
                    c *= sqrt(2.0) /4;
                else
                    c *= cos(i*dPI/16) /2;

                mp->dQConst[j*8+i] = c;
            }
        }
    }

	END_FLOATING(FloatSave,Ntstatus);

    return mp;
}
//---------------------------------------------------------------------------

void CJPEG__CJPEG(CJPEG *mp)
{
#if defined(__LINUX__)
    rvfree(mp, sizeof(CJDEC));
#elif defined(__KERNEL_MODE__)
	if(mp!=NULL)
		ExFreePoolWithTag(mp,'SNX');
#else
    free(mp);
#endif
}
//---------------------------------------------------------------------------

void CJPEG_ReadJPEGParamFromReg(CJPEG *mp, DWORD *pJPEGParams)
{
    if (pJPEGParams == NULL)
        return;
}
//---------------------------------------------------------------------------

void CJPEG_Reset(CJPEG *mp)
{
}
//---------------------------------------------------------------------------

void CJPEG_WriteJPEGParamToReg(CJPEG *mp, DWORD *pJPEGParams)
{
    if (pJPEGParams == NULL)
        return;
}
//---------------------------------------------------------------------------

BOOL CJPEG_SetJPEGSize(CJPEG *mp, SInt32 iStrmHSize, SInt32 iStrmVSize, SInt32 iOutBufHSize,
    SInt32 iOutBufVSize, SInt32 iDecHStart, SInt32 iDecVStart, SInt32 iDecHSize, SInt32 iDecVSize)
{
    if ((iDecHStart+iDecHSize>iOutBufHSize) || (iDecVStart+iDecVSize>iOutBufVSize))
        return FALSE;

    iDecHSize = iDecHSize + (iDecHStart%16);
    iDecVSize = iDecVSize + (iDecVStart%8);
    iDecHStart = (iDecHStart/16) *16;
    iDecVStart = (iDecVStart/8)  *8;
    iDecHSize = (iDecHSize/16) *16 + ((iDecHSize%16)? 16: 0);
    iDecVSize = (iDecVSize/8)  *8  + ((iDecVSize%8)? 8: 0);

    mp->Param.iStrmHSize = iStrmHSize;
    mp->Param.iStrmVSize = iStrmVSize;
    mp->Param.iOutBufHSize = iOutBufHSize;
    mp->Param.iOutBufVSize = iOutBufVSize;
    mp->Param.iDecHStart = iDecHStart;
    mp->Param.iDecVStart = iDecVStart;
    mp->Param.iDecHSize = iDecHSize;
    mp->Param.iDecVSize = iDecVSize;

    mp->Param.bMirror = FALSE;
    mp->Param.bFlip   = FALSE;

    return TRUE;
}
//---------------------------------------------------------------------------

void CJPEG_Set_Y_QTAB(CJPEG *mp, UInt8 *pY_QTAB)
{
    memcpy(mp->Param.Y_QTAB, pY_QTAB, sizeof(mp->Param.Y_QTAB));
    SInt16 iTmpYQTable[64];

	FLOATING FloatSave;
	NTSTATUS Ntstatus;
	START_FLOATING(FloatSave,Ntstatus);
	CHECK_FLOATING_VOID(Ntstatus);

    for (int i = 0; i < 64; i++)
        iTmpYQTable[ZAG[i]] = (SInt16)
        (mp->dQConst[ZAG[i]] * pY_QTAB[i] *Q_SCALER +0.5);

    for (int j = 0; j < 8; j++)
    {
        for (int i = 0; i < 8; i++)
        {
            mp->pYQTable[ZAG2IDCT[j*8 +i]] = iTmpYQTable[j*8 +i];
            if ((j ==3) || (j==4) || (j==6) || (j==7))
                mp->pYQTable[64+ZAG2IDCT[j*8 +i]] = -iTmpYQTable[j*8 +i];
            else
                mp->pYQTable[64+ZAG2IDCT[j*8 +i]] = iTmpYQTable[j*8 +i];
        }
    }

	END_FLOATING(FloatSave,Ntstatus);
}
//-----------------------------------------------------------------------------

void CJPEG_Set_C_QTAB(CJPEG *mp, UInt8 *pC_QTAB)
{
    memcpy(mp->Param.UV_QTAB, pC_QTAB, sizeof(mp->Param.UV_QTAB));
    SInt16 iTmpCQTable[64];

	FLOATING FloatSave;
	NTSTATUS Ntstatus;
	START_FLOATING(FloatSave,Ntstatus);
	CHECK_FLOATING_VOID(Ntstatus);

    for (int i = 0; i < 64; i++)
        iTmpCQTable[ZAG[i]] = (SInt16)
        (mp->dQConst[ZAG[i]] * pC_QTAB[i] *Q_SCALER +0.5);

    for (int j = 0; j < 8; j++)
    {
        for (int i = 0; i < 8; i++)
        {
            mp->pCQTable[ZAG2IDCT[j*8 +i]] = iTmpCQTable[j*8 +i];
            if ((j ==3) || (j==4) || (j==6) || (j==7))
                mp->pCQTable[64+ZAG2IDCT[j*8 +i]] = -iTmpCQTable[j*8 +i];
            else
                mp->pCQTable[64+ZAG2IDCT[j*8 +i]] = iTmpCQTable[j*8 +i];
        }
    }

	END_FLOATING(FloatSave,Ntstatus);
}
//-----------------------------------------------------------------------------

void CJPEG_GetHeader(CJPEG *mp, UInt8 *pJPEGHeader)
{
	CJPEG_Set_Y_QTAB(mp,pJPEGHeader+26);
	CJPEG_Set_C_QTAB(mp,pJPEGHeader+91);
}
//-----------------------------------------------------------------------------

JPEGParam  *CJPEG_GetParam(CJPEG *mp)
{
    return &(mp->Param);
}
//-----------------------------------------------------------------------------

JPEGReport *CJPEG_GetReport(CJPEG *mp)
{
    return &(mp->Report);
}
//-----------------------------------------------------------------------------

void CJPEG_JPEGDecode(CJPEG *mp, UInt8 *pStream, UInt8 *pOutBuf, UInt32 iJFIFLen)
{
	CJPEG_GetHeader(mp, pStream);
    mp->Report.iJFIFLen = iJFIFLen;

    StreamBuf *pStreamBuf = &mp->sStreamBuf;
    pStreamBuf->pStreamPtr = pStream +iJPEGHeaderLen;
    pStreamBuf->iBitsLeft = 48;
    pStreamBuf->iBitsUsed = 0;
    CJPEG_GetBits(pStreamBuf, 16);
    CJPEG_GetBits(pStreamBuf, 16);
    CJPEG_GetBits(pStreamBuf, 16);
    CJPEG_GetBits(pStreamBuf, 16);

    UInt32 iLastDC[3] = {0, 0, 0};
    HuffTable *pYDCHuffTable = mp->pHuffTable[0];
    HuffTable *pCDCHuffTable = mp->pHuffTable[1];
    HuffTable *pYACHuffTable = mp->pHuffTable[2];
    HuffTable *pCACHuffTable = mp->pHuffTable[3];

    SInt16 *pYQTable = mp->pYQTable;
    SInt16 *pCQTable = mp->pCQTable;

    mp->Report.bJPEGError = TRUE;
    mp->Report.iErrorCode = JPGD_DECODING;
    
    BOOL bMirror = mp->Param.bMirror;
    BOOL bFlip   = mp->Param.bFlip;

    SInt32 iMCU_H_Cnt = (mp->Param.iStrmHSize +15)/16;
    SInt32 iDstMCU_H_START, iDstMCU_H_STOP, iSrcMCU_H_START, iSrcMCU_H_STOP;
    if (!bMirror)
    {
        iDstMCU_H_START = mp->Param.iDecHStart /16;
        iDstMCU_H_STOP  = (mp->Param.iDecHStart +mp->Param.iDecHSize +15) /16 -1;
        iSrcMCU_H_START = iDstMCU_H_START;
        iSrcMCU_H_STOP  = iDstMCU_H_STOP; 
    }
    else
    {
        iDstMCU_H_START = (mp->Param.iDecHStart +mp->Param.iDecHSize +15) /16 -1;
        iDstMCU_H_STOP  = mp->Param.iDecHStart /16;
        iSrcMCU_H_START = iMCU_H_Cnt-1-iDstMCU_H_START;
        iSrcMCU_H_STOP  = iMCU_H_Cnt-1-iDstMCU_H_STOP; 
    }
    SInt32 iMCU_H_SIZE  = iSrcMCU_H_STOP -iSrcMCU_H_START +1;

    SInt32 iMCU_V_Cnt = (mp->Param.iStrmVSize +7)/8;
    SInt32 iDstMCU_V_START, iDstMCU_V_STOP, iSrcMCU_V_START, iSrcMCU_V_STOP;
    if (!bFlip)
    {
        iDstMCU_V_START = mp->Param.iDecVStart /8;
        iDstMCU_V_STOP  = (mp->Param.iDecVStart +mp->Param.iDecVSize +7) /8 -1;
        iSrcMCU_V_START = iDstMCU_V_START;
        iSrcMCU_V_STOP  = iDstMCU_V_STOP;
    }
    else
    {
        iDstMCU_V_START = (mp->Param.iDecVStart +mp->Param.iDecVSize +7) /8 -1;
        iDstMCU_V_STOP  = mp->Param.iDecVStart /8;
        iSrcMCU_V_START = iMCU_V_Cnt-1-iDstMCU_V_START;
        iSrcMCU_V_STOP  = iMCU_V_Cnt-1-iDstMCU_V_STOP;         
    }

    SInt32 iYOutBufHSize = mp->Param.iOutBufHSize;
    SInt32 iYOutBufVSize = mp->Param.iOutBufVSize;
    SInt32 iCOutBufHSize = iYOutBufHSize/2;
    SInt32 iCOutBufVSize = iYOutBufVSize/2;

    SInt16 *pIDCTSrc = mp->pDeHuffBuf +iSrcMCU_H_START*256;

    UInt8 *pIDCTYDst  = pOutBuf;
    UInt8 *pIDCTCbDst = pIDCTYDst  +iYOutBufHSize*iYOutBufVSize;
    UInt8 *pIDCTCrDst = pIDCTCbDst +iCOutBufHSize*iCOutBufVSize;
    SInt32 iIDCTYDstStep, iIDCTCDstStep, iIDCTYLineStep, iIDCTCLineStep;
    if (!bFlip)
    {
        iIDCTYDstStep = iYOutBufHSize*8;
        iIDCTCDstStep = iCOutBufHSize*4;
        iIDCTYLineStep = iYOutBufHSize;
        iIDCTCLineStep = iCOutBufHSize;
        pIDCTYDst  = pIDCTYDst  + iDstMCU_V_START*8*iYOutBufHSize +iDstMCU_H_START*16;
        pIDCTCbDst = pIDCTCbDst + iDstMCU_V_START*4*iCOutBufHSize +iDstMCU_H_START*8;
        pIDCTCrDst = pIDCTCrDst + iDstMCU_V_START*4*iCOutBufHSize +iDstMCU_H_START*8;
    }
    else
    {
        iIDCTYDstStep = -iYOutBufHSize*8;
        iIDCTCDstStep = -iCOutBufHSize*4;
        iIDCTYLineStep = -iYOutBufHSize;
        iIDCTCLineStep = -iCOutBufHSize;
        pIDCTYDst  = pIDCTYDst  + ((iDstMCU_V_START+1)*8-1)*iYOutBufHSize +iDstMCU_H_START*16;
        pIDCTCbDst = pIDCTCbDst + ((iDstMCU_V_START+1)*4-1)*iCOutBufHSize +iDstMCU_H_START*8;
        pIDCTCrDst = pIDCTCrDst + ((iDstMCU_V_START+1)*4-1)*iCOutBufHSize +iDstMCU_H_START*8;
    }

    SInt32 k, r, s;
    for (SInt32 iMCU_V = 0; ; iMCU_V++)
    {
        if (iMCU_V == iMCU_V_Cnt)
        {
            // Find end of image (EOI) marker, so we can return to the user the
            // exact size of the input stream.
            CJPEG_GetBits(pStreamBuf,  pStreamBuf->iBitsLeft %8);
            int c = CJPEG_GetBits(pStreamBuf, 16);

            if (c != 0xffd9)
                mp->Report.iErrorCode = JPGD_STREAM_READ;
            else
                mp->Report.bJPEGError = FALSE;
            break;
        }
        else if ((iMCU_V>=iSrcMCU_V_START) && (iMCU_V<=iSrcMCU_V_STOP))
        {
            #if defined(__KERNEL_MODE__)
                RtlZeroMemory(pIDCTSrc,  iMCU_H_SIZE <<9);
            #else
                memset(pIDCTSrc, 0, iMCU_H_SIZE <<9);
            #endif
            SInt16 *pDeHuffPtr = mp->pDeHuffBuf;

            //Decode VLC to FLC
            for (int iMCU_H = 0; iMCU_H < iMCU_H_Cnt; iMCU_H++, pDeHuffPtr += 256)
            {
                //=====Y0=====
                if ((s = CJPEG_HuffDecode(pStreamBuf, pYDCHuffTable)) != 0)
                {
                    r = CJPEG_GetBits(pStreamBuf, s);
                    s = CJPEG_HuffExtend(r, s);
                }
                iLastDC[0] = (s += iLastDC[0]);
                pDeHuffPtr[0] = (short)s;

                for (k = 1; k < 64; k++)
                {
                    s = CJPEG_HuffDecode(pStreamBuf, pYACHuffTable);
                    r = s >> 4;
                    s &= 15;

                    if (s)
                    {
                        if (r)
                        {
                            if ((k + r) > 63)
                            {
                                mp->Report.iErrorCode = JPGD_DECODE_ERROR;
                                return;
                            }
                            k += r;
                        }
                        r = CJPEG_GetBits(pStreamBuf, s);
                        s = CJPEG_HuffExtend(r, s);
                        pDeHuffPtr[DeHuffOrder[k]] = (short)s;
                    }
                    else
                    {
                        if (r == 15)
                        {
                            if (k > 48)
                            {
                                mp->Report.iErrorCode = JPGD_DECODE_ERROR;
                                return;
                            }
                            k += 15;
                        }
                        else    //EOB
                            break;
                    }
                }

                //=====Y1=====
                if ((s = CJPEG_HuffDecode(pStreamBuf, pYDCHuffTable)) != 0)
                {
                    r = CJPEG_GetBits(pStreamBuf, s);
                    s = CJPEG_HuffExtend(r, s);
                }
                iLastDC[0] = (s += iLastDC[0]);
                pDeHuffPtr[64] = (short)s;

                for (k = 1; k < 64; k++)
                {
                    s = CJPEG_HuffDecode(pStreamBuf, pYACHuffTable);
                    r = s >> 4;
                    s &= 15;

                    if (s)
                    {
                        if (r)
                        {
                            if ((k + r) > 63)
                            {
                                mp->Report.iErrorCode = JPGD_DECODE_ERROR;
                                return;
                            }
                            k += r;
                        }
                        r = CJPEG_GetBits(pStreamBuf, s);
                        s = CJPEG_HuffExtend(r, s);
                        pDeHuffPtr[DeHuffOrder[k]+64] = (short)s;
                    }
                    else
                    {
                        if (r == 15)
                        {
                            if (k > 48)
                            {
                                mp->Report.iErrorCode = JPGD_DECODE_ERROR;
                                return;
                            }
                            k += 15;
                        }
                        else    //EOB
                            break;
                    }
                }

                //=====Cb=====
                if ((s = CJPEG_HuffDecode(pStreamBuf, pCDCHuffTable)) != 0)
                {
                    r = CJPEG_GetBits(pStreamBuf, s);
                    s = CJPEG_HuffExtend(r, s);
                }
                iLastDC[1] = (s += iLastDC[1]);
                pDeHuffPtr[128] = (short)s;

                for (k = 1; k < 64; k++)
                {
                    s = CJPEG_HuffDecode(pStreamBuf, pCACHuffTable);
                    r = s >> 4;
                    s &= 15;

                    if (s)
                    {
                        if (r)
                        {
                            if ((k + r) > 63)
                            {
                                mp->Report.iErrorCode = JPGD_DECODE_ERROR;
                                return;
                            }
                            k += r;
                        }
                        r = CJPEG_GetBits(pStreamBuf, s);
                        s = CJPEG_HuffExtend(r, s);
                        pDeHuffPtr[DeHuffOrder[k]+128] = (short)s;
                    }
                    else
                    {
                        if (r == 15)
                        {
                            if (k > 48)
                            {
                                mp->Report.iErrorCode = JPGD_DECODE_ERROR;
                                return;
                            }
                            k += 15;
                        }
                        else    //EOB
                            break;
                    }
                }

                //=====Cr=====
                if ((s = CJPEG_HuffDecode(pStreamBuf, pCDCHuffTable)) != 0)
                {
                    r = CJPEG_GetBits(pStreamBuf, s);
                    s = CJPEG_HuffExtend(r, s);
                }
                iLastDC[2] = (s += iLastDC[2]);
                pDeHuffPtr[192] = (short)s;

                for (k = 1; k < 64; k++)
                {
                    s = CJPEG_HuffDecode(pStreamBuf, pCACHuffTable);
                    r = s >> 4;
                    s &= 15;

                    if (s)
                    {
                        if (r)
                        {
                            if ((k + r) > 63)
                            {
                                mp->Report.iErrorCode = JPGD_DECODE_ERROR;
                                return;
                            }
                            k += r;
                        }
                        r = CJPEG_GetBits(pStreamBuf, s);
                        s = CJPEG_HuffExtend(r, s);
                        pDeHuffPtr[DeHuffOrder[k]+192] = (short)s;
                    }
                    else
                    {
                        if (r == 15)
                        {
                            if (k > 48)
                            {
                                mp->Report.iErrorCode = JPGD_DECODE_ERROR;
                                return;
                            }
                            k += 15;
                        }
                        else    //EOB
                            break;
                    }
                }
            }

            //DeQuantization + 2D IDCT & move whole (block) row into processbuf
            {
                SInt16 *pSrc = pIDCTSrc;
                UInt8  *pYDst  = pIDCTYDst;
                UInt8  *pCbDst = pIDCTCbDst;
                UInt8  *pCrDst = pIDCTCrDst;
                if (!bMirror)
                {
                    for (int iMCU_H = 0; iMCU_H < iMCU_H_SIZE; iMCU_H++, 
                        pSrc+=256, pYDst+=16, pCbDst+=8, pCrDst+=8)
                    {
                        IDCTY(pSrc,     pYQTable, pYDst,   mp->pIDCTProcBuf, iIDCTYLineStep, bMirror);
                        IDCTY(pSrc+64,  pYQTable, pYDst+8, mp->pIDCTProcBuf, iIDCTYLineStep, bMirror);
                        IDCTC(pSrc+128, pCQTable, pCbDst,  mp->pIDCTProcBuf, iIDCTCLineStep, bMirror);
                        IDCTC(pSrc+192, pCQTable, pCrDst,  mp->pIDCTProcBuf, iIDCTCLineStep, bMirror);
                    }
                }
                else
                {
                    for (int iMCU_H = 0; iMCU_H < iMCU_H_SIZE; iMCU_H++,
                        pSrc+=256, pYDst-=16, pCbDst-=8, pCrDst-=8)
                    {
                        IDCTY(pSrc+64,  pYQTable, pYDst,   mp->pIDCTProcBuf, iIDCTYLineStep, bMirror);
                        IDCTY(pSrc,     pYQTable, pYDst+8, mp->pIDCTProcBuf, iIDCTYLineStep, bMirror);
                        IDCTC(pSrc+128, pCQTable, pCbDst,  mp->pIDCTProcBuf, iIDCTCLineStep, bMirror);
                        IDCTC(pSrc+192, pCQTable, pCrDst,  mp->pIDCTProcBuf, iIDCTCLineStep, bMirror);
                    }
                }
            }
            pIDCTYDst+=iIDCTYDstStep;
            pIDCTCbDst+=iIDCTCDstStep;
            pIDCTCrDst+=iIDCTCDstStep;
        }
        else
        {
            #if defined(__KERNEL_MODE__)
                RtlZeroMemory(pIDCTSrc,  iMCU_H_SIZE <<9);
            #else
                memset(pIDCTSrc, 0, iMCU_H_SIZE <<9);
            #endif
            SInt16 *pDeHuffPtr = mp->pDeHuffBuf;

            //Decode VLC to FLC
            for (int iMCU_H = 0; iMCU_H < iMCU_H_Cnt; iMCU_H++, pDeHuffPtr += 256)
            {
                //=====Y0=====
                if ((s = CJPEG_HuffDecode(pStreamBuf, pYDCHuffTable)) != 0)
                {
                    r = CJPEG_GetBits(pStreamBuf, s);
                    s = CJPEG_HuffExtend(r, s);
                }
                iLastDC[0] = (s += iLastDC[0]);
                pDeHuffPtr[0] = (short)s;

                for (k = 1; k < 64; k++)
                {
                    s = CJPEG_HuffDecode(pStreamBuf, pYACHuffTable);
                    r = s >> 4;
                    s &= 15;

                    if (s)
                    {
                        if (r)
                        {
                            if ((k + r) > 63)
                            {
                                mp->Report.iErrorCode = JPGD_DECODE_ERROR;
                                return;
                            }
                            k += r;
                        }
                        r = CJPEG_GetBits(pStreamBuf, s);
                    }
                    else
                    {
                        if (r == 15)
                        {
                            if (k > 48)
                            {
                                mp->Report.iErrorCode = JPGD_DECODE_ERROR;
                                return;
                            }
                            k += 15;
                        }
                        else    //EOB
                            break;
                    }
                }

                //=====Y1=====
                if ((s = CJPEG_HuffDecode(pStreamBuf, pYDCHuffTable)) != 0)
                {
                    r = CJPEG_GetBits(pStreamBuf, s);
                    s = CJPEG_HuffExtend(r, s);
                }
                iLastDC[0] = (s += iLastDC[0]);
                pDeHuffPtr[64] = (short)s;

                for (k = 1; k < 64; k++)
                {
                    s = CJPEG_HuffDecode(pStreamBuf, pYACHuffTable);
                    r = s >> 4;
                    s &= 15;

                    if (s)
                    {
                        if (r)
                        {
                            if ((k + r) > 63)
                            {
                                mp->Report.iErrorCode = JPGD_DECODE_ERROR;
                                return;
                            }
                            k += r;
                        }
                        r = CJPEG_GetBits(pStreamBuf, s);
                    }
                    else
                    {
                        if (r == 15)
                        {
                            if (k > 48)
                            {
                                mp->Report.iErrorCode = JPGD_DECODE_ERROR;
                                return;
                            }
                            k += 15;
                        }
                        else    //EOB
                            break;
                    }
                }

                //=====Cb=====
                if ((s = CJPEG_HuffDecode(pStreamBuf, pCDCHuffTable)) != 0)
                {
                    r = CJPEG_GetBits(pStreamBuf, s);
                    s = CJPEG_HuffExtend(r, s);
                }
                iLastDC[1] = (s += iLastDC[1]);
                pDeHuffPtr[128] = (short)s;

                for (k = 1; k < 64; k++)
                {
                    s = CJPEG_HuffDecode(pStreamBuf, pCACHuffTable);
                    r = s >> 4;
                    s &= 15;

                    if (s)
                    {
                        if (r)
                        {
                            if ((k + r) > 63)
                            {
                                mp->Report.iErrorCode = JPGD_DECODE_ERROR;
                                return;
                            }
                            k += r;
                        }
                        r = CJPEG_GetBits(pStreamBuf, s);
                    }
                    else
                    {
                        if (r == 15)
                        {
                            if (k > 48)
                            {
                                mp->Report.iErrorCode = JPGD_DECODE_ERROR;
                                return;
                            }
                            k += 15;
                        }
                        else    //EOB
                            break;
                    }
                }

                //=====Cr=====
                if ((s = CJPEG_HuffDecode(pStreamBuf, pCDCHuffTable)) != 0)
                {
                    r = CJPEG_GetBits(pStreamBuf, s);
                    s = CJPEG_HuffExtend(r, s);
                }
                iLastDC[2] = (s += iLastDC[2]);
                pDeHuffPtr[192] = (short)s;

                for (k = 1; k < 64; k++)
                {
                    s = CJPEG_HuffDecode(pStreamBuf, pCACHuffTable);
                    r = s >> 4;
                    s &= 15;

                    if (s)
                    {
                        if (r)
                        {
                            if ((k + r) > 63)
                            {
                                mp->Report.iErrorCode = JPGD_DECODE_ERROR;
                                return;
                            }
                            k += r;
                        }
                        r = CJPEG_GetBits(pStreamBuf, s);
                    }
                    else
                    {
                        if (r == 15)
                        {
                            if (k > 48)
                            {
                                mp->Report.iErrorCode = JPGD_DECODE_ERROR;
                                return;
                            }
                            k += 15;
                        }
                        else    //EOB
                            break;
                    }
                }
            }
        }
    }
	return;
}
//-----------------------------------------------------------------------------

#if defined(__MMX__)
inline UInt32 CJPEG_GetBits(StreamBuf *pStreamBuf, SInt32 iBitsNum)
#else
UInt32 CJPEG_GetBits(StreamBuf *pStreamBuf, SInt32 iBitsNum)
#endif
{
    #if defined(__MMX__)
    __asm
    {
        mov         esi,                pStreamBuf;
        mov         ecx,                iBitsNum;
        //-------------------------------------------------
        movq        mm0,                [esi].iBitBuf;
        mov         edx,                [esi].iBitsLeft;
        movq        mm1,                mm0;
        movd        mm7,                ecx;
        psrlq       mm1,                48;
        cmp         edx,                ecx;    //(iBitsLeft>iBitsNum)?
        psllq       mm1,                mm7;
        jg          gb2_noload;
        //-------------------------------------------------
    gb2_load:
        movd        mm6,                edx;    //mm6=iBitsLeft
        psrlq       mm1,                16;
        psllq       mm0,                mm6;
        xor         eax,                eax;
        mov         edi,                [esi].pStreamPtr;
        mov         ax,                 [edi];
        mov         ebx,                [edi+2];
        bswap       eax;
        bswap       ebx;
        movd        mm2,                eax;
        movd        mm3,                ebx;
        psllq       mm2,                16;
        pcmpeqb     mm4,                mm4;
        por         mm2,                mm3;
        psubusw     mm7,                mm6;    //mm7 = iBitsNum-iBitsLeft
        pcmpeqb     mm4,                mm2;
        packssdw    mm4,                mm4;
        movd        eax,                mm4;
        neg         ecx;                        //ecx = -iBitsNum
        cmp         eax,                0;
        jz          gb2_fast_load;
        //-------------------------------------------------
        push        ecx;                        //0xff Marker exist, check it!
        xor         ebx,                ebx;
        mov         ecx,                2;
    gb2_slow_load_1:                            //to laoad 2 bytes
        shl         ebx,                8;
        mov         ax,                 [edi];
        cmp         ax,                 0x00ff;
        jz          gb2_slow_load_0xff00_1;
        mov         bl,                 al;
        lea         edi,                [edi+1];
        jmp         gb2_slow_load_jmp_1;
    gb2_slow_load_0xff00_1:
        mov         bl,                 0xff;
        lea         edi,                [edi+2];//0xff 0x00 in bitstream=> return 0xff, skip 0x00
    gb2_slow_load_jmp_1:
        loop        gb2_slow_load_1;
        movd        mm2,                ebx;
        //-------------------------------------------------
        xor         ebx,                ebx;
        mov         ecx,                4;
    gb2_slow_load_2:                            //to load 4 bytes
        shl         ebx,                8;
        mov         ax,                 [edi];
        cmp         ax,                 0x00ff;
        jz          gb2_slow_load_0xff00_2;
        mov         bl,                 al;
        lea         edi,                [edi+1];
        jmp         gb2_slow_load_jmp_2;
    gb2_slow_load_0xff00_2:
        mov         bl,                 0xff;
        lea         edi,                [edi+2];//0xff 0x00 in bitstream=> return 0xff, skip 0x00
    gb2_slow_load_jmp_2:
        loop        gb2_slow_load_2;
        psllq       mm2,                32;
        movd        mm3,                ebx;
        //-------------------------------------------------
        por         mm0,                mm2;
        pop         ecx;
        por         mm0,                mm3;
        lea         edx,                [edx+ecx+48];
        psllq       mm0,                mm7;
        mov         [esi].iBitsLeft,    edx;
        mov         [esi].pStreamPtr,   edi;
        jmp         gb2_Return;
        //-------------------------------------------------
    gb2_fast_load:
        por         mm0,                mm2;
        lea         edx,                [edx+ecx+48];   //edx = iBitsLeft+48-iBitsNum
        psllq       mm0,                mm7;            //ecx = iBitsNum
        lea         edi,                [edi+6];
        mov         [esi].iBitsLeft,    edx;
        mov         [esi].pStreamPtr,   edi;
        jmp         gb2_Return;
        //-------------------------------------------------
    gb2_noload:
        psllq       mm0,                mm7;
        sub         edx,                ecx;
        psrlq       mm1,                16;
        mov         [esi].iBitsLeft,    edx;
        //-------------------------------------------------
    gb2_return:
        movq        [esi].iBitBuf,      mm0;
        movd        eax,                mm1;
        emms;
    }
    #else
    {
		int i;
		__int64 mm[4];
		char str[16], str1[16];
		unsigned char *edi;		
		unsigned long eax = 0, ebx = 0, ecx = 0, ecx_tmp = 0;

		//Initialize
		ecx = iBitsNum;
		for(i = 0; i < 4; i++)
			mm[i] = 0;		
		if(mm0_tmp == 0)
			mm[0] = (pStreamBuf->iBitBuf << 32) | pStreamBuf->iBitBuf;
		else
			mm[0] = (mm0_tmp & 0xffffffff00000000) | pStreamBuf->iBitBuf;
		mm[1] = ((mm[0] >> 48) & 0x000000000000ffff) << iBitsNum;
		if(pStreamBuf->iBitsLeft > iBitsNum)
			goto gb2_noload;

gb2_load:
		mm[1] >>= 16;
		mm[0] <<= pStreamBuf->iBitsLeft;
		eax = 0;
		ecx = (~ecx) + 1;
		iBitsNum -= pStreamBuf->iBitsLeft;
		edi = pStreamBuf->pStreamPtr;
		eax = *((unsigned long*)edi) & 0x0000ffff;
		ebx = *((unsigned long*)(edi + 2));
		eax = ((eax & 0xff000000) >> 24) | ((eax & 0x00ff0000) >> 8) | ((eax & 0x0000ff00) << 8) | ((eax & 0x000000ff) << 24);
		ebx = ((ebx & 0xff000000) >> 24) | ((ebx & 0x00ff0000) >> 8) | ((ebx & 0x0000ff00) << 8) | ((ebx & 0x000000ff) << 24);
		mm[2] = eax;
		mm[3] = ebx;
		mm[2] = (mm[2] << 16) | mm[3];
		sprintf(str, "%x", (mm[2] >> 32));
		sprintf(str1, "%x", mm[2]);
		if(NULL == strstr(str, "ff") && NULL == strstr(str1, "ff"))
			goto gb2_fast_load;
		ecx_tmp = ecx;
		ebx = 0;
		ecx = 2;

gb2_slow_load_1:
		ebx <<= 8;
		eax = *((unsigned long*)edi) & 0x0000ffff;
		if((eax & 0x0000ffff) == 0x000000ff)
			goto gb2_slow_load_0xff00_1;
		ebx = (ebx & 0xffffff00) | (eax & 0x000000ff);
		edi++;
		goto gb2_slow_load_jmp_1;

gb2_slow_load_0xff00_1:
		ebx = (ebx & 0xffffff00) | 0x000000ff;
		edi += 2;

gb2_slow_load_jmp_1:
		if(--ecx != 0)
			goto gb2_slow_load_1;
		mm[2] = ebx;
		ebx = 0;
		ecx = 4;

gb2_slow_load_2:
		ebx <<= 8;
		eax = *((unsigned long*)edi) & 0x0000ffff;
		if((eax & 0x0000ffff) == 0x00ff)
			goto gb2_slow_load_0xff00_2;
		ebx = (ebx & 0xffffff00) | (eax & 0x000000ff);
		edi++;
		goto gb2_slow_load_jmp_2;

gb2_slow_load_0xff00_2:
		ebx = (ebx & 0xffffff00) | 0x000000ff;
		edi += 2;

gb2_slow_load_jmp_2:
		if(--ecx != 0)
			goto gb2_slow_load_2;
		mm[2] <<= 32;
		mm[3] = ebx;
		mm[0] = (mm[0] | mm[2] | mm[3]) << iBitsNum;
		ecx = ecx_tmp;
		pStreamBuf->iBitsLeft += ecx + 48;
		pStreamBuf->pStreamPtr = edi;
		goto gb2_return;

gb2_fast_load:
		mm[0] = (mm[0] | mm[2]) << iBitsNum;
		pStreamBuf->iBitsLeft += ecx + 48;
		pStreamBuf->pStreamPtr = edi + 6;
		goto gb2_return;

gb2_noload:
		mm[0] <<= iBitsNum;
		mm[1] >>= 16;
		pStreamBuf->iBitsLeft -= ecx;

gb2_return:
		pStreamBuf->iBitBuf = mm[0];
		pStreamBuf->iBitBuf64[0] = mm[0];
		pStreamBuf->iBitBuf64[1] = mm[0] >> 32;
		eax = mm[1];
		mm0_tmp = mm[0];
		return eax;
    }
    #endif
}
//------------------------------------------------------------------------------

inline SInt32 CJPEG_HuffDecode(StreamBuf *pStreamBuf, HuffTable *pHuffTable)
{
    int iSymbol;
    unsigned int d = (pStreamBuf->iBitBuf64[1]>>16);
    if ((iSymbol = pHuffTable->LookUp[d>>8]) < 0)
    {
        unsigned int ofs = 7;
        do
        {
            iSymbol = pHuffTable->Tree[~iSymbol + ((d >> ofs) & 1)];
            ofs--;
        }
        while (iSymbol < 0);
    }

    CJPEG_GetBits(pStreamBuf, iSymbol>>8);
    iSymbol&=0xff;

    return iSymbol;
}
//------------------------------------------------------------------------------

inline SInt32 CJPEG_HuffExtend(SInt32 iCode, SInt32 iBitNum)
{
    int x = 1<<(iBitNum-1);
    if (iCode < x)
        iCode = -((~iCode) & ((x<<1)-1));
    return iCode;
}
//------------------------------------------------------------------------------

void IDCTY(SInt16 *pIn, SInt16 *pCQ, UInt8 *pOut, SInt16 *pProcBuf,
    SInt32 iLineStep, BOOL bMirror) // pIn/pCQ: column order, pOut: row order
{
    #if defined(__MMX__)
    {
        UInt64 iTmp1, iTmp2;
        __asm
        {
            mov         esi,            pIn;
            mov         edi,            pProcBuf;
            mov         edx,            pCQ;
            //=================================================================
            mov         ecx,            16;
        IDCTY1_Start:
            movq        mm0,            [esi+ecx];
            por         mm0,            [esi+ecx+8];
            psrld       mm0,            16;
            por         mm0,            [esi+32+ecx];
            por         mm0,            [esi+32+ecx+8];
            por         mm0,            [esi+64+ecx];
            por         mm0,            [esi+64+ecx+8];
            por         mm0,            [esi+96+ecx];
            por         mm0,            [esi+96+ecx+8];
            packssdw    mm0,            mm0;
            movd        eax,            mm0;
            cmp         eax,            0;
            jne         IDCTY1_Slow;
            //-----------------------------------------------------------------
        IDCTY1_Fast:
            pcmpeqd     mm7,            mm7;
            movq        mm0,            [esi+ecx];
            movq        mm1,            [esi+ecx+8];
            psrld       mm7,            16;
            movq        mm2,            [edx+ecx];
            movq        mm3,            [edx+ecx+8];
            pand        mm2,            mm7;
            pand        mm3,            mm7;
            pmaddwd     mm0,            mm2;
            pmaddwd     mm1,            mm3;
            psrad       mm0,            6;
            psrad       mm1,            6;
            packssdw    mm0,            mm1;
            movq        mm2,            mm0;
            movq        mm4,            mm0;
            punpcklwd   mm0,            mm4;
            punpckhwd   mm2,            mm4;
            movq        mm1,            mm0;
            movq        mm4,            mm0;
            movq        mm3,            mm2;
            movq        mm5,            mm2;
            punpckldq   mm0,            mm4;
            punpckhdq   mm1,            mm4;
            punpckldq   mm2,            mm5;
            punpckhdq   mm3,            mm5;
            movq        [edi+ecx*4],    mm0;
            movq        [edi+ecx*4+8],  mm0;
            movq        [edi+ecx*4+16], mm1;
            movq        [edi+ecx*4+24], mm1;
            movq        [edi+ecx*4+32], mm2;
            movq        [edi+ecx*4+40], mm2;
            movq        [edi+ecx*4+48], mm3;
            movq        [edi+ecx*4+56], mm3;
            jmp         IDCTY1_End;
        IDCTY1_Slow:
            movq        mm0,            [esi+ecx+32];
            movq        mm2,            [esi+ecx+32+8];
            movq        mm1,            mm0;                //mm0 = mm1
            movq        mm3,            mm2;                //mm2 = mm3
            pmaddwd     mm0,            [edx+ecx+32];       //mm0 = F11+F71 F10+F70
            pmaddwd     mm1,            [edx+ecx+128+32];   //mm1 = F11-F71 F10-F70
            pmaddwd     mm2,            [edx+ecx+32+8];     //mm2 = F13+F73 F12+F72
            pmaddwd     mm3,            [edx+ecx+128+32+8]; //mm3 = F13-F73 F12-F72
            psrad       mm0,            2;
            psrad       mm1,            2;
            psrad       mm2,            2;
            psrad       mm3,            2;
            packssdw    mm0,            mm2;                //mm0 = tmp1
            packssdw    mm1,            mm3;                //mm1 = a6
            //-----------------------------------------------------------------
            movq        mm2,            [esi+ecx+96];
            movq        mm4,            [esi+ecx+96+8];
            movq        mm3,            mm2;                //mm2 = mm3
            movq        mm5,            mm4;                //mm4 = mm5
            pmaddwd     mm2,            [edx+ecx+96];       //mm2 =  F31+F51  F30+F50
            pmaddwd     mm3,            [edx+ecx+128+96];   //mm3 = -F31+F51 -F30+F50
            pmaddwd     mm4,            [edx+ecx+96+8];     //mm4 =  F33+F53  F32+F52
            pmaddwd     mm5,            [edx+ecx+128+96+8]; //mm5 = -F33+F53 -F32+F52
            psrad       mm2,            2;
            psrad       mm3,            2;
            psrad       mm4,            2;
            psrad       mm5,            2;
            packssdw    mm2,            mm4;                //mm2 = tmp2
            packssdw    mm3,            mm5;                //mm3 = a4
            //-----------------------------------------------------------------
            movq        mm5,            mm0;
            psubsw      mm0,            mm2;
            paddsw      mm5,            mm2;
            pmulhw      mm0,            PC4_12;             //mm0 = b5
            psraw       mm5,            4;                  //mm5 = n7
            //-----------------------------------------------------------------
            movq        mm2,            mm3;
            punpckhwd   mm3,            mm1;
            punpcklwd   mm2,            mm1;
            movq        mm4,            mm3;                // mm3 = mm4 = a6 a4 a6 a4
            movq        mm1,            mm2;                // mm1 = mm2 = a6 a4 a6 a4
            pmaddwd     mm1,            PC2_C6n_12;
            pmaddwd     mm2,            PC6n_C2n_12;
            pmaddwd     mm3,            PC2_C6n_12;
            pmaddwd     mm4,            PC6n_C2n_12;
            psrad       mm1,            16;
            psrad       mm2,            16;
            psrad       mm3,            16;
            psrad       mm4,            16;
            packssdw    mm1,            mm3;                //mm1 = b6
            packssdw    mm2,            mm4;                //mm2 = n5
            //-----------------------------------------------------------------
            psubsw      mm1,            mm5;
            movq        mm3,            mm1;                //mm3 = n4
            psubsw      mm1,            mm0;                //mm1 = n0
            psubsw      mm2,            mm1;                //mm2 = m7
            //-----------------------------------------------------------------
            movq        iTmp1,          mm5;                //n7
            movq        iTmp2,          mm1;                //n0
            //mm0=n7  mm1=n0, mm2=m7, mm3=n4
            //=================================================================
            movq        mm4,            [esi+ecx];
            movq        mm6,            [esi+ecx+8];
            movq        mm5,            mm4;                //mm4 = mm5
            movq        mm7,            mm6;                //mm6 = mm7
            pmaddwd     mm5,            [edx+ecx];          //mm5 = F01+F41 F00+F40
            pmaddwd     mm4,            [edx+ecx+128];      //mm4 = F01-F41 F00-F40
            pmaddwd     mm7,            [edx+ecx+8];        //mm7 = F03+F43 F02+F42
            pmaddwd     mm6,            [edx+ecx+128+8];    //mm6 = F03-F43 F02-F42
            psrad       mm4,            6;
            psrad       mm5,            6;
            psrad       mm6,            6;
            psrad       mm7,            6;
            packssdw    mm4,            mm6;                //mm4 = n1
            packssdw    mm5,            mm7;                //mm5 = n3
            //-----------------------------------------------------------------
            movq        mm6,            [esi+ecx+64];
            movq        mm0,            [esi+ecx+64+8];
            movq        mm7,            mm6;                //mm6 = mm7
            movq        mm1,            mm0;                //mm0 = mm3
            pmaddwd     mm7,            [edx+ecx+64];       //mm7 = F21+F61 F20+F60
            pmaddwd     mm6,            [edx+ecx+64+128];   //mm6 = F21-F61 F20-F60
            pmaddwd     mm1,            [edx+ecx+64+8];     //mm1 = F23+F63 F22+F62
            pmaddwd     mm0,            [edx+ecx+64+128+8]; //mm0 = F23-F63 F22-F62
            psrad       mm7,            6;
            psrad       mm6,            2;
            psrad       mm1,            6;
            psrad       mm0,            2;
            packssdw    mm6,            mm0;
            packssdw    mm7,            mm1;                //mm7 = n6
            pmulhw      mm6,            PC4_12;             //mm6 = b2
            psubsw      mm6,            mm7;                //mm6 = n2
            //-----------------------------------------------------------------
            movq        mm1,            iTmp2;              //mm1 = n0
            movq        mm0,            mm4;
            paddsw      mm0,            mm6;                //mm0 = m3
            psubsw      mm4,            mm6;                //mm4 = m5
            movq        mm6,            mm0;
            paddsw      mm0,            mm3;                //mm0 = X1
            psubsw      mm6,            mm3;                //mm6 = X6
            movq        mm3,            mm4;
            psubsw      mm4,            mm1;                //mm4 = X2
            paddsw      mm3,            mm1;                //mm3 = X5
            //-----------------------------------------------------------------
            movq        mm1,            mm5;
            paddsw      mm1,            mm7;                //mm1 = m4
            psubsw      mm5,            mm7;                //mm5 = m6
            movq        mm7,            mm5;
            psubsw      mm5,            mm2;                //mm5 = X3
            paddsw      mm7,            mm2;                //mm7 = X4
            movq        mm2,            mm1;
            paddsw      mm1,            iTmp1;              //mm1 = X0
            psubsw      mm2,            iTmp1;              //mm2 = X7
            //=================================================================
        IDCTY1_Store:
            movq        iTmp2,          mm0;
            movq        mm0,            mm7;
            punpcklwd   mm7,            mm3;                //mm7 = X51 X41 X50 X40
            punpckhwd   mm0,            mm3;                //mm0 = X53 X43 X52 X42
            movq        mm3,            mm6;
            punpcklwd   mm6,            mm2;                //mm6 = X71 X61 X70 X60
            punpckhwd   mm3,            mm2;                //mm3 = X73 X63 X72 X62
            //-----------------------------------------------------------------
            movq        mm2,            mm7;
            punpckldq   mm7,            mm6;                //mm7 = F70 F60 F50 F40
            punpckhdq   mm2,            mm6;                //mm2 = F71 F61 F51 F41
            movq        mm6,            mm0;
            punpckldq   mm0,            mm3;                //mm0 = F72 F62 F52 F42
            punpckhdq   mm6,            mm3;                //mm6 = F73 F63 F53 F43
            //-----------------------------------------------------------------
            movq        mm3,            iTmp2;
            movq        [edi+ecx*4+8],  mm7;
            movq        [edi+ecx*4+24], mm2;
            movq        [edi+ecx*4+40], mm0;
            movq        [edi+ecx*4+56], mm6;
            //-----------------------------------------------------------------
            movq        mm2,            mm1;
            punpcklwd   mm1,            mm3;                //mm1 = X11 X01 X10 X00
            punpckhwd   mm2,            mm3;                //mm2 = X13 X03 X12 X02
            movq        mm3,            mm4;
            punpcklwd   mm4,            mm5;                //mm4 = X31 X21 X30 X20
            punpckhwd   mm3,            mm5;                //mm3 = X33 X23 X32 X22
            //-----------------------------------------------------------------
            movq        mm5,            mm1;
            punpckldq   mm1,            mm4;                //mm1 = F30 F20 F10 F00
            punpckhdq   mm5,            mm4;                //mm5 = F31 F21 F11 F01
            movq        mm4,            mm2;
            punpckldq   mm2,            mm3;                //mm2 = F32 F22 F12 F02
            punpckhdq   mm4,            mm3;                //mm4 = F33 F23 F13 F03
            //-----------------------------------------------------------------
            movq        [edi+ecx*4],    mm1;
            movq        [edi+ecx*4+16], mm5;
            movq        [edi+ecx*4+32], mm2;
            movq        [edi+ecx*4+48], mm4;
            //-----------------------------------------------------------------
        IDCTY1_End:
            sub         ecx,            16;
            jge         IDCTY1_Start;
            //emms;
        }
    }
    #else
    {
        SInt32 iDCTCol;
        SInt32 F1, F2, F3, F5, F6, F7;
        SInt32 tmp1, tmp2;
        SInt32 a4, a6;
        SInt32 b0, b1, b2, b5, b6;
        SInt32 n0, n1, n2, n3, n4, n5, n6, n7;
        SInt32 m3, m4, m5, m6, m7;

        SInt16 *pF   = pIn;
        SInt16 *pCQt = pCQ;
        SInt16 *pX   = pProcBuf;
        for (iDCTCol=0; iDCTCol<8; iDCTCol++)
        {
            if ((pF[8] | pF[16] | pF[17] | pF[24] | pF[25] | pF[32] | pF[33]) == 0)
            {
                pX[0] = pX[1] = pX[2] = pX[3] = pX[4] = 
                    pX[5] = pX[6] = pX[7] = (SInt16)((pF[0] * pCQt[0]) >>6);
            }
            else
            {           
                //[15.7] = ([8.0] * [7.9]) >>2
                F1 = (pF[16] * pCQt[16]);
                F7 = (pF[17] * pCQt[17]);
                tmp1 = (F1 + F7)>>2;
                a6   = (F1 - F7)>>2;

                F3 = (pF[48] * pCQt[48]);
                F5 = (pF[49] * pCQt[49]);
                tmp2 = ( F3 + F5)>>2;
                a4   = (-F3 + F5)>>2;

                b5 = ((tmp1 - tmp2) * C4_12) >>16;
                n7 = (tmp1 + tmp2) >>4;                 //---for later used---

                b6 = (a4 * C6n_12 + a6 * C2_12 ) >>16;
                n5 = (a4 * C2n_12 + a6 * C6n_12) >>16;

                n4 = b6 - n7;                           //---for later used---
                n0 = n4 - b5;                           //---for later used---
                m7 = n5 - n0;                           //---for later used---
                //-------------------------------------------------------------

                b0 = (pF[0] * pCQt[0]);
                b1 = (pF[1] * pCQt[1]);
                n1 = (b0 - b1)>>6;
                n3 = (b0 + b1)>>6;

                F2 = (pF[32] * pCQt[32]);
                F6 = (pF[33] * pCQt[33]);
                b2 = (((F2 - F6)>>2) * C4_12) >>16;
                n6 = (F2 + F6) >>6;
                n2 = b2 - n6;

                m3 = n1 + n2;                           //---for later used---
                m5 = n1 - n2;                           //---for later used---
                m4 = n3 + n6;                           //---for later used---
                m6 = n3 - n6;                           //---for later used---
                //-------------------------------------------------------------

                pX[0] = (SInt16)(m4 + n7);
                pX[1] = (SInt16)(m3 + n4);
                pX[2] = (SInt16)(m5 - n0);
                pX[3] = (SInt16)(m6 - m7);
                pX[4] = (SInt16)(m6 + m7);
                pX[5] = (SInt16)(m5 + n0);
                pX[6] = (SInt16)(m3 - n4);
                pX[7] = (SInt16)(m4 - n7);
            }
            pF+=2;
            pCQt+=2;
            pX+=8;
        }
    }
    #endif

    #if defined(__MMX__)
    {
        UInt64 iTmp1;
        const UInt64 iPShift = 0x0404040404040404;
        __asm
        {   
            mov         esi,            pProcBuf;
            mov         edi,            pOut;
            mov         edx,            iLineStep;
            xor         ecx,            ecx;
            cmp         bMirror,        1;
            jne         IDCTY2_Start;
            lea         edi,            [edi+4];
            //=================================================================
        IDCTY2_Start:
            movq        mm0,            [esi+ecx+16];   //mm0 = F1
            movq        mm2,            [esi+ecx+112];  //mm2 = F7
            movq        mm1,            mm0;
            paddsw      mm0,            mm2;            //mm0 = tmp1
            psubsw      mm1,            mm2;            //mm1 = a6
            //-----------------------------------------------------------------
            movq        mm2,            [esi+ecx+80];   //mm2 = F5
            movq        mm4,            [esi+ecx+48];   //mm4 = F3
            movq        mm3,            mm2;
            paddsw      mm2,            mm4;            //mm2 = tmp2
            psubsw      mm3,            mm4;            //mm3 = a4
            //-----------------------------------------------------------------
            movq        mm4,            mm0;
            psubsw      mm0,            mm2;
            paddsw      mm4,            mm2;            //mm4 = n7
            psllw       mm0,            2;
            pmulhw      mm0,            PC4_14;         //mm0 = b5
            //-----------------------------------------------------------------
            psllw       mm1,            2;
            psllw       mm3,            2;
            movq        mm2,            mm1;            //mm1 = mm2 = a6<<2
            movq        mm5,            mm3;            //mm3 = mm5 = a4<<2
            pmulhw      mm1,            PC2_14;
            pmulhw      mm2,            PC6n_14;
            pmulhw      mm3,            PC6n_14;
            pmulhw      mm5,            PC2n_14;
            paddsw      mm1,            mm3;            //mm1 = b6
            paddsw      mm2,            mm5;            //mm2 = n5
            //-----------------------------------------------------------------
            psubsw      mm1,            mm4;            //mm4   = n7
            movq        iTmp1,          mm1;            //iTmp1 = n4
            psubsw      mm1,            mm0;            //mm1   = n0
            psubsw      mm2,            mm1;            //mm2   = m7
            //=================================================================
            movq        mm0,            [esi+ecx+32];
            movq        mm5,            [esi+ecx+96];
            movq        mm3,            mm0;
            psubsw      mm0,            mm5;
            paddsw      mm3,            mm5;            //mm3 = n6
            psllw       mm0,            2;
            pmulhw      mm0,            PC4_14;
            psubsw      mm0,            mm3;            //mm0 = n2
            //-----------------------------------------------------------------
            movq        mm5,            [esi+ecx];
            movq        mm7,            [esi+ecx+64];
            movq        mm6,            mm5;
            psubsw      mm5,            mm7;            //mm5 = n1
            paddsw      mm6,            mm7;            //mm6 = n3
            //-----------------------------------------------------------------
            movq        mm7,            mm5;
            paddsw      mm5,            mm0;            //mm5 = m3
            psubsw      mm7,            mm0;            //mm7 = m5
            //-----------------------------------------------------------------
            movq        mm0,            mm6;
            paddsw      mm6,            mm3;            //mm6 = m4
            psubsw      mm0,            mm3;            //mm0 = m6
            //-----------------------------------------------------------------
            movq        mm3,            iPShift;
            cmp         bMirror,        1;
            paddsw      mm5,            mm3;
            paddsw      mm7,            mm3;
            paddsw      mm6,            mm3;
            paddsw      mm0,            mm3;
            //=================================================================
            je          IDCTY2_M;
        IDCTY2_M_:
            push        edi;
            //-----------------------------------------------------------------
            movq        mm3,            mm6;
            paddsw      mm6,            mm4;
            psubsw      mm3,            mm4;
            psraw       mm6,            3;
            psraw       mm3,            3;
            packuswb    mm6,            mm6;            //mm6 = X0
            packuswb    mm3,            mm3;            //mm3 = X7
            movd        [edi],          mm6;
            //-----------------------------------------------------------------
            movq        mm4,            iTmp1;          //mm4 = n4
            movq        mm6,            mm5;
            paddsw      mm5,            mm4;
            psubsw      mm6,            mm4;
            psraw       mm5,            3;
            psraw       mm6,            3;
            packuswb    mm5,            mm5;            //mm5 = X1
            packuswb    mm6,            mm6;            //mm6 = X6
            movd        [edi+edx],      mm5;
            lea         edi,            [edi+edx*2];
            //-----------------------------------------------------------------
            movq        mm4,            mm7;
            psubsw      mm7,            mm1;
            paddsw      mm4,            mm1;
            psraw       mm7,            3;
            psraw       mm4,            3;
            packuswb    mm7,            mm7;            //mm7 = X2
            packuswb    mm4,            mm4;            //mm4 = X5
            movd        [edi],          mm7;
            //-----------------------------------------------------------------
            movq        mm1,            mm0;
            psubsw      mm0,            mm2;
            paddsw      mm1,            mm2;
            psraw       mm0,            3;
            psraw       mm1,            3;
            packuswb    mm0,            mm0;            //mm0 = X3
            packuswb    mm1,            mm1;            //mm1 = X4
            movd        [edi+edx],      mm0;
            lea         edi,            [edi+edx*2];
            //-----------------------------------------------------------------
            movd        [edi],          mm1;
            movd        [edi+edx],      mm4;
            lea         edi,            [edi+edx*2];
            movd        [edi],          mm6;
            movd        [edi+edx],      mm3;
            pop         edi;
            add         edi,            4;
            jmp         IDCTY2_End;
            //-----------------------------------------------------------------
        IDCTY2_M:
            push        edi;
            //-----------------------------------------------------------------
            movq        mm3,            mm6;
            paddsw      mm6,            mm4;
            psubsw      mm3,            mm4;
            psraw       mm6,            3;
            psraw       mm3,            3;
            packuswb    mm6,            mm6;            //mm6 = X0
            packuswb    mm3,            mm3;            //mm3 = X7
            movd        eax,            mm6;
            //-----------------------------------------------------------------
            movq        mm4,            iTmp1;          //mm4 = n4
            movq        mm6,            mm5;
            paddsw      mm5,            mm4;
            psubsw      mm6,            mm4;
            psraw       mm5,            3;
            psraw       mm6,            3;
            packuswb    mm5,            mm5;            //mm5 = X1
            packuswb    mm6,            mm6;            //mm6 = X6
            movd        ebx,            mm5;
            //-----------------------------------------------------------------
            bswap       eax;
            bswap       ebx;
            mov         [edi],          eax;
            mov         [edi+edx],      ebx;
            lea         edi,            [edi+edx*2];
            //-----------------------------------------------------------------
            movq        mm4,            mm7;
            psubsw      mm7,            mm1;
            paddsw      mm4,            mm1;
            psraw       mm7,            3;
            psraw       mm4,            3;
            packuswb    mm7,            mm7;            //mm7 = X2
            packuswb    mm4,            mm4;            //mm4 = X5
            movd        eax,            mm7;
            //-----------------------------------------------------------------
            movq        mm1,            mm0;
            psubsw      mm0,            mm2;
            paddsw      mm1,            mm2;
            psraw       mm0,            3;
            psraw       mm1,            3;
            packuswb    mm0,            mm0;            //mm0 = X3
            packuswb    mm1,            mm1;            //mm1 = X4
            movd        ebx,            mm0;
            //-----------------------------------------------------------------
            bswap       eax;
            bswap       ebx;
            mov         [edi],          eax;
            mov         [edi+edx],      ebx;
            lea         edi,            [edi+edx*2];
            //-----------------------------------------------------------------
            movd        eax,            mm1;
            movd        ebx,            mm4;
            bswap       eax;
            bswap       ebx;
            mov         [edi],          eax;
            mov         [edi+edx],      ebx;
            lea         edi,            [edi+edx*2];
            movd        eax,            mm6;
            movd        ebx,            mm3;
            bswap       eax;
            bswap       ebx;
            mov         [edi],          eax;
            mov         [edi+edx],      ebx;
            pop         edi;
            sub         edi,            4;
            //=================================================================
        IDCTY2_End:
            lea         ecx,            [ecx+8];
            cmp         ecx,            8;
            jle         IDCTY2_Start;
            //emms;
        }
    }
    #else
    {
        SInt32 iIdxStart, iIdxStep;
        if (bMirror)
        {
            iIdxStart = 7;
            iIdxStep  = -1;
        }
        else
        {
            iIdxStart = 0;
            iIdxStep  = 1;
        }

        SInt32 iDCTCol;
        SInt32 F1, F2, F3, F5, F6, F7;
        SInt32 tmp1, tmp2;
        SInt32 a4, a6;
        SInt32 b0, b1, b2, b5, b6;
        SInt32 n0, n1, n2, n3, n4, n5, n6, n7;
        SInt32 m3, m4, m5, m6, m7;

        SInt16 *pF   = pProcBuf;
        UInt8  *pX   = pOut;
        SInt32 x0, x1, x2, x3, x4, x5, x6, x7;
        for (iDCTCol=0; iDCTCol<8; iDCTCol++, iIdxStart+=iIdxStep)
        {
            SInt32 iIdx = iIdxStart;
            if ((pF[8] | pF[16] | pF[24] | pF[32] | pF[40] | pF[48] | pF[56]) == 0)
            {
                SInt16 iDC;
                UInt8 iDC8;
                if ((iDC=pF[0]) < -1024)
                {

                    pX[iIdx] = 0; 
                    pX[iIdx+=iLineStep] = 0; 
                    pX[iIdx+=iLineStep] = 0; 
                    pX[iIdx+=iLineStep] = 0; 
                    pX[iIdx+=iLineStep] = 0; 
                    pX[iIdx+=iLineStep] = 0;
                    pX[iIdx+=iLineStep] = 0;
                    pX[iIdx+=iLineStep] = 0;
                }
                else if (iDC > 1016)
                {
                    pX[iIdx] = 255;
                    pX[iIdx+=iLineStep] = 255;
                    pX[iIdx+=iLineStep] = 255;
                    pX[iIdx+=iLineStep] = 255;
                    pX[iIdx+=iLineStep] = 255;
                    pX[iIdx+=iLineStep] = 255;
                    pX[iIdx+=iLineStep] = 255;
                    pX[iIdx+=iLineStep] = 255;
                }
                else
                {
                    pX[iIdx] = (iDC8 = (UInt8)((iDC +1024 +4) >>3));
                    pX[iIdx+=iLineStep] = iDC8;
                    pX[iIdx+=iLineStep] = iDC8;
                    pX[iIdx+=iLineStep] = iDC8; 
                    pX[iIdx+=iLineStep] = iDC8; 
                    pX[iIdx+=iLineStep] = iDC8;
                    pX[iIdx+=iLineStep] = iDC8; 
                    pX[iIdx+=iLineStep] = iDC8;
                }
            }
            else
            {
                tmp1 = (F1=pF[8]) + (F7=pF[56]);
                a6   = F1 - F7;

                tmp2 = (F3=pF[24]) + (F5=pF[40]);
                a4   = F5 - F3;

                b5 = ((tmp1 - tmp2) * C4_16) >>16;
                //b5 = (((tmp1 - tmp2) <<2) * C4_14) >>16; //mmx implementation
                n7 = tmp1 + tmp2;                       //---for later used---

                b6 = (a4 * C6n_16 + a6 * C2_16 ) >>16;
                n5 = (a4 * C2n_16 + a6 * C6n_16) >>16;
                //b6 = (((a4<<2) * C6n_14) >>16) + (((a6<<2) * C2_14 ) >>16); //mmx implementation
                //n5 = (((a4<<2) * C2n_14) >>16) + (((a6<<2) * C6n_14) >>16); //mmx implementation

                n4 = b6 - n7;                           //---for later used---
                n0 = n4 - b5;                           //---for later used---
                m7 = n5 - n0;                           //---for later used---
                //-------------------------------------------------------------

                b2 = (((F2=pF[16]) - (F6=pF[48])) * C4_16) >>16;
                //b2 = ((((F2=pF[16]) - (F6=pF[48]))<<2) * C4_14) >>16; //mmx implementation
                n6 = F2 + F6;
                n2 = b2 - n6;

                n1 = (b0=pF[0]) - (b1=pF[32]);
                n3 = b0 + b1;

                m3 = n1 + n2;                           //---for later used---
                m5 = n1 - n2;                           //---for later used---

                m4 = n3 + n6;                           //---for later used---
                m6 = n3 - n6;                           //---for later used---
                //-------------------------------------------------------------

                if ((x0 = m4 + n7) < -1024)
                    pX[iIdx] = 0;
                else if (x0 > 1016)
                    pX[iIdx] = 255;
                else
                    pX[iIdx] = (UInt8)((x0 + 1024 + 4) >>3);
                iIdx+=iLineStep;

                if ((x1 = m3 + n4) < -1024)
                    pX[iIdx] = 0;
                else if (x1 > 1016)
                    pX[iIdx] = 255;
                else
                    pX[iIdx] = (UInt8)((x1 + 1024 + 4) >>3);
                iIdx+=iLineStep;
                
                if ((x2 = m5 - n0) < -1024)
                    pX[iIdx] = 0;
                else if (x2 > 1016)
                    pX[iIdx] = 255;
                else
                    pX[iIdx] = (UInt8)((x2 + 1024 + 4) >>3);
                iIdx+=iLineStep;

                if ((x3 = m6 - m7) < -1024)
                    pX[iIdx] = 0;
                else if (x3 > 1016)
                    pX[iIdx] = 255;
                else
                    pX[iIdx] = (UInt8)((x3 + 1024 + 4) >>3);
                iIdx+=iLineStep;

                if ((x4 = m6 + m7) < -1024)
                    pX[iIdx] = 0;
                else if (x4 > 1016)
                    pX[iIdx] = 255;
                else
                    pX[iIdx] = (UInt8)((x4 + 1024 + 4) >>3);
                iIdx+=iLineStep;

                if ((x5 = m5 + n0) < -1024)
                    pX[iIdx] = 0;
                else if (x5 > 1016)
                    pX[iIdx] = 255;
                else
                    pX[iIdx] = (UInt8)((x5 + 1024) >>3);
                iIdx+=iLineStep;

                if ((x6 = m3 - n4) < -1024)
                    pX[iIdx] = 0;
                else if (x6 > 1016)
                    pX[iIdx] = 255;
                else
                    pX[iIdx] = (UInt8)((x6 + 1024 + 4) >>3);
                iIdx+=iLineStep;

                if ((x7 = m4 - n7) < -1024)
                    pX[iIdx] = 0;
                else if (x7 > 1016)
                    pX[iIdx] = 255;
                else
                    pX[iIdx] = (UInt8)((x7 + 1024 + 4) >>3);
                iIdx+=iLineStep;
            }
            pF++;
        }
    }
    #endif
}
//-----------------------------------------------------------------------------

void IDCTC(SInt16 *pIn, SInt16 *pCQ, UInt8 *pOut, SInt16 *pProcBuf, 
    SInt32 iLineStep, BOOL bMirror) // pIn/pCQ: column order, pOut: row order
{
    #if defined(__MMX__)
    {
        UInt64 iTmp1, iTmp2;
        __asm
        {
            mov         esi,            pIn;
            mov         edi,            pProcBuf;
            mov         edx,            pCQ;
            //=================================================================
            mov         ecx,            16;
        IDCTC_Start:
            movq        mm0,            [esi+ecx];
            por         mm0,            [esi+ecx+8];
            psrld       mm0,            16;
            por         mm0,            [esi+32+ecx];
            por         mm0,            [esi+32+ecx+8];
            por         mm0,            [esi+64+ecx];
            por         mm0,            [esi+64+ecx+8];
            por         mm0,            [esi+96+ecx];
            por         mm0,            [esi+96+ecx+8];
            packssdw    mm0,            mm0;
            movd        eax,            mm0;
            cmp         eax,            0;
            jne         IDCTC_Slow;
            //-----------------------------------------------------------------
        IDCTC_Fast:
            pcmpeqd     mm7,            mm7;
            movq        mm0,            [esi+ecx];
            movq        mm1,            [esi+ecx+8];
            psrld       mm7,            16;
            movq        mm2,            [edx+ecx];
            movq        mm3,            [edx+ecx+8];
            pand        mm2,            mm7;
            pand        mm3,            mm7;
            pmaddwd     mm0,            mm2;
            pmaddwd     mm1,            mm3;
            psrad       mm0,            6;
            psrad       mm1,            6;
            packssdw    mm0,            mm1;
            movq        mm2,            mm0;
            movq        mm4,            mm0;
            punpcklwd   mm0,            mm4;
            punpckhwd   mm2,            mm4;
            movq        mm1,            mm0;
            movq        mm4,            mm0;
            movq        mm3,            mm2;
            movq        mm5,            mm2;
            punpckldq   mm0,            mm4;
            punpckhdq   mm1,            mm4;
            punpckldq   mm2,            mm5;
            punpckhdq   mm3,            mm5;
            movq        [edi+ecx*4],    mm0;
            movq        [edi+ecx*4+8],  mm0;
            movq        [edi+ecx*4+16], mm1;
            movq        [edi+ecx*4+24], mm1;
            movq        [edi+ecx*4+32], mm2;
            movq        [edi+ecx*4+40], mm2;
            movq        [edi+ecx*4+48], mm3;
            movq        [edi+ecx*4+56], mm3;
            jmp         IDCTC_End;
        IDCTC_Slow:
            movq        mm0,            [esi+ecx+32];
            movq        mm2,            [esi+ecx+32+8];
            movq        mm1,            mm0;                //mm0 = mm1
            movq        mm3,            mm2;                //mm2 = mm3
            pmaddwd     mm0,            [edx+ecx+32];       //mm0 = F11+F71 F10+F70
            pmaddwd     mm1,            [edx+ecx+128+32];   //mm1 = F11-F71 F10-F70
            pmaddwd     mm2,            [edx+ecx+32+8];     //mm2 = F13+F73 F12+F72
            pmaddwd     mm3,            [edx+ecx+128+32+8]; //mm3 = F13-F73 F12-F72
            psrad       mm0,            2;
            psrad       mm1,            2;
            psrad       mm2,            2;
            psrad       mm3,            2;
            packssdw    mm0,            mm2;                //mm0 = tmp1
            packssdw    mm1,            mm3;                //mm1 = a6
            //-----------------------------------------------------------------
            movq        mm2,            [esi+ecx+96];
            movq        mm4,            [esi+ecx+96+8];
            movq        mm3,            mm2;                //mm2 = mm3
            movq        mm5,            mm4;                //mm4 = mm5
            pmaddwd     mm2,            [edx+ecx+96];       //mm2 =  F31+F51  F30+F50
            pmaddwd     mm3,            [edx+ecx+128+96];   //mm3 = -F31+F51 -F30+F50
            pmaddwd     mm4,            [edx+ecx+96+8];     //mm4 =  F33+F53  F32+F52
            pmaddwd     mm5,            [edx+ecx+128+96+8]; //mm5 = -F33+F53 -F32+F52
            psrad       mm2,            2;
            psrad       mm3,            2;
            psrad       mm4,            2;
            psrad       mm5,            2;
            packssdw    mm2,            mm4;                //mm2 = tmp2
            packssdw    mm3,            mm5;                //mm3 = a4
            //-----------------------------------------------------------------
            movq        mm5,            mm0;
            psubsw      mm0,            mm2;
            paddsw      mm5,            mm2;
            pmulhw      mm0,            PC4_12;             //mm0 = b5
            psraw       mm5,            4;                  //mm5 = n7
            //-----------------------------------------------------------------
            movq        mm2,            mm3;
            punpckhwd   mm3,            mm1;                // mm3 = a6 a4 a6 a4
            punpcklwd   mm2,            mm1;                // mm2 = a6 a4 a6 a4
            movq        mm4,            mm3;
            movq        mm1,            mm2;
            pmaddwd     mm1,            PC2_C6n_12;
            pmaddwd     mm2,            PC6n_C2n_12;
            pmaddwd     mm3,            PC2_C6n_12;
            pmaddwd     mm4,            PC6n_C2n_12;
            psrad       mm1,            16;
            psrad       mm2,            16;
            psrad       mm3,            16;
            psrad       mm4,            16;
            packssdw    mm1,            mm3;                //mm1 = b6
            packssdw    mm2,            mm4;                //mm2 = n5
            //-----------------------------------------------------------------
            psubsw      mm1,            mm5;
            movq        mm3,            mm1;                //mm3 = n4
            psubsw      mm1,            mm0;                //mm1 = n0
            psubsw      mm2,            mm1;                //mm2 = m7
            //-----------------------------------------------------------------
            movq        iTmp1,          mm5;                //n7
            movq        iTmp2,          mm1;                //n0
            //mm0=n7  mm1=n0, mm2=m7, mm3=n4
            //=================================================================
            movq        mm4,            [esi+ecx];
            movq        mm6,            [esi+ecx+8];
            movq        mm5,            mm4;                //mm4 = mm5
            movq        mm7,            mm6;                //mm6 = mm7
            pmaddwd     mm5,            [edx+ecx];          //mm5 = F01+F41 F00+F40
            pmaddwd     mm4,            [edx+ecx+128];      //mm4 = F01-F41 F00-F40
            pmaddwd     mm7,            [edx+ecx+8];        //mm7 = F03+F43 F02+F42
            pmaddwd     mm6,            [edx+ecx+128+8];    //mm6 = F03-F43 F02-F42
            psrad       mm4,            6;
            psrad       mm5,            6;
            psrad       mm6,            6;
            psrad       mm7,            6;
            packssdw    mm4,            mm6;                //mm4 = n1
            packssdw    mm5,            mm7;                //mm5 = n3
            //-----------------------------------------------------------------
            movq        mm6,            [esi+ecx+64];
            movq        mm0,            [esi+ecx+64+8];
            movq        mm7,            mm6;                //mm6 = mm7
            movq        mm1,            mm0;                //mm0 = mm3
            pmaddwd     mm7,            [edx+ecx+64];       //mm7 = F21+F61 F20+F60
            pmaddwd     mm6,            [edx+ecx+64+128];   //mm6 = F21-F61 F20-F60
            pmaddwd     mm1,            [edx+ecx+64+8];     //mm1 = F23+F63 F22+F62
            pmaddwd     mm0,            [edx+ecx+64+128+8]; //mm0 = F23-F63 F22-F62
            psrad       mm7,            6;
            psrad       mm6,            2;
            psrad       mm1,            6;
            psrad       mm0,            2;
            packssdw    mm6,            mm0;
            packssdw    mm7,            mm1;                //mm7 = n6
            pmulhw      mm6,            PC4_12;             //mm6 = b2
            psubsw      mm6,            mm7;                //mm6 = n2
            //-----------------------------------------------------------------
            movq        mm1,            iTmp2;              //mm1 = n0
            movq        mm0,            mm4;
            paddsw      mm0,            mm6;                //mm0 = m3
            psubsw      mm4,            mm6;                //mm4 = m5
            movq        mm6,            mm0;
            paddsw      mm0,            mm3;                //mm0 = X1
            psubsw      mm6,            mm3;                //mm6 = X6
            movq        mm3,            mm4;
            psubsw      mm4,            mm1;                //mm4 = X2
            paddsw      mm3,            mm1;                //mm3 = X5
            //-----------------------------------------------------------------
            movq        mm1,            mm5;
            paddsw      mm1,            mm7;                //mm1 = m4
            psubsw      mm5,            mm7;                //mm5 = m6
            movq        mm7,            mm5;
            psubsw      mm5,            mm2;                //mm5 = X3
            paddsw      mm7,            mm2;                //mm7 = X4
            movq        mm2,            mm1;
            paddsw      mm1,            iTmp1;              //mm1 = X0
            psubsw      mm2,            iTmp1;              //mm2 = X7
            //=================================================================
        IDCTC_Store:
            movq        iTmp2,          mm0;
            movq        mm0,            mm7;
            punpcklwd   mm7,            mm3;                //mm7 = X51 X41 X50 X40
            punpckhwd   mm0,            mm3;                //mm0 = X53 X43 X52 X42
            movq        mm3,            mm6;
            punpcklwd   mm6,            mm2;                //mm6 = X71 X61 X70 X60
            punpckhwd   mm3,            mm2;                //mm3 = X73 X63 X72 X62
            //-----------------------------------------------------------------
            movq        mm2,            mm7;
            punpckldq   mm7,            mm6;                //mm7 = F70 F60 F50 F40
            punpckhdq   mm2,            mm6;                //mm2 = F71 F61 F51 F41
            movq        mm6,            mm0;
            punpckldq   mm0,            mm3;                //mm0 = F72 F62 F52 F42
            punpckhdq   mm6,            mm3;                //mm6 = F73 F63 F53 F43
            //-----------------------------------------------------------------
            movq        mm3,            iTmp2;
            movq        [edi+ecx*4+8],  mm7;
            movq        [edi+ecx*4+24], mm2;
            movq        [edi+ecx*4+40], mm0;
            movq        [edi+ecx*4+56], mm6;
            //-----------------------------------------------------------------
            movq        mm2,            mm1;
            punpcklwd   mm1,            mm3;                //mm1 = X11 X01 X10 X00
            punpckhwd   mm2,            mm3;                //mm2 = X13 X03 X12 X02
            movq        mm3,            mm4;
            punpcklwd   mm4,            mm5;                //mm4 = X31 X21 X30 X20
            punpckhwd   mm3,            mm5;                //mm3 = X33 X23 X32 X22
            //-----------------------------------------------------------------
            movq        mm5,            mm1;
            punpckldq   mm1,            mm4;                //mm1 = F30 F20 F10 F00
            punpckhdq   mm5,            mm4;                //mm5 = F31 F21 F11 F01
            movq        mm4,            mm2;
            punpckldq   mm2,            mm3;                //mm2 = F32 F22 F12 F02
            punpckhdq   mm4,            mm3;                //mm4 = F33 F23 F13 F03
            //-----------------------------------------------------------------
            movq        [edi+ecx*4],    mm1;
            movq        [edi+ecx*4+16], mm5;
            movq        [edi+ecx*4+32], mm2;
            movq        [edi+ecx*4+48], mm4;
            //-----------------------------------------------------------------
        IDCTC_End:
            sub         ecx,            16;
            jge         IDCTC_Start;
            //emms;
        }
    }
    #else
    {
        SInt32 iDCTCol;
        SInt32 F1, F2, F3, F5, F6, F7;
        SInt32 tmp1, tmp2;
        SInt32 a4, a6;
        SInt32 b0, b1, b2, b5, b6;
        SInt32 n0, n1, n2, n3, n4, n5, n6, n7;
        SInt32 m3, m4, m5, m6, m7;

        SInt16 *pF   = pIn;
        SInt16 *pCQt = pCQ;
        SInt16 *pX   = pProcBuf;
        for (iDCTCol=0; iDCTCol<8; iDCTCol++)
        {
            if ((pF[8] | pF[16] | pF[17] | pF[24] | pF[25] | pF[32] | pF[33]) == 0)
            {
                pX[0] = pX[1] = pX[2] = pX[3] = pX[4] = 
                    pX[5] = pX[6] = pX[7] = (SInt16)((pF[0] * pCQt[0]) >>6);
            }
            else
            {           
                //[15.7] = ([8.0] * [7.9]) >>2
                F1 = (pF[16] * pCQt[16]);
                F7 = (pF[17] * pCQt[17]);
                tmp1 = (F1 + F7)>>2;
                a6   = (F1 - F7)>>2;

                F3 = (pF[48] * pCQt[48]);
                F5 = (pF[49] * pCQt[49]);
                tmp2 = ( F3 + F5)>>2;
                a4   = (-F3 + F5)>>2;

                b5 = ((tmp1 - tmp2) * C4_12) >>16;
                n7 = (tmp1 + tmp2) >>4;                 //---for later used---

                b6 = (a4 * C6n_12 + a6 * C2_12 ) >>16;
                n5 = (a4 * C2n_12 + a6 * C6n_12) >>16;

                n4 = b6 - n7;                           //---for later used---
                n0 = n4 - b5;                           //---for later used---
                m7 = n5 - n0;                           //---for later used---
                //-------------------------------------------------------------

                b0 = (pF[0] * pCQt[0]);
                b1 = (pF[1] * pCQt[1]);
                n1 = (b0 - b1)>>6;
                n3 = (b0 + b1)>>6;

                F2 = (pF[32] * pCQt[32]);
                F6 = (pF[33] * pCQt[33]);
                b2 = (((F2 - F6)>>2) * C4_12) >>16;
                n6 = (F2 + F6) >>6;
                n2 = b2 - n6;

                m3 = n1 + n2;                           //---for later used---
                m5 = n1 - n2;                           //---for later used---
                m4 = n3 + n6;                           //---for later used---
                m6 = n3 - n6;                           //---for later used---
                //-------------------------------------------------------------

                pX[0] = (SInt16)(m4 + n7);
                pX[1] = (SInt16)(m3 + n4);
                pX[2] = (SInt16)(m5 - n0);
                pX[3] = (SInt16)(m6 - m7);
                pX[4] = (SInt16)(m6 + m7);
                pX[5] = (SInt16)(m5 + n0);
                pX[6] = (SInt16)(m3 - n4);
                pX[7] = (SInt16)(m4 - n7);
            }
            pF+=2;
            pCQt+=2;
            pX+=8;
        }
    }
    #endif

    #if defined(__MMX__)
    {
        const UInt64 iPShift = 0x0404040404040404;
        __asm
        {
            mov         esi,            pProcBuf;
            mov         edi,            pOut;
            mov         edx,            iLineStep;
            xor         ecx,            ecx;
            cmp         bMirror,        1;
            jne         IDCTC2_Start;
            lea         edi,            [edi+4];
            //=================================================================
        IDCTC2_Start:
            movq        mm0,            [esi+ecx+16];   //mm0 = F1
            movq        mm2,            [esi+ecx+112];  //mm2 = F7
            movq        mm1,            mm0;
            paddsw      mm0,            mm2;            //mm0 = tmp1
            psubsw      mm1,            mm2;            //mm1 = a6
            //-----------------------------------------------------------------
            movq        mm2,            [esi+ecx+80];   //mm2 = F5
            movq        mm4,            [esi+ecx+48];   //mm4 = F3
            movq        mm3,            mm2;
            paddsw      mm2,            mm4;            //mm2 = tmp2
            psubsw      mm3,            mm4;            //mm3 = a4
            //-----------------------------------------------------------------
            movq        mm4,            mm0;
            psubsw      mm0,            mm2;
            paddsw      mm4,            mm2;            //mm4 = n7
            psllw       mm0,            2;
            pmulhw      mm0,            PC4_14;         //mm0 = b5
            //-----------------------------------------------------------------
            psllw       mm1,            2;              //mm1 = a6<<2
            psllw       mm3,            2;              //mm3 = a4<<2
            pmulhw      mm1,            PC2_14;
            pmulhw      mm3,            PC6n_14;
            paddsw      mm1,            mm3;            //mm1 = b6
            //-----------------------------------------------------------------
            paddsw      mm0,            mm4;            //mm0 = b5+n7
            psubsw      mm1,            mm0;            //mm1 = n0
            //=================================================================
            movq        mm0,            [esi+ecx+32];
            movq        mm5,            [esi+ecx+96];
            movq        mm3,            mm0;
            psubsw      mm0,            mm5;
            paddsw      mm3,            mm5;            //mm3 = n6
            psllw       mm0,            2;
            pmulhw      mm0,            PC4_14;
            psubsw      mm0,            mm3;            //mm0 = n2
            //-----------------------------------------------------------------
            movq        mm5,            [esi+ecx];
            movq        mm7,            [esi+ecx+64];
            movq        mm6,            mm5;
            psubsw      mm5,            mm7;            //mm5 = n1
            paddsw      mm6,            mm7;            //mm6 = n3
            //-----------------------------------------------------------------
            psubsw      mm5,            mm0;            //mm5 = m5
            paddsw      mm6,            mm3;            //mm6 = m4
            //-----------------------------------------------------------------
            movq        mm0,            iPShift;
            paddsw      mm5,            mm0;
            paddsw      mm6,            mm0;
            //=================================================================
            cmp         bMirror,        1;
            je          IDCTC2_M;            
        IDCTC2_M_:
            push        edi;
            //-----------------------------------------------------------------
            movq        mm3,            mm5;            //mm3 = mm5 = m5
            movq        mm7,            mm6;            //mm6 = mm7 = m4
            paddsw      mm6,            mm4;            //mm6 = X0
            psubsw      mm7,            mm4;            //mm7 = X7
            paddsw      mm3,            mm1;            //mm3 = X5
            psubsw      mm5,            mm1;            //mm5 = X2
            psraw       mm6,            3;
            psraw       mm7,            3;
            psraw       mm3,            3;
            psraw       mm5,            3;
            packuswb    mm6,            mm3;
            packuswb    mm7,            mm5;
            packuswb    mm3,            mm6;
            packuswb    mm5,            mm7;
            movd        [edi],          mm6;
            movd        [edi+edx],      mm5;
            lea         edi,            [edi+edx*2];
            movd        [edi],          mm3;
            movd        [edi+edx],      mm7;
            pop         edi;
            add         edi,            4;
            jmp         IDCTC2_End;
            //-----------------------------------------------------------------
        IDCTC2_M:
            push        edi;
            //-----------------------------------------------------------------
            movq        mm3,            mm5;            //mm3 = mm5 = m5
            movq        mm7,            mm6;            //mm6 = mm7 = m4
            paddsw      mm6,            mm4;            //mm6 = X0
            psubsw      mm7,            mm4;            //mm7 = X7
            paddsw      mm3,            mm1;            //mm3 = X5
            psubsw      mm5,            mm1;            //mm5 = X2
            psraw       mm6,            3;
            psraw       mm7,            3;
            psraw       mm3,            3;
            psraw       mm5,            3;
            packuswb    mm6,            mm3;
            packuswb    mm7,            mm5;
            packuswb    mm3,            mm6;
            packuswb    mm5,            mm7;
            movd        eax,            mm6;
            movd        ebx,            mm5;
            bswap       eax;
            bswap       ebx;
            mov         [edi],          eax;
            mov         [edi+edx],      ebx;
            lea         edi,            [edi+edx*2];
            movd        eax,            mm3;
            movd        ebx,            mm7;
            bswap       eax;
            bswap       ebx;
            mov         [edi],          eax;
            mov         [edi+edx],      ebx;
            pop         edi;
            sub         edi,            4;
            //=================================================================
        IDCTC2_End:
            lea         ecx,            [ecx+8];
            cmp         ecx,            8;
            jle         IDCTC2_Start;
            //emms;
        }            
    }
    #else
    {
        SInt32 iIdxStart, iIdxStep;
        if (bMirror)
        {
            iIdxStart = 7;
            iIdxStep  = -1;
        }
        else
        {
            iIdxStart = 0;
            iIdxStep  = 1;
        }

        SInt32 iDCTCol;
        SInt32 F1, F2, F3, F5, F6, F7;
        SInt32 tmp1, tmp2;
        SInt32 a4, a6;
        SInt32 b0, b1, b2, b5, b6;
        SInt32 n0, n1, n2, n3, n4, n5, n6, n7;
        SInt32 m3, m4, m5, m6, m7;

        SInt16 *pF   = pProcBuf;
        UInt8  *pX   = pOut;
        SInt32 x0, x1, x2, x3, x4, x5, x6, x7;
        for (iDCTCol=0; iDCTCol<8; iDCTCol++, iIdxStart+=iIdxStep)
        {
            SInt32 iIdx = iIdxStart;
            /*if ((pF[8] | pF[16] | pF[24] | pF[32] | pF[40] | pF[48] | pF[56]) == 0)
            {
                SInt16 iDC;
                UInt8 iDC8;
                if ((iDC=pF[0]) < -1024)
                {

                    pX[iIdx] = 0; 
                    pX[iIdx+=iLineStep] = 0; 
                    pX[iIdx+=iLineStep] = 0; 
                    pX[iIdx+=iLineStep] = 0; 
                    //pX[iIdx+=iLineStep] = 0; 
                    //pX[iIdx+=iLineStep] = 0;
                    //pX[iIdx+=iLineStep] = 0;
                    //pX[iIdx+=iLineStep] = 0;
                }
                else if (iDC > 1016)
                {
                    pX[iIdx] = 255;
                    pX[iIdx+=iLineStep] = 255;
                    pX[iIdx+=iLineStep] = 255;
                    pX[iIdx+=iLineStep] = 255;
                    //pX[iIdx+=iLineStep] = 255;
                    //pX[iIdx+=iLineStep] = 255;
                    //pX[iIdx+=iLineStep] = 255;
                    //pX[iIdx+=iLineStep] = 255;
                }
                else
                {
                    pX[iIdx] = (iDC8 = (UInt8)((iDC +1024 +4) >>3));
                    pX[iIdx+=iLineStep] = iDC8;
                    pX[iIdx+=iLineStep] = iDC8;
                    pX[iIdx+=iLineStep] = iDC8; 
                    //pX[iIdx+=iLineStep] = iDC8; 
                    //pX[iIdx+=iLineStep] = iDC8;
                    //pX[iIdx+=iLineStep] = iDC8; 
                    //pX[iIdx+=iLineStep] = iDC8;
                }
            }
            else*/
            {
                tmp1 = (F1=pF[8]) + (F7=pF[56]);
                a6   = F1 - F7;

                tmp2 = (F3=pF[24]) + (F5=pF[40]);
                a4   = F5 - F3;

                b5 = ((tmp1 - tmp2) * C4_16) >>16;
                //b5 = (((tmp1 - tmp2) <<2) * C4_14) >>16; //mmx implementation
                n7 = tmp1 + tmp2;                       //---for later used---

                b6 = (a4 * C6n_16 + a6 * C2_16 ) >>16;
                //n5 = (a4 * C2n_16 + a6 * C6n_16) >>16;
                //b6 = (((a4<<2) * C6n_14) >>16) + (((a6<<2) * C2_14 ) >>16); //mmx implementation
                //n5 = (((a4<<2) * C2n_14) >>16) + (((a6<<2) * C6n_14) >>16); //mmx implementation

                n4 = b6 - n7;                           //---for later used---
                n0 = n4 - b5;                           //---for later used---
                //m7 = n5 - n0;                           //---for later used---
                //-------------------------------------------------------------

                b2 = (((F2=pF[16]) - (F6=pF[48])) * C4_16) >>16;
                //b2 = ((((F2=pF[16]) - (F6=pF[48]))<<2) * C4_14) >>16; //mmx implementation
                n6 = F2 + F6;
                n2 = b2 - n6;

                n1 = (b0=pF[0]) - (b1=pF[32]);
                n3 = b0 + b1;

                //m3 = n1 + n2;                           //---for later used---
                m5 = n1 - n2;                           //---for later used---

                m4 = n3 + n6;                           //---for later used---
                //m6 = n3 - n6;                           //---for later used---
                //-------------------------------------------------------------

                if ((x0 = m4 + n7) < -1024)
                    pX[iIdx] = 0;
                else if (x0 > 1016)
                    pX[iIdx] = 255;
                else
                    pX[iIdx] = (UInt8)((x0 + 1024 + 4) >>3);
                iIdx+=iLineStep;

                /*if ((x1 = m3 + n4) < -1024)
                    pX[iIdx] = 0;
                else if (x1 > 1016)
                    pX[iIdx] = 255;
                else
                    pX[iIdx] = (UInt8)((x1 + 1024 + 4) >>3);
                iIdx+=iLineStep;*/
                    
                if ((x2 = m5 - n0) < -1024)
                    pX[iIdx] = 0;
                else if (x2 > 1016)
                    pX[iIdx] = 255;
                else
                    pX[iIdx] = (UInt8)((x2 + 1024 + 4) >>3);
                iIdx+=iLineStep;

                /*if ((x3 = m6 - m7) < -1024)
                    pX[iIdx] = 0;
                else if (x3 > 1016)
                    pX[iIdx] = 255;
                else
                    pX[iIdx] = (UInt8)((x3 + 1024 + 4) >>3);
                iIdx+=iLineStep;*/

                /*if ((x4 = m6 + m7) < -1024)
                    pX[iIdx] = 0;
                else if (x4 > 1016)
                    pX[iIdx] = 255;
                else
                    pX[iIdx] = (UInt8)((x4 + 1024 + 4) >>3);
                iIdx+=iLineStep;*/

                if ((x5 = m5 + n0) < -1024)
                    pX[iIdx] = 0;
                else if (x5 > 1016)
                    pX[iIdx] = 255;
                else
                    pX[iIdx] = (UInt8)((x5 + 1024) >>3);
                iIdx+=iLineStep;

                /*if ((x6 = m3 - n4) < -1024)
                    pX[iIdx] = 0;
                else if (x6 > 1016)
                    pX[iIdx] = 255;
                else
                    pX[iIdx] = (UInt8)((x6 + 1024 + 4) >>3);
                iIdx+=iLineStep;*/

                if ((x7 = m4 - n7) < -1024)
                    pX[iIdx] = 0;
                else if (x7 > 1016)
                    pX[iIdx] = 255;
                else
                    pX[iIdx] = (UInt8)((x7 + 1024 + 4) >>3);
                //iIdx+=iLineStep;
            }
            pF++;
        }
    }
    #endif
}
//-----------------------------------------------------------------------------

void CJPEG_JPEGDecodeCheckError(CJPEG *mp, UInt8 *pStream, UInt32 iJFIFLen)
{

	CJPEG_GetHeader(mp, pStream);
    mp->Report.iJFIFLen = iJFIFLen;

    StreamBuf *pStreamBuf = &mp->sStreamBuf;
    pStreamBuf->pStreamPtr = pStream +iJPEGHeaderLen;
    pStreamBuf->iBitsLeft = 48;
    pStreamBuf->iBitsUsed = 0;
    CJPEG_GetBits(pStreamBuf, 16);
    CJPEG_GetBits(pStreamBuf, 16);
    CJPEG_GetBits(pStreamBuf, 16);
    CJPEG_GetBits(pStreamBuf, 16);

    UInt32 iLastDC[3] = {0, 0, 0};
    HuffTable *pYDCHuffTable = mp->pHuffTable[0];
    HuffTable *pCDCHuffTable = mp->pHuffTable[1];
    HuffTable *pYACHuffTable = mp->pHuffTable[2];
    HuffTable *pCACHuffTable = mp->pHuffTable[3];

    SInt16 *pYQTable = mp->pYQTable;
    SInt16 *pCQTable = mp->pCQTable;

    mp->Report.bJPEGError = TRUE;
    mp->Report.iErrorCode = JPGD_DECODING;
    
    BOOL bMirror = mp->Param.bMirror;
    BOOL bFlip   = mp->Param.bFlip;

    SInt32 iMCU_H_Cnt = (mp->Param.iStrmHSize +15)/16;
    SInt32 iDstMCU_H_START, iDstMCU_H_STOP, iSrcMCU_H_START, iSrcMCU_H_STOP;
    if (!bMirror)
    {
        iDstMCU_H_START = mp->Param.iDecHStart /16;
        iDstMCU_H_STOP  = (mp->Param.iDecHStart +mp->Param.iDecHSize +15) /16 -1;
        iSrcMCU_H_START = iDstMCU_H_START;
        iSrcMCU_H_STOP  = iDstMCU_H_STOP; 
    }
    else
    {
        iDstMCU_H_START = (mp->Param.iDecHStart +mp->Param.iDecHSize +15) /16 -1;
        iDstMCU_H_STOP  = mp->Param.iDecHStart /16;
        iSrcMCU_H_START = iMCU_H_Cnt-1-iDstMCU_H_START;
        iSrcMCU_H_STOP  = iMCU_H_Cnt-1-iDstMCU_H_STOP; 
    }
    SInt32 iMCU_H_SIZE  = iSrcMCU_H_STOP -iSrcMCU_H_START +1;

    SInt32 iMCU_V_Cnt = (mp->Param.iStrmVSize +7)/8;
    SInt32 iDstMCU_V_START, iDstMCU_V_STOP, iSrcMCU_V_START, iSrcMCU_V_STOP;
    if (!bFlip)
    {
        iDstMCU_V_START = mp->Param.iDecVStart /8;
        iDstMCU_V_STOP  = (mp->Param.iDecVStart +mp->Param.iDecVSize +7) /8 -1;
        iSrcMCU_V_START = iDstMCU_V_START;
        iSrcMCU_V_STOP  = iDstMCU_V_STOP;
    }
    else
    {
        iDstMCU_V_START = (mp->Param.iDecVStart +mp->Param.iDecVSize +7) /8 -1;
        iDstMCU_V_STOP  = mp->Param.iDecVStart /8;
        iSrcMCU_V_START = iMCU_V_Cnt-1-iDstMCU_V_START;
        iSrcMCU_V_STOP  = iMCU_V_Cnt-1-iDstMCU_V_STOP;         
    }

    SInt32 iYOutBufHSize = mp->Param.iOutBufHSize;
    SInt32 iYOutBufVSize = mp->Param.iOutBufVSize;
    SInt32 iCOutBufHSize = iYOutBufHSize/2;
    SInt32 iCOutBufVSize = iYOutBufVSize/2;

    SInt16 *pIDCTSrc = mp->pDeHuffBuf +iSrcMCU_H_START*256;

    /*UInt8 *pIDCTYDst  = pOutBuf;
    UInt8 *pIDCTCbDst = pIDCTYDst  +iYOutBufHSize*iYOutBufVSize;
    UInt8 *pIDCTCrDst = pIDCTCbDst +iCOutBufHSize*iCOutBufVSize; */
    SInt32 iIDCTYDstStep, iIDCTCDstStep, iIDCTYLineStep, iIDCTCLineStep;
    if (!bFlip)
    {
        iIDCTYDstStep = iYOutBufHSize*8;
        iIDCTCDstStep = iCOutBufHSize*4;
        iIDCTYLineStep = iYOutBufHSize;
        iIDCTCLineStep = iCOutBufHSize;
        //pIDCTYDst  = pIDCTYDst  + iDstMCU_V_START*8*iYOutBufHSize +iDstMCU_H_START*16;
        //pIDCTCbDst = pIDCTCbDst + iDstMCU_V_START*4*iCOutBufHSize +iDstMCU_H_START*8;
        //pIDCTCrDst = pIDCTCrDst + iDstMCU_V_START*4*iCOutBufHSize +iDstMCU_H_START*8;
    }
    else
    {
        iIDCTYDstStep = -iYOutBufHSize*8;
        iIDCTCDstStep = -iCOutBufHSize*4;
        iIDCTYLineStep = -iYOutBufHSize;
        iIDCTCLineStep = -iCOutBufHSize;
        //pIDCTYDst  = pIDCTYDst  + ((iDstMCU_V_START+1)*8-1)*iYOutBufHSize +iDstMCU_H_START*16;
        //pIDCTCbDst = pIDCTCbDst + ((iDstMCU_V_START+1)*4-1)*iCOutBufHSize +iDstMCU_H_START*8;
        //pIDCTCrDst = pIDCTCrDst + ((iDstMCU_V_START+1)*4-1)*iCOutBufHSize +iDstMCU_H_START*8;
    }

    SInt32 k, r, s;
    for (SInt32 iMCU_V = 0; ; iMCU_V++)
    {
        if (iMCU_V == iMCU_V_Cnt)
        {
            // Find end of image (EOI) marker, so we can return to the user the
            // exact size of the input stream.
            CJPEG_GetBits(pStreamBuf,  pStreamBuf->iBitsLeft %8);
            int c = CJPEG_GetBits(pStreamBuf, 16);

            if (c != 0xffd9)
                mp->Report.iErrorCode = JPGD_STREAM_READ;
            else
                mp->Report.bJPEGError = FALSE;
            break;
        }
        else if ((iMCU_V>=iSrcMCU_V_START) && (iMCU_V<=iSrcMCU_V_STOP))
        {
            #if defined(__KERNEL_MODE__)
                RtlZeroMemory(pIDCTSrc,  iMCU_H_SIZE <<9);
            #else
                memset(pIDCTSrc, 0, iMCU_H_SIZE <<9);
            #endif
            SInt16 *pDeHuffPtr = mp->pDeHuffBuf;

            //Decode VLC to FLC
            for (int iMCU_H = 0; iMCU_H < iMCU_H_Cnt; iMCU_H++, pDeHuffPtr += 256)
            {
                //=====Y0=====
                if ((s = CJPEG_HuffDecode(pStreamBuf, pYDCHuffTable)) != 0)
                {
                    r = CJPEG_GetBits(pStreamBuf, s);
                    s = CJPEG_HuffExtend(r, s);
                }
                iLastDC[0] = (s += iLastDC[0]);
                pDeHuffPtr[0] = (short)s;

                for (k = 1; k < 64; k++)
                {
                    s = CJPEG_HuffDecode(pStreamBuf, pYACHuffTable);
                    r = s >> 4;
                    s &= 15;

                    if (s)
                    {
                        if (r)
                        {
                            if ((k + r) > 63)
                            {
                                mp->Report.iErrorCode = JPGD_DECODE_ERROR;
                                return;
                            }
                            k += r;
                        }
                        r = CJPEG_GetBits(pStreamBuf, s);
                        s = CJPEG_HuffExtend(r, s);
                        pDeHuffPtr[DeHuffOrder[k]] = (short)s;
                    }
                    else
                    {
                        if (r == 15)
                        {
                            if (k > 48)
                            {
                                mp->Report.iErrorCode = JPGD_DECODE_ERROR;
                                return;
                            }
                            k += 15;
                        }
                        else    //EOB
                            break;
                    }
                }

                //=====Y1=====
                if ((s = CJPEG_HuffDecode(pStreamBuf, pYDCHuffTable)) != 0)
                {
                    r = CJPEG_GetBits(pStreamBuf, s);
                    s = CJPEG_HuffExtend(r, s);
                }
                iLastDC[0] = (s += iLastDC[0]);
                pDeHuffPtr[64] = (short)s;

                for (k = 1; k < 64; k++)
                {
                    s = CJPEG_HuffDecode(pStreamBuf, pYACHuffTable);
                    r = s >> 4;
                    s &= 15;

                    if (s)
                    {
                        if (r)
                        {
                            if ((k + r) > 63)
                            {
                                mp->Report.iErrorCode = JPGD_DECODE_ERROR;
                                return;
                            }
                            k += r;
                        }
                        r = CJPEG_GetBits(pStreamBuf, s);
                        s = CJPEG_HuffExtend(r, s);
                        pDeHuffPtr[DeHuffOrder[k]+64] = (short)s;
                    }
                    else
                    {
                        if (r == 15)
                        {
                            if (k > 48)
                            {
                                mp->Report.iErrorCode = JPGD_DECODE_ERROR;
                                return;
                            }
                            k += 15;
                        }
                        else    //EOB
                            break;
                    }
                }

                //=====Cb=====
                if ((s = CJPEG_HuffDecode(pStreamBuf, pCDCHuffTable)) != 0)
                {
                    r = CJPEG_GetBits(pStreamBuf, s);
                    s = CJPEG_HuffExtend(r, s);
                }
                iLastDC[1] = (s += iLastDC[1]);
                pDeHuffPtr[128] = (short)s;

                for (k = 1; k < 64; k++)
                {
                    s = CJPEG_HuffDecode(pStreamBuf, pCACHuffTable);
                    r = s >> 4;
                    s &= 15;

                    if (s)
                    {
                        if (r)
                        {
                            if ((k + r) > 63)
                            {
                                mp->Report.iErrorCode = JPGD_DECODE_ERROR;
                                return;
                            }
                            k += r;
                        }
                        r = CJPEG_GetBits(pStreamBuf, s);
                        s = CJPEG_HuffExtend(r, s);
                        pDeHuffPtr[DeHuffOrder[k]+128] = (short)s;
                    }
                    else
                    {
                        if (r == 15)
                        {
                            if (k > 48)
                            {
                                mp->Report.iErrorCode = JPGD_DECODE_ERROR;
                                return;
                            }
                            k += 15;
                        }
                        else    //EOB
                            break;
                    }
                }

                //=====Cr=====
                if ((s = CJPEG_HuffDecode(pStreamBuf, pCDCHuffTable)) != 0)
                {
                    r = CJPEG_GetBits(pStreamBuf, s);
                    s = CJPEG_HuffExtend(r, s);
                }
                iLastDC[2] = (s += iLastDC[2]);
                pDeHuffPtr[192] = (short)s;

                for (k = 1; k < 64; k++)
                {
                    s = CJPEG_HuffDecode(pStreamBuf, pCACHuffTable);
                    r = s >> 4;
                    s &= 15;

                    if (s)
                    {
                        if (r)
                        {
                            if ((k + r) > 63)
                            {
                                mp->Report.iErrorCode = JPGD_DECODE_ERROR;
                                return;
                            }
                            k += r;
                        }
                        r = CJPEG_GetBits(pStreamBuf, s);
                        s = CJPEG_HuffExtend(r, s);
                        pDeHuffPtr[DeHuffOrder[k]+192] = (short)s;
                    }
                    else
                    {
                        if (r == 15)
                        {
                            if (k > 48)
                            {
                                mp->Report.iErrorCode = JPGD_DECODE_ERROR;
                                return;
                            }
                            k += 15;
                        }
                        else    //EOB
                            break;
                    }
                }
            }

            //DeQuantization + 2D IDCT & move whole (block) row into processbuf
            //2008/10/30 04:05¤U¤È
            /*{
                SInt16 *pSrc = pIDCTSrc;
                UInt8  *pYDst  = pIDCTYDst;
                UInt8  *pCbDst = pIDCTCbDst;
                UInt8  *pCrDst = pIDCTCrDst;
                if (!bMirror)
                {
                    for (int iMCU_H = 0; iMCU_H < iMCU_H_SIZE; iMCU_H++, 
                        pSrc+=256, pYDst+=16, pCbDst+=8, pCrDst+=8)
                    {
                        IDCTY(pSrc,     pYQTable, pYDst,   mp->pIDCTProcBuf, iIDCTYLineStep, bMirror);
                        IDCTY(pSrc+64,  pYQTable, pYDst+8, mp->pIDCTProcBuf, iIDCTYLineStep, bMirror);
                        IDCTC(pSrc+128, pCQTable, pCbDst,  mp->pIDCTProcBuf, iIDCTCLineStep, bMirror);
                        IDCTC(pSrc+192, pCQTable, pCrDst,  mp->pIDCTProcBuf, iIDCTCLineStep, bMirror);
                    }
                }
                else
                {
                    for (int iMCU_H = 0; iMCU_H < iMCU_H_SIZE; iMCU_H++,
                        pSrc+=256, pYDst-=16, pCbDst-=8, pCrDst-=8)
                    {
                        IDCTY(pSrc+64,  pYQTable, pYDst,   mp->pIDCTProcBuf, iIDCTYLineStep, bMirror);
                        IDCTY(pSrc,     pYQTable, pYDst+8, mp->pIDCTProcBuf, iIDCTYLineStep, bMirror);
                        IDCTC(pSrc+128, pCQTable, pCbDst,  mp->pIDCTProcBuf, iIDCTCLineStep, bMirror);
                        IDCTC(pSrc+192, pCQTable, pCrDst,  mp->pIDCTProcBuf, iIDCTCLineStep, bMirror);
                    }
                }
            }
            pIDCTYDst+=iIDCTYDstStep;
            pIDCTCbDst+=iIDCTCDstStep;
            pIDCTCrDst+=iIDCTCDstStep;*/
        }
        else
        {
            #if defined(__KERNEL_MODE__)
                RtlZeroMemory(pIDCTSrc,  iMCU_H_SIZE <<9);
            #else
                memset(pIDCTSrc, 0, iMCU_H_SIZE <<9);
            #endif
            SInt16 *pDeHuffPtr = mp->pDeHuffBuf;

            //Decode VLC to FLC
            for (int iMCU_H = 0; iMCU_H < iMCU_H_Cnt; iMCU_H++, pDeHuffPtr += 256)
            {
                //=====Y0=====
                if ((s = CJPEG_HuffDecode(pStreamBuf, pYDCHuffTable)) != 0)
                {
                    r = CJPEG_GetBits(pStreamBuf, s);
                    s = CJPEG_HuffExtend(r, s);
                }
                iLastDC[0] = (s += iLastDC[0]);
                pDeHuffPtr[0] = (short)s;

                for (k = 1; k < 64; k++)
                {
                    s = CJPEG_HuffDecode(pStreamBuf, pYACHuffTable);
                    r = s >> 4;
                    s &= 15;

                    if (s)
                    {
                        if (r)
                        {
                            if ((k + r) > 63)
                            {
                                mp->Report.iErrorCode = JPGD_DECODE_ERROR;
                                return;
                            }
                            k += r;
                        }
                        r = CJPEG_GetBits(pStreamBuf, s);
                    }
                    else
                    {
                        if (r == 15)
                        {
                            if (k > 48)
                            {
                                mp->Report.iErrorCode = JPGD_DECODE_ERROR;
                                return;
                            }
                            k += 15;
                        }
                        else    //EOB
                            break;
                    }
                }

                //=====Y1=====
                if ((s = CJPEG_HuffDecode(pStreamBuf, pYDCHuffTable)) != 0)
                {
                    r = CJPEG_GetBits(pStreamBuf, s);
                    s = CJPEG_HuffExtend(r, s);
                }
                iLastDC[0] = (s += iLastDC[0]);
                pDeHuffPtr[64] = (short)s;

                for (k = 1; k < 64; k++)
                {
                    s = CJPEG_HuffDecode(pStreamBuf, pYACHuffTable);
                    r = s >> 4;
                    s &= 15;

                    if (s)
                    {
                        if (r)
                        {
                            if ((k + r) > 63)
                            {
                                mp->Report.iErrorCode = JPGD_DECODE_ERROR;
                                return;
                            }
                            k += r;
                        }
                        r = CJPEG_GetBits(pStreamBuf, s);
                    }
                    else
                    {
                        if (r == 15)
                        {
                            if (k > 48)
                            {
                                mp->Report.iErrorCode = JPGD_DECODE_ERROR;
                                return;
                            }
                            k += 15;
                        }
                        else    //EOB
                            break;
                    }
                }

                //=====Cb=====
                if ((s = CJPEG_HuffDecode(pStreamBuf, pCDCHuffTable)) != 0)
                {
                    r = CJPEG_GetBits(pStreamBuf, s);
                    s = CJPEG_HuffExtend(r, s);
                }
                iLastDC[1] = (s += iLastDC[1]);
                pDeHuffPtr[128] = (short)s;

                for (k = 1; k < 64; k++)
                {
                    s = CJPEG_HuffDecode(pStreamBuf, pCACHuffTable);
                    r = s >> 4;
                    s &= 15;

                    if (s)
                    {
                        if (r)
                        {
                            if ((k + r) > 63)
                            {
                                mp->Report.iErrorCode = JPGD_DECODE_ERROR;
                                return;
                            }
                            k += r;
                        }
                        r = CJPEG_GetBits(pStreamBuf, s);
                    }
                    else
                    {
                        if (r == 15)
                        {
                            if (k > 48)
                            {
                                mp->Report.iErrorCode = JPGD_DECODE_ERROR;
                                return;
                            }
                            k += 15;
                        }
                        else    //EOB
                            break;
                    }
                }

                //=====Cr=====
                if ((s = CJPEG_HuffDecode(pStreamBuf, pCDCHuffTable)) != 0)
                {
                    r = CJPEG_GetBits(pStreamBuf, s);
                    s = CJPEG_HuffExtend(r, s);
                }
                iLastDC[2] = (s += iLastDC[2]);
                pDeHuffPtr[192] = (short)s;

                for (k = 1; k < 64; k++)
                {
                    s = CJPEG_HuffDecode(pStreamBuf, pCACHuffTable);
                    r = s >> 4;
                    s &= 15;

                    if (s)
                    {
                        if (r)
                        {
                            if ((k + r) > 63)
                            {
                                mp->Report.iErrorCode = JPGD_DECODE_ERROR;
                                return;
                            }
                            k += r;
                        }
                        r = CJPEG_GetBits(pStreamBuf, s);
                    }
                    else
                    {
                        if (r == 15)
                        {
                            if (k > 48)
                            {
                                mp->Report.iErrorCode = JPGD_DECODE_ERROR;
                                return;
                            }
                            k += 15;
                        }
                        else    //EOB
                            break;
                    }
                }
            }
        }
    }
	return;
}
//-----------------------------------------------------------------------------

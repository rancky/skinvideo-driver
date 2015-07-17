#ifndef __ScaleH__
#define __ScaleH__

#if !defined(__KERNEL_MODE__) && !defined(__LINUX__)
    #include "BaseType.h"
#endif
//---------------------------------------------------------------------------

#define iDstBlkSizePower   7
#define iDstBlkSize        128
#define iDstBlkSizePowerx2 iDstBlkSizePower*2

enum
{
    scalepTest,
    scalepCount
};

typedef struct
{
    int iBufHSize, iBufVSize;
    int iHStart,   iVStart;
    int iHiSize,   iViSize;
    int iHoSize,   iVoSize;
    int iSWSubSample;
    int iSrcBlkSize;
    int a32SWSubSample[2];
    int a32SrcBlkSize[2];
} ScaleParam;
//---------------------------------------------------------------------------

typedef struct
{
} ScaleReport;
//---------------------------------------------------------------------------

typedef struct _CScale
{
    ScaleParam  Param;
    ScaleReport Report;
    
    int iScaleLineIdx[iDstBlkSize];

    int aa32ScaleIdx[2][iDstBlkSize];
    int aa32ScaleTap[2][iDstBlkSize];

#if defined(__MMX__)
    UInt64 iVScaleTap64[iDstBlkSize][2];
    UInt16 iHScaleTap16[iDstBlkSize*2];
#endif
} CScale;

void CScale_CreateScalingTap(CScale *mp);
//---------------------------------------------------------------------------

CScale* CScale_CScale();
void CScale__CScale(CScale *mp);

void CScale_ReadScaleParamFromReg(CScale *mp, DWORD *pScaleParams);
void CScale_Reset(CScale *mp);
void CScale_WriteScaleParamToReg(CScale *mp, DWORD *pScaleParams);

ScaleParam  *CScale_GetParam(CScale *mp);
ScaleReport *CScale_GetReport(CScale *mp);

BOOL CScale_SetScaleSize(CScale *mp, int iBufHSize, int iBufVSize, int iHStart, int iVStart, int iHiSize, int iViSize, int iHoSize, int iVoSize, int a32SWSubSample[2], int a32SrcBlkSize[2]);
void CScale_SubSample(CScale *mp, UInt8 *pSrc, UInt8 *pDst);
void CScale_UpSample(CScale *mp, UInt8 *pSrc, UInt8 *pDst);
//---------------------------------------------------------------------------
#endif
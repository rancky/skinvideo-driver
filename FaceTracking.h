#ifndef __FaceH__
#define __FaceH__

#if !defined(__KERNEL_MODE__) && !defined(__LINUX__)
    #include "..\BaseType.h"
#endif

#define MAX_FACES			10	// 單次畫面可偵測人臉之上限值
#define MAX_STABLE_FACE		2	// 一段時間Stable 的人臉數
#define MAX_DIST_SIZE		248//213*cos + 160*sin	// 影像中兩點可處理之最大距離
#define SCALE_NUM			9
#define ROTATE_NUM			7
#define MaxTableNum			SCALE_NUM*ROTATE_NUM // max scale and rotate table number
#define MAX_FACES_RECORD	80
#define ProcessWidth		213 //160
#define ProcessHeight		160 //120
#define InputWidth			1616//20061208 modify
#define InputHeight			1216//20061208 modify
#define VGA_CIF				3		//4
#define WHITE				255
#define WAITING_FOR_MOTOR	1

#define MARK_FACE_LEN		10

#define PARAM_Smin			22.0f	//30.0f
#define PARAM_Smax			90.0f	//75.0f

#define PARAM_Rmin			-15
#define PARAM_Rmax			15
#define PARAM_Rres			5

const int CensusBase[9]		= {1,2,4,8,16,32,64,128,256};
const double Scale_Ratio[]	= {	22/PARAM_Smax ,
								22/PARAM_Smax + (22/PARAM_Smin - 22/PARAM_Smax)		/ 8.0,
								22/PARAM_Smax + (22/PARAM_Smin - 22/PARAM_Smax) * 2 / 8.0,
								22/PARAM_Smax + (22/PARAM_Smin - 22/PARAM_Smax) * 3 / 8.0,
								22/PARAM_Smax + (22/PARAM_Smin - 22/PARAM_Smax) * 4 / 8.0,
								22/PARAM_Smax + (22/PARAM_Smin - 22/PARAM_Smax) * 5 / 8.0,
								22/PARAM_Smax + (22/PARAM_Smin - 22/PARAM_Smax) * 6 / 8.0,
								22/PARAM_Smax + (22/PARAM_Smin - 22/PARAM_Smax) * 7 / 8.0,
								22/PARAM_Smin };
/*const float Rotate_Angle[]	= {	PARAM_Rmin, 
								PARAM_Rmin + PARAM_Rres,
								PARAM_Rmin + PARAM_Rres * 2,
								PARAM_Rmin + PARAM_Rres * 3,
								PARAM_Rmin + PARAM_Rres * 4,
								PARAM_Rmin + PARAM_Rres * 5,
								PARAM_Rmax};
*/
//const float Rotate_Angle[]	= {0, 5, -5,  10, -10, 15, -15};
//const float Rotate_Angle[]	= {5, 0, -5, -15, -10, 10,  15};
const float Rotate_Angle[]	= {-5, 0, 5, 10, 15, -15, -10};
typedef unsigned char UInt8;

enum
{
    ftpFaceTrackEnable,
	ftpMaxFaceRecord,					//	8
	ftpProcessWidth,					//	160
	ftpProcessHeight,					//	120
	ftpHX1num,							//	20
	ftpHX2num,							//	40
	ftpHX3num,							//	238
	ftpStableFaceThresholdRatio,		//	0.8
	ftpCenterTolerance,					//	5
	ftpZoomInPanStep,					//	2
	ftpZoomInTileStep,					//	2
	ftpZoomInZoomStep,					//	1
	ftpZoomOutZoomStep,					//	-4
	ftpZoomInWindowSize,				//	66
	ftpTargetCenterThreshold,			//	8
	ftpTargetZoomThreshold,				//	20
	ftpStage1Threshold,					//	90
	ftpStage2Threshold,					//	200
	ftpStage3Threshold,					//	770
	ftpStage4Threshold,					//	60
	ftpMaxFrameCount,					//	ftpMaxFaceRecord*5
	ftpMaxContinueMissCount,			//  ftpMaxFaceRecord*2
	ftpFaceVerticalPositionRatio,		//	0.5
	ftpDrawFaceRect,					//	false
	ftpFlipImage,						//	false
	ftpMirrorImage,						//	false
	ftpMotorEnable,
	ftpWeightThreshold,					//	4
	ftpContinuousThreshold,				//	4
	ftpNewFaceDisThreshold,				//	1.0

    ftpCount
};

struct Point
{
    int		x;
    int		y;
};

struct STRUCT_hx
{
    int     x;
    int     y;
    float   val[512];
};

struct ScaleRotateTable
{
    int   Iwidth;
    int   Iheight;
    float Rotate;
    float Scale;
    UInt8 *table;
};

struct STRUCT_DETECT_FACE
{
    int     x, y;
	int		abs_x,abs_y;
	int		center_x,center_y;
    int     h1_val;
    int     h2_val;
    int     h3_val;
    float   h4_val;
    float   rotate;
    float   scale;
};

struct STRUCT_STABLE_FACE
{
	Point	center;
	int		weight;
	int		continuous;
};

typedef struct
{
    BOOL	bFaceTrackEnable;

	int		iMaxFaceRecord;						//	8
	int		iProcessWidth;						//	160
	int		iProcessHeight;						//	120
	int		iHX1num;							//	20
	int		iHX2num;							//	40
	int		iHX3num;							//	238
	double	dStableFaceThresholdRatio;			//	0.8
	int		iCenterTolerance;					//	5
	int		iZoomInPanStep;						//	2
	int		iZoomInTileStep;					//	2
	int		iZoomInZoomStep;					//	1
	int		iZoomOutZoomStep;					//	-4
	int		iZoomInWindowSize;					//	66
	int		iTargetCenterThreshold;				//	8
	int		iTargetZoomThreshold;				//	20
	int		s1_thre;							//	90
	int		s2_thre;							//	200
	int		s3_thre;							//	770
	int		fs_thre;							//	60
	int		iMaxFrameCount;						//	ftpMaxFaceRecord*5
	int		iMaxContinueMissCount;				//  ftpMaxFaceRecord*2
	double	dFaceVerticalPositionRatio;			//	0.5
	BOOL	bDrawFaceRect;						//	false
	BOOL	bFlipImage;							//	false
	BOOL	bMirrorImage;						//	false
	BOOL	bMotorEnable;

	int		iWeightThreshold;					//	4
	int		iContinuousThreshold;				//	4
	double	dNewFaceDisThreshold;				//	1

	// Vincent [6/27/2006]   for move motor using UI param 
	int		iMT_STEP;
	int		iMT_PRD;
	int		iMT_PW;
}FaceParam;

typedef struct
{
    int	    oFace_num;						//face num
    int     zoom, pan, tile;				//report PTZ
	int		PCam_zoom,PCam_pan,PCam_tile;	//system PTZ

    STRUCT_DETECT_FACE *oFace;				//face information
} FaceReport;

typedef struct _CFace
{
    FaceParam  Param;
    FaceReport Report;

	int		s,r;
	int		s_start, r_start;
    int		count;		//system count
	Point	oldFace;	//
	bool	findframeCtrlDoing;
	int		MotorMove;
    int		tar_pan;
    int		tar_tile;
    int		tar_zoom;
	
// Vincent [6/21/2006]    for caculate recognition ratio
	int		TOTAL_FRAME;
	int		HaveFace;

    int		Face_count;
    int		Face_continue_miss_count;

    float	cos_table[180];
    float	sin_table[180];
    float	cos_sin_table[180];
    float	sin_sin_table[180];
    ScaleRotateTable m_ScaleRotateTable[MaxTableNum];

	UInt8	*inp_image;
	UInt8	*ip_srcImage, *ip_dstImage;
    UInt8	ip_image1[ProcessWidth*ProcessHeight];	//ip_srcImage
	UInt8	ip_image2[ProcessWidth*ProcessHeight];	//ip_dstImage

	bool	doSearchSF;
	UInt8	centerPt_count;
	bool	isFace;
	bool	noFace;
	int		frame_count;
	int		centerPt_x[MAX_FACES_RECORD];
	int		centerPt_y[MAX_FACES_RECORD];
	int		face_scaleID_count[SCALE_NUM];
	int		face_rotateID_count[ROTATE_NUM];

	int		pre_scale;
	int		pre_rotate;
	POINT	pre_c;

    short   rs_gamma[MAX_DIST_SIZE][MAX_DIST_SIZE];
    UInt8   r_image[MAX_DIST_SIZE][MAX_DIST_SIZE];

	int		dFace_num;	//maybe face num
    STRUCT_DETECT_FACE	dFace[MAX_FACES];

	int		sFace_num;
	int		sFace_Max;
	bool	sensitive;
	STRUCT_STABLE_FACE	Stable_Face[MAX_STABLE_FACE];
}CFace;

//-----------------------------------------------------------------------------
CFace* CFace_CFace();
void CFace__CFace(CFace *mp);

FaceParam  *CFace_GetParam(CFace *mp);
FaceReport *CFace_GetReport(CFace *mp);

void	CFace_WriteFaceTrackParamToReg(CFace *mp, DWORD* pFaceTrackParams);
void	CFace_ReadFaceTrackParamFromReg(CFace *mp, DWORD* pFaceTrackParams);
//-----------------------------------------------------------------------------

void	CFace_FaceTrack(CFace *mp, UInt8 *in_img);

void	CFace_CreateScaleRotateTable(CFace *mp, float rotate_angle,float scale_ratio,int TableNum);
void	CFace_ParseFaceLooktable(CFace *mp);
void	CFace_ScaleRotateImg(CFace *mp, int rotate, int scale, int *out_width, int *out_height);
void	CFace_ParseSingleImage(CFace *mp, int width, int height, int rotate, int scale);
void	CFace_Concatenation(CFace *mp, int width, int height);
int		CFace_Rotate2Origin(STRUCT_DETECT_FACE &face ,int width, int height);
int		CFace_Convert2FacePos(CFace *mp,STRUCT_DETECT_FACE *f_info);
void	CFace_FlipMirrorImage(CFace *mp);
void	CFace_InitProcessImage(CFace *mp);
int		find_median(int *data, int n);
void	CFace_searchStableFace(CFace *mp);
void	CFace_DrawFaceRect(CFace *mp, UInt8 *in_img);
void	CFace_FindFace(CFace *mp, UInt8 *in_img);
void	CFace_CheckMissFace(CFace *mp);
void	CFace_DoZoomOut(CFace *mp);
void	CFace_DoZoom(CFace *mp);
void	CFace_ReSetParam(CFace *mp);
void	CFace_Reset(CFace *mp);

//------------------------------------------------------------------------
void	CFace_FaceTrack_PTZ(CFace *mp, UInt8 *in_img);

//------------------------------------------------------------------------
void CFace_searchStableFace_PTZ(CFace *mp);
void CFace_ParseSingleImage_PTZ(CFace *mp, int width, int height, int rotate, int scale);
void CFace_ParseFaceLooktable_PTZ(CFace *mp);
void CFace_FindFace_PTZ(CFace *mp, UInt8 *in_img);

#endif
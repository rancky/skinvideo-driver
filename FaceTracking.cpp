#if defined(__LINUX__)
	#include <snxcam.h>
#elif defined(__KERNEL_MODE__)
	#include <sncam.h>
	#include "FaceData.h"
#else
	#include <math.h>
	#include <stdlib.h>
	#include <string.h>
	#include "FaceTracking.h"
	#include "FaceData.h"
#endif 

CFace* CFace_CFace()
{
	FLOATING FloatSave;
	NTSTATUS Ntstatus;
	START_FLOATING(FloatSave,Ntstatus);
	CHECK_FLOATING_BOOL(Ntstatus);

	CFace *mp;

	#if defined(__LINUX__)
	{
		mp = (CFace *)rvmalloc(sizeof(CFace));
		memset(&mp->Param,  0, sizeof(mp->Param));
		memset(&mp->Report, 0, sizeof(mp->Report));
	}
	#elif defined(__KERNEL_MODE__)
	{
		mp = (CFace *)ExAllocatePool(NonPagedPool, sizeof(CFace));
		RtlZeroMemory(&mp->Param,  sizeof(mp->Param));
		RtlZeroMemory(&mp->Report, sizeof(mp->Report));
	}
	#else
	{
		mp = (CFace *)malloc(sizeof(CFace));
		memset(&mp->Param,  0, sizeof(mp->Param));
		memset(&mp->Report, 0, sizeof(mp->Report));
	}
	#endif

	mp->tar_pan					= 0;
	mp->tar_tile				= 0;
	mp->tar_zoom				= 0;
	mp->count					= 0;
	mp->Face_count				= 0;
	mp->oldFace.x				= 0;
	mp->oldFace.y				= 0;
	mp->Face_continue_miss_count= 0;
	mp->centerPt_count			= 0;
	mp->isFace					= false;
	mp->doSearchSF				= false;
	mp->frame_count				= 0;
	mp->noFace					= false;
	mp->findframeCtrlDoing		= false;
	// Vincent [6/15/2006]   pause working when motor moving
	mp->MotorMove				= 0;

	mp->Report.pan	 			= 0;
	mp->Report.tile				= 0;
	mp->Report.zoom				= 0;

	// Vincent [6/23/2006]	
	mp->TOTAL_FRAME				= 0;
	mp->HaveFace				= 0;

	// Vincent [6/26/2006]   
	mp->pre_scale				= -1;
	mp->pre_rotate				= -1;

	mp->s_start					= 0;
	mp->r_start					= 0;
	mp->s						= -1;
	mp->r						= -1;	
	mp->Param.iProcessHeight	= 160;//120;
	mp->Param.iProcessWidth		= 213;//160;	

	float *COS_table			= mp->cos_table;
	float *SIN_table			= mp->sin_table;
	float *COS_SIN_table		= mp->cos_sin_table;
	float *SIN_SIN_table		= mp->sin_sin_table;
	for (int i=0 ; i<180 ; i++)
	{
		double theta	 = 0.017452f*(i-90);
		COS_table[i]	 = (float)cos(theta);
		SIN_table[i]	 = (float)sin(theta);
		COS_SIN_table[i] = COS_table[i] * SIN_table[i];
		SIN_SIN_table[i] = SIN_table[i] * SIN_table[i];
	}

	int TableNum = 0;
	for (int s = 0; s < SCALE_NUM; s++)
	{
		for (int r = 0; r < ROTATE_NUM; r++)
		{
			CFace_CreateScaleRotateTable(mp, Rotate_Angle[r], Scale_Ratio[s] ,TableNum);
			TableNum++;
		}
	}	

	memset(mp->face_scaleID_count, 0, sizeof(int)*SCALE_NUM);
	memset(mp->face_rotateID_count, 0, sizeof(int)*ROTATE_NUM);	

	// Vincent [7/3/2006]   for stable stable face 
	mp->sFace_Max = -1;
	mp->sFace_num = 0;
	mp->sensitive = false;
	memset(mp->Stable_Face, 0, sizeof(STRUCT_STABLE_FACE)*MAX_STABLE_FACE);
	
	END_FLOATING(FloatSave,Ntstatus);

	return mp;
}

void CFace__CFace(CFace *mp)
{	
	#if defined(__LINUX__)
	   for (int i=0;i<MaxTableNum;i++)
		   rvfree(mp->m_ScaleRotateTable[i].table);	
	   
	   rvfree(mp, sizeof(CFace));
	#elif defined(__KERNEL_MODE__)
	   for (int i=0;i<MaxTableNum;i++)
		   ExFreePool(mp->m_ScaleRotateTable[i].table);

	   ExFreePool(mp);
	#else
	   for (int i=0;i<MaxTableNum;i++)
		   free(mp->m_ScaleRotateTable[i].table);

	   free(mp);
	#endif
}

void CFace_Reset(CFace *mp)
{
	FaceParam *pParam = &mp->Param;
	
	pParam->iProcessWidth		= 213;
	pParam->iProcessHeight		= 160;

	mp->TOTAL_FRAME				= 0;
	mp->HaveFace				= 0;

	// Vincent [6/26/2006]   
	mp->pre_scale				= -1;
	mp->pre_rotate				= -1;

	// Vincent [7/3/2006]   for stable stable face 
	mp->sFace_Max = -1;
	mp->sFace_num = 0;
	mp->sensitive = false;
	memset(mp->Stable_Face, 0, sizeof(STRUCT_STABLE_FACE)*MAX_STABLE_FACE);
}

void CFace_ReSetParam(CFace *mp)
{
	mp->Face_count				= 0;
	mp->oldFace.x				= 0;
	mp->oldFace.y				= 0;
	mp->Face_continue_miss_count= 0;
	mp->centerPt_count			= 0;
	mp->isFace					= false;
	mp->doSearchSF				= false;
	mp->noFace					= false;
	mp->findframeCtrlDoing		= false;
	// Vincent [6/15/2006]   pause working when motor moving
	mp->MotorMove				= 0;
	
	mp->Report.pan				= 0;
	mp->Report.tile				= 0;
	mp->Report.zoom				= 0;
	// Vincent [6/23/2006]	
	mp->TOTAL_FRAME				= 0;
	mp->HaveFace				= 0;
	
	mp->tar_pan					= 0;
	mp->tar_tile				= 0;
	mp->tar_zoom				= 0;
	mp->Report.PCam_pan			= 0;
	mp->Report.PCam_tile		= 0;
	mp->Report.PCam_zoom		= 0;

	mp->s_start					= 0;
	mp->r_start					= 0;

	// Vincent [6/26/2006]   
	mp->pre_scale				= -1;
	mp->pre_rotate				= -1;
	mp->pre_c.x					= -1;
	mp->pre_c.y					= -1;

	memset(mp->face_scaleID_count, 0, sizeof(int)*SCALE_NUM);
	memset(mp->face_rotateID_count, 0, sizeof(int)*ROTATE_NUM);	

	// Vincent [7/3/2006]   for stable stable face 
	mp->sFace_Max = -1;
	mp->sFace_num = 0;
	mp->sensitive = false;
	memset(mp->Stable_Face, 0, sizeof(STRUCT_STABLE_FACE)*MAX_STABLE_FACE);
}

void CFace_FlipMirrorImage(CFace *mp)
{
	if ( !mp->Param.bFlipImage && !mp->Param.bMirrorImage )
		return;

	UInt8	*src,*dst;
	int		line_step;

	line_step	= (mp->Param.bFlipImage) ? -mp->Param.iProcessWidth : mp->Param.iProcessWidth;
	src			= mp->ip_srcImage;
	dst			= (mp->Param.bFlipImage) ? (mp->ip_dstImage + (mp->Param.iProcessHeight-1)*mp->Param.iProcessWidth) : mp->ip_dstImage;

	for (int y=0 ; y<mp->Param.iProcessHeight ; y++)
	{
		if (!mp->Param.bMirrorImage)
			memcpy(dst, src, mp->Param.iProcessWidth);
		else
		{
			for (int x=0 ; x<mp->Param.iProcessWidth ; x++)
				dst[x] = src[mp->Param.iProcessWidth - x - 1];
		}		
		src += mp->Param.iProcessWidth;
		dst += line_step;
	}

	UInt8 *ip_temp;
	ip_temp			= mp->ip_srcImage;
	mp->ip_srcImage	= mp->ip_dstImage;
	mp->ip_dstImage = ip_temp;
}

void CFace_InitProcessImage(CFace *mp)
{
	mp->ip_srcImage = mp->ip_image1;
	
	for (int y=0 ; y<mp->Param.iProcessHeight ; y++)
	{
		for (int x=0 ; x<mp->Param.iProcessWidth ; x++)
		{
			int addr = min(max((x+y*InputWidth) * VGA_CIF, 1), InputWidth*InputHeight-2);

			mp->ip_srcImage[y*mp->Param.iProcessWidth+x] = 
				(mp->inp_image[addr-1] + mp->inp_image[addr] + mp->inp_image[addr+1])/3;
		}
	}
	mp->ip_dstImage = mp->ip_image2;
}

int find_median(int *data, int n)
{
	int	i, less, greater, equal;
	int min, max, guess, maxltguess, mingtguess;

	min = max = data[0] ;
	for (i=1 ; i<n ; i++)
	{
		if ( data[i]<min ) 
			min = data[i];
		if ( data[i]>max ) 
			max = data[i];
	}

	while (1) 
	{
		guess		= (min+max)/2;
		less		= 0; 
		greater		= 0; 
		equal		= 0;
		maxltguess	= min;
		mingtguess	= max;
		for (i=0; i<n; i++) 
		{
			if (data[i]<guess) 
			{
				less++;
				if (data[i]>maxltguess) 
					maxltguess = data[i];
			} 
			else if (data[i]>guess) 
			{
				greater++;
				if (data[i]<mingtguess) 
					mingtguess = data[i];
			} 
			else equal++;
		}
		if (less <= (n+1)/2 && greater <= (n+1)/2) 
			break ; 
		else if (less>greater) 
			max = maxltguess;
		else 
			min = mingtguess;
	}

	if (less >= (n+1)/2) 
		return maxltguess;
	else if (less+equal >= (n+1)/2) 
		return guess;
	else 
		return mingtguess;	
}

void CFace_searchStableFace(CFace *mp)
{
	FLOATING FloatSave;
	NTSTATUS Ntstatus;
	START_FLOATING(FloatSave,Ntstatus);
	CHECK_FLOATING_VOID(Ntstatus);

	int x_min, x_max, y_min, y_max, i;

	//1. find median x,y
	int median_x = find_median(mp->centerPt_x, mp->centerPt_count);
	int median_y = find_median(mp->centerPt_y, mp->centerPt_count);

	x_min = median_x - mp->Param.iCenterTolerance;
	x_max = median_x + mp->Param.iCenterTolerance;
	y_min = median_y - mp->Param.iCenterTolerance;
	y_max = median_y + mp->Param.iCenterTolerance;

	// 2. find mean x,y	
	int count = 0;
	int x_sum = 0;
	int y_sum = 0;
	for (i=0 ; i<mp->centerPt_count ; i++)
	{
		int cx = mp->centerPt_x[i];
		int cy = mp->centerPt_y[i];
		if ((cx < x_min) || (cx > x_max))
			continue;
		if ((cy < y_min) || (cy > y_max))
			continue;

		x_sum += cx;
		y_sum += cy;
		count++;
	}

	// Vincent [6/30/2006]   if face_scaleID_count  have > 2 scale face, skip it
	int scale_count = 0;
	for (i=0 ; i<SCALE_NUM ; i++)
	{
		if (mp->face_scaleID_count[i] > 0)
			scale_count++;
	}

	int threshold = (int) (mp->Param.iMaxFaceRecord * mp->Param.dStableFaceThresholdRatio);
	if (count<threshold || count==0 || scale_count>2)
	{
		mp->isFace					= false;
		mp->centerPt_count			= 0;
		mp->doSearchSF				= false;
		mp->s_start					= 0;
		mp->r_start					= 0;
		
		memset(mp->face_scaleID_count, 0, sizeof(int)*SCALE_NUM);
		memset(mp->face_rotateID_count, 0, sizeof(int)*ROTATE_NUM);
		
		END_FLOATING(FloatSave,Ntstatus);
		
		return;
	}

	// 3. find scale index
	int index			= 0;
	int max_scale_count = mp->face_scaleID_count[0];
	for (i=1;i<SCALE_NUM;i++)
	{
		// Vincent [6/27/2006]   filter noise face 
		if (mp->face_scaleID_count[i] == max_scale_count)
		{
			if (i == mp->pre_scale)
			{
				index			= i;
				max_scale_count = mp->face_scaleID_count[i];
			}
			if (index == mp->pre_scale)
				continue;
		}

		else if ( mp->face_scaleID_count[i] > max_scale_count)
		{
			index			= i;
			max_scale_count = mp->face_scaleID_count[i];
		}
	}

	mp->pre_scale	= index;

/*	// Vincent [6/30/2006]   use max rotate num
	int index2				= 0;;
	int max_rotate_count	= mp->face_rotateID_count[0];
	for (i=1;i<ROTATE_NUM;i++)
	{
		if ( mp->face_rotateID_count[i] > max_rotate_count)
		{
			index2			= i;
			max_rotate_count = mp->face_rotateID_count[i];
		}
	}
	mp->pre_rotate	= index2;
*/
	// 4. set report
	mp->isFace		= true;

	STRUCT_DETECT_FACE	*oFace = mp->Report.oFace;
	if(oFace == NULL)
	{
		DbgPrint("james : oFace == NULL \n");
		
		mp->isFace					= false;
		mp->centerPt_count			= 0;
		mp->doSearchSF				= false;
		mp->s_start					= 0;
		mp->r_start					= 0;
		
		memset(mp->face_scaleID_count, 0, sizeof(int)*SCALE_NUM);
		memset(mp->face_rotateID_count, 0, sizeof(int)*ROTATE_NUM);

		END_FLOATING(FloatSave,Ntstatus);
		return;
	}

	oFace->center_x	= (int) (x_sum / (float) count);
	oFace->center_y	= (int) (y_sum / (float) count);
	oFace->scale	= Scale_Ratio[index];
	//oFace->rotate	= Rotate_Angle[index2];
	// Vincent [6/30/2006]	
	mp->pre_c.x		= oFace->center_x;
	mp->pre_c.y		= oFace->center_y;
	
	// Vincent [7/3/2006]  find stable  stable face 
	if (mp->sFace_Max == -1)
	{
		mp->Stable_Face[0].center.x = oFace->center_x;
		mp->Stable_Face[0].center.y	= oFace->center_y;
		mp->Stable_Face[0].weight	= 1;
		mp->sFace_Max				= 0;
		mp->sFace_num				= 1;
	}
	else
	{
		bool hit = false;
		for (i=0 ; i<mp->sFace_num ; i++)
		{
			mp->Stable_Face[i].weight = min(max(mp->Stable_Face[i].weight, 0), 50);
			
			if (hit)
			{
				mp->Stable_Face[i].continuous = 0;
				continue;
			}

			if (abs(oFace->center_x-mp->Stable_Face[i].center.x)+abs(oFace->center_y-mp->Stable_Face[i].center.y) > 
				(22/oFace->scale)*mp->Param.dNewFaceDisThreshold)
			{
				mp->Stable_Face[i].continuous = 0;
				
				if (i+1 == mp->sFace_num)	//new stable face
				{
					mp->Stable_Face[(MAX_STABLE_FACE-mp->sFace_Max)-1].center.x = oFace->center_x;
					mp->Stable_Face[(MAX_STABLE_FACE-mp->sFace_Max)-1].center.y	= oFace->center_y;
					mp->Stable_Face[(MAX_STABLE_FACE-mp->sFace_Max)-1].weight	= 1;

					if (mp->sFace_num < MAX_STABLE_FACE)
						mp->sFace_num	++;
				}
				else
				{
					continue;
				}
			}
			else
			{
				mp->Stable_Face[i].continuous++;

				if (mp->sFace_num == 1)
					mp->Stable_Face[i].weight += 1;
				else
				{
					mp->Stable_Face[i].weight += mp->Stable_Face[i].continuous;
					mp->Stable_Face[mp->sFace_num-i-1].weight -= 2;
				}

				hit	= true;

				if (mp->sFace_Max==i && mp->Stable_Face[i].continuous >=mp->Param.iContinuousThreshold)
					mp->sensitive = false;
				else
					mp->sensitive = true;
			}
		}
/*
		TRACE("STABLE 0...weight=%3d continuous=%3d\nSTABLE 1...weight=%3d continuous=%3d\n\n",
			mp->Stable_Face[0].weight, mp->Stable_Face[0].continuous, 
			mp->Stable_Face[1].weight, mp->Stable_Face[1].continuous);
*/
		int max_weight	= mp->Stable_Face[0].weight;
		mp->sFace_Max	= 0;
		for (i=1 ; i<mp->sFace_num ; i++)
		{
			if (mp->Stable_Face[i].weight > max_weight)
			{
				mp->sFace_Max	= i;
				max_weight		= mp->Stable_Face[i].weight;
			}
		}
	}

/*
TRACE("=======> (%3d,%3d)  %.3f  %.0f\n", 
		oFace->center_x, oFace->center_y, oFace->scale, oFace->rotate);

FILE *fp = fopen("d:\\StableFace.CSV","a");
fprintf(fp, "%d,%d,%f\n", 
		oFace->center_x, oFace->center_y, oFace->scale);
fclose(fp);
*/
	// 5. reset parameters
	mp->centerPt_count				= 0;
	mp->doSearchSF					= false;
	mp->frame_count					= 0;

	memset(mp->face_scaleID_count, 0, sizeof(int)*SCALE_NUM);

	END_FLOATING(FloatSave,Ntstatus);
}

void CFace_DrawFaceRect(CFace *mp, UInt8 *in_img)
{
	if (mp->Report.oFace_num == 0)
		return;

	FLOATING FloatSave;
	NTSTATUS Ntstatus;
	START_FLOATING(FloatSave,Ntstatus);
	CHECK_FLOATING_VOID(Ntstatus);

	int center_x = mp->Report.oFace->center_x * VGA_CIF;
	int center_y = mp->Report.oFace->center_y * VGA_CIF; 
	int size	 = (int) (22/mp->Report.oFace->scale)*VGA_CIF;
	int bottom   = min(center_y+size/2 , InputHeight);
	int top	  = max(center_y-size/2 , 0);
	int right	= min(center_x+size/2 , InputWidth);
	int left	 = max(center_x-size/2 , 0); 
	int i;
	for (i=1 ; i<MARK_FACE_LEN ; i++)
	{
		in_img[bottom*InputWidth	 +left+i]  = WHITE;
		in_img[(bottom-i)*InputWidth +left]	= WHITE;

		in_img[bottom*InputWidth	 +right-i] = WHITE;
		in_img[(bottom-i)*InputWidth +right]   = WHITE;

		in_img[top*InputWidth		+left+i]  = WHITE;
		in_img[(top+i)*InputWidth	+left]	= WHITE;

		in_img[top*InputWidth		+right-i] = WHITE;
		in_img[(top+i)*InputWidth	+right]   = WHITE;
	}

	if (mp->isFace)
	{
		for (i=0;i<21;i++)
		{
			for (int j=0;j<21;j++)
				in_img[(center_y + i - 10)*InputWidth + (center_x + j - 10)]  = WHITE;			
		}
		mp->isFace		= false;
	}

	END_FLOATING(FloatSave,Ntstatus);
}

void CFace_FindFace(CFace *mp, UInt8 *in_img)
{	
	if( !mp->findframeCtrlDoing )
	{
		mp->inp_image = in_img;
		mp->frame_count ++;
		CFace_InitProcessImage(mp);
		CFace_FlipMirrorImage(mp);
		CFace_ParseFaceLooktable(mp);
	}
	else
		CFace_ParseFaceLooktable(mp);
}

void CFace_CheckMissFace(CFace *mp)
{	
		// Vincent [6/26/2006]   
	if (mp->Face_continue_miss_count > 2)
	{
		mp->pre_scale					= -1;
		mp->pre_c.x						= -1;
		mp->pre_c.y						= -1;		
	}
	
	if ((mp->frame_count > mp->Param.iMaxFrameCount) || (mp->Face_continue_miss_count > mp->Param.iMaxContinueMissCount))
	{	
		mp->noFace						= true;
		mp->isFace						= false;
		mp->centerPt_count				= 0;
		mp->doSearchSF					= false;
		mp->Face_continue_miss_count	= 0;
		mp->frame_count					= 0;
		mp->tar_pan						= 0;
		mp->tar_tile					= 0;
		mp->tar_zoom					= 0;
		mp->s_start						= 0;
		mp->r_start						= 0;
		
		memset(mp->face_scaleID_count, 0, sizeof(int)*SCALE_NUM);
		memset(mp->face_rotateID_count, 0, sizeof(int)*ROTATE_NUM);
		

		// Vincent [7/3/2006]   for stable stable face 
		mp->sFace_Max = -1;
		mp->sFace_num = 0;
		mp->sensitive = false;
		memset(mp->Stable_Face, 0, sizeof(STRUCT_STABLE_FACE)*MAX_STABLE_FACE);
	}	
}

void CFace_DoZoomOut(CFace *mp)
{	
	mp->Report.zoom = mp->Param.iZoomOutZoomStep;
	if(mp->Report.PCam_zoom == 0)
	{
		mp->noFace		= false;	
		mp->Report.pan	= 0;
		mp->Report.tile	= 0;
		mp->Report.zoom	= 0;
		return;
	}
}

void CFace_DoZoom(CFace *mp)
{	
	int	pan, tile, zoom;

	if (mp->tar_pan	> (mp->Report.PCam_pan + mp->Param.iZoomInPanStep))
		pan = mp->Param.iZoomInPanStep;
	else if (mp->tar_pan < (mp->Report.PCam_pan - mp->Param.iZoomInPanStep))
		pan = -mp->Param.iZoomInPanStep;
	else
		pan = 0;

	if (mp->tar_tile > (mp->Report.PCam_tile + mp->Param.iZoomInTileStep))
		tile = mp->Param.iZoomInTileStep;
	else if (mp->tar_tile < (mp->Report.PCam_tile - mp->Param.iZoomInTileStep))
		tile = -mp->Param.iZoomInTileStep;
	else
		tile = 0;

	if (mp->tar_zoom > (mp->Report.PCam_zoom + mp->Param.iZoomInZoomStep))
		zoom = mp->Param.iZoomInZoomStep;
	else if (mp->tar_zoom < (mp->Report.PCam_zoom - mp->Param.iZoomInZoomStep))
		zoom = -mp->Param.iZoomInZoomStep;
	else
		zoom = 0;

	mp->Report.zoom	= zoom;
	mp->Report.pan	= pan;
	mp->Report.tile = tile;
}

void CFace_FaceTrack(CFace *mp, UInt8 *in_img)
{
	//if (!mp->Param.bFaceTrackEnable)
	// bFaceTrackEnable = 0 , 1 => disable
	// bFaceTrackEnable = 2		=> enable
	if (mp->Param.bFaceTrackEnable != 2)
	{
		CFace_ReSetParam(mp);
		return;
	}

	if (mp->centerPt_count	== mp->Param.iMaxFaceRecord)
		mp->doSearchSF	= true;

	if ( mp->count != 0 && !mp->doSearchSF )
		CFace_FindFace(mp,in_img);

	if ( mp->doSearchSF )
	{
		CFace_searchStableFace(mp);

		if (mp->Stable_Face[mp->sFace_Max].weight >= mp->Param.iWeightThreshold)
		{
			//mp->Stable_Face[mp->sFace_Max].weight = 2;

			int pan, tile, zoom, v_offset;

			v_offset = (int) ((22/mp->Report.oFace->scale)*VGA_CIF*mp->Param.dFaceVerticalPositionRatio);
			if (mp->Param.bFlipImage)
				v_offset = -v_offset;

			pan	 = mp->Report.oFace->center_x * VGA_CIF - 320;
			tile = mp->Report.oFace->center_y * VGA_CIF - (240 - v_offset);
			float z	 = min(((mp->Param.iProcessHeight*mp->Report.oFace->scale) / mp->Param.iZoomInWindowSize) , 4);
			zoom = (int) (z * 32);

			if (!mp->sensitive)
			{
				if (	(abs(mp->oldFace.x - mp->Report.oFace->center_x) > mp->Param.iTargetCenterThreshold)
					||  (abs(mp->oldFace.y - mp->Report.oFace->center_y) > mp->Param.iTargetCenterThreshold) 
					||  (abs(zoom - mp->tar_zoom) > mp->Param.iTargetZoomThreshold ))
				{
					mp->tar_zoom	= zoom;
					mp->tar_pan		= pan;
					mp->tar_tile	= tile;
					mp->oldFace.x	= mp->Report.oFace->center_x;
					mp->oldFace.y	= mp->Report.oFace->center_y;
				}
			}
		}
	}

	if ( mp->Param.bDrawFaceRect )
		CFace_DrawFaceRect(mp, in_img);

	CFace_CheckMissFace(mp);

	if (mp->noFace)
		CFace_DoZoomOut(mp);
	else
		CFace_DoZoom(mp);

	if (mp->Param.iZoomInZoomStep == 0)
		CFace_DoZoomOut(mp);
}

void CFace_CreateScaleRotateTable(CFace *mp, float rotate_angle,float scale_ratio,int TableNum)
{
	float *COS_table	 = mp->cos_table;
	float *SIN_table	 = mp->sin_table;
	float *COS_SIN_table = mp->cos_sin_table;
	float *SIN_SIN_table = mp->sin_sin_table;
	ScaleRotateTable *RS_table  = &mp->m_ScaleRotateTable[TableNum];
	UInt8 *data_pointer;

	int out_Swidth   = (int)(mp->Param.iProcessWidth  * scale_ratio);
	int out_Sheight  = (int)(mp->Param.iProcessHeight * scale_ratio);
	int theta		= (int)rotate_angle + 90;
	int abs_theta	= (int)fabs(rotate_angle) + 90;
	int out_Rwidth   = (int)(out_Swidth *COS_table[abs_theta] +
							 out_Sheight*SIN_table[abs_theta]);
	int out_Rheight  = (int)(out_Swidth *SIN_table[abs_theta] +
							 out_Sheight*COS_table[abs_theta]);

	RS_table->Iwidth  = out_Rwidth;
	RS_table->Iheight = out_Rheight;
	RS_table->Scale   = scale_ratio;
	RS_table->Rotate  = rotate_angle;
	
#if defined(__LINUX__)
	RS_table->table = (UInt8 *)rvmalloc(sizeof(UInt8)*out_Rwidth*out_Rheight*2);	
#elif defined(__KERNEL_MODE__)
	RS_table->table = (UInt8 *)ExAllocatePool(NonPagedPool,sizeof(UInt8)*out_Rwidth*out_Rheight*2);
#else
	RS_table->table = (UInt8 *)malloc(sizeof(UInt8)*out_Rwidth*out_Rheight*2);
#endif
	
	data_pointer	= RS_table->table;

	float baseX , baseY , floatX , floatY;
	if (rotate_angle > 0)
	{
		baseX = -out_Sheight * COS_SIN_table[theta];
		baseY =  out_Sheight * SIN_SIN_table[theta];
	}
	else
	{
		baseX = out_Swidth * SIN_SIN_table[theta];
		baseY = out_Swidth * COS_SIN_table[theta];
	}

	float cos_data = COS_table[theta];
	float sin_data = SIN_table[theta];
	for (int y=0 ; y<out_Rheight ; y++)
	{
		baseX   = baseX + sin_data;
		baseY   = baseY + cos_data;
		floatX  = baseX + sin_data;
		floatY  = baseY + cos_data;
		for (int x=0 ; x<out_Rwidth ; x++)
		{
			floatX += cos_data;
			floatY -= sin_data;

			if((floatX<0) || (floatX>=out_Swidth) || (floatY<0) || (floatY>=out_Sheight))
			{
				*(data_pointer) = WHITE;// null, no mapping
				data_pointer++;
				*(data_pointer) = WHITE;
				data_pointer++;
			}
			else
			{
				int x1 = (int)(floatX/scale_ratio);
				int y1 = (int)(floatY/scale_ratio);
				*(data_pointer) = (UInt8)x1;// x first
				data_pointer++;
				*(data_pointer) = (UInt8)y1; // then y
				data_pointer++;
			}
		}
	}
}

void CFace_ParseFaceLooktable(CFace *mp)
{
	mp->Report.oFace_num = 0;

	if( !mp->findframeCtrlDoing )
	{
		mp->findframeCtrlDoing	= true;
		mp->s					= mp->s_start;
	}

	if (mp->r == -1)
		mp->r = mp->r_start;

	int r_width, r_height;

	mp->TOTAL_FRAME++;
//	TRACE("%d %d\n",s,r);
	CFace_ScaleRotateImg(mp, mp->r, mp->s, &r_width, &r_height);
	CFace_ParseSingleImage(mp, r_width, r_height, mp->r, mp->s);

	if ( mp->dFace_num )
	{
		mp->HaveFace++;
		float tmp = (float) mp->HaveFace/mp->TOTAL_FRAME;
//TRACE("Ratio= %.4f\n",tmp);
		mp->s_start						= min(max(mp->s-1,0), SCALE_NUM);
		mp->r_start						= min(max(mp->r-1,0), ROTATE_NUM);
		mp->findframeCtrlDoing			= false;
		mp->Face_continue_miss_count	= 0;
		mp->r = -1;
		return;
	}
	mp->r++;
	if (mp->r == ROTATE_NUM)
		mp->r = -1;

	mp->s++;
	if (mp->s == SCALE_NUM)
	{
		mp->s_start					= 0;
		mp->r_start					= 0;
		mp->findframeCtrlDoing		= false;
		mp->Face_continue_miss_count ++;
//TRACE("/*****************************************\\n");
	}
}

void CFace_ScaleRotateImg(CFace *mp, int rotate, int scale, int *out_width, int *out_height)
{	
	int TableNum = scale*ROTATE_NUM + rotate;

	ScaleRotateTable *RS_table  = &mp->m_ScaleRotateTable[TableNum];

	*out_width  = RS_table->Iwidth;
	*out_height = RS_table->Iheight;

	UInt8 *pointer   = RS_table->table;
	UInt8 *INP_image = mp->ip_srcImage;

	for (int y=0 ; y<*out_height ; y++)
	{
		for (int x=0 ; x<*out_width ; x++)
		{
			int PositionX = *(pointer);
			pointer++;
			int PositionY = *(pointer);
			pointer++;

			if(PositionX == WHITE)
			{
				mp->r_image[x][y] = WHITE;	  // 多出來的空間預設為白色
			}
			else
			{
				mp->r_image[x][y] = INP_image[PositionX+PositionY*mp->Param.iProcessWidth];	//I420
			}
		}
	}
}

void CFace_ParseSingleImage(CFace *mp, int width, int height, int rotate, int scale)
{
	FLOATING FloatSave;
	NTSTATUS Ntstatus;
	START_FLOATING(FloatSave,Ntstatus);
	CHECK_FLOATING_VOID(Ntstatus);

	// Vincent [6/30/2006]	
	if ((mp->pre_scale != -1 && abs(scale-mp->pre_scale)>1)	/*||
		(mp->pre_rotate!= -1 && fabs(Rotate_Angle[rotate]-Rotate_Angle[mp->pre_rotate])>10)*/)
	{
		mp->dFace_num = 0;
		return;
	}

	// shawn 2010/09/15 fix crash bug for **crash point** +++++
	//static int x_start	= 0;
	//static int y_start	= 0;
	static int x_start	= 1;
	static int y_start	= 1;
	// shawn 2010/09/15 fix crash bug for **crash point** -----

	int		h1, h2, h3, dFace_num;
	float	h4, h_min;

	dFace_num	= 0;
	h_min		= 9999;

	CFace_Concatenation(mp, width, height);	

	FaceParam *pParam = &mp->Param;	

	for (int x=x_start; x<width-22 ; x+=2)
	{
		for (int y=y_start; y<height-22 ; y+=2)
		{
			int i, r;

			h1 = h2 = h3 = 0;
			h4 = 0.0;

			for (i=0 ; i<pParam->iHX1num ; i++)
			{
				r  =  mp->rs_gamma[x + hx1[i].x][y + hx1[i].y];
				h1 += (int)hx1[i].val[r];
			}
			if (h1 > pParam->s1_thre)
				continue;

			for (i=0 ; i<pParam->iHX2num ; i++)
			{
				r  =  mp->rs_gamma[x + hx2[i].x][y + hx2[i].y];
				h2 += (int)hx2[i].val[r];
			}
			if (h2 > pParam->s2_thre)
				continue;

			for (i=0 ; i<pParam->iHX3num ; i++)
			{
				r  =  mp->rs_gamma[x + hx3[i].x][y + hx3[i].y];
				h3 += (int)hx3[i].val[r];
			}
			if (h3 > pParam->s3_thre)
				continue;

			for (int x1=0 ; x1<22 ; x1++)
			{
				for (int y1=0 ; y1<22 ; y1++)
				{
					r  =  mp->rs_gamma[x+x1][y+y1];
					h4 += hx4[x1][y1][r];	// shawn 2010/09/15 **crash point**
				}
			}
			if (h4>pParam->fs_thre || h4<0)
				continue;

			if (h1+h2+h3+h4 < h_min)
				h_min = h1+h2+h3+h4;
			else
				continue;

			mp->dFace[dFace_num].x	  = x;
			mp->dFace[dFace_num].y	  = y;
			mp->dFace[dFace_num].scale  = Scale_Ratio[scale];
			mp->dFace[dFace_num].rotate = Rotate_Angle[rotate];
			mp->dFace[dFace_num].h1_val = h1;
			mp->dFace[dFace_num].h2_val = h2;
			mp->dFace[dFace_num].h3_val = h3;
			mp->dFace[dFace_num].h4_val = h4;

			dFace_num++;

			if (dFace_num >= MAX_FACES)
				break;
		}
		if (dFace_num >= MAX_FACES)
			break;
	}

	if (dFace_num >= 1)
	{
		STRUCT_DETECT_FACE *dFace = &mp->dFace[dFace_num-1];

		if (!CFace_Rotate2Origin(*dFace, mp->Param.iProcessWidth, mp->Param.iProcessHeight))
			return;			

		if (!CFace_Convert2FacePos(mp, dFace))
			return;

		if (pParam->bFlipImage)
			dFace->center_y = pParam->iProcessHeight - dFace->center_y;
		if (pParam->bMirrorImage)
			dFace->center_x = pParam->iProcessWidth - dFace->center_x;

		mp->centerPt_x[mp->centerPt_count]	= dFace->center_x;
		mp->centerPt_y[mp->centerPt_count]	= dFace->center_y;
		mp->face_scaleID_count[scale] ++;
		mp->centerPt_count ++;
		// Vincent [6/30/2006]   
		mp->face_rotateID_count[rotate] ++;

		// Vincent [6/30/2006]	
		if (mp->pre_c.x!=-1 && mp->pre_c.y!=-1)
		{
			if (abs(dFace->center_x-mp->pre_c.x)+abs(dFace->center_y-mp->pre_c.y) > 100)
			{
				mp->dFace_num = 0;
				return;
			}
		}

		mp->Report.oFace	 = dFace;
		mp->Report.oFace_num = 1;
		dFace_num = 1;

		// Vincent [7/4/2006]   
		if (mp->sFace_Max != -1)
		{
			if (mp->Stable_Face[mp->sFace_Max].weight >=4 &&
				abs(dFace->center_x-mp->Stable_Face[mp->sFace_Max].center.x)+
				abs(dFace->center_y-mp->Stable_Face[mp->sFace_Max].center.y) < (22/dFace->scale)*mp->Param.dNewFaceDisThreshold)
			{
				mp->sensitive	= true;
				mp->frame_count	= 0;
			}
		}
/*
TRACE("(%3d,%3d)  %.3f  %.0f\n", 
		dFace->center_x, dFace->center_y, dFace->scale, dFace->rotate);

		FILE *fp = fopen("d:\\EveryFace.CSV","a");
		fprintf(fp, "%d,%d,%d,%f,%f,%f,%d,%d\n", 
			dFace->h1_val, dFace->h2_val, dFace->h3_val, dFace->h4_val, 
			dFace->rotate, dFace->scale, dFace->center_x, dFace->center_y);
		fclose(fp);
*/
	}
	mp->dFace_num = dFace_num;

	// shawn 2010/09/15 fix crash bug for **crash point** +++++
	//x_start = (dFace_num) ? max(mp->dFace[0].x - 2,0) : 0;
	//y_start = (dFace_num) ? max(mp->dFace[0].y - 2,0) : 0;
	x_start = (dFace_num) ? max(mp->dFace[0].x - 2,1) : 1;
	y_start = (dFace_num) ? max(mp->dFace[0].y - 2,1) : 1;
	// shawn 2010/09/15 fix crash bug for **crash point** -----

	END_FLOATING(FloatSave,Ntstatus);
}


void CFace_Concatenation(CFace *mp, int width, int height)
{
	FLOATING FloatSave;
	NTSTATUS Ntstatus;
	START_FLOATING(FloatSave,Ntstatus);
	CHECK_FLOATING_VOID(Ntstatus);
	
	int i1, i2, i3, ctr, val, I_mean;
	bool begin;

	for (int y=1 ; y<height-1 ; y++)
	{
		begin=1;

		for (int x=1 ; x<width-1 ; x++)
		{	
			switch (begin)
			{
			case 1 :
				i1	  = mp->r_image[x-1][y-1] + mp->r_image[x-1][y] + mp->r_image[x-1][y+1];
				i2	  = mp->r_image[x][y-1]   + mp->r_image[x][y]   + mp->r_image[x][y+1];
				i3	  = mp->r_image[x+1][y-1] + mp->r_image[x+1][y] + mp->r_image[x+1][y+1];
				I_mean  = i1+i2+i3;
				begin   = 0;
				break;

			case 0:
				I_mean  =  i2+i3;
				i2	  =  i3;
				i3	  =  mp->r_image[x+1][y-1] + mp->r_image[x+1][y] + mp->r_image[x+1][y+1];
				I_mean  += i3;
				break;
			}			
			
			I_mean = (int) (I_mean/9.0f);

			ctr = 0;
			val = 0;
			for (int i=x-1 ; i<=x+1 ; i++)
			{
				for (int j=y-1 ; j<=y+1 ; j++)
				{
					if (I_mean < mp->r_image[i][j])
						val += CensusBase[ctr];
					ctr ++;
				}
			}
			mp->rs_gamma[x][y] = val;
		}
	}

	END_FLOATING(FloatSave,Ntstatus);
}

// 由旋轉座標回轉至原始座標
BOOL CFace_Rotate2Origin(STRUCT_DETECT_FACE &face ,int width, int height)
{
	FLOATING FloatSave;
	NTSTATUS Ntstatus;
	START_FLOATING(FloatSave,Ntstatus);
	CHECK_FLOATING_BOOL(Ntstatus);
	
	/*	
	double theta = 0.017452* face.rotate;	// pi/180
	double sin_theta,cos_theta;
	sin_theta	= sin(theta);
	cos_theta	= cos(theta);
	// rotate matrix
	//		| cos sin |   |x|
	//		|		  | * | |
	//		|-sin cos |   |y|
	face.abs_x = (int)((cos_theta * face.x) + (sin_theta * face.y));
	face.abs_y = (int)((cos_theta * face.y) - (sin_theta * face.x));
*/
	double rx = face.x/face.scale;
	double ry = face.y/face.scale;
	double theta = 0.017452* face.rotate;	// pi/180
	double p0x, p0y, dist_OP, diffX, diffY, temp;
	double arfa, arfa_theta;

	int x,y;

	if (theta > 0)
	{	   
		p0x = height*sin(theta);
		diffX = rx - p0x;
		diffY = ry;
		dist_OP = sqrt(diffX*diffX + diffY*diffY);
		arfa_theta = asin(diffY/dist_OP);

		if (rx > p0x)   // 角度變換
			arfa = arfa_theta - theta;
		else
			arfa = 3.1415 - arfa_theta - theta;

		x = (int)(dist_OP * cos(arfa));
		y = (int)(dist_OP * sin(arfa));
	}
	else
	{	   
		p0y		= width*sin(-theta);
		diffX	= rx;
		diffY	= fabs(ry - p0y);
		temp	= diffX*diffX + diffY*diffY;
		dist_OP = (temp<=1)? 1.0: sqrt(temp);
		arfa_theta = asin(diffX/dist_OP);

		if (ry > p0y)   // 角度變換
			arfa = arfa_theta + theta;
		else
			arfa = 3.1415 - arfa_theta + theta;

		x = (int)(dist_OP * sin(arfa));
		y = (int)(dist_OP * cos(arfa));
	}

	END_FLOATING(FloatSave,Ntstatus);

	if (x < 0 || y < 0)
		return FALSE;
	
	face.abs_x = x;
	face.abs_y = y;
	return TRUE;
}

//compute 4 corners and center
BOOL CFace_Convert2FacePos(CFace *mp,STRUCT_DETECT_FACE *f_info)
{
	FLOATING FloatSave;
	NTSTATUS Ntstatus;
	START_FLOATING(FloatSave,Ntstatus);
	CHECK_FLOATING_BOOL(Ntstatus);
	
	//p1 p2
	//p3 p4
	int new_size, LUx, LUy, RBx, RBy, angle;
	int Dx, Dy;
	int theta;
	int left,right,top,bottom;

	LUx			= f_info->abs_x;
	LUy			= f_info->abs_y;
	new_size	= (int)(22/f_info->scale);
	RBx			= LUx + new_size;
	RBy			= LUy + new_size;
	angle		= (int)(-1*f_info->rotate);
	
	Dx = RBx - LUx;
	Dy = RBy - LUy;
	theta = angle + 90;
	//get p1 2 3 4
	Point p1,p2,p3,p4;
	p1.x = LUx;
	p1.y = LUy;
	p2.x = p1.x + (int)( Dx*mp->cos_table[theta]);
	p2.y = p1.y + (int)( Dx*mp->sin_table[theta]);
	p3.x = p1.x + (int)(-Dy*mp->sin_table[theta]);
	p3.y = p1.y + (int)( Dy*mp->cos_table[theta]);
	p4.x = p3.x + (int)( Dx*mp->cos_table[theta]);
	p4.y = p3.y + (int)( Dx*mp->sin_table[theta]);
	
	left	= ( p1.x < p3.x ) ? p1.x : p3.x;
	right	= ( p2.x < p4.x ) ? p4.x : p2.x;
	top		= ( p1.y < p2.y ) ? p1.y : p2.y;
	bottom	= ( p3.y < p4.y ) ? p4.y : p3.y;

	END_FLOATING(FloatSave,Ntstatus);

	if (left < 0 || top < 0 || right > mp->Param.iProcessWidth || bottom > mp->Param.iProcessHeight)
		return FALSE;

	//get center
	f_info->center_x = (p1.x + p4.x) /2;
	f_info->center_y = (p1.y + p4.y) /2;	

	return TRUE;
}

FaceParam *CFace_GetParam(CFace *mp)
{
	return &mp->Param;
}

FaceReport *CFace_GetReport(CFace *mp)
{
	return &mp->Report;
}

void CFace_WriteFaceTrackParamToReg(CFace *mp, DWORD* pFaceTrackParams)
{
	if (pFaceTrackParams == NULL)
		return;
	FLOATING FloatSave;
	NTSTATUS Ntstatus;
	START_FLOATING(FloatSave,Ntstatus);
	CHECK_FLOATING_VOID(Ntstatus);
	
	FaceParam *pParam = &mp->Param;

	pFaceTrackParams[ftpFaceTrackEnable]			= (DWORD) pParam->bFaceTrackEnable;
	pFaceTrackParams[ftpMaxFaceRecord]				= (DWORD) pParam->iMaxFaceRecord;	
	pFaceTrackParams[ftpProcessWidth]				= (DWORD) pParam->iProcessWidth;
	pFaceTrackParams[ftpProcessHeight]				= (DWORD) pParam->iProcessHeight;
	pFaceTrackParams[ftpHX1num]						= (DWORD) pParam->iHX1num;					
	pFaceTrackParams[ftpHX2num]						= (DWORD) pParam->iHX2num;
	pFaceTrackParams[ftpHX3num]						= (DWORD) pParam->iHX3num;
	pFaceTrackParams[ftpStableFaceThresholdRatio]	= (DWORD)(pParam->dStableFaceThresholdRatio*10000.0);
	pFaceTrackParams[ftpCenterTolerance]			= (DWORD) pParam->iCenterTolerance;
	pFaceTrackParams[ftpZoomInPanStep]				= (DWORD) pParam->iZoomInPanStep;
	pFaceTrackParams[ftpZoomInTileStep]				= (DWORD) pParam->iZoomInTileStep;
	pFaceTrackParams[ftpZoomInZoomStep]				= (DWORD) pParam->iZoomInZoomStep;
	pFaceTrackParams[ftpZoomOutZoomStep]			= (DWORD) pParam->iZoomOutZoomStep;
	pFaceTrackParams[ftpZoomInWindowSize]			= (DWORD) pParam->iZoomInWindowSize;
	pFaceTrackParams[ftpTargetCenterThreshold]		= (DWORD) pParam->iTargetCenterThreshold;
	pFaceTrackParams[ftpTargetZoomThreshold]		= (DWORD) pParam->iTargetZoomThreshold;
	pFaceTrackParams[ftpStage1Threshold]			= (DWORD) pParam->s1_thre;
	pFaceTrackParams[ftpStage2Threshold]			= (DWORD) pParam->s2_thre;
	pFaceTrackParams[ftpStage3Threshold]			= (DWORD) pParam->s3_thre;
	pFaceTrackParams[ftpStage4Threshold]			= (DWORD) pParam->fs_thre;
	pFaceTrackParams[ftpMaxFrameCount]				= (DWORD) pParam->iMaxFrameCount;
	pFaceTrackParams[ftpMaxContinueMissCount]		= (DWORD) pParam->iMaxContinueMissCount;
	pFaceTrackParams[ftpFaceVerticalPositionRatio]	= (DWORD)(pParam->dFaceVerticalPositionRatio*10000.0);
	pFaceTrackParams[ftpDrawFaceRect]				= (DWORD) pParam->bDrawFaceRect;
	pFaceTrackParams[ftpDrawFaceRect]				= (DWORD) mp->Param.bDrawFaceRect;
	pFaceTrackParams[ftpFlipImage]					= (DWORD) mp->Param.bFlipImage;
	pFaceTrackParams[ftpMirrorImage]				= (DWORD) mp->Param.bMirrorImage;
	pFaceTrackParams[ftpWeightThreshold]			= (DWORD) mp->Param.iWeightThreshold;
	pFaceTrackParams[ftpContinuousThreshold]		= (DWORD) mp->Param.iContinuousThreshold;
	pFaceTrackParams[ftpNewFaceDisThreshold]		= (DWORD)(mp->Param.dNewFaceDisThreshold*10000.0);

	END_FLOATING(FloatSave,Ntstatus);
}

void CFace_ReadFaceTrackParamFromReg(CFace *mp, DWORD* pFaceTrackParams)
{
	if (pFaceTrackParams == NULL)
		return;

	FLOATING FloatSave;
	NTSTATUS Ntstatus;
	START_FLOATING(FloatSave,Ntstatus);
	CHECK_FLOATING_VOID(Ntstatus);
	
	FaceParam *pParam = &mp->Param;

#if defined(PARAMETER_FROM_TESTAP)
	pParam->bFaceTrackEnable			= 0;
	pParam->iMaxFaceRecord				= 6;						//  8	
	pParam->iProcessWidth				= 213;						//	160
	pParam->iProcessHeight				= 160;						//	120
	pParam->iHX1num						= 20;						//	20
	pParam->iHX2num						= 40;						//	40
	pParam->iHX3num						= 238;						//	238
	pParam->dStableFaceThresholdRatio	= 0.8;						//	0.8
	pParam->iCenterTolerance			= 5;						//	5
	pParam->iZoomInPanStep				= 2;						//	2
	pParam->iZoomInTileStep				= 2;						//	2
	pParam->iZoomInZoomStep				= 1;						//	1
	pParam->iZoomOutZoomStep			= -4;						//	-4
	pParam->iZoomInWindowSize			= 66;						//	66
	pParam->iTargetCenterThreshold		= 8;						//	8
	pParam->iTargetZoomThreshold		= 20;						//	20
	pParam->s1_thre						= 90;						//	90
	pParam->s2_thre						= 200;						//	200
	pParam->s3_thre						= 700;						//	770
	pParam->fs_thre						= 60;						//	60
	pParam->iMaxFrameCount				= pParam->iMaxFaceRecord*5;	//	ftpMaxFaceRecord*5
	pParam->iMaxContinueMissCount		= pParam->iMaxFaceRecord*2;	//  ftpMaxFaceRecord*2
	pParam->dFaceVerticalPositionRatio	= 0.25;						//	180
	pParam->bDrawFaceRect				= false;					//	false
	pParam->bFlipImage					= false;					//	false
	pParam->bMirrorImage				= false;					//	false
	pParam->bMotorEnable				= false;
	pParam->iWeightThreshold			= 3;						//	4
	pParam->iContinuousThreshold		= 4;						//	4
	pParam->dNewFaceDisThreshold		= 1.0;						//	1.0
	
#else
	pParam->bFaceTrackEnable			= (BOOL)pFaceTrackParams[ftpFaceTrackEnable];
	pParam->iMaxFaceRecord				= (int)pFaceTrackParams[ftpMaxFaceRecord];	
	pParam->iProcessWidth				= (int)pFaceTrackParams[ftpProcessWidth];
	pParam->iProcessHeight				= (int)pFaceTrackParams[ftpProcessHeight];
	pParam->iHX1num						= (int)pFaceTrackParams[ftpHX1num];					
	pParam->iHX2num						= (int)pFaceTrackParams[ftpHX2num];
	pParam->iHX3num						= (int)pFaceTrackParams[ftpHX3num];
	pParam->dStableFaceThresholdRatio	= (double)(pFaceTrackParams[ftpStableFaceThresholdRatio]/10000.0);
	pParam->iCenterTolerance			= (int)pFaceTrackParams[ftpCenterTolerance];
	pParam->iZoomInPanStep				= (int)pFaceTrackParams[ftpZoomInPanStep];
	pParam->iZoomInTileStep				= (int)pFaceTrackParams[ftpZoomInTileStep];
	pParam->iZoomInZoomStep				= (int)pFaceTrackParams[ftpZoomInZoomStep];
	pParam->iZoomOutZoomStep			= (int)pFaceTrackParams[ftpZoomOutZoomStep];
	pParam->iZoomInWindowSize			= (int)pFaceTrackParams[ftpZoomInWindowSize];
	pParam->iTargetCenterThreshold		= (int)pFaceTrackParams[ftpTargetCenterThreshold];
	pParam->iTargetZoomThreshold		= (int)pFaceTrackParams[ftpTargetZoomThreshold];
	pParam->s1_thre						= (int)pFaceTrackParams[ftpStage1Threshold];
	pParam->s2_thre						= (int)pFaceTrackParams[ftpStage2Threshold];
	pParam->s3_thre						= (int)pFaceTrackParams[ftpStage3Threshold];
	pParam->fs_thre						= (int)pFaceTrackParams[ftpStage4Threshold];
	pParam->iMaxFrameCount				= (int)pFaceTrackParams[ftpMaxFrameCount];
	pParam->iMaxContinueMissCount		= (int)pFaceTrackParams[ftpMaxContinueMissCount];
	pParam->dFaceVerticalPositionRatio	= (double)(pFaceTrackParams[ftpFaceVerticalPositionRatio]/10000.0);
	pParam->bDrawFaceRect				= (BOOL)pFaceTrackParams[ftpDrawFaceRect];
	pParam->bFlipImage					= (BOOL)pFaceTrackParams[ftpFlipImage];
	pParam->bMirrorImage				= (BOOL)pFaceTrackParams[ftpMirrorImage];
	pParam->bMotorEnable				= (BOOL)pFaceTrackParams[ftpMotorEnable];
	pParam->iWeightThreshold			= (int)pFaceTrackParams[ftpWeightThreshold];
	pParam->iContinuousThreshold		= (int)pFaceTrackParams[ftpContinuousThreshold];
	pParam->dNewFaceDisThreshold		= (double)(pFaceTrackParams[ftpNewFaceDisThreshold]/10000.0);
	
#endif
	END_FLOATING(FloatSave,Ntstatus);
}
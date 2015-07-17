
struct RGB {
	unsigned char r,g,b;
};
struct TransIfo {
	struct RGB rgb;
	int        trans;
};
typedef struct TransIfo TRANSIFO, *PTRANSIFO;

#ifndef	_NOEFFECT
void EffectConvert( unsigned char* pImgIn, 					
				    unsigned int   ImgWidth,
					unsigned int   ImgHeight,
					unsigned char  Effect,
					unsigned char* pMemory );

// HSize,VSize : buffer width/height
// HiSize,ViSize : image width/height
void EffectConvertI420( unsigned char* pImgIn, 				
						unsigned int   HSize,
                        unsigned int   HiSize,
  						unsigned int   VSize,
                        unsigned int   ViSize,
						unsigned char  Effect,
						unsigned char* pMemory );

//2007/11/29 11:20¤W¤È
void EffectConvertYUY2( unsigned char* pImgIn,
										unsigned int HSize,
										unsigned int HiSize,
               			unsigned int VSize,
               			unsigned int ViSize,
               			unsigned char Effect,
               			unsigned char* pMemory );
					
BOOL SetFrameFile( PCWSTR fname,int gtrans,int trans_tl,struct TransIfo *tinfo,unsigned char* pMemory);
#endif
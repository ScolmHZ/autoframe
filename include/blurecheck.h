



typedef char                 S8;   /* typdef char                 S8         */
typedef short                S16;  /* typedef short               S16        */
typedef int                  S32;  /* typedef int                 S32        */
typedef long long            S64;  /* typedef long long           S64        */

typedef unsigned char        U8;   /* typedef unsigned char       U8         */
typedef unsigned short       U16;  /* typedef unsigned short      U16        */
typedef unsigned int         U32;  /* typedef unsigned int        U32        */
typedef unsigned long long   U64;  /* typedef unsigned long long  U64        */
typedef float                F32;

typedef char*                pS8;  /* typedef char*               pS8        */
typedef short*               pS16; /* typedef short*              pS16       */
typedef int*                 pS32; /* typedef int*                pS32       */
typedef long long*           pS64; /* typedef long*               pS64       */

typedef const int *          pcU32;/* typedef const int *         pcU32      */
typedef unsigned char*       pU8;  /* typedef unsigned char*      pU8        */
typedef unsigned short*      pU16; /* typedef unsigned short*     pU16       */
typedef unsigned int*        pU32; /* typedef unsigned int*       pU32       */
typedef unsigned long long*  pU64; /* typedef unsigned long*      pU64       */
typedef float *              pF32;
typedef void *               pV;   /* typedef void *               pV		 */
typedef void                 V;    /* typedef void                 V		 */
typedef unsigned int Se_fn_t; /* fixed-pattern bit string using n bits       */
typedef          int Se_in_t; /* signed int using n bits		   	         */
typedef unsigned int Se_un_t; /* unsigned int using n bits			         */
typedef          int Se_sev_t;/* int exp-golomb coded syntax element         */
typedef unsigned int Se_uev_t;/* unsigned int exp-golomb coded syntax element*/
typedef unsigned int Se_tev_t;/* truncated exp-golomb coded syntax element   */
typedef unsigned int Se_uv_t; /* length dependent on other syntax			 */

typedef int          IDX;     /* for Index									 */
typedef char         C_BOOL;

typedef struct tagVDGrad
{
    S32 iGradVertical[1088];
    F32 fGradDirect[1920];
    F32 fGradDirectPre[1920];
    F32 fGradDirectSub[1920];
}VD_GRAD_S;

typedef struct tagVD_RGB_IMG_S
{
	U32 uiWidth;
	U32 uiHeight;
	pU8 pucRGB;
}VD_RGB_IMG_S;


typedef struct tagIMGINFO_S
{
	pU8 pucImage;
	S32 iImgHgt;
	S32 iImgWid;
	U32 iStride;
	S16 iBitCnt;
}IMGINFO_S;


/******************************************************************************
*                                �궨��                                       *
******************************************************************************/
/*��������ָʾ*/
#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifndef INOUT
#define INOUT
#endif

/*����ֵ�궨���б�*/
#define DSP_VD_SUCCESS         (0)    /* �ɹ� */
#define DSP_VD_FAIL            (-1)   /* ʧ�� */
#define DSP_VD_MEM_ERR         (-2)   /* �ڴ������� */
#define DSP_VD_PARA_INVALID    (-3)   /* ���������Ч */
#define DSP_VD_PARAPTR_NULL    (-4)   /* ʹ�õ����ָ��Ϊ�� */


/******************************************************************************
*                                ���Ͷ���                                     *
******************************************************************************/

typedef enum tagDSP_VD_DataType
{
    DSP_VD_DATA_TYPE_TS,   
    DSP_VD_DATA_TYPE_RAW,  
    DSP_VD_DATA_TYPE_JPEG,
    DSP_VD_DATA_TYPE_YUV,  
    DSP_VD_STREAM_TYPE_TS, 
    DSP_VD_STREAM_TYPE_YUV /*������YUV*/
}DSP_VD_DATATYPE_E;

/*���������Ŀ����*/
typedef enum tagDSP_VD_DiagItem
{
    DSP_VD_TYPE_BRIGHTNESS = 0x1,      /*��Ƶ�����쳣*/
    DSP_VD_TYPE_COLOR      = 0x2,      /*��Ƶ��ɫ�쳣*/
    DSP_VD_TYPE_CONTRAST   = 0x4,      /*��Ƶ�Աȶ��쳣*/
    DSP_VD_TYPE_BLUR       = 0x8,      /*��Ƶ�������쳣*/
    DSP_VD_TYPE_MISSING    = 0x10,     /*��Ƶ�ź�ȱʧ*/
    DSP_VD_TYPE_FLAKE      = 0x20,     /*��������*/
    DSP_VD_TYPE_VIBRATE    = 0x40,     /*����*/
    DSP_VD_TYPE_ROLL       = 0x80,     /*����*/
    DSP_VD_TYPE_FREEZE     = 0x100,    /*���涳��*/
    DSP_VD_TYPE_CROSSBAND  = 0x200,    /*�������*/
    DSP_VD_TYPE_DISTORT    = 0x400,    /*Ť������*/
    DSP_VD_TYPE_RIPPLE     = 0x800,    /*���Ƹ���*/
    DSP_VD_TYPE_PTZ        = 0x2000,    /*PTZ���й���*/
    DSP_VD_TYPE_OFFLINE    = 0x40000000 /*������ߣ���������*/
}DSP_VD_DIAGITEM_E;


/* ���ͨ�����豸��Ϣ */
typedef struct tagDSP_VD_CameraInfo {
    int iChanID;      
    int iCameraType;  
    int iDiagItem;    
    int iNightFlag;
    int iWidth;
    int iHeight;
    /*¼��ص�����*/
    /*PTZ���ƻص�����*/
    /*���ݿ�ӿں���*/
}DSP_VD_CAM_INFO_S;

/* ���ͨ�����õ��㷨���� */
typedef struct tagDSP_VD_AlgParameter {
    int iThreshold; /*�㷨��ֵ*/
    
}DSP_VD_DIAG_PARAM_S;

/*YUV ͼ��ṹ�嶨��*/
typedef struct tagDspYuv420
{
    unsigned char *pucData[4];    /*data[0]:Y ƽ��ָ��,data[1]:U ƽ��ָ��,data[2]:V ƽ��ָ��*/
    unsigned int uiLineSize[4]; /*linesize[0]:Yƽ��ÿ�п��, linesize[1]:Uƽ��ÿ�п��, linesize[2]:Vƽ��ÿ�п��*/
    unsigned int uiPicWidth;    /*ͼ�����ؿ�*/
    unsigned int uiPicHeight;   /*ͼ�����ظ�*/
}DSP_YUV420_S;

/*RAW ͼ��ṹ�嶨��*/
typedef struct tagDspRAW
{
    unsigned char *pucRAWData;  /*ָ�����RAW����ָ��*/
    unsigned int uiPicWidth;    /*ͼ�����ؿ�*/
    unsigned int uiPicHeight;   /*ͼ�����ظ�*/
}DSP_RAW_S;


/* ���ͨ���������ݲ��� */
typedef struct tagDSP_VD_CameraData {
    int   iDataType; /*������������, TS��, JPEGͼƬ, Raw����, YUVͼƬ*/
    DSP_RAW_S    stRAWData;
    DSP_YUV420_S stYUVData;
    
}DSP_VD_CAM_DATA_S;

/* ���ͨ���������ݲ��� */
typedef struct tagDSP_VD_DiagnosisResult {
    int iBrightResult;     /*������Ͻ��,����100����*/
    int iColorResult;      /*ɫ����Ͻ��,����100����*/
    int iContrastResult;   /*�Աȶ���Ͻ��,����100����*/
    int iFocusResult;      /*��������Ͻ��,����100����*/
    int iSignalLossResult; /*�źŶ�ʧ��Ͻ��,����100����*/
    
}DSP_VD_DIAG_RESULT_S;

/* ����ļ���Ϣ */
typedef struct tagDSP_VD_FileInfo {
    int iFileType;  /*֧���ļ����ͣ�TS��JPEG��YUV����*/
    DSP_RAW_S    stRAWData;
    DSP_YUV420_S stYUVData;
    
}DSP_VD_FILE_INFO_S;


//S32 ImgRgb2Grey( IN IMGINFO_S *pstImgC, OUT IMGINFO_S *pstImgG,U32 datalen);
S32 VD_GetROIDat(IN VD_RGB_IMG_S * pstRGB, IN pU32 pucYData, U32 uixs, U32 uixe, U32 uiys, U32 uiye, void *pBuffer,IN U32 size);
S32 VD_BlurGetDat(IN pU8 pucDat, INOUT void *pBuffer, IN U32 uiRows, IN U32 uiCols);
F32 VD_ColorVar(pU8 pucDatColor, U32 uixs, U32 uixe, U32 uiys, U32 uiye, U32 uiCols, S32* iGrayMeanBlock);



/////////////////////////////////////////////
//Start Define


#define  LEVEL_SHOW_FATAL   0
#define  LEVEL_SHOW_ERROR   1
#define  LEVEL_SHOW_ALL     2
#define  LEVEL_SHOW_DEBUG   3

/*ģ��������,�ļ����18���ַ�,�������26���ַ�,�к��4���ַ�,����ǰ����һ���ո�,����3��*/
/*��Ϣ��ʾ��־��0����ʾ�κ���־��1��ʾerror��־��2��ʾ������־*/
#define  INFO_SHOW_FATAL  LEVEL_SHOW_FATAL,__FILE__,__LINE__
#define  INFO_SHOW_ERROR  LEVEL_SHOW_ERROR,__FILE__,__LINE__
#define  INFO_SHOW_ALL    LEVEL_SHOW_ALL,__FILE__,__LINE__
#define  INFO_SHOW_DEBUG  LEVEL_SHOW_DEBUG,__FILE__,__LINE__

/*��־����ֵ�ж�*/
#define LOG_ERROR        -1
#define LOG_OK            1



typedef struct tagIMGYUV_S
{
	U32 *pucImgY;
	U32 *pucImgU;
	U32 *pucImgV;
	S32 iImgWid;
	S32 iImgHgt;
	S32 iStride;
}IMGYUV_S;

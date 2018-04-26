



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
*                                宏定义                                       *
******************************************************************************/
/*参数方向指示*/
#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifndef INOUT
#define INOUT
#endif

/*返回值宏定义列表*/
#define DSP_VD_SUCCESS         (0)    /* 成功 */
#define DSP_VD_FAIL            (-1)   /* 失败 */
#define DSP_VD_MEM_ERR         (-2)   /* 内存分配错误 */
#define DSP_VD_PARA_INVALID    (-3)   /* 输入参数无效 */
#define DSP_VD_PARAPTR_NULL    (-4)   /* 使用的入参指针为空 */


/******************************************************************************
*                                类型定义                                     *
******************************************************************************/

typedef enum tagDSP_VD_DataType
{
    DSP_VD_DATA_TYPE_TS,   
    DSP_VD_DATA_TYPE_RAW,  
    DSP_VD_DATA_TYPE_JPEG,
    DSP_VD_DATA_TYPE_YUV,  
    DSP_VD_STREAM_TYPE_TS, 
    DSP_VD_STREAM_TYPE_YUV /*解码后的YUV*/
}DSP_VD_DATATYPE_E;

/*诊断任务项目类型*/
typedef enum tagDSP_VD_DiagItem
{
    DSP_VD_TYPE_BRIGHTNESS = 0x1,      /*视频亮度异常*/
    DSP_VD_TYPE_COLOR      = 0x2,      /*视频颜色异常*/
    DSP_VD_TYPE_CONTRAST   = 0x4,      /*视频对比度异常*/
    DSP_VD_TYPE_BLUR       = 0x8,      /*视频清晰度异常*/
    DSP_VD_TYPE_MISSING    = 0x10,     /*视频信号缺失*/
    DSP_VD_TYPE_FLAKE      = 0x20,     /*噪声干扰*/
    DSP_VD_TYPE_VIBRATE    = 0x40,     /*抖动*/
    DSP_VD_TYPE_ROLL       = 0x80,     /*滚屏*/
    DSP_VD_TYPE_FREEZE     = 0x100,    /*画面冻结*/
    DSP_VD_TYPE_CROSSBAND  = 0x200,    /*横道干扰*/
    DSP_VD_TYPE_DISTORT    = 0x400,    /*扭曲干扰*/
    DSP_VD_TYPE_RIPPLE     = 0x800,    /*波纹干扰*/
    DSP_VD_TYPE_PTZ        = 0x2000,    /*PTZ运行故障*/
    DSP_VD_TYPE_OFFLINE    = 0x40000000 /*相机离线，网络层诊断*/
}DSP_VD_DIAGITEM_E;


/* 诊断通道的设备信息 */
typedef struct tagDSP_VD_CameraInfo {
    int iChanID;      
    int iCameraType;  
    int iDiagItem;    
    int iNightFlag;
    int iWidth;
    int iHeight;
    /*录像回调函数*/
    /*PTZ控制回调函数*/
    /*数据库接口函数*/
}DSP_VD_CAM_INFO_S;

/* 诊断通道配置的算法参数 */
typedef struct tagDSP_VD_AlgParameter {
    int iThreshold; /*算法阈值*/
    
}DSP_VD_DIAG_PARAM_S;

/*YUV 图像结构体定义*/
typedef struct tagDspYuv420
{
    unsigned char *pucData[4];    /*data[0]:Y 平面指针,data[1]:U 平面指针,data[2]:V 平面指针*/
    unsigned int uiLineSize[4]; /*linesize[0]:Y平面每行跨距, linesize[1]:U平面每行跨距, linesize[2]:V平面每行跨距*/
    unsigned int uiPicWidth;    /*图像象素宽*/
    unsigned int uiPicHeight;   /*图像象素高*/
}DSP_YUV420_S;

/*RAW 图像结构体定义*/
typedef struct tagDspRAW
{
    unsigned char *pucRAWData;  /*指向处理后RAW数据指针*/
    unsigned int uiPicWidth;    /*图像象素宽*/
    unsigned int uiPicHeight;   /*图像象素高*/
}DSP_RAW_S;


/* 诊断通道输入数据参数 */
typedef struct tagDSP_VD_CameraData {
    int   iDataType; /*输入数据类型, TS流, JPEG图片, Raw码流, YUV图片*/
    DSP_RAW_S    stRAWData;
    DSP_YUV420_S stYUVData;
    
}DSP_VD_CAM_DATA_S;

/* 诊断通道输入数据参数 */
typedef struct tagDSP_VD_DiagnosisResult {
    int iBrightResult;     /*亮度诊断结果,反馈100分制*/
    int iColorResult;      /*色度诊断结果,反馈100分制*/
    int iContrastResult;   /*对比度诊断结果,反馈100分制*/
    int iFocusResult;      /*清晰度诊断结果,反馈100分制*/
    int iSignalLossResult; /*信号丢失诊断结果,反馈100分制*/
    
}DSP_VD_DIAG_RESULT_S;

/* 诊断文件信息 */
typedef struct tagDSP_VD_FileInfo {
    int iFileType;  /*支持文件类型，TS，JPEG，YUV类型*/
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

/*模块名更新,文件名最长18个字符,函数名最长26个字符,行号最长4个字符,各在前面留一个空格,后面3个*/
/*信息显示标志，0不显示任何日志，1显示error日志，2显示所有日志*/
#define  INFO_SHOW_FATAL  LEVEL_SHOW_FATAL,__FILE__,__LINE__
#define  INFO_SHOW_ERROR  LEVEL_SHOW_ERROR,__FILE__,__LINE__
#define  INFO_SHOW_ALL    LEVEL_SHOW_ALL,__FILE__,__LINE__
#define  INFO_SHOW_DEBUG  LEVEL_SHOW_DEBUG,__FILE__,__LINE__

/*日志返回值判断*/
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

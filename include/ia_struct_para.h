
#ifndef _IVE_STRUCT_PARA
#define _IVE_STRUCT_PARA

#include "hi_comm_ive.h"

#include "hi_type.h"
#include "hi_ive.h"

#include "imp_algo_type.h"
#include "imp_algo_urp_param.h"
#include "imp_avd_para.h"

//#define _XM_IA_PRINT_
//#define _XM_IA_DEBUG_
#define _XM_IA_lINUX_
//#define _XM_IA_WINDOWS_

#ifdef _XM_IA_WINDOWS_
#include "ia_point.h"
#endif

#ifdef _XM_IA_lINUX_
//#include "hi_comm_sys.h"
//#include "mpi_sys.h"
#endif

#define HI_VOID void
#define CCL_AREA_THR_STEP 6
#define MAX_OBJ_NUM 6
#define IVE_ALIGN (16)
#define MAX_POINT_NUM 15		//一个目标区域里（CCL联通区域）的最大点数
#define MIN_DIST 5
#define CORNER_INIT_LEVEL 150
#define OBJ_MIN_AREA 60;//初始设置220
#define HISTORY_FRAME 5
#define THETA 0.7
#define FG_VALUE 255
#define FRAME_RATE 15
#define MAX_MB_WID 80  //最大图像宽为1280 【1280/16】
#define MAX_MB_HGT 45   //最大图像高为720 【720/16】

//typedef struct FGObjectMBMark
//{
//
//    HI_U16 u16TagWid; //源图宽度
//    HI_U16 u16TagHgt; //源图高度
//
//    HI_U16 u16RectMinX;
//    HI_U16 u16RectMinY;
//    HI_U16 u16RectMaxX;
//    HI_U16 u16RectMaxY;
//
//    HI_U16 u16MBWid;  //当前处理的图像宏块宽度
//    HI_U16 u16MBHgt;  //当前处理的图像宏块高度
//    HI_U16 u16TagMBWid; //源图宏块宽度
//    HI_U16 u16TagMBHgt; //源图宏块高度
//
//    HI_U16 u16HorMBNum;    //当前处理的图像水平宏块数量
//    HI_U16 u16TagHorMBNum; //源图水平宏块数量
//
//    HI_U16 u16MBNum;   //当前处理的图像宏块总数
//    HI_U16 u16TagMBNum;//源图宏块总数
//
//    HI_U16 u16MBPixThd; //当前处理的图像宏块前景像素数阈值
//
//    /* 数组标记宏块内是否存在前景: 1 表示存在， 0 表示不存在 */
//    HI_S8 as8Mark[MAX_MB_WID * MAX_MB_HGT];    //当前处理的图像前景宏块区标记
//    HI_S8 as8TagMark[MAX_MB_WID * MAX_MB_HGT]; //源图前景宏块区标记
//}FG_OBJMARK_S;  

typedef struct hiIVE_RECT_S
{
	HI_U16 u16X;
	HI_U16 u16Y;
	HI_U16 u16Width;
	HI_U16 u16Height;
}IVE_RECT_S;

typedef struct{
	int s9q7Dx;
	int s9q7Dy;
	int s32Status;
}IVE_MV_S9Q7_S;

typedef struct tagIntel_Coding
{

    int highQP;
    int lowQP;
    int hight;
    int width;
    IVE_IMAGE_S stIveImage;
}INTEL_CODING;

typedef struct tagQP_Result
{
    char Flag;
    char *QPmap;
    IVE_RECT_S ROIseat[6];
}QP_RESULT;


typedef struct SELF_IVE_RECT_S
{
	/****************************************矩形的四个点************************************/
	/********************************0-左上 1-右上 2-右下 3-左下*****************************/
	POINT_S astPoint[4];
	HI_U32 u32Area;//CCL联通区域面积（像素数）
}IVE_CCLRECT_S;

typedef struct{
	unsigned int u0q8Epsilon;
	unsigned int u0q8MinEigThr;
	unsigned short u16CornerNum;
	unsigned char u8IterCount;
}IVE_LK_OPTICAL_FLOW_CTRL_S;

typedef struct hiSAMPLE_RECT_ARRAY_S
{
	HI_U16 u16Num;
	IVE_CCLRECT_S astRect[MAX_OBJ_NUM];
}RECT_ARRAY_S;

typedef struct hiSAMPLE_IVE_GMM_S
{
    IVE_SRC_IMAGE_S stResizeSrc;
	IVE_SRC_IMAGE_S stSrc;		//GMM婧愬浘鍍?
	IVE_DST_IMAGE_S stFg;		//缁撴灉鍓嶆櫙鍥惧儚
	IVE_DST_IMAGE_S stBg;		//缁撴灉鑳屾櫙鍥惧儚
	IVE_DST_IMAGE_S stSobel;    //sobel算子存图
	IVE_DST_IMAGE_S stSobelDst;
    IVE_SRC_IMAGE_S stNCCSrc1;   //NCC子图1
    IVE_DST_IMAGE_S stNCCDst1;   //NCC判别图1
    IVE_SRC_IMAGE_S stNCCSrc2;   //NCC子图2
    IVE_DST_IMAGE_S stNCCDst2;   //NCC判别图2
    IVE_SRC_IMAGE_S stNCCSrc3;   //NCC子图3
    IVE_DST_IMAGE_S stNCCDst3;   //NCC判别图3
    IVE_SRC_IMAGE_S stNCCSrc4;   //NCC子图4
    IVE_DST_IMAGE_S stNCCDst4;   //NCC判别图4
    IVE_SRC_IMAGE_S stPolySrc;   //Poly判别图1
    IVE_DST_IMAGE_S stPolyDst;   //Poly判别图2

	IVE_IMAGE_S	stDILATEImg;			//鑶ㄨ儉澶勭悊缁撴灉鍥惧儚
	IVE_IMAGE_S	stERODEImg;			//鑵愯殌澶勭悊缁撴灉鍥惧儚
	IVE_MEM_INFO_S  stModel;	//Gmm妯″瀷
	IVE_GMM_CTRL_S  stGmmCtrl;	//GMM鎺у埗鍙傛暟
	IVE_DILATE_CTRL_S stDilateCtrl;	//鑶ㄨ儉鎺у埗鍙傛暟
	IVE_ERODE_CTRL_S stErodeCtrl;	//鑵愯殌鎺у埗鍙傛暟
	IVE_DST_MEM_INFO_S stBlob;		//CCL鍙傛暟
	IVE_CCL_CTRL_S  stCclCtrl;		//CCL鎺у埗鍙傛暟
	//IVE_RESIZE_CTRL_S stResizeCtrl; //resize
	RECT_ARRAY_S stRegion;
    IVE_SOBEL_CTRL_S  stSobelCtrl;
    IVE_16BIT_TO_8BIT_CTRL_S st16to8Ctrl;
}IVE_GMM_S;

typedef struct hiSAMPLE_IVE_ST_LK_S
{
	IVE_SRC_IMAGE_S  stSrc;				//ST角点检测计算候选角点源图像
	IVE_DST_IMAGE_S  stDst;				//ST角点检测按规则挑选角点				没用到
    IVE_SRC_IMAGE_S  stPoly;           //遗留物初始图像
    IVE_DST_IMAGE_S  stPolyDst;        //遗留物帧差图像输出
    IVE_SRC_IMAGE_S  stPilfer;          //物品盗移初始图片
    IVE_DST_IMAGE_S  stPilferDst;       //物品盗移输出图片
    IVE_SRC_IMAGE_S  stSceneCh;         //场景变换初始图片
    IVE_DST_IMAGE_S  stSceneChDst;      //场景变换输出图片
    IVE_SRC_IMAGE_S  stNCC;
    IVE_SRC_IMAGE_S  stNCCObj;
    IVE_DST_MEM_INFO_S  stNCCRDst;
    IVE_DST_MEM_INFO_S  stNCCRObjDst;
    //WWWWWWWWWWWWWWWWWW
	IVE_SRC_IMAGE_S	 astPrePyr[2];
	IVE_SRC_IMAGE_S	 astCurPyr[2];
	IVE_IMAGE_S stPyrTmp;
	IVE_DST_MEM_INFO_S stDstCorner;
	IVE_MEM_INFO_S   stMv;
	//WWWWWWWWWWWWWWWWWWWWWWWWWWW
	IVE_SRC_MEM_INFO_S astPoint[2];
	IVE_ST_CANDI_CORNER_CTRL_S  stStCandiCornerCtrl;
	IVE_ST_CORNER_CTRL_S	 stStCornerCtrl;
	IVE_LK_OPTICAL_FLOW_CTRL_S	stLkCtrl;
	IVE_POINT_S25Q7_S astPointTmp[MAX_OBJ_NUM * MAX_POINT_NUM];			//所有CCL区域的角点写入一个数组

	IVE_ST_CORNER_INFO_S *pstCornerInfo;
	IVE_POINT_S25Q7_S *pstPoint[3];
	IVE_MV_S9Q7_S *pstMv;
}IVE_ST_LK_S;

typedef struct ObjFeaPtInfo
{
	int PointNum;//目标所含特征点总数
	IVE_POINT_S25Q7_S Point[MAX_POINT_NUM];//包含特征点在角点数组中的下标//还是直接存点坐标吧
	int PointMotionX[MAX_POINT_NUM];
	int PointMotionY[MAX_POINT_NUM];
}ObjFeaturePointInfo;

typedef struct Mv
{
	int dx;
	int dy;
}MotionInfo;

typedef struct TargetObject
{
    int State;//状态变量，在图中为1，不在图中为0，部分在部分不在大于1；
    int ObjectID;//ID
    ObjFeaturePointInfo PointInfo;//包含的特征点信息，总数和在角点数组中的下标
    int CCLRegionNum;//对应着第几个CCL区域
    int CCLRegionArea;//对应CCL区域的面积
    IMP_RECT_S stRect;
    IMP_RECT_S stPolyRect;//遗留物框域
    IMP_RECT_S stPilferRect;//物品盗移区域
    IVE_POINT_U16_S center;
    IVE_POINT_U16_S Origin_center;
    MotionInfo Motion;
    MotionInfo CenterMotion;
    MotionInfo PreditMotion;
    int Obj_Last_Time;
    int History_MotionX[HISTORY_FRAME];
    int History_MotionY[HISTORY_FRAME];
    int HistoryNum;
    int iTouchLine;
    int iCount;
    int iMark;
    int iPolyStart; //物品遗留功能启动判别
    int iPolyMark;  //閬楃暀鐗╁瓨鍦ㄦ爣璇
    int iPolylock;  //报警锁死
    int iPolyClose; //閬楃暀鐗╂秷澶辨爣璇
    int iSceneClose;//场景变换停止标识
    int u16RemnantFrameCount;   //閬楃暀鐘舵€佹寔缁殑甯ф暟锛屽ぇ浜庤瀹氬€紆16DetectRes瑙﹀彂
}Struct_Object;

typedef struct HI_LK_GMM
{
	IVE_ST_LK_S stLK;
	IVE_GMM_S stGMM;
}IVE_GMM_LK_S;

typedef struct temp_frame
{
	int DifFrameX[MAX_OBJ_NUM];
	int DifFrameY[MAX_OBJ_NUM];
}Dif_Arry;

typedef struct Frame_Obj
{
	Struct_Object Object[MAX_OBJ_NUM];
	Dif_Arry Dif_Frame;
	int ObjectNum;
}Frame_Object;


typedef struct tagRemnantDectctionInfo
{
	IVE_IMAGE_S stImageDiffFg;
	HI_U16  usiSpeedArry[MAX_OBJ_NUM];
	HI_U16  usiLastTimeArry[MAX_OBJ_NUM];

	IVE_RECT_S stDetectionRct;	//系统给定的禁区框（范围），对前景区域是否在这里进行判断
	HI_U16     usiSensititvity; //0: Low; 1: Normal(default); 2: Hight;
}XM_REMNANTDETECTION_INFO_S;

typedef struct tagRemnantDectctionRes
{
	HI_U16 u16DetectRes; //0: not detect; >0: detected;
	IMP_RECT_S stDetectedRct[MAX_OBJ_NUM];//遗留物数组
}XM_REMNANTDETECTION_RES_S;
typedef struct X_MAN
{

	IVE_GMM_LK_S *pstGmmLk;
	Frame_Object *pstCur_Frame_Obj;
	int *CurrentCCL;// [MAX_OBJ_NUM] 大小数组，当前CCL是否已被归为某目标
	int *DoNewCorner;//对于MAX_OBJ_NUM个CCL区域，有新目标进入时要不要对其区域做corner，0是做，1是不做
	IVE_POINT_S25Q7_S *LKPoint;// [MAX_OBJ_NUM * MAX_POINT_NUM];
	IVE_POINT_S25Q7_S *TempPoint;// [MAX_OBJ_NUM * MAX_POINT_NUM];
	IVE_MV_S9Q7_S *TempMv;// [MAX_OBJ_NUM * MAX_POINT_NUM]  ;
	RESULT_S *pstResult;
	URP_PARA_S *pstURPpara;		//URP鍙傛暟锛岃鍒欎俊鎭?
	IMP_AVD_PARA_S *stAvdPara;
	XM_REMNANTDETECTION_INFO_S *pstRemnantDetectionInfo;
	XM_REMNANTDETECTION_RES_S *pstRemnantDetectionRes;
	ALGO_MODULE_E eAlgType;
	IVE_SUB_CTRL_S *pstSubCtrl;
	IVE_THRESH_CTRL_S *pstThrCtrl;
//	IVE_SRC_IMAGE_S *pstFirstFrame;	//与一段时间的起始图像做帧差（备用）
	int TotalCorner;
    int PolyGon;//閬楃暀鐗╃洰鏍囨爣璇?
    int iObjLock;//多余目标锁死
    int iSceneGo;//场景变换触发
    int iSceneMark;//场景变换标志位
    int iSceneCount;//场景变换计数
	int QuasiObjNum;
	int TotalObjNum;//ID=TotalObjNum+1;
	int framenum;
	int iImgWidth;
	int iImgHeigth;
	int iObjMinArea;//传入的百分比乘以宽高得到的，传入的数据存在规则里面
	int iObjMaxArea; //传入的百分比乘以宽高得到的，传入的数据存在规则里面
}XM_INIT_PARA;

#endif

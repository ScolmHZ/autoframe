
#ifndef _IVE_GMM_TEXT
#define _IVE_GMM_TEXT


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "math.h"
#include "hi_comm_ive.h"
#include "hi_type.h"
#include "hi_ive.h"
#include "ia_struct_para.h"
#include "imp_avd_para.h"


#ifdef _XM_IA_lINUX_
#include <pthread.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "hi_comm_sys.h"
#include "mpi_sys.h"
#endif


//#define _XM_IA_DEBUG_
#define _XM_IA_PRINT_

#ifdef __cplusplus
extern "C" {
#endif

#define HISI_IVE_FAIL (4)

#ifndef MIN
#define MIN(A, B) (A>=B?B:A)
#endif
#ifndef MAX
#define MAX(A,B) (A>B?A:B)
#endif

HI_U32 Frame_Sub_Thr(XM_INIT_PARA *pstInitPara);

HI_U16 XM_IA_IVE_CalcStride(HI_U16 u16Width, HI_U16 u16Align);

FILE* ReadTxtSequence(char* filepath, int filenum, char* filetype, FILE* frp);

void GMM_DES_IMAGE_S_TO_TXT(char* Fg_Output_Filepath, char* Bg_Output_Filepath,
	int output_filenum, char* filetype, IVE_GMM_S* Gmm);

HI_S32 IVE_CREATE_IMAGE_S(IVE_IMAGE_S *pstImg, IVE_IMAGE_TYPE_E enType, HI_U16 u16Width, HI_U16 u16Height);

HI_S32 IVE_CreateMemInfo(IVE_MEM_INFO_S*pstMemInfo, HI_U32 u32Size);

HI_VOID COMM_IVE_BlobToRect(IVE_CCBLOB_S *pstBlob, RECT_ARRAY_S *pstRect,
	HI_U16 u16RectMaxNum, HI_U16 u16AreaThrStep, int imageWid, int imageHgt);

void RectToTxt(char* Rect_Output_Filepath, int output_filenum, char* filetype, RECT_ARRAY_S stRegion);

HI_S32 CCLProc(IVE_GMM_LK_S *pstGmmLk, int num);

HI_S32 IVE_GMM_LK_INIT(IVE_GMM_LK_S *pstGmmLk, HI_U16 u16Width, HI_U16 u16Height, ALGO_MODULE_E eAlgType);

HI_S32 SAMPLE_IVE_PyrDown(IVE_ST_LK_S *pstStLk, IVE_SRC_IMAGE_S *pstSrc, IVE_DST_IMAGE_S *pstDst);

HI_S32 SAMPLE_IVE_CopyPyr222(IVE_IMAGE_S *pstPyrSrc, IVE_IMAGE_S *pstPyrDst, HI_U32 u32Level);

void ConstructFileName(char* Corner_Output_Filepath, int output_filenum, char* filetype);

HI_S32 CclCorner(IVE_HANDLE* IveHandle, XM_INIT_PARA* pstInitPara);

HI_S32 PointBelongCCL(IVE_POINT_S25Q7_S point, RECT_ARRAY_S stRegion);

HI_S32 LK_Pre_Proc(IVE_GMM_LK_S *pstGmmLk, int TotalCorner);

HI_S32 LK_Follow_Proc(IVE_GMM_LK_S *pstGmmLk, int TotalCorner);

HI_VOID	Corner_CCL_Distribute(IVE_GMM_LK_S *pstGmmLk, Frame_Object *Cur_Frame_Obj, XM_INIT_PARA *pstInitPara, int* TotalCorner, int* TotalObjNum,
	IVE_POINT_S25Q7_S LKPoint[], IVE_POINT_S25Q7_S TempPoint[], IVE_MV_S9Q7_S TempMv[]);

HI_S32 Cur_Frame_Obj_Order(XM_INIT_PARA *pstInitPara);

HI_S32  New_Corner(XM_INIT_PARA *pstInitPara);

HI_U32 Frame_Sub(XM_INIT_PARA *pstInitPara);

int mean(int *a, int count);

void Orientation1(XM_INIT_PARA *pstInitPara);

void Orientation2(Frame_Object* Cur_Frame_Obj);


HI_S32 xmREGIONToCCBLOB(RECT_ARRAY_S *pstRect, IVE_CCBLOB_S *pstBlob);

float triangleArea(URP_IMP_POINT_S a, URP_IMP_POINT_S b, URP_IMP_POINT_S c);

int pInQuadrangle_Init(URP_IMP_POINT_S a, URP_IMP_POINT_S b, URP_IMP_POINT_S c, URP_IMP_POINT_S d, URP_IMP_POINT_S p);

int pInQuadrangle_InOut(URP_IMP_POINT_S a, URP_IMP_POINT_S b, URP_IMP_POINT_S c, URP_IMP_POINT_S d, URP_IMP_POINT_S p, URP_IMP_POINT_S rect, int level, int Mode);

int IveImageCopy(IVE_IMAGE_S *pstSrc, IVE_IMAGE_S *pstDst);

int LineQuadrangleJudge(URP_IMP_POINT_S s1, URP_IMP_POINT_S s2, URP_IMP_POINT_S c, int level);

float CentDistance(URP_IMP_POINT_S oCent, URP_IMP_POINT_S sCent);

void SceneChange_Judge(XM_INIT_PARA *pstInitPara);

void Polygon_Judge(XM_INIT_PARA *pstInitPara);

void Pilfer_Judge(XM_INIT_PARA *pstInitPara);

float SceneChange_NCC(XM_INIT_PARA *pstInitPara, IMP_RECT_S stCut,IVE_SRC_IMAGE_S *stNCCSrc,IVE_DST_IMAGE_S *stNCCDst);

float FrameObj_NCC(XM_INIT_PARA *pstInitPara, IMP_RECT_S stCut,IVE_SRC_IMAGE_S *stNCCSrc,IVE_DST_IMAGE_S *stNCCDst);

int FrameSub_Objects(XM_INIT_PARA *pstInitPara, IVE_SRC_IMAGE_S stOriginal, IVE_DST_IMAGE_S stOriginalDst, IMP_RECT_S stRect, int k);

void IA_Printf(const char *format,...);

#ifdef __cplusplus
}
#endif


#endif
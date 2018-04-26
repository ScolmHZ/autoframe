
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hi_comm_ive.h"
#include "hi_type.h"
#include "hi_ive.h"
#include "mpi_ive.h"

#include "ia_interface.h"
#include "ia_subfunc.h"
#include "ia_remnantdetection.h"
#include "imp_avd_para.h"

#ifdef _XM_IA_lINUX_
#include "hi_comm_sys.h"
#include "mpi_sys.h"
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>
#endif

//extern URP_PARA_S stURPpara;

#ifdef _XM_IA_MEM_
#endif

int XM_Video_ENDEC(XM_INIT_PARA *pstInitPara, INTEL_CODING *pstMapCfg, QP_RESULT* stQPResult)
{
    HI_U32 i, j, k;
    HI_U32 m, n;
    HI_U32 u32RatX;
    HI_U32 u32RatY;

   // HI_S32 s32HighQP;
    //HI_S32 s32LowQp;

    HI_U16 u16Width;
    HI_U16 u16Height;
    HI_U16 u16MBWid = 8;
    HI_U16 u16MBHgt = 8;

    HI_U16 u16MinX = 0;
    HI_U16 u16MinY = 0;
    HI_U16 u16MaxX = 0;
    HI_U16 u16MaxY = 0;

    HI_U16 u16TagHgt;
    HI_U16 u16TagWid;
    HI_U16 u16TagMBWid = 16;
    HI_U16 u16TagMBHgt = 16;

    HI_U16 u16MBIdx;
    HI_U16 u16MBNum;
    HI_U16 u16PixVal;
    HI_U16 u16PixNum;
    HI_U16 u16HorMBNum;
    HI_U16 u16VerMBNum;
//    HI_U16 u16TagHorMBNum;
    HI_U16 u16ThdVal = 8;

//    HI_S8 s8MBMark;
    HI_S8 *as8Mark;

    RECT_ARRAY_S *pstObjRect;
    IVE_IMAGE_S *pstFgImg;

    pstObjRect = &pstInitPara->pstGmmLk->stGMM.stRegion;
    pstFgImg = &pstInitPara->pstGmmLk->stGMM.stERODEImg;

    u16Width = pstInitPara->iImgWidth;
    u16Height = pstInitPara->iImgHeigth;

//    s32HighQP = pstMapCfg->highQP;
    //s32LowQp = pstMapCfg->lowQP;
    u16TagWid = pstMapCfg->width;
    u16TagHgt = pstMapCfg->hight;
    
    u16HorMBNum = (u16Width + u16MBWid - 1) / u16MBWid;
    u16VerMBNum = (u16Height + u16MBHgt - 1) / u16MBHgt;
    u16MBNum = u16HorMBNum * u16VerMBNum;

//    u16TagHorMBNum = (u16TagWid + u16TagMBWid - 1) / u16TagMBWid;

    as8Mark = (HI_S8*)malloc(sizeof(HI_S8) * u16MBNum);
    
    if (as8Mark == NULL)
    {
        printf("as8Mark malloc failed!");
    }
    
    memset(as8Mark, 0, sizeof(HI_S8) * u16MBNum);

    memset(stQPResult->ROIseat, 0, sizeof(IVE_RECT_S) * 6);
    
    u32RatX = (u16TagWid << 8) / u16Width;
    u32RatY = (u16TagHgt << 8) / u16Height;

    if (pstObjRect->u16Num <= 0)
    {
        stQPResult->Flag = 0;
        return 0;
    }
    stQPResult->Flag = pstObjRect->u16Num;    

    for (k = 0; k < pstObjRect->u16Num; k++)
    {
        HI_U16 u16RoiMinX = pstObjRect->astRect[k].astPoint[0].s32X;
        HI_U16 u16RoiMinY = pstObjRect->astRect[k].astPoint[0].s32Y;
        HI_U16 u16RoiMaxX = pstObjRect->astRect[k].astPoint[2].s32X;
        HI_U16 u16RoiMaxY = pstObjRect->astRect[k].astPoint[2].s32Y;

        HI_U16 u16MBMinC = (u16RoiMinX / u16MBWid) * u16MBWid;
        HI_U16 u16MBMaxC = (u16RoiMaxX / u16MBWid) * u16MBWid;
        HI_U16 u16MBMinR = (u16RoiMinY / u16MBHgt) * u16MBHgt;
        HI_U16 u16MBMaxR = (u16RoiMaxY / u16MBHgt) * u16MBHgt;

        u16MinX = u16MBMinC * u32RatX >> 8;
        u16MinY = u16MBMinR * u32RatY >> 8;
        u16MaxX = u16MBMaxC * u32RatX >> 8;
        u16MaxY = u16MBMaxR * u32RatY >> 8;

        u16MinX = u16MinX / u16TagMBWid * u16TagMBWid;
        u16MinY = u16MinY / u16TagMBHgt * u16TagMBHgt;
        u16MaxX = u16MaxX / u16TagMBWid * u16TagMBWid;
        u16MaxY = u16MaxY / u16TagMBHgt * u16TagMBHgt;

        stQPResult->ROIseat[k].u16X = MAX(u16MinX - u16TagMBWid, 0);
        stQPResult->ROIseat[k].u16Y = MAX(u16MinY - u16TagMBHgt, 0);
        stQPResult->ROIseat[k].u16Width = MIN(u16MaxX + 4 * u16TagMBWid, u16TagWid - 1) - stQPResult->ROIseat[k].u16X;
        stQPResult->ROIseat[k].u16Height = MIN(u16MaxY + 4 * u16TagMBHgt, u16TagHgt - 1) - stQPResult->ROIseat[k].u16Y;

        for (i = u16MBMinR; i <= u16MBMaxR; i += u16MBHgt)
        {
            for (j = u16MBMinC; j <= u16MBMaxC; j += u16MBWid)
            {
                u16PixNum = 0;
                u16MBIdx = (i / u16MBHgt) * u16HorMBNum + j / u16MBWid ;
                for (m = 0; m < u16MBHgt; m++)
                {
                    for (n = 0; n < u16MBWid; n++)
                    {
                        u16PixVal = pstFgImg->pu8VirAddr[0][(m + i) * u16Width + (n + j)];
                        if (u16PixVal > 0)
                        {
                            u16PixNum += 1;
                        }
                    }
                }
                if (u16PixNum > u16ThdVal)
                {
                    as8Mark[u16MBIdx] = 1;
                }
            }
        } 
    }

    /* 利用小图标记映射到源图宏块 */
 #if 0
    for (k = 0; k < u16MBNum; k++)
    {
        s8MBMark = as8Mark[k];
        if (s8MBMark != 1)
        {
            continue;
        }
        HI_U16 u16MBCol = (k % u16HorMBNum) * u16MBWid;;
        HI_U16 u16MBRow = (k / u16HorMBNum) * u16MBHgt;;
        HI_U16 u16MBMinX = (u16MBCol * u32RatX) >> 8;
        HI_U16 u16MBMinY = (u16MBRow * u32RatY) >> 8;
        HI_U16 u16MBMaxX = ((u16MBCol + u16MBWid) * u32RatX) >> 8;
        HI_U16 u16MBMaxY = ((u16MBRow + u16MBHgt) * u32RatY) >> 8;
        
        u16MBMinX = (u16MBMinX / u16TagMBWid) * u16TagMBWid;
        u16MBMinY = (u16MBMinY / u16TagMBHgt) * u16TagMBHgt;

        u16MBMinX = MAX(u16MBMinX - u16TagMBWid, 0);
        u16MBMinY = MAX(u16MBMinY - u16TagMBHgt, 0);
        u16MBMaxX = MIN(u16MBMaxX + u16TagMBWid, u16TagWid - 1);
        u16MBMaxY = MIN(u16MBMaxY + u16TagMBHgt, u16TagHgt - 1);

        for (i = u16MBMinY; i < u16MBMaxY; i += u16TagMBWid)
        {
            for (j = u16MBMinX; j < u16MBMaxX; j += u16TagMBHgt)
            {
                u16MBIdx = (i / u16TagMBWid) * u16TagHorMBNum + j / u16TagMBHgt;
             /  stQPResult->QPmap[u16MBIdx] = (char)s32HighQP;
            }
        }
    }
#endif
    free(as8Mark);

    return 0;
}

int XM_Image_Resize(IVE_IMAGE_S *pstImgI, IVE_IMAGE_S *pstImgO)
{
    HI_U8* RESTRICT pucImgSrc = pstImgI->pu8VirAddr[0];
    HI_U8* RESTRICT pucImgDst = pstImgO->pu8VirAddr[0];
    HI_S32 iSrcHgt = pstImgI->u16Height, iSrcWid = pstImgI->u16Width;
    HI_S32 iDstHgt = pstImgO->u16Height, iDstWid = pstImgO->u16Width;
    HI_S32 iSrcStride = (pstImgI->u16Stride[0]);
    HI_S32 iDstStride = (pstImgO->u16Stride[0]);

    HI_U8 * RESTRICT pucSrc = NULL, *RESTRICT pucTmp = NULL, *RESTRICT pucPp = NULL;
    HI_S32 iXnxt = 0, iXpre = 0, iXnxt1 = 0, iXpre1 = 0;
    HI_S32 i = 0, j = 0, iXdiff = 0, iXdiff1 = 0, iYdiff1 = 0;
    HI_S32 iX = 0, iY = 0, iX0 = 0, iX1 = 0, iXinv = 0, iYInv = 0;
    HI_S32 iFyInt = 0, iFxInt = 0, iZoomX = 0, iZoomY = 0;

    iZoomX = (iSrcWid << 10) / iDstWid;
    iZoomY = (iSrcHgt << 10) / iDstHgt;

    for (i = 0; i < iDstHgt; i++)
    {
        iYInv = i * iZoomY;

        /* 取上面的整数行 */
        iY = (iYInv >> 10);

        /* 计算转换误差 */
        iFyInt = iYInv - (iY << 10);

        pucSrc = pucImgSrc + iY * iSrcStride;

        for (j = 0; j < iDstWid; j++)
        {
            /* 变换到原图像对应的浮点列数值 */
            iXinv = j * iZoomX;

            /* 取上面的整数列 */
            iX = (iXinv >> 10);

            /* 计算当前像素地址 */
            pucTmp = pucSrc + iX;

            pucPp = &pucImgDst[i * iDstStride + j];

            /* 计算转换误差 */
            iFxInt = (iXinv - (iX << 10));

            iXpre = *pucTmp++;
            iXnxt = *pucTmp++;

            pucTmp = pucTmp + iSrcStride - 2;

            iXpre1 = *pucTmp++;
            iXnxt1 = *pucTmp++;

            iXdiff = iXnxt - iXpre;
            iXdiff = iFxInt * iXdiff;
            iXdiff >>= 10;

            iXdiff1 = iXnxt1 - iXpre1;
            iXdiff1 = iFxInt * iXdiff1;
            iXdiff1 >>= 10;

            iX0 = iXpre + iXdiff;
            iX1 = iXpre1 + iXdiff1;

            iYdiff1 = iX1 - iX0;
            iYdiff1 *= iFyInt;
            iYdiff1 = (iYdiff1 >> 10);
            *pucPp = (HI_U8)(iX0 + iYdiff1);

        }
    }

    return 0;
}

int XM_IA_Init(int iImageWid, int iImageHgt, ALGO_MODULE_E eAlgType, void **pvHandle)   //eAlgType的初始化没有做
{
    int i;
    HI_S32 s32Ret;
    int iSuccess = 0;

    if(iImageWid < 156)
    {
        printf("ERROR! XM_PARA_FAULT ! Input Image Width should be greater than 256!\n");
        return XM_PARA_FAULT;
    }
    if(iImageHgt < 156)
    {
        printf("ERROR! XM_PARA_FAULT ! Input Image Height should be greater than 256!\n");
        return XM_PARA_FAULT;
    }
   
    XM_INIT_PARA *pstInitPara = (XM_INIT_PARA *)malloc(sizeof(XM_INIT_PARA));
    
    if (NULL == pstInitPara)
    {
        printf("pstInitPara malloc failed!\n");
        return XM_MALLOC_FAIL;
    }
    memset(pstInitPara, 0, sizeof(XM_INIT_PARA));
  
#ifdef _XM_IA_MEM_
    size += sizeof(XM_INIT_PARA);
#endif

    *pvHandle = pstInitPara;
   
    pstInitPara->pstGmmLk = (IVE_GMM_LK_S *)malloc(sizeof(IVE_GMM_LK_S));
 
    if (pstInitPara->pstGmmLk == NULL)
    {
        printf("pstInitPara->pstGmmLk malloc failed!\n");
        return XM_MALLOC_FAIL;
    }
    printf("pstInitPara->pstGmmLk = 0x%x\n",(int)pstInitPara->pstGmmLk);
#ifdef _XM_IA_MEM_
    size += sizeof(IVE_GMM_LK_S);
#endif
    
    iSuccess = IVE_GMM_LK_INIT(pstInitPara->pstGmmLk, iImageWid, iImageHgt, eAlgType);//类型决定学习率
    if (HI_SUCCESS != iSuccess)
    {
        printf("IVE_GMM_LK_INIT failed!\n");
        return XM_MALLOC_FAIL;
    }
 
    pstInitPara->pstCur_Frame_Obj = (Frame_Object *)malloc(sizeof(Frame_Object));
    if (NULL == pstInitPara->pstCur_Frame_Obj)
    {
        printf("pstInitPara->pstCur_Frame_Obj malloc failed!\n");
        return XM_MALLOC_FAIL;
    }
    memset(pstInitPara->pstCur_Frame_Obj, 0, sizeof(Frame_Object));
   
#ifdef _XM_IA_MEM_
    size += sizeof(Frame_Object);
#endif

    for (i = 0; i < MAX_OBJ_NUM; i++)
    {
        pstInitPara->pstCur_Frame_Obj->Object[i].CCLRegionNum = 100;
    }

    pstInitPara->CurrentCCL = (int *)malloc(sizeof(int) * MAX_OBJ_NUM);
    if (NULL == pstInitPara->CurrentCCL)
    {
        printf("pstInitPara->CurrentCCL malloc failed!\n");
        return XM_MALLOC_FAIL;
    }
    memset(pstInitPara->CurrentCCL, 0, sizeof(int) * MAX_OBJ_NUM);

#ifdef _XM_IA_MEM_
    size += sizeof(int) * MAX_OBJ_NUM;
#endif

    pstInitPara->DoNewCorner = (int *)malloc(sizeof(int) * MAX_OBJ_NUM);
    if (NULL == pstInitPara->DoNewCorner)
    {
        printf("pstInitPara->DoNewCorner malloc failed!\n");
        return XM_MALLOC_FAIL;
    }
    memset(pstInitPara->DoNewCorner, 0, sizeof(int) * MAX_OBJ_NUM);

#ifdef _XM_IA_MEM_
    size += sizeof(int) * MAX_OBJ_NUM;
#endif
    
    pstInitPara->LKPoint = (IVE_POINT_S25Q7_S *)malloc(sizeof(IVE_POINT_S25Q7_S) * MAX_OBJ_NUM * MAX_POINT_NUM);
    if (NULL == pstInitPara->LKPoint)
    {
        printf("pstInitPara->LKPoint malloc failed!\n");
        return XM_MALLOC_FAIL;
    }
    memset(pstInitPara->LKPoint, 0, sizeof(IVE_POINT_S25Q7_S) * MAX_OBJ_NUM * MAX_POINT_NUM);

#ifdef _XM_IA_MEM_
    size += sizeof(IVE_POINT_S25Q7_S) * MAX_OBJ_NUM * MAX_POINT_NUM;
#endif

    pstInitPara->TempPoint = (IVE_POINT_S25Q7_S *)malloc(sizeof(IVE_POINT_S25Q7_S) * MAX_OBJ_NUM * MAX_POINT_NUM);
    if (NULL == pstInitPara->TempPoint)
    {
        printf("pstInitPara->TempPoint malloc failed!\n");
        return XM_MALLOC_FAIL;
    }
    memset(pstInitPara->TempPoint, 0, sizeof(IVE_POINT_S25Q7_S) * MAX_OBJ_NUM * MAX_POINT_NUM);

#ifdef _XM_IA_MEM_
    size += sizeof(IVE_POINT_S25Q7_S) * MAX_OBJ_NUM * MAX_POINT_NUM;
#endif

    pstInitPara->TempMv = (IVE_MV_S9Q7_S *)malloc(sizeof(IVE_MV_S9Q7_S) * MAX_OBJ_NUM * MAX_POINT_NUM);
    if (NULL == pstInitPara->TempMv)
    {
        printf("pstInitPara->TempMv malloc failed!\n");
        return XM_MALLOC_FAIL;
    }
    memset(pstInitPara->TempMv, 0, sizeof(IVE_MV_S9Q7_S) * MAX_OBJ_NUM * MAX_POINT_NUM);

#ifdef _XM_IA_MEM_
    size += sizeof(IVE_MV_S9Q7_S) * MAX_OBJ_NUM * MAX_POINT_NUM;
#endif
   
    pstInitPara->pstURPpara = (URP_PARA_S *)malloc(sizeof(URP_PARA_S));
    if (NULL == pstInitPara->pstURPpara)
    {
        printf("pstInitPara->pstURPpara malloc failed!\n");
        return XM_MALLOC_FAIL;
    }
    memset(pstInitPara->pstURPpara, 0, sizeof(URP_PARA_S));

#ifdef _XM_IA_MEM_
    size += sizeof(URP_PARA_S);
#endif

    pstInitPara->stAvdPara = (IMP_AVD_PARA_S *)malloc(sizeof(IMP_AVD_PARA_S));
    if (NULL == pstInitPara->stAvdPara)
    {
        printf("pstInitPara->stAvdPara malloc failed!\n");
        return XM_MALLOC_FAIL;
    }
    memset(pstInitPara->stAvdPara, 0, sizeof(IMP_AVD_PARA_S));

#ifdef _XM_IA_MEM_
        size += sizeof(IMP_AVD_PARA_S);
#endif

    pstInitPara->pstResult = (RESULT_S *)malloc(sizeof(RESULT_S));
    if (NULL == pstInitPara->pstResult)
    {
        printf("pstInitPara->pstResult malloc failed!\n");
        return XM_MALLOC_FAIL;
    }
    memset(pstInitPara->pstResult, 0, sizeof(RESULT_S));

#ifdef _XM_IA_MEM_
    size += sizeof(RESULT_S);
#endif
  
    pstInitPara->pstRemnantDetectionInfo = (XM_REMNANTDETECTION_INFO_S *)malloc(sizeof(XM_REMNANTDETECTION_INFO_S));
    if (NULL == pstInitPara->pstRemnantDetectionInfo)
    {
        printf("pstInitPara->pstRemnantDetectionInfo malloc failed!\n");
        return XM_MALLOC_FAIL;
    }
    memset(pstInitPara->pstRemnantDetectionInfo, 0, sizeof(XM_REMNANTDETECTION_INFO_S));

#ifdef _XM_IA_MEM_
    size += sizeof(XM_REMNANTDETECTION_INFO_S);
#endif

    s32Ret = IVE_CREATE_IMAGE_S(&(pstInitPara->pstRemnantDetectionInfo->stImageDiffFg), IVE_IMAGE_TYPE_U8C1, iImageWid, iImageHgt);
    if (s32Ret != HI_SUCCESS)
    {
        printf("Mmz Alloc stImageDiffFg fail,Error(%#x)\n", s32Ret);
        return HISI_IVE_FAIL;
    }

    pstInitPara->pstRemnantDetectionRes = (XM_REMNANTDETECTION_RES_S *)malloc(sizeof(XM_REMNANTDETECTION_RES_S));
    if (NULL == pstInitPara->pstRemnantDetectionRes)
    {
        printf("pstInitPara->pstRemnantDetectionRes malloc failed!\n");
        return XM_MALLOC_FAIL;
    }
    memset(pstInitPara->pstRemnantDetectionRes, 0, sizeof(XM_REMNANTDETECTION_RES_S));

#ifdef _XM_IA_MEM_
    size += sizeof(XM_REMNANTDETECTION_RES_S);
#endif

    pstInitPara->pstSubCtrl = (IVE_SUB_CTRL_S *)malloc(sizeof(IVE_SUB_CTRL_S));
    if (NULL == pstInitPara->pstSubCtrl)
    {
        printf("pstInitPara->pstSubCtrl malloc failed!\n");
        return XM_MALLOC_FAIL;
    }
    pstInitPara->pstSubCtrl->enMode = IVE_SUB_MODE_ABS;

#ifdef _XM_IA_MEM_
    size += sizeof(IVE_SUB_CTRL_S);
#endif

    pstInitPara->pstThrCtrl = (IVE_THRESH_CTRL_S *)malloc(sizeof(IVE_THRESH_CTRL_S));
    if (NULL == pstInitPara->pstThrCtrl)
    {
        printf("pstInitPara->pstThrCtrl malloc failed!\n");
        return XM_MALLOC_FAIL;
    }

#ifdef _XM_IA_MEM_
    size += sizeof(IVE_THRESH_CTRL_S);
#endif


    pstInitPara->pstThrCtrl->enMode = IVE_THRESH_MODE_BINARY;
    pstInitPara->pstThrCtrl->u8LowThr = 20;
    pstInitPara->pstThrCtrl->u8MinVal = 0;
    pstInitPara->pstThrCtrl->u8MaxVal = 255;

    pstInitPara->TotalCorner = 0;
    pstInitPara->QuasiObjNum = 0;
    pstInitPara->TotalObjNum = 0;
    pstInitPara->framenum = 1;
    pstInitPara->eAlgType = eAlgType;
    pstInitPara->iImgHeigth = iImageHgt;
    pstInitPara->iImgWidth= iImageWid;

    //默认的最小目标尺寸
    pstInitPara->iObjMinArea = OBJ_MIN_AREA;
    pstInitPara->iObjMaxArea = pstInitPara->iImgHeigth * pstInitPara->iImgWidth;

//    IA_Printf("XM_IA_Init OK\n");

    return XM_SUCCESS;
}

int XM_IA_Destroy(void **pvHandle)
{
#ifdef _XM_IA_lINUX_
    unsigned int i;
    XM_INIT_PARA *pstInitPara = *pvHandle;

    if (NULL != pstInitPara->pstGmmLk)
    {
        if (NULL != pstInitPara->pstGmmLk)
        {
            HI_MPI_SYS_MmzFree(pstInitPara->pstGmmLk->stGMM.stSrc.u32PhyAddr[0],
                pstInitPara->pstGmmLk->stGMM.stSrc.pu8VirAddr[0]);
            HI_MPI_SYS_MmzFree(pstInitPara->pstGmmLk->stGMM.stFg.u32PhyAddr[0],
                pstInitPara->pstGmmLk->stGMM.stFg.pu8VirAddr[0]);
            HI_MPI_SYS_MmzFree(pstInitPara->pstGmmLk->stGMM.stBg.u32PhyAddr[0],
                pstInitPara->pstGmmLk->stGMM.stBg.pu8VirAddr[0]);
            HI_MPI_SYS_MmzFree(pstInitPara->pstGmmLk->stGMM.stDILATEImg.u32PhyAddr[0],
                pstInitPara->pstGmmLk->stGMM.stDILATEImg.pu8VirAddr[0]);
            HI_MPI_SYS_MmzFree(pstInitPara->pstGmmLk->stGMM.stERODEImg.u32PhyAddr[0],
                pstInitPara->pstGmmLk->stGMM.stERODEImg.pu8VirAddr[0]);
            HI_MPI_SYS_MmzFree(pstInitPara->pstGmmLk->stGMM.stModel.u32PhyAddr,
                pstInitPara->pstGmmLk->stGMM.stModel.pu8VirAddr);
            HI_MPI_SYS_MmzFree(pstInitPara->pstGmmLk->stGMM.stBlob.u32PhyAddr,
                pstInitPara->pstGmmLk->stGMM.stBlob.pu8VirAddr);


            HI_MPI_SYS_MmzFree(pstInitPara->pstGmmLk->stLK.stSrc.u32PhyAddr[0],
                pstInitPara->pstGmmLk->stLK.stSrc.pu8VirAddr[0]);
            HI_MPI_SYS_MmzFree(pstInitPara->pstGmmLk->stLK.stDst.u32PhyAddr[0],
                pstInitPara->pstGmmLk->stLK.stDst.pu8VirAddr[0]);
            HI_MPI_SYS_MmzFree(pstInitPara->pstGmmLk->stLK.stPyrTmp.u32PhyAddr[0],
                pstInitPara->pstGmmLk->stLK.stPyrTmp.pu8VirAddr[0]);
            HI_MPI_SYS_MmzFree(pstInitPara->pstGmmLk->stLK.stStCandiCornerCtrl.stMem.u32PhyAddr,
                pstInitPara->pstGmmLk->stLK.stStCandiCornerCtrl.stMem.pu8VirAddr);
            HI_MPI_SYS_MmzFree(pstInitPara->pstGmmLk->stLK.stDstCorner.u32PhyAddr,
                pstInitPara->pstGmmLk->stLK.stDstCorner.pu8VirAddr);

            HI_MPI_SYS_MmzFree(pstInitPara->pstGmmLk->stLK.stPoly.u32PhyAddr[0],
                pstInitPara->pstGmmLk->stLK.stPoly.pu8VirAddr[0]);
            HI_MPI_SYS_MmzFree(pstInitPara->pstGmmLk->stLK.stPolyDst.u32PhyAddr[0],
                pstInitPara->pstGmmLk->stLK.stPolyDst.pu8VirAddr[0]);

            HI_MPI_SYS_MmzFree(pstInitPara->pstGmmLk->stLK.stPilfer.u32PhyAddr[0],
                pstInitPara->pstGmmLk->stLK.stPilfer.pu8VirAddr[0]);
            HI_MPI_SYS_MmzFree(pstInitPara->pstGmmLk->stLK.stPilferDst.u32PhyAddr[0],
                pstInitPara->pstGmmLk->stLK.stPilferDst.pu8VirAddr[0]);

            HI_MPI_SYS_MmzFree(pstInitPara->pstGmmLk->stLK.stSceneCh.u32PhyAddr[0],
                pstInitPara->pstGmmLk->stLK.stSceneCh.pu8VirAddr[0]);
            HI_MPI_SYS_MmzFree(pstInitPara->pstGmmLk->stLK.stSceneChDst.u32PhyAddr[0],
                pstInitPara->pstGmmLk->stLK.stSceneChDst.pu8VirAddr[0]);

            HI_MPI_SYS_MmzFree(pstInitPara->pstGmmLk->stLK.stNCCObj.u32PhyAddr[0],
                pstInitPara->pstGmmLk->stLK.stNCCObj.pu8VirAddr[0]);
            HI_MPI_SYS_MmzFree(pstInitPara->pstGmmLk->stLK.stNCCRObjDst.u32PhyAddr,
                pstInitPara->pstGmmLk->stLK.stNCCRObjDst.pu8VirAddr);

            HI_MPI_SYS_MmzFree(pstInitPara->pstGmmLk->stLK.stNCC.u32PhyAddr[0],
                pstInitPara->pstGmmLk->stLK.stNCC.pu8VirAddr[0]);
            HI_MPI_SYS_MmzFree(pstInitPara->pstGmmLk->stLK.stNCCRDst.u32PhyAddr,
                pstInitPara->pstGmmLk->stLK.stNCCRDst.pu8VirAddr);

            HI_MPI_SYS_MmzFree(pstInitPara->pstGmmLk->stGMM.stSobel.u32PhyAddr[0],
                          pstInitPara->pstGmmLk->stGMM.stSobel.pu8VirAddr[0]);

            HI_MPI_SYS_MmzFree(pstInitPara->pstGmmLk->stGMM.stSobelDst.u32PhyAddr[0],
                          pstInitPara->pstGmmLk->stGMM.stSobelDst.pu8VirAddr[0]);

            HI_MPI_SYS_MmzFree(pstInitPara->pstGmmLk->stGMM.stNCCDst1.u32PhyAddr[0],
                            pstInitPara->pstGmmLk->stGMM.stNCCDst1.pu8VirAddr[0]);

            HI_MPI_SYS_MmzFree(pstInitPara->pstGmmLk->stGMM.stNCCSrc1.u32PhyAddr[0],
                            pstInitPara->pstGmmLk->stGMM.stNCCSrc1.pu8VirAddr[0]);

           /* HI_MPI_SYS_MmzFree(pstInitPara->pstGmmLk->stGMM.stNCCDst2.u32PhyAddr[0],
                            pstInitPara->pstGmmLk->stGMM.stNCCDst2.pu8VirAddr[0]);

            HI_MPI_SYS_MmzFree(pstInitPara->pstGmmLk->stGMM.stNCCSrc2.u32PhyAddr[0],
                            pstInitPara->pstGmmLk->stGMM.stNCCSrc2.pu8VirAddr[0]);

            HI_MPI_SYS_MmzFree(pstInitPara->pstGmmLk->stGMM.stNCCDst3.u32PhyAddr[0],
                            pstInitPara->pstGmmLk->stGMM.stNCCDst3.pu8VirAddr[0]);

            HI_MPI_SYS_MmzFree(pstInitPara->pstGmmLk->stGMM.stNCCSrc3.u32PhyAddr[0],
                            pstInitPara->pstGmmLk->stGMM.stNCCSrc3.pu8VirAddr[0]);

            HI_MPI_SYS_MmzFree(pstInitPara->pstGmmLk->stGMM.stNCCDst4.u32PhyAddr[0],
                            pstInitPara->pstGmmLk->stGMM.stNCCDst4.pu8VirAddr[0]);

            HI_MPI_SYS_MmzFree(pstInitPara->pstGmmLk->stGMM.stNCCSrc4.u32PhyAddr[0],
                            pstInitPara->pstGmmLk->stGMM.stNCCSrc4.pu8VirAddr[0]);*/

 			HI_MPI_SYS_MmzFree(pstInitPara->pstGmmLk->stGMM.stPolySrc.u32PhyAddr[0],
                    		pstInitPara->pstGmmLk->stGMM.stPolySrc.pu8VirAddr[0]);

    		HI_MPI_SYS_MmzFree(pstInitPara->pstGmmLk->stGMM.stPolyDst.u32PhyAddr[0],
                    		pstInitPara->pstGmmLk->stGMM.stPolyDst.pu8VirAddr[0]);

            for (i = 3; i > 0; i--)
            {
                HI_MPI_SYS_MmzFree(pstInitPara->pstGmmLk->stLK.astPrePyr[i].u32PhyAddr[0],
                    pstInitPara->pstGmmLk->stLK.astPrePyr[i].pu8VirAddr[0]);
                HI_MPI_SYS_MmzFree(pstInitPara->pstGmmLk->stLK.astCurPyr[i].u32PhyAddr[0],
                    pstInitPara->pstGmmLk->stLK.astCurPyr[i].pu8VirAddr[0]);
                HI_MPI_SYS_MmzFree(pstInitPara->pstGmmLk->stLK.astPoint[i].u32PhyAddr,
                    pstInitPara->pstGmmLk->stLK.astPoint[i].pu8VirAddr);
            }

            HI_MPI_SYS_MmzFree(pstInitPara->pstGmmLk->stLK.stMv.u32PhyAddr,
                pstInitPara->pstGmmLk->stLK.stMv.pu8VirAddr);
        }

        free(pstInitPara->pstGmmLk);
    }

    if (NULL != pstInitPara->pstCur_Frame_Obj)
        free(pstInitPara->pstCur_Frame_Obj);
    if (NULL != pstInitPara->CurrentCCL)
        free(pstInitPara->CurrentCCL);
    if (NULL != pstInitPara->DoNewCorner)
        free(pstInitPara->DoNewCorner);
    if (NULL != pstInitPara->LKPoint)
        free(pstInitPara->LKPoint);
    if (NULL != pstInitPara->TempPoint)
        free(pstInitPara->TempPoint);
    if (NULL != pstInitPara->TempMv)
        free(pstInitPara->TempMv);

    if (NULL != pstInitPara->pstRemnantDetectionInfo)
    {
        HI_MPI_SYS_MmzFree(pstInitPara->pstRemnantDetectionInfo->stImageDiffFg.u32PhyAddr[0],
            pstInitPara->pstRemnantDetectionInfo->stImageDiffFg.pu8VirAddr[0]);

        free(pstInitPara->pstRemnantDetectionInfo);
    }

    if (NULL != pstInitPara->pstRemnantDetectionRes)
        free(pstInitPara->pstRemnantDetectionRes);

    if (NULL != pstInitPara->pstURPpara)
    {
        free(pstInitPara->pstURPpara);
    }

    if (NULL != pstInitPara->pstResult)
    {
        free(pstInitPara->pstResult);
    }

    if (NULL != pstInitPara->pstSubCtrl)
    {
        free(pstInitPara->pstSubCtrl);
    }

    if (NULL != pstInitPara->pstThrCtrl)
    {
        free(pstInitPara->pstThrCtrl);
    }

    pstInitPara->pstGmmLk = NULL;
    pstInitPara->pstCur_Frame_Obj = NULL;
    pstInitPara->CurrentCCL = NULL;
    pstInitPara->DoNewCorner = NULL;
    pstInitPara->LKPoint = NULL;
    pstInitPara->TempPoint = NULL;
    pstInitPara->TempMv = NULL;
    pstInitPara->pstURPpara = NULL;
    pstInitPara->pstRemnantDetectionInfo = NULL;
    pstInitPara->pstRemnantDetectionRes = NULL;
    pstInitPara->pstSubCtrl = NULL;
    pstInitPara->pstThrCtrl = NULL;

    if (NULL != pstInitPara)
    {
        free(pstInitPara);
    }
    pstInitPara = NULL;

//    printf("XM_IA_Destroy OK\n");

#endif
    return XM_SUCCESS;
}

int XM_IA_JudgeLine(Struct_Object *pstCurObj, URP_LINE_S *pstLine)
{
	///判断中心点有没有越线
    /*-------------返回值-------------*/
    /*----若垂直：左边返回1，右边返回-1*/
    /*----不垂直：上面返回2，下面返回-2*/
	
    int iRet = 0;
    int iDiffX = pstLine->stStartPt.s16X - pstLine->stEndPt.s16X;
    int iDiffY = pstLine->stStartPt.s16Y - pstLine->stEndPt.s16Y;

    //绊线垂直
    if (0 == iDiffX)
    {
        if (((HI_S16)pstCurObj->center.u16X - pstLine->stStartPt.s16X) < 0)//中心点在线的左边
        {
            iRet = -2;
        }
        else
        {
            iRet = 2;
        }
    }
    if (iDiffX != 0)
    {
        float flScope = ((float)iDiffY) / iDiffX;//斜率
        float flDist = pstLine->stStartPt.s16Y - flScope * pstLine->stStartPt.s16X;//截距

        int iCentY = pstCurObj->center.u16Y - flScope * pstCurObj->center.u16X - flDist;//坐标Y分量正负没区分

        //在绊线的方位
        if (iCentY > 0)
        {
            iRet = -2;//在设定线下方
        }
        else
        {
            iRet = 2;//在设定线上方
        }
    }

    short sMaxX = pstLine->stEndPt.s16X;
    short sMinX = pstLine->stStartPt.s16X;
    short sMaxY = pstLine->stEndPt.s16Y;
    short sMinY = pstLine->stStartPt.s16Y;
    short sDistX = 0, sDistY = 0;

    if (pstLine->stEndPt.s16X < pstLine->stStartPt.s16X)
    {
        sMaxX = pstLine->stStartPt.s16X;
        sMinX = pstLine->stEndPt.s16X;
    }

    if (pstLine->stEndPt.s16Y < pstLine->stStartPt.s16Y)
    {
        sMaxY = pstLine->stStartPt.s16Y;
        sMinY = pstLine->stEndPt.s16Y;
    }

    sDistX = sMaxX - sMinX;
    sDistY = sMaxY - sMinY;

    if (sDistX > sDistY)
    {
        if (pstCurObj->center.u16X > sMaxX || pstCurObj->center.u16X < sMinX)
        {
            IA_Printf("Obj Id %d not touch line\n", pstCurObj->ObjectID);

            iRet = iRet/2;
        }
    }
    else
    {
        if (pstCurObj->center.u16Y > sMaxY || pstCurObj->center.u16Y < sMinY)
        {
            IA_Printf("Obj Id %d not touch line\n", pstCurObj->ObjectID);
            iRet = iRet/2;
        }
    }

    return iRet;
}

void XM_IA_Rule_Judge(void *pvHandle, URP_PARA_S *pstURPpara)
{
    int i, j, k;
    int iEventNum = 0;//事件数目
    int iRet = -1, iBreakRule = 0, iRet2 = 0,iRangelevel = 1;
    float fDistance;
    int iGonRes ;

    //警戒等级
    iRangelevel = pstURPpara->stRuleSet.astRule[0].u32Level;

    XM_INIT_PARA *pstInitPara = (XM_INIT_PARA *)pvHandle;

    for (i = 0; i < pstInitPara->pstCur_Frame_Obj->ObjectNum; i++)
    {
        iBreakRule = 0;
        //for (k = 0; k < IMP_URP_MAX_NUM_RULE_NUM; k++)
        for (k = 0; k < 1; k++)
        {
#ifdef _XM_IA_PRINT_
            if (IMP_FUNC_TRIPWIRE == (pstURPpara->stRuleSet.astRule[k].u32Mode & IMP_FUNC_TRIPWIRE))
            {
                printf("Judge  u32Mode %x Line start[%d %d] End[%d %d] ForbiddenDirection %d \n",
                    pstURPpara->stRuleSet.astRule[k].u32Mode,
                    pstURPpara->stRuleSet.astRule[k].stPara.stTripwireRulePara.astLines[0].stLine.stStartPt.s16X,
                    pstURPpara->stRuleSet.astRule[k].stPara.stTripwireRulePara.astLines[0].stLine.stStartPt.s16Y,
                    pstURPpara->stRuleSet.astRule[k].stPara.stTripwireRulePara.astLines[0].stLine.stEndPt.s16X,
                    pstURPpara->stRuleSet.astRule[k].stPara.stTripwireRulePara.astLines[0].stLine.stEndPt.s16Y,
                    pstURPpara->stRuleSet.astRule[k].stPara.stTripwireRulePara.astLines[0].s32ForbiddenDirection);
            }
            else if (IMP_FUNC_PERIMETER == (pstURPpara->stRuleSet.astRule[k].u32Mode & IMP_FUNC_PERIMETER))
            {
                printf("Judge  u32Mode %x P1[%d %d] P2[%d %d] P3[%d %d] P4[%d %d] ForbiddenDirection %d \n",
                    pstURPpara->stRuleSet.astRule[k].u32Mode,
                    pstURPpara->stRuleSet.astRule[k].stPara.stPerimeterRulePara.stLimitPara.stBoundary.astBoundaryPts[0].s16X,
                    pstURPpara->stRuleSet.astRule[k].stPara.stPerimeterRulePara.stLimitPara.stBoundary.astBoundaryPts[0].s16Y,
                    pstURPpara->stRuleSet.astRule[k].stPara.stPerimeterRulePara.stLimitPara.stBoundary.astBoundaryPts[1].s16X,
                    pstURPpara->stRuleSet.astRule[k].stPara.stPerimeterRulePara.stLimitPara.stBoundary.astBoundaryPts[1].s16Y,
                    pstURPpara->stRuleSet.astRule[k].stPara.stPerimeterRulePara.stLimitPara.stBoundary.astBoundaryPts[2].s16X,
                    pstURPpara->stRuleSet.astRule[k].stPara.stPerimeterRulePara.stLimitPara.stBoundary.astBoundaryPts[2].s16Y,
                    pstURPpara->stRuleSet.astRule[k].stPara.stPerimeterRulePara.stLimitPara.stBoundary.astBoundaryPts[3].s16X,
                    pstURPpara->stRuleSet.astRule[k].stPara.stPerimeterRulePara.stLimitPara.stBoundary.astBoundaryPts[3].s16Y,
                    pstURPpara->stRuleSet.astRule[k].stPara.stPerimeterRulePara.stLimitPara.s32ForbiddenDirection);
            }
            else if (IMP_FUNC_OSC == (pstURPpara->stRuleSet.astRule[k].u32Mode & IMP_FUNC_OSC))
            {
                printf("Judge  u32Mode %x P1[%d %d] P2[%d %d] P3[%d %d] P4[%d %d]\n",
                    pstURPpara->stRuleSet.astRule[k].u32Mode,
                    pstInitPara->pstURPpara->stRuleSet.astRule[k].stPara.stOscRulePara.astSpclRgs[0].stOscRg.astPoint[0].s16X,
                    pstInitPara->pstURPpara->stRuleSet.astRule[k].stPara.stOscRulePara.astSpclRgs[0].stOscRg.astPoint[0].s16Y,
                    pstInitPara->pstURPpara->stRuleSet.astRule[k].stPara.stOscRulePara.astSpclRgs[0].stOscRg.astPoint[1].s16X,
                    pstInitPara->pstURPpara->stRuleSet.astRule[k].stPara.stOscRulePara.astSpclRgs[0].stOscRg.astPoint[1].s16Y,
                    pstInitPara->pstURPpara->stRuleSet.astRule[k].stPara.stOscRulePara.astSpclRgs[0].stOscRg.astPoint[2].s16X,
                    pstInitPara->pstURPpara->stRuleSet.astRule[k].stPara.stOscRulePara.astSpclRgs[0].stOscRg.astPoint[2].s16Y,
                    pstInitPara->pstURPpara->stRuleSet.astRule[k].stPara.stOscRulePara.astSpclRgs[0].stOscRg.astPoint[3].s16X,
                    pstInitPara->pstURPpara->stRuleSet.astRule[k].stPara.stOscRulePara.astSpclRgs[0].stOscRg.astPoint[3].s16Y);
            }
#endif

            URP_IMP_POINT_S stCent;
            stCent.s16X = pstInitPara->pstCur_Frame_Obj->Object[i].center.u16X;
            stCent.s16Y = pstInitPara->pstCur_Frame_Obj->Object[i].center.u16Y;

            URP_IMP_POINT_S stOrCent;
            stOrCent.s16X = pstInitPara->pstCur_Frame_Obj->Object[i].Origin_center.u16X;
            stOrCent.s16Y = pstInitPara->pstCur_Frame_Obj->Object[i].Origin_center.u16Y;

            //移动距离
            fDistance = CentDistance(stOrCent, stCent);

            if (IMP_FUNC_TRIPWIRE == (pstURPpara->stRuleSet.astRule[k].u32Mode & IMP_FUNC_TRIPWIRE))
            {
                //for (j = 0; j < IMP_URP_MAX_TRIPWIRE_CNT; j++)
                for (j = 0; j < 1; j++)
                {
                    iRet = XM_IA_JudgeLine(pstInitPara->pstCur_Frame_Obj->Object + i,
                        &pstURPpara->stRuleSet.astRule[k].stPara.stTripwireRulePara.astLines[j].stLine);

                    //触发标识
                    iRet2 = LineQuadrangleJudge(pstURPpara->stRuleSet.astRule[k].stPara.stTripwireRulePara.astLines[0].stLine.stStartPt,
                        pstURPpara->stRuleSet.astRule[k].stPara.stTripwireRulePara.astLines[0].stLine.stEndPt,
                        stCent, iRangelevel/*敏感等级*/);

#ifdef _XM_IA_PRINT_
                    printf("Judge  ID %d  iRet %d iTouchLine : %d Cent[%d %d] dx %d dy %d\n",
                        pstInitPara->pstCur_Frame_Obj->Object[i].ObjectID, iRet,
                        pstInitPara->pstCur_Frame_Obj->Object[i].iTouchLine,
                        pstInitPara->pstCur_Frame_Obj->Object[i].center.u16X,
                        pstInitPara->pstCur_Frame_Obj->Object[i].center.u16Y,
                        pstInitPara->pstCur_Frame_Obj->Object[i].Motion.dx,
                        pstInitPara->pstCur_Frame_Obj->Object[i].Motion.dy);
                    printf("Oringin_centerX:%d Oringin_centerY:%d\n", stOrCent.s16X, stOrCent.s16Y);
                    printf("juli:%d ", pstURPpara->stRuleSet.astRule[0].stPara.stTripwireRulePara.stLimitPara.s32MinDist);
                    printf(" Distance:%f\n", fDistance);
                    printf(" iMark:%d\n", pstInitPara->pstCur_Frame_Obj->Object[i].iMark);
#endif
                    if ((0 == pstInitPara->pstCur_Frame_Obj->Object[i].iTouchLine) && (iRet2 < 0))
                    {
                        pstInitPara->pstCur_Frame_Obj->Object[i].iTouchLine = iRet;
                    }


                    if(pstInitPara->pstCur_Frame_Obj->Object[i].iCount > 0)
                    {
                        pstInitPara->pstCur_Frame_Obj->Object[i].iCount ++;
                    }
                    if(pstInitPara->pstCur_Frame_Obj->Object[i].iCount > 10)
                    {
                        pstInitPara->pstCur_Frame_Obj->Object[i].iCount = 0;
                    }

                    if (((iRet2 > 0) && (pstInitPara->pstCur_Frame_Obj->Object[i].iMark == 0)) &&
                       (fDistance > pstURPpara->stRuleSet.astRule[0].stPara.stTripwireRulePara.stLimitPara.s32MinDist))
                    {
                        IA_Printf("Object:%d--ALARM!!!!\n", pstInitPara->pstCur_Frame_Obj->Object[i].ObjectID);

                        //从绊线下方穿过绊线上方
                        if ((((pstInitPara->pstCur_Frame_Obj->Object[i].iTouchLine < 0) && (iRet2 > 0) && (iRet == -2)) &&
                            ((2 == (2 & pstURPpara->stRuleSet.astRule[k].stPara.stTripwireRulePara.astLines[j].s32ForbiddenDirection)) ||
                            (3 == (3 &  pstURPpara->stRuleSet.astRule[k].stPara.stTripwireRulePara.astLines[j].s32ForbiddenDirection))))&&
                            (pstInitPara->pstCur_Frame_Obj->Object[i].iCount == 0))
                        {
                            iBreakRule = 1;
                            IA_Printf("Object:%d banxian down2up\n" , pstInitPara->pstCur_Frame_Obj->Object[i].ObjectID);
                            pstInitPara->pstCur_Frame_Obj->Object[i].iMark = 1;
                            pstInitPara->pstCur_Frame_Obj->Object[i].iCount ++;
                        }

                        else if((((pstInitPara->pstCur_Frame_Obj->Object[i].iTouchLine > 0) && (iRet2 > 0) && (iRet == 2)) &&
                            ((1 == (1 & pstURPpara->stRuleSet.astRule[k].stPara.stTripwireRulePara.astLines[j].s32ForbiddenDirection)) ||
                            (3 == (3 & pstURPpara->stRuleSet.astRule[k].stPara.stTripwireRulePara.astLines[j].s32ForbiddenDirection))))&&
                            (pstInitPara->pstCur_Frame_Obj->Object[i].iCount == 0))
                        {
                            iBreakRule = 1;
                            IA_Printf("Object:%d banxian up2down\n" , pstInitPara->pstCur_Frame_Obj->Object[i].ObjectID);
                            pstInitPara->pstCur_Frame_Obj->Object[i].iMark = 1;
                            pstInitPara->pstCur_Frame_Obj->Object[i].iCount ++;
                        }
                    }
                }
            }

            else if (IMP_FUNC_PERIMETER == (pstURPpara->stRuleSet.astRule[k].u32Mode & IMP_FUNC_PERIMETER))
            {
                int iTmp = 0;

                //判定区分点。
                URP_IMP_POINT_S stRectCent;
                stRectCent.s16X = abs(pstInitPara->pstCur_Frame_Obj->Object[i].stRect.s16X1 - pstInitPara->pstCur_Frame_Obj->Object[i].stRect.s16X2);
                stRectCent.s16Y = abs(pstInitPara->pstCur_Frame_Obj->Object[i].stRect.s16Y1 - pstInitPara->pstCur_Frame_Obj->Object[i].stRect.s16Y2);

                URP_IMP_POINT_S stCent;
                stCent.s16X = pstInitPara->pstCur_Frame_Obj->Object[i].center.u16X;
                stCent.s16Y = pstInitPara->pstCur_Frame_Obj->Object[i].center.u16Y;

                //标识位置初始化
                if (pstInitPara->pstCur_Frame_Obj->Object[i].iMark == 0)
                {
                    pstInitPara->pstCur_Frame_Obj->Object[i].iMark = pInQuadrangle_Init(pstURPpara->stRuleSet.astRule[k].stPara.stPerimeterRulePara.stLimitPara.stBoundary.astBoundaryPts[0],
                        pstURPpara->stRuleSet.astRule[k].stPara.stPerimeterRulePara.stLimitPara.stBoundary.astBoundaryPts[1],
                        pstURPpara->stRuleSet.astRule[k].stPara.stPerimeterRulePara.stLimitPara.stBoundary.astBoundaryPts[2],
                        pstURPpara->stRuleSet.astRule[k].stPara.stPerimeterRulePara.stLimitPara.stBoundary.astBoundaryPts[3],
                        stCent);
                }

                //标识在外  3标识在过界
                if ((pstInitPara->pstCur_Frame_Obj->Object[i].iMark == 1) || (pstInitPara->pstCur_Frame_Obj->Object[i].iMark == 3))
                {
                    iTmp = pInQuadrangle_InOut(pstURPpara->stRuleSet.astRule[k].stPara.stPerimeterRulePara.stLimitPara.stBoundary.astBoundaryPts[0],
                        pstURPpara->stRuleSet.astRule[k].stPara.stPerimeterRulePara.stLimitPara.stBoundary.astBoundaryPts[1],
                        pstURPpara->stRuleSet.astRule[k].stPara.stPerimeterRulePara.stLimitPara.stBoundary.astBoundaryPts[2],
                        pstURPpara->stRuleSet.astRule[k].stPara.stPerimeterRulePara.stLimitPara.stBoundary.astBoundaryPts[3],
                        stCent, stRectCent, iRangelevel, 1);
                }

                //标识在内
                if ((pstInitPara->pstCur_Frame_Obj->Object[i].iMark == 2) || (pstInitPara->pstCur_Frame_Obj->Object[i].iMark == 4))
                {
                    iTmp = pInQuadrangle_InOut(pstURPpara->stRuleSet.astRule[k].stPara.stPerimeterRulePara.stLimitPara.stBoundary.astBoundaryPts[0],
                        pstURPpara->stRuleSet.astRule[k].stPara.stPerimeterRulePara.stLimitPara.stBoundary.astBoundaryPts[1],
                        pstURPpara->stRuleSet.astRule[k].stPara.stPerimeterRulePara.stLimitPara.stBoundary.astBoundaryPts[2],
                        pstURPpara->stRuleSet.astRule[k].stPara.stPerimeterRulePara.stLimitPara.stBoundary.astBoundaryPts[3],
                        stCent, stRectCent, iRangelevel, 2);
                }

#ifdef _XM_IA_PRINT_
                printf("Judge  ID %d  iTmp %d iTouchLine : %d Cent[%d %d] dx %d dy %d\n  s32MinDist:%d Distance：%f\n Oringin_centerX:%d Oringin_centerY:%d\n  Mark：%d\n",
                    pstInitPara->pstCur_Frame_Obj->Object[i].ObjectID, iTmp,
                    pstInitPara->pstCur_Frame_Obj->Object[i].iTouchLine,
                    pstInitPara->pstCur_Frame_Obj->Object[i].center.u16X,
                    pstInitPara->pstCur_Frame_Obj->Object[i].center.u16Y,
                    pstInitPara->pstCur_Frame_Obj->Object[i].Motion.dx,
                    pstInitPara->pstCur_Frame_Obj->Object[i].Motion.dy,
                    pstURPpara->stRuleSet.astRule[0].stPara.stPerimeterRulePara.stLimitPara.s32MinDist, fDistance,
                    stOrCent.s16X, stOrCent.s16Y,
                    pstInitPara->pstCur_Frame_Obj->Object[i].iMark);
#endif

                if (0 == pstInitPara->pstCur_Frame_Obj->Object[i].iTouchLine)
                {
                    pstInitPara->pstCur_Frame_Obj->Object[i].iTouchLine = iTmp;
                }

                //灵敏度点所在位置判定调整
                if ((pstInitPara->pstCur_Frame_Obj->Object[i].iMark == 3) && (iTmp < 0))
                {
                    pstInitPara->pstCur_Frame_Obj->Object[i].iMark = 1;
                }
                if ((pstInitPara->pstCur_Frame_Obj->Object[i].iMark == 4) && (iTmp > 0))
                {
                    pstInitPara->pstCur_Frame_Obj->Object[i].iMark = 2;
                }

                //点移动判据
                if (fDistance>pstURPpara->stRuleSet.astRule[0].stPara.stPerimeterRulePara.stLimitPara.s32MinDist)
                {
                    if (pstInitPara->pstCur_Frame_Obj->Object[i].iMark == 1)
                    {
                        //触发禁止进入
                        if ((pstInitPara->pstCur_Frame_Obj->Object[i].iTouchLine < 0) && (iTmp > 0))
                        {
                            pstInitPara->pstCur_Frame_Obj->Object[i].iMark = 4;

                            if((IMP_URP_PMODE_INTRUSION == pstURPpara->stRuleSet.astRule[k].stPara.stPerimeterRulePara.s32Mode) ||
                                (IMP_URP_PMODE_ENTER == pstURPpara->stRuleSet.astRule[k].stPara.stPerimeterRulePara.s32Mode))
                            {
                                iBreakRule = 1;
                                IA_Printf("Object:%d zhoujie jinzhijinru\n" , pstInitPara->pstCur_Frame_Obj->Object[i].ObjectID);

                            }
                        }
                    }

                   if (pstInitPara->pstCur_Frame_Obj->Object[i].iMark == 2)
                   {
                        //触发禁止离开
                        if ((pstInitPara->pstCur_Frame_Obj->Object[i].iTouchLine > 0) && (iTmp < 0))
                        {
                            pstInitPara->pstCur_Frame_Obj->Object[i].iMark = 3;
                            if((IMP_URP_PMODE_INTRUSION == pstURPpara->stRuleSet.astRule[k].stPara.stPerimeterRulePara.s32Mode) ||
                                (IMP_URP_PMODE_EXIT == pstURPpara->stRuleSet.astRule[k].stPara.stPerimeterRulePara.s32Mode))
                            {
                                iBreakRule = 1;
                                IA_Printf("Object:%d zhoujie jinzhilikai\n" , pstInitPara->pstCur_Frame_Obj->Object[i].ObjectID);
                            }
                        }
                    }
                        pstInitPara->pstCur_Frame_Obj->Object[i].iTouchLine = iTmp;
                 }
            }
            else if (IMP_FUNC_OSC == (pstURPpara->stRuleSet.astRule[k].u32Mode & IMP_FUNC_OSC))//遗留物规则
            {
                //遗留物检测
                //遗留物检测区域设定（只做一个区域的禁区设定,要是多个的话需要对后面的下标进行遍历）
                URP_POLYGON_REGION_S *pstRuleRegion = &pstInitPara->pstURPpara->stRuleSet.astRule[k].stPara.stOscRulePara.astSpclRgs[0].stOscRg;

                pstInitPara->pstRemnantDetectionInfo->stDetectionRct.u16X = (HI_U16)pstRuleRegion->astPoint[0].s16X;
                pstInitPara->pstRemnantDetectionInfo->stDetectionRct.u16Y = (HI_U16)pstRuleRegion->astPoint[0].s16Y;
                pstInitPara->pstRemnantDetectionInfo->stDetectionRct.u16Width = (HI_U16)(pstRuleRegion->astPoint[2].s16X - pstRuleRegion->astPoint[0].s16X);
                pstInitPara->pstRemnantDetectionInfo->stDetectionRct.u16Height = (HI_U16)(pstRuleRegion->astPoint[2].s16Y - pstRuleRegion->astPoint[0].s16Y);

                IVE_CCLRECT_S stCclRect = pstInitPara->pstGmmLk->stGMM.stRegion.astRect[pstInitPara->pstCur_Frame_Obj->Object[i].CCLRegionNum];

#ifdef _XM_IA_PRINT_
                printf("OSC Rule X %d Y %d Width %d Height %d\n",
                    pstRuleRegion->astPoint[0].s16X,
                    pstRuleRegion->astPoint[0].s16Y,
                    pstRuleRegion->astPoint[2].s16X,
                    pstRuleRegion->astPoint[2].s16Y);

                printf("OBJ Rect[%d %d %d %d] CCL Rect[%d %d %d %d]\n",
                    pstInitPara->pstCur_Frame_Obj->Object[i].stRect.s16X1, pstInitPara->pstCur_Frame_Obj->Object[i].stRect.s16Y1,
                    pstInitPara->pstCur_Frame_Obj->Object[i].stRect.s16X2, pstInitPara->pstCur_Frame_Obj->Object[i].stRect.s16Y2,
                    stCclRect.astPoint[0].s32X, stCclRect.astPoint[0].s32Y, stCclRect.astPoint[2].s32X, stCclRect.astPoint[2].s32Y);
#endif

                xmRemnantDetection(pstInitPara, stCclRect, &pstInitPara->pstGmmLk->stGMM.stFg,i);
                if ((pstInitPara->pstRemnantDetectionRes->u16DetectRes > 0) ||
                    (pstInitPara->pstCur_Frame_Obj->Object[i].iPolyStart == 1)||
                    (pstInitPara->pstCur_Frame_Obj->Object[i].iPolyMark >= 1))
                {
                    pstInitPara->pstCur_Frame_Obj->Object[i].u16RemnantFrameCount++;
                }

                int iImgSize = pstInitPara->iImgHeigth * pstInitPara->iImgWidth;

                int iObjSize = (pstInitPara->pstCur_Frame_Obj->Object[i].stRect.s16X2 -
                    pstInitPara->pstCur_Frame_Obj->Object[i].stRect.s16X1)*
                    (pstInitPara->pstCur_Frame_Obj->Object[i].stRect.s16Y2 -
                    pstInitPara->pstCur_Frame_Obj->Object[i].stRect.s16Y1);

                int iLastTime = pstURPpara->stRuleSet.astRule[k].stPara.stOscRulePara.stOscPara.s32TimeMin *
                    FRAME_RATE;

                //如果检测到有物体，持续时间大于设定
                if (((pstInitPara->pstCur_Frame_Obj->Object[i].u16RemnantFrameCount > iLastTime)&&
                    (iObjSize > pstURPpara->stRuleSet.astRule[k].stPara.stOscRulePara.stOscPara.s32SizeMin * iImgSize / 100) &&
                    (iObjSize < pstURPpara->stRuleSet.astRule[k].stPara.stOscRulePara.stOscPara.s32SizeMax * iImgSize / 100))&&
                    (pstInitPara->pstCur_Frame_Obj->Object[i].iPolyMark == 1)&&
                    (pstInitPara->pstCur_Frame_Obj->Object[i].iPolylock == 0))
                {
                    iBreakRule = 1;
                    IA_Printf("Object:%d wupin yiliu\n" , pstInitPara->pstCur_Frame_Obj->Object[i].ObjectID);
//                    pstInitPara->pstCur_Frame_Obj->Object[i].iPolylock == 1 ;
                }
            }
            else if (IMP_FUNC_OBJSTOLEN == (pstURPpara->stRuleSet.astRule[k].u32Mode & IMP_FUNC_OBJSTOLEN))//物品盗移规则
            {
                //物品盗移检测
                //物品盗移功能区域设定
                URP_POLYGON_REGION_S *pstRuleRegion = &pstInitPara->pstURPpara->stRuleSet.astRule[k].stPara.stOscRulePara.astSpclRgs[0].stOscRg;

                pstInitPara->pstRemnantDetectionInfo->stDetectionRct.u16X = (HI_U16)pstRuleRegion->astPoint[0].s16X;
                pstInitPara->pstRemnantDetectionInfo->stDetectionRct.u16Y = (HI_U16)pstRuleRegion->astPoint[0].s16Y;
                pstInitPara->pstRemnantDetectionInfo->stDetectionRct.u16Width = (HI_U16)(pstRuleRegion->astPoint[2].s16X - pstRuleRegion->astPoint[0].s16X);
                pstInitPara->pstRemnantDetectionInfo->stDetectionRct.u16Height = (HI_U16)(pstRuleRegion->astPoint[2].s16Y - pstRuleRegion->astPoint[0].s16Y);

                IVE_CCLRECT_S stCclRect = pstInitPara->pstGmmLk->stGMM.stRegion.astRect[pstInitPara->pstCur_Frame_Obj->Object[i].CCLRegionNum];

#ifdef _XM_IA_PRINT_
                printf("STOLEN Rule X %d Y %d Width %d Height %d\n",
                    pstInitPara->pstRemnantDetectionInfo->stDetectionRct.u16X,
                    pstInitPara->pstRemnantDetectionInfo->stDetectionRct.u16Y,
                    pstInitPara->pstRemnantDetectionInfo->stDetectionRct.u16Width,
                    pstInitPara->pstRemnantDetectionInfo->stDetectionRct.u16Height);

                printf("OBJ Rect[%d %d %d %d] CCL Rect[%d %d %d %d]\n",
                    pstInitPara->pstCur_Frame_Obj->Object[i].stRect.s16X1, pstInitPara->pstCur_Frame_Obj->Object[i].stRect.s16Y1,
                    pstInitPara->pstCur_Frame_Obj->Object[i].stRect.s16X2, pstInitPara->pstCur_Frame_Obj->Object[i].stRect.s16Y2,
                    stCclRect.astPoint[0].s32X, stCclRect.astPoint[0].s32Y, stCclRect.astPoint[2].s32X, stCclRect.astPoint[2].s32Y);
#endif

                IVE_CCBLOB_S *pstCCBlob;

                pstCCBlob = (IVE_CCBLOB_S *)pstInitPara->pstGmmLk->stGMM.stBlob.pu8VirAddr;
                //区域内是否有前景区域判断

                iGonRes = xmBlob(pstInitPara,pstCCBlob);

                if(pstInitPara->pstCur_Frame_Obj->Object[i].u16RemnantFrameCount > 0)
                {
                    pstInitPara->iObjLock = pstInitPara->iObjLock + 2;
                }

                if (((pstInitPara->iObjLock == 0)||(pstInitPara->pstCur_Frame_Obj->Object[i].u16RemnantFrameCount > 0))&&
                    ((iGonRes>0) ||
                    (pstInitPara->pstCur_Frame_Obj->Object[i].iPolyStart == 1) ||
                    (pstInitPara->pstCur_Frame_Obj->Object[i].iPolyMark >= 1)))
                {
                    pstInitPara->pstCur_Frame_Obj->Object[i].u16RemnantFrameCount++;
                }

                int iImgSize = pstInitPara->iImgHeigth * pstInitPara->iImgWidth;
                int iObjSize = pstInitPara->pstRemnantDetectionInfo->stDetectionRct.u16Width *
                    pstInitPara->pstRemnantDetectionInfo->stDetectionRct.u16Height;
                int iLastTime = pstURPpara->stRuleSet.astRule[k].stPara.stOscRulePara.stOscPara.s32TimeMin *
                    FRAME_RATE;

                //pstInitPara->pstCur_Frame_Obj->Object[i].iPolyClose = iLastTime + 50;

                //如果检测到物品遗失,持续时间大于判定
                if (((pstInitPara->pstCur_Frame_Obj->Object[i].u16RemnantFrameCount > iLastTime)&&
                    (iObjSize > pstURPpara->stRuleSet.astRule[k].stPara.stOscRulePara.stOscPara.s32SizeMin * iImgSize / 100) &&
                    (iObjSize < pstURPpara->stRuleSet.astRule[k].stPara.stOscRulePara.stOscPara.s32SizeMax * iImgSize / 100))&&
                    (pstInitPara->pstCur_Frame_Obj->Object[i].iPolyMark == 1)&&
                    (pstInitPara->pstCur_Frame_Obj->Object[i].iPolylock == 0))
                    {
                        iBreakRule = 1;
                        IA_Printf("Object:%d wupin daoyi\n" , pstInitPara->pstCur_Frame_Obj->Object[i].ObjectID);
                        pstInitPara->pstCur_Frame_Obj->Object[i].iPolylock = 1;
                    }


            }

            if (1 == iBreakRule)
            {
                pstInitPara->pstResult->stEventSet.astEvents[iEventNum].u32Level = 1;               //这个level是什么鬼！？
                if (IMP_FUNC_TRIPWIRE == (pstURPpara->stRuleSet.astRule[k].u32Mode & IMP_FUNC_TRIPWIRE))
                {
                    pstInitPara->pstResult->stEventSet.astEvents[iEventNum].u32Type = IMP_EVT_TYPE_AlarmTripwire;   /**< 单警戒线检测 */
                    pstInitPara->pstResult->stEventSet.astEvents[iEventNum].u32Status = IMP_EVT_STATUS_START;
                }
                else if (IMP_FUNC_PERIMETER == (pstURPpara->stRuleSet.astRule[k].u32Mode & IMP_FUNC_PERIMETER))
                {
                    pstInitPara->pstResult->stEventSet.astEvents[iEventNum].u32Type = IMP_EVT_TYPE_AlarmPerimeter;   /**< 周界保护 */
                    pstInitPara->pstResult->stEventSet.astEvents[iEventNum].u32Status = IMP_EVT_STATUS_START;
                }
                else if (IMP_FUNC_OSC == (pstURPpara->stRuleSet.astRule[k].u32Mode & IMP_FUNC_OSC))
                {
                    pstInitPara->pstResult->stEventSet.astEvents[iEventNum].u32Type = IMP_EVT_TYPE_AlarmOsc;   /**< 物品遗留 */
                    pstInitPara->pstResult->stEventSet.astEvents[iEventNum].u32Status = IMP_EVT_STATUS_START;
                }
                else if (IMP_FUNC_OBJSTOLEN == (pstURPpara->stRuleSet.astRule[k].u32Mode & IMP_FUNC_OBJSTOLEN))
                {
                    pstInitPara->pstResult->stEventSet.astEvents[iEventNum].u32Type = IMP_EVT_TYPE_AlarmObjStolen;   /**<  物品盗移 */
                    pstInitPara->pstResult->stEventSet.astEvents[iEventNum].u32Status = IMP_EVT_STATUS_START;
                }
                iEventNum++;
            }
        }
    }

    if(1 == pstInitPara->stAvdPara->stChangePara.u32Enable)//场景变化检测
    {
        SceneChange_Judge(pstInitPara);

        if (1 == pstInitPara->iSceneMark)
        {
            iBreakRule = 1;
            IA_Printf("changjingbianhuan\n");
        }
    }

    if (1 == iBreakRule)
    {

        if (1 == pstInitPara->stAvdPara->stChangePara.u32Enable)
        {
            pstInitPara->pstResult->stEventSet.astEvents[iEventNum].u32Type = IMP_EVT_TYPE_AlarmSceneChg;   /**<  场景变换 */
            pstInitPara->pstResult->stEventSet.astEvents[iEventNum].u32Status = IMP_EVT_STATUS_START;
        }
        iEventNum++;
    }

    pstInitPara->pstResult->stEventSet.s32EventNum = iEventNum;

    return;
}

int XM_IA_Work(void *pvHandle, IVE_IMAGE_S *pstImage, INTEL_CODING *pstMapCfg, QP_RESULT* stQPResult)
{
    if (NULL == pstImage)
    {
        IA_Printf("pstImage is NULL!\n");
        return XM_NULL_POINTER;
    }
    if (NULL == pvHandle)
    {
        IA_Printf("pvHandle is NULL!\n");
        return XM_NULL_POINTER;
    }

    HI_U64 pu64GmmStartPts = 0, pu64GmmEndPts = 0;
    HI_U64 pu64ThrStartPts = 0, pu64ThrEndPts = 0;
    HI_U64 pu64PreStartPts = 0, pu64PreEndPts = 0;
    HI_U64 pu64RectStartPts = 0, pu64RectEndPts = 0;
    HI_U64 pu64EndecStartPts = 0, pu64EndecEndPts = 0;


    XM_INIT_PARA *pstInitPara = (XM_INIT_PARA *)pvHandle;

    IVE_HANDLE IveHandle;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_BOOL bInstant = HI_FALSE;
	
#ifdef _XM_IA_lINUX_
    HI_BOOL bBlock = HI_TRUE;
    HI_BOOL bFinish = HI_FALSE;
#endif

    IVE_CCBLOB_S *pstCCBlob;

    pstCCBlob = (IVE_CCBLOB_S *)pstInitPara->pstGmmLk->stGMM.stBlob.pu8VirAddr;
	
    
	
#ifdef _XM_IA_lINUX_
    HI_MPI_SYS_GetCurPts(&pu64PreStartPts);
#endif

    //  IVE_IMAGE_S 复制
    IveImageCopy(pstImage, &pstInitPara->pstGmmLk->stGMM.stSrc);
	
    //图像缩放
    //XM_Image_Resize(&pstInitPara->pstGmmLk->stGMM.stResizeSrc, &pstInitPara->pstGmmLk->stGMM.stSrc);
	
#ifdef _XM_IA_lINUX_
    HI_MPI_SYS_GetCurPts(&pu64PreEndPts);
	
  //  printf("Copy_Resize_Time = %ld\n",(long int)(pu64PreEndPts - pu64PreStartPts));

    HI_MPI_SYS_GetCurPts(&pu64ThrStartPts);
#endif

    //与上一帧图像相减并阈值二值化
    s32Ret = Frame_Sub_Thr(pstInitPara);
	
    if (s32Ret != HI_SUCCESS)
    {
        IA_Printf("Frame_Sub_Thr fail\n");
    }

#ifdef _XM_IA_lINUX_
    HI_MPI_SYS_GetCurPts(&pu64ThrEndPts);
	
//    printf("FrmSub_Time = %ld\n", (long int)(pu64ThrEndPts - pu64ThrStartPts));

    HI_MPI_SYS_GetCurPts(&pu64GmmStartPts);
#endif

#ifdef _XM_IA_lINUX_
//    HI_MPI_SYS_MmzFlushCache(pstInitPara->pstGmmLk->stGMM.stSrc.u32PhyAddr[0], pstInitPara->pstGmmLk->stGMM.stSrc.pu8VirAddr[0],
//        pstInitPara->pstGmmLk->stGMM.stSrc.u16Height*pstInitPara->pstGmmLk->stGMM.stSrc.u16Stride[0]);
#endif

    /*************************************Sobel任务***********************************************/

    s32Ret = HI_MPI_IVE_Sobel(&IveHandle,&(pstInitPara->pstGmmLk->stGMM.stSrc),
    &(pstInitPara->pstGmmLk->stGMM.stSobel),NULL,&(pstInitPara->pstGmmLk->stGMM.stSobelCtrl), bInstant);
    if (s32Ret != HI_SUCCESS)
    {
        IA_Printf("Sobel fail,Error(%#x)\n", s32Ret);
    }

    /*************************************Gmm任务***********************************************/
    s32Ret = HI_MPI_IVE_GMM(&IveHandle, &(pstInitPara->pstGmmLk->stGMM.stSrc),
        &(pstInitPara->pstGmmLk->stGMM.stFg), &(pstInitPara->pstGmmLk->stGMM.stBg),
        &(pstInitPara->pstGmmLk->stGMM.stModel), &(pstInitPara->pstGmmLk->stGMM.stGmmCtrl), bInstant);
    if (s32Ret != HI_SUCCESS)
    {
        IA_Printf("GMM fail,Error(%#x)\n", s32Ret);
    }
	
    /*************************************膨胀任务*********************************************/
    s32Ret = HI_MPI_IVE_Dilate(&IveHandle, &(pstInitPara->pstGmmLk->stGMM.stFg),
            &(pstInitPara->pstGmmLk->stGMM.stDILATEImg), &(pstInitPara->pstGmmLk->stGMM.stDilateCtrl), bInstant);
    if (s32Ret != HI_SUCCESS)
    {
        IA_Printf("Dilate fail,Error(%#x)\n", s32Ret);
    }

    /**************************************腐蚀任务**********************************************/
    s32Ret = HI_MPI_IVE_Erode(&IveHandle, &(pstInitPara->pstGmmLk->stGMM.stDILATEImg),
            &(pstInitPara->pstGmmLk->stGMM.stERODEImg), &(pstInitPara->pstGmmLk->stGMM.stErodeCtrl), bInstant);
    if (s32Ret != HI_SUCCESS)
    {
        IA_Printf("Erode fail,Error(%#x)\n", s32Ret);
    }  


    /************************************CCL任务*********************************************/
    bInstant = HI_TRUE;
    s32Ret = HI_MPI_IVE_CCL(&IveHandle, &(pstInitPara->pstGmmLk->stGMM.stERODEImg),
                &(pstInitPara->pstGmmLk->stGMM.stBlob), &(pstInitPara->pstGmmLk->stGMM.stCclCtrl), bInstant);
    if (s32Ret != HI_SUCCESS)
    {
        IA_Printf("CCL fail,Error(%x)\n", s32Ret);
    }

#ifdef _XM_IA_lINUX_
    s32Ret = HI_MPI_IVE_Query(IveHandle, &bFinish, bBlock);
	
    while (HI_ERR_IVE_QUERY_TIMEOUT == s32Ret)
    {

        usleep(200);

        s32Ret = HI_MPI_IVE_Query(IveHandle, &bFinish, bBlock);
    }

    if (s32Ret != HI_SUCCESS)
    {
        IA_Printf("HI_MPI_IVE_Query fail,Error(%#x)\n", s32Ret);

        return HISI_IVE_FAIL;
    }
#endif

#ifdef _XM_IA_lINUX_
    HI_MPI_SYS_GetCurPts(&pu64GmmEndPts);
	
 //   printf("GMM_Time = %ld\n", (long int)(pu64GmmEndPts - pu64GmmStartPts));

    HI_MPI_SYS_GetCurPts(&pu64RectStartPts);
#endif

    COMM_IVE_BlobToRect(pstCCBlob, &(pstInitPara->pstGmmLk->stGMM.stRegion), MAX_OBJ_NUM, CCL_AREA_THR_STEP,
        pstInitPara->pstGmmLk->stGMM.stSrc.u16Stride[0], pstInitPara->pstGmmLk->stGMM.stSrc.u16Height);

#ifdef _XM_IA_lINUX_
    HI_MPI_SYS_GetCurPts(&pu64RectEndPts);
	
 //   printf("CCL_RECT_Time = %ld\n", (long int)(pu64RectEndPts - pu64RectStartPts));
	
    HI_MPI_SYS_GetCurPts(&pu64EndecStartPts);
#endif
    //get SrcImage MB_Flag
    XM_Video_ENDEC(pstInitPara, pstMapCfg, stQPResult);
	
#ifdef _XM_IA_lINUX_
    HI_MPI_SYS_GetCurPts(&pu64EndecEndPts);
	
//    printf("ENDEC_Process_Time = %ld\n", (long int)(pu64EndecEndPts - pu64EndecStartPts));
#endif
    
//    printf("u16TagWid = %d; u16TagHgt = %d\n", pstMapCfg->width, pstMapCfg->hight);
    
#if 0//Hi3516C V300不支持HI_MPI_IVE_LKOpticalFlow接口

#ifdef _XM_IA_lINUX_
    HI_MPI_SYS_GetCurPts(&pu64PreStartPts);
#endif


    s32Ret = LK_Pre_Proc(pstInitPara->pstGmmLk, pstInitPara->TotalCorner);
    if (s32Ret != HI_SUCCESS)
    {
        IA_Printf("LK_Pre_Proc fail\n");
    }

    //Target目标数据清零
    pstInitPara->pstResult->stTargetSet.s32TargetNum = 0;
    for (k = 0; k < MAX_OBJ_NUM; k++)
    {
        pstInitPara->pstResult->stTargetSet.astTargets[k].u32Id = 0;
        pstInitPara->pstResult->stTargetSet.astTargets[k].stRect.s16X1 = 0;
        pstInitPara->pstResult->stTargetSet.astTargets[k].stRect.s16Y1 = 0;
        pstInitPara->pstResult->stTargetSet.astTargets[k].stRect.s16X2 = 0;
        pstInitPara->pstResult->stTargetSet.astTargets[k].stRect.s16Y2 = 0;
    }

#ifdef _XM_IA_lINUX_
    HI_MPI_SYS_GetCurPts(&pu64PreEndPts);
#endif

    //保存初始图像
    if ((pstInitPara->framenum == 20)&&(IMP_FUNC_OBJSTOLEN == (pstInitPara->pstURPpara->stRuleSet.astRule[0].u32Mode &  IMP_FUNC_OBJSTOLEN)))
    {
        s32Ret =IveImageCopy(&pstInitPara->pstGmmLk->stGMM.stSrc, &pstInitPara->pstGmmLk->stLK.stPilfer);
        printf("IveImageCopyOK\n");
    }
    if (s32Ret != HI_SUCCESS)
    {
        IA_Printf("XM_Pilfer fail,Error(%#x)\n", s32Ret);
    }

    if (pstInitPara->framenum > 20)
    {
        //角点更新部分
        pstInitPara->QuasiObjNum = 0;
        for (j = 0; j < pstInitPara->pstGmmLk->stGMM.stRegion.u16Num; j++)
        {
            //联通区域面积大于OBJ_MIN_AREA认为是目标
            if (pstInitPara->pstGmmLk->stGMM.stRegion.astRect[j].u32Area > pstInitPara->iObjMinArea &&
                pstInitPara->pstGmmLk->stGMM.stRegion.astRect[j].u32Area < pstInitPara->iObjMaxArea)
            {
                //准目标数量
                pstInitPara->QuasiObjNum++;
            }
        }
        if (pstInitPara->QuasiObjNum > pstInitPara->pstCur_Frame_Obj->ObjectNum)
        {
            iNewCorner = 1;
        }
        else
        {
            iNewCorner = 0;
        }
        //如果有角点，先做LK跟踪（预处理在有角点的后面做）
        if ((pstInitPara->TotalCorner > 0)||(pstInitPara->PolyGon == 1))//LK跟踪部分
        {
            //LK跟踪任务
#ifdef _XM_IA_lINUX_
            HI_MPI_SYS_GetCurPts(&pu64LKStartPts);
#endif

            s32Ret = LK_Follow_Proc(pstInitPara->pstGmmLk, pstInitPara->TotalCorner);
            if (s32Ret != HI_SUCCESS)
            {
                IA_Printf("LK_Follow_Proc fail\n");
            }

#ifdef _XM_IA_lINUX_
            HI_MPI_SYS_GetCurPts(&pu64LKEndPts);

            HI_MPI_SYS_GetCurPts(&pu64ObjStartPts);
            //角点分配
            HI_MPI_SYS_GetCurPts(&pu64ObjFPStartPts);
#endif

            Corner_CCL_Distribute(pstInitPara->pstGmmLk, pstInitPara->pstCur_Frame_Obj, pstInitPara,&pstInitPara->TotalCorner,
                        &pstInitPara->TotalObjNum, pstInitPara->LKPoint, pstInitPara->TempPoint, pstInitPara->TempMv);

#ifdef _XM_IA_lINUX_
            HI_MPI_SYS_GetCurPts(&pu64ObjFPEndPts);
            HI_MPI_SYS_GetCurPts(&pu64ObjOrderStartPts);
#endif
            //对当前目标数组进行整理
            iNewCorner = Cur_Frame_Obj_Order(pstInitPara);

#ifdef _XM_IA_lINUX_
            HI_MPI_SYS_GetCurPts(&pu64ObjOrderEndPts);

            HI_MPI_SYS_GetCurPts(&pu64ObjOriStartPts);
#endif

            //当前目标方向整理
            Orientation1(pstInitPara);

            Orientation2(pstInitPara->pstCur_Frame_Obj);

#ifdef _XM_IA_lINUX_
            HI_MPI_SYS_GetCurPts(&pu64ObjOriEndPts);

            HI_MPI_SYS_GetCurPts(&pu64ObjEndPts);
#endif

            if (1 == iNewCorner)
            {
                IA_Printf("--------NewCorner!--------\n");

#ifdef _XM_IA_lINUX_
                HI_MPI_SYS_GetCurPts(&pu64CornerStartPts);
#endif

                New_Corner(pstInitPara);

#ifdef _XM_IA_lINUX_
                HI_MPI_SYS_GetCurPts(&pu64CornerEndPts);
#endif
            }
        }
        else if (0 == pstInitPara->TotalCorner && pstInitPara->QuasiObjNum > 0)
        {
            IA_Printf("--------NewCorner!--------\n");
#ifdef _XM_IA_lINUX_
            HI_MPI_SYS_GetCurPts(&pu64CornerStartPts);
#endif
            New_Corner(pstInitPara);
#ifdef _XM_IA_lINUX_
            HI_MPI_SYS_GetCurPts(&pu64CornerEndPts);
#endif
        }

        //场景变换前期判断
        pstInitPara->PolyGon = 0;
        //遗留物前期判断
        Polygon_Judge(pstInitPara);
        //物品盗取前期判断
        Pilfer_Judge(pstInitPara);

        //目标输出
        pstInitPara->pstResult->stTargetSet.s32TargetNum = pstInitPara->pstCur_Frame_Obj->ObjectNum;
        for (k = 0; k < pstInitPara->pstCur_Frame_Obj->ObjectNum; k++)
        {
            pstInitPara->pstResult->stTargetSet.astTargets[k].u32Id=pstInitPara->pstCur_Frame_Obj->Object[k].ObjectID;
            pstInitPara->pstResult->stTargetSet.astTargets[k].stRect.s16X1=pstInitPara->pstCur_Frame_Obj->Object[k].stRect.s16X1 * 8192 / pstInitPara->iImgWidth;
            pstInitPara->pstResult->stTargetSet.astTargets[k].stRect.s16Y1=pstInitPara->pstCur_Frame_Obj->Object[k].stRect.s16Y1 * 8192 / pstInitPara->iImgHeigth;
            pstInitPara->pstResult->stTargetSet.astTargets[k].stRect.s16X2=pstInitPara->pstCur_Frame_Obj->Object[k].stRect.s16X2 * 8192 / pstInitPara->iImgWidth;
            pstInitPara->pstResult->stTargetSet.astTargets[k].stRect.s16Y2=pstInitPara->pstCur_Frame_Obj->Object[k].stRect.s16Y2 * 8192 / pstInitPara->iImgHeigth;
            if (IMP_FUNC_TRIPWIRE == (pstInitPara->pstURPpara->stRuleSet.astRule[k].u32Mode & IMP_FUNC_TRIPWIRE))
            {
                pstInitPara->pstResult->stTargetSet.astTargets[k].u32Event = IMP_TGT_EVENT_TRIPWIRE;
                pstInitPara->pstResult->stTargetSet.astTargets[k].u32Type = IMP_EVT_TYPE_AlarmTripwire;
            }
            else if (IMP_FUNC_PERIMETER == (pstInitPara->pstURPpara->stRuleSet.astRule[k].u32Mode & IMP_FUNC_PERIMETER))
            {
                pstInitPara->pstResult->stTargetSet.astTargets[k].u32Event = IMP_TGT_EVENT_PERIMETER;
                pstInitPara->pstResult->stTargetSet.astTargets[k].u32Type = IMP_EVT_TYPE_AlarmPerimeter;
            }
            else if (IMP_FUNC_OSC == (pstInitPara->pstURPpara->stRuleSet.astRule[k].u32Mode & IMP_FUNC_OSC))
            {
                pstInitPara->pstResult->stTargetSet.astTargets[k].u32Event = IMP_TGT_EVENT_ABANDUM;
                pstInitPara->pstResult->stTargetSet.astTargets[k].u32Type = IMP_EVT_TYPE_AlarmAbabdum;
            }
            else if (IMP_FUNC_OBJSTOLEN == (pstInitPara->pstURPpara->stRuleSet.astRule[k].u32Mode & IMP_FUNC_OBJSTOLEN))
            {
                pstInitPara->pstResult->stTargetSet.astTargets[k].u32Event = IMP_TGT_EVENT_OBJSTOLEN;
                pstInitPara->pstResult->stTargetSet.astTargets[k].u32Type = IMP_EVT_TYPE_AlarmObjStolen;
            }
            else if (1 == pstInitPara->stAvdPara->stChangePara.u32Enable)
            {
                pstInitPara->pstResult->stTargetSet.astTargets[k].u32Event = IMP_TGT_EVENT_AlarmSceneChg;
                pstInitPara->pstResult->stTargetSet.astTargets[k].u32Type = IMP_EVT_TYPE_AlarmSceneChg;
            }
        }
    }

#ifdef _XM_IA_lINUX_
    HI_MPI_SYS_GetCurPts(&pu64PyStartPts);
#endif

    //当前帧变成前一帧
    s32Ret =  SAMPLE_IVE_CopyPyr222(pstInitPara->pstGmmLk->stLK.astCurPyr, pstInitPara->pstGmmLk->stLK.astPrePyr, 3);
    if (s32Ret != HI_SUCCESS)
    {
        IA_Printf("Func %s IVE_CopyPyr fail,Error(%#x)\n", __FUNCTION__, s32Ret);
        return HISI_IVE_FAIL;
    }
#ifdef _XM_IA_lINUX_
    HI_MPI_SYS_GetCurPts(&pu64PyEndPts);
#endif

    //重置
    pstInitPara->iObjLock = 0;

    //规则判断
#ifdef _XM_IA_lINUX_
    HI_MPI_SYS_GetCurPts(&pu64JudgeStartPts);
#endif

    XM_IA_Rule_Judge(pvHandle, pstInitPara->pstURPpara);

#ifdef _XM_IA_lINUX_
    HI_MPI_SYS_GetCurPts(&pu64JudgeEndPts);
#endif

#endif//Hi3516C V300不支持HI_MPI_IVE_LKOpticalFlow接口

    return XM_SUCCESS;
}

int XM_IA_Config(void *pvHandle, URP_PARA_S *pstURPpara)
{
    if (NULL == pstURPpara || NULL == pvHandle)
    {
        IA_Printf("XM_IA_Config input Failed %d %d\n", (int)pvHandle, (int)pstURPpara);
        return XM_NULL_POINTER;
    }

    int i;
    XM_INIT_PARA *pstInitPara = (XM_INIT_PARA *)pvHandle;
    memcpy(pstInitPara->pstURPpara, pstURPpara, sizeof(URP_PARA_S));

    //for (k = 0; k < IMP_URP_MAX_NUM_RULE_NUM; k++)
    for (i = 0; i < 1; i++)
    {
        if (IMP_PEA_AGLO_MODULE == pstInitPara->eAlgType)
        {
            if (IMP_FUNC_TRIPWIRE == (pstURPpara->stRuleSet.astRule[i].u32Mode & IMP_FUNC_TRIPWIRE))
            {
                pstInitPara->pstURPpara->stRuleSet.astRule[i].stPara.stTripwireRulePara.astLines[0].stLine.stStartPt.s16X =
                    pstURPpara->stRuleSet.astRule[i].stPara.stTripwireRulePara.astLines[0].stLine.stStartPt.s16X * pstInitPara->iImgWidth / 8192;
                pstInitPara->pstURPpara->stRuleSet.astRule[i].stPara.stTripwireRulePara.astLines[0].stLine.stStartPt.s16Y =
                    pstURPpara->stRuleSet.astRule[i].stPara.stTripwireRulePara.astLines[0].stLine.stStartPt.s16Y * pstInitPara->iImgHeigth/ 8192;
                pstInitPara->pstURPpara->stRuleSet.astRule[i].stPara.stTripwireRulePara.astLines[0].stLine.stEndPt.s16X =
                    pstURPpara->stRuleSet.astRule[i].stPara.stTripwireRulePara.astLines[0].stLine.stEndPt.s16X * pstInitPara->iImgWidth / 8192;
                pstInitPara->pstURPpara->stRuleSet.astRule[i].stPara.stTripwireRulePara.astLines[0].stLine.stEndPt.s16Y =
                    pstURPpara->stRuleSet.astRule[i].stPara.stTripwireRulePara.astLines[0].stLine.stEndPt.s16Y * pstInitPara->iImgHeigth / 8192;

                //移动范围后报警
                pstInitPara->pstURPpara->stRuleSet.astRule[0].stPara.stTripwireRulePara.stLimitPara.s32MinDist =
                    ((pstURPpara->stRuleSet.astRule[0].stPara.stTripwireRulePara.stLimitPara.s32MinDist*pstInitPara->iImgWidth) / 100);

                //根据绊线坐标的起始点和结束点的斜率向右旋转90度为禁止方向
                //s32ForbiddenDirection:1表示从绊线上方穿过下方，2表示下方穿过上方,3双向禁止
                if (0 == pstInitPara->pstURPpara->stRuleSet.astRule[i].stPara.stTripwireRulePara.astLines[0].s32IsDoubleDirection)
                {
                }
                else
                {
                    pstInitPara->pstURPpara->stRuleSet.astRule[i].stPara.stTripwireRulePara.astLines[0].s32ForbiddenDirection = 3;
                }
            }
            else if (IMP_FUNC_PERIMETER == (pstURPpara->stRuleSet.astRule[i].u32Mode & IMP_FUNC_PERIMETER))
            {
                //移动范围后报警
                pstInitPara->pstURPpara->stRuleSet.astRule[0].stPara.stPerimeterRulePara.stLimitPara.s32MinDist =
                    ((pstURPpara->stRuleSet.astRule[0].stPara.stPerimeterRulePara.stLimitPara.s32MinDist*(pstInitPara->iImgWidth)) / 100);

                if (pstURPpara->stRuleSet.astRule[i].stPara.stPerimeterRulePara.stLimitPara.stBoundary.s32BoundaryPtNum > 4)
                {
                    printf("Rule Point Num > 4! \n");
                    return XM_PARA_FAULT;
                }
                pstInitPara->pstURPpara->stRuleSet.astRule[i].stPara.stPerimeterRulePara.stLimitPara.stBoundary.astBoundaryPts[0].s16X =
                    pstURPpara->stRuleSet.astRule[i].stPara.stPerimeterRulePara.stLimitPara.stBoundary.astBoundaryPts[0].s16X * pstInitPara->iImgWidth / 8192;
                pstInitPara->pstURPpara->stRuleSet.astRule[i].stPara.stPerimeterRulePara.stLimitPara.stBoundary.astBoundaryPts[0].s16Y =
                    pstURPpara->stRuleSet.astRule[i].stPara.stPerimeterRulePara.stLimitPara.stBoundary.astBoundaryPts[0].s16Y * pstInitPara->iImgHeigth / 8192;

                pstInitPara->pstURPpara->stRuleSet.astRule[i].stPara.stPerimeterRulePara.stLimitPara.stBoundary.astBoundaryPts[1].s16X =
                    pstURPpara->stRuleSet.astRule[i].stPara.stPerimeterRulePara.stLimitPara.stBoundary.astBoundaryPts[1].s16X * pstInitPara->iImgWidth / 8192;
                pstInitPara->pstURPpara->stRuleSet.astRule[i].stPara.stPerimeterRulePara.stLimitPara.stBoundary.astBoundaryPts[1].s16Y =
                    pstURPpara->stRuleSet.astRule[i].stPara.stPerimeterRulePara.stLimitPara.stBoundary.astBoundaryPts[1].s16Y * pstInitPara->iImgHeigth / 8192;

                pstInitPara->pstURPpara->stRuleSet.astRule[i].stPara.stPerimeterRulePara.stLimitPara.stBoundary.astBoundaryPts[2].s16X =
                    pstURPpara->stRuleSet.astRule[i].stPara.stPerimeterRulePara.stLimitPara.stBoundary.astBoundaryPts[2].s16X * pstInitPara->iImgWidth / 8192;
                pstInitPara->pstURPpara->stRuleSet.astRule[i].stPara.stPerimeterRulePara.stLimitPara.stBoundary.astBoundaryPts[2].s16Y =
                    pstURPpara->stRuleSet.astRule[i].stPara.stPerimeterRulePara.stLimitPara.stBoundary.astBoundaryPts[2].s16Y * pstInitPara->iImgHeigth / 8192;

                pstInitPara->pstURPpara->stRuleSet.astRule[i].stPara.stPerimeterRulePara.stLimitPara.stBoundary.astBoundaryPts[3].s16X =
                    pstURPpara->stRuleSet.astRule[i].stPara.stPerimeterRulePara.stLimitPara.stBoundary.astBoundaryPts[3].s16X * pstInitPara->iImgWidth / 8192;
                pstInitPara->pstURPpara->stRuleSet.astRule[i].stPara.stPerimeterRulePara.stLimitPara.stBoundary.astBoundaryPts[3].s16Y =
                    pstURPpara->stRuleSet.astRule[i].stPara.stPerimeterRulePara.stLimitPara.stBoundary.astBoundaryPts[3].s16Y * pstInitPara->iImgHeigth / 8192;
            }
        }
        else if (IMP_OSC_AGLO_MODULE == pstInitPara->eAlgType)
        {
            if(IMP_FUNC_OSC == (pstURPpara->stRuleSet.astRule[i].u32Mode & IMP_FUNC_OSC))
            {
                //遗留物检测，学习率减小，延长前景存在时间
                if (pstInitPara->pstURPpara->stRuleSet.astRule[i].stPara.stOscRulePara.astSpclRgs[0].stOscRg.s32PointNum > 4)
                {
                    printf("Rule Point Num > 4! \n");
                    return XM_PARA_FAULT;
                }

            pstInitPara->iObjMinArea = (int)(((float)pstURPpara->stRuleSet.astRule[i].stPara.stOscRulePara.stOscPara.s32SizeMin) /
            100 * pstInitPara->iImgHeigth * pstInitPara->iImgWidth);
            pstInitPara->iObjMaxArea = (int)(((float)pstURPpara->stRuleSet.astRule[i].stPara.stOscRulePara.stOscPara.s32SizeMax) /
            100 * pstInitPara->iImgHeigth * pstInitPara->iImgWidth);

            pstInitPara->pstURPpara->stRuleSet.astRule[i].stPara.stOscRulePara.astSpclRgs[0].stOscRg.astPoint[0].s16X =
                pstURPpara->stRuleSet.astRule[i].stPara.stOscRulePara.astSpclRgs[0].stOscRg.astPoint[0].s16X * pstInitPara->iImgWidth / 8192;
            pstInitPara->pstURPpara->stRuleSet.astRule[i].stPara.stOscRulePara.astSpclRgs[0].stOscRg.astPoint[0].s16Y =
                pstURPpara->stRuleSet.astRule[i].stPara.stOscRulePara.astSpclRgs[0].stOscRg.astPoint[0].s16Y * pstInitPara->iImgHeigth / 8192;

            pstInitPara->pstURPpara->stRuleSet.astRule[i].stPara.stOscRulePara.astSpclRgs[0].stOscRg.astPoint[1].s16X =
                pstURPpara->stRuleSet.astRule[i].stPara.stOscRulePara.astSpclRgs[0].stOscRg.astPoint[1].s16X * pstInitPara->iImgWidth / 8192;
            pstInitPara->pstURPpara->stRuleSet.astRule[i].stPara.stOscRulePara.astSpclRgs[0].stOscRg.astPoint[1].s16Y =
                pstURPpara->stRuleSet.astRule[i].stPara.stOscRulePara.astSpclRgs[0].stOscRg.astPoint[1].s16Y * pstInitPara->iImgHeigth / 8192;

                pstInitPara->pstURPpara->stRuleSet.astRule[i].stPara.stOscRulePara.astSpclRgs[0].stOscRg.astPoint[2].s16X =
                    pstURPpara->stRuleSet.astRule[i].stPara.stOscRulePara.astSpclRgs[0].stOscRg.astPoint[2].s16X * pstInitPara->iImgWidth / 8192;
                pstInitPara->pstURPpara->stRuleSet.astRule[i].stPara.stOscRulePara.astSpclRgs[0].stOscRg.astPoint[2].s16Y =
                    pstURPpara->stRuleSet.astRule[i].stPara.stOscRulePara.astSpclRgs[0].stOscRg.astPoint[2].s16Y * pstInitPara->iImgHeigth / 8192;
                pstInitPara->pstURPpara->stRuleSet.astRule[i].stPara.stOscRulePara.astSpclRgs[0].stOscRg.astPoint[3].s16X =
                    pstURPpara->stRuleSet.astRule[i].stPara.stOscRulePara.astSpclRgs[0].stOscRg.astPoint[3].s16X * pstInitPara->iImgWidth / 8192;
                pstInitPara->pstURPpara->stRuleSet.astRule[i].stPara.stOscRulePara.astSpclRgs[0].stOscRg.astPoint[3].s16Y =
                    pstURPpara->stRuleSet.astRule[i].stPara.stOscRulePara.astSpclRgs[0].stOscRg.astPoint[3].s16Y * pstInitPara->iImgHeigth / 8192;
            }
            else if(IMP_FUNC_OBJSTOLEN == (pstURPpara->stRuleSet.astRule[i].u32Mode & IMP_FUNC_OBJSTOLEN))
            {
                //初始化图像帧数以便读入初始图像
                pstInitPara->framenum = 1;

                //物品盗移检测
                if (pstInitPara->pstURPpara->stRuleSet.astRule[i].stPara.stOscRulePara.astSpclRgs[0].stOscRg.s32PointNum > 4)
                {
                    printf("Rule Point Num > 4! \n");
                    return XM_PARA_FAULT;
                }

                pstInitPara->iObjMinArea = (int)(((float)pstURPpara->stRuleSet.astRule[i].stPara.stOscRulePara.stOscPara.s32SizeMin) /
                100 * pstInitPara->iImgHeigth * pstInitPara->iImgWidth);
                pstInitPara->iObjMaxArea = (int)(((float)pstURPpara->stRuleSet.astRule[i].stPara.stOscRulePara.stOscPara.s32SizeMax) /
                100 * pstInitPara->iImgHeigth * pstInitPara->iImgWidth);

                pstInitPara->pstURPpara->stRuleSet.astRule[i].stPara.stOscRulePara.astSpclRgs[0].stOscRg.astPoint[0].s16X =
                    pstURPpara->stRuleSet.astRule[i].stPara.stOscRulePara.astSpclRgs[0].stOscRg.astPoint[0].s16X * pstInitPara->iImgWidth / 8192;
                pstInitPara->pstURPpara->stRuleSet.astRule[i].stPara.stOscRulePara.astSpclRgs[0].stOscRg.astPoint[0].s16Y =
                    pstURPpara->stRuleSet.astRule[i].stPara.stOscRulePara.astSpclRgs[0].stOscRg.astPoint[0].s16Y * pstInitPara->iImgHeigth / 8192;

                pstInitPara->pstURPpara->stRuleSet.astRule[i].stPara.stOscRulePara.astSpclRgs[0].stOscRg.astPoint[1].s16X =
                    pstURPpara->stRuleSet.astRule[i].stPara.stOscRulePara.astSpclRgs[0].stOscRg.astPoint[1].s16X * pstInitPara->iImgWidth / 8192;
                pstInitPara->pstURPpara->stRuleSet.astRule[i].stPara.stOscRulePara.astSpclRgs[0].stOscRg.astPoint[1].s16Y =
                    pstURPpara->stRuleSet.astRule[i].stPara.stOscRulePara.astSpclRgs[0].stOscRg.astPoint[1].s16Y * pstInitPara->iImgHeigth / 8192;

                pstInitPara->pstURPpara->stRuleSet.astRule[i].stPara.stOscRulePara.astSpclRgs[0].stOscRg.astPoint[2].s16X =
                    pstURPpara->stRuleSet.astRule[i].stPara.stOscRulePara.astSpclRgs[0].stOscRg.astPoint[2].s16X * pstInitPara->iImgWidth / 8192;
                pstInitPara->pstURPpara->stRuleSet.astRule[i].stPara.stOscRulePara.astSpclRgs[0].stOscRg.astPoint[2].s16Y =
                    pstURPpara->stRuleSet.astRule[i].stPara.stOscRulePara.astSpclRgs[0].stOscRg.astPoint[2].s16Y * pstInitPara->iImgHeigth / 8192;
                pstInitPara->pstURPpara->stRuleSet.astRule[i].stPara.stOscRulePara.astSpclRgs[0].stOscRg.astPoint[3].s16X =
                    pstURPpara->stRuleSet.astRule[i].stPara.stOscRulePara.astSpclRgs[0].stOscRg.astPoint[3].s16X * pstInitPara->iImgWidth / 8192;
                pstInitPara->pstURPpara->stRuleSet.astRule[i].stPara.stOscRulePara.astSpclRgs[0].stOscRg.astPoint[3].s16Y =
                    pstURPpara->stRuleSet.astRule[i].stPara.stOscRulePara.astSpclRgs[0].stOscRg.astPoint[3].s16Y * pstInitPara->iImgHeigth / 8192;
            }

#ifdef _XM_IA_PRINT_
            printf("RemnantRect: %d %d %d %d  MinSize :　%d  MaxSize : %d \n iImgWidth:%d,iImgHeight:%d \n",
                pstInitPara->pstURPpara->stRuleSet.astRule[i].stPara.stOscRulePara.astSpclRgs[0].stOscRg.astPoint[0].s16X,
                pstInitPara->pstURPpara->stRuleSet.astRule[i].stPara.stOscRulePara.astSpclRgs[0].stOscRg.astPoint[0].s16Y,
                pstInitPara->pstURPpara->stRuleSet.astRule[i].stPara.stOscRulePara.astSpclRgs[0].stOscRg.astPoint[2].s16X,
                pstInitPara->pstURPpara->stRuleSet.astRule[i].stPara.stOscRulePara.astSpclRgs[0].stOscRg.astPoint[2].s16Y,
                pstInitPara->iObjMinArea,  pstInitPara->iObjMaxArea, pstInitPara->iImgWidth,pstInitPara->iImgHeigth );

#endif
        }
    }

#ifdef _XM_IA_PRINT_
    printf("Set Line : (%d , %d)   (%d, %d)\n", pstURPpara->stRuleSet.astRule[0].stPara.stTripwireRulePara.astLines[0].stLine.stStartPt.s16X,
        pstURPpara->stRuleSet.astRule[0].stPara.stTripwireRulePara.astLines[0].stLine.stStartPt.s16Y,
        pstURPpara->stRuleSet.astRule[0].stPara.stTripwireRulePara.astLines[0].stLine.stEndPt.s16X,
        pstURPpara->stRuleSet.astRule[0].stPara.stTripwireRulePara.astLines[0].stLine.stEndPt.s16Y);
#endif

    return XM_SUCCESS;
}

int XM_IA_AvdConfig(void *pvHandle, IMP_AVD_PARA_S *pstAvdPara)
{

    if (NULL == pstAvdPara || NULL == pvHandle)
    {
        IA_Printf("XM_IA_Config input Failed %d %d\n", (int)pvHandle, (int)pstAvdPara);
        return XM_NULL_POINTER;
    }

    XM_INIT_PARA *pstInitPara = (XM_INIT_PARA *)pvHandle;

    memcpy(pstInitPara->stAvdPara, pstAvdPara, sizeof(IMP_AVD_PARA_S));

    if(0 == pstInitPara->stAvdPara->stChangePara.u32AlarmLevel)
    {
        pstInitPara->stAvdPara->stChangePara.u32AlarmLevel = 3;
    }

    return XM_SUCCESS;

}

int XM_IA_GetResults(void *pvHandle, RESULT_S *pstResult)
{
    if (NULL == pstResult)
    {
        printf("pstResult is NULL!\n");
        return XM_NULL_POINTER;
    }
    XM_INIT_PARA *pstInitPara = (XM_INIT_PARA *)pvHandle;

#ifdef _XM_IA_PRINT_
    printf("pstResult-iTargetNum:  %d EventNum %d  TarId: %d\n",
        pstInitPara->pstResult->stTargetSet.s32TargetNum, pstInitPara->pstResult->stEventSet.s32EventNum,
        pstInitPara->pstResult->stTargetSet.astTargets[0].u32Id);
#endif

    memcpy(pstResult, pstInitPara->pstResult, sizeof(RESULT_S));
    memset(pstInitPara->pstResult, 0, sizeof(RESULT_S));

    return XM_SUCCESS;
}

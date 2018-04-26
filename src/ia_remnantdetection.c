#include "ia_remnantdetection.h"

HI_S32 xmRemnantDetection(XM_INIT_PARA *pstInitPara, IVE_CCLRECT_S stCclRect, IVE_IMAGE_S *pstImageGmmFg,int i)
{
    HI_U16  usiSpeedArry[MAX_OBJ_NUM];
    HI_U16  usiLastTimeArry[MAX_OBJ_NUM];
    IVE_RECT_S stDetectionRct = pstInitPara->pstRemnantDetectionInfo->stDetectionRct;
    IMP_RECT_S stObjRct;
    HI_S32 iInFlag = 0;

    HI_S32 iFgPixelNumInDiff = 0;
    HI_S32 iFgPixelNumInGmm = 0;
    float fRatio = 0.;

    memset(pstInitPara->pstRemnantDetectionRes, 0, sizeof(XM_REMNANTDETECTION_RES_S));
    memcpy(usiSpeedArry, pstInitPara->pstRemnantDetectionInfo->usiSpeedArry, MAX_OBJ_NUM *sizeof(HI_U16));
    memcpy(usiLastTimeArry, pstInitPara->pstRemnantDetectionInfo->usiLastTimeArry, MAX_OBJ_NUM *sizeof(HI_U16));

    stObjRct.s16X1 = (HI_S16)pstInitPara->pstCur_Frame_Obj->Object[i].stRect.s16X1;
    stObjRct.s16Y1 = (HI_S16)pstInitPara->pstCur_Frame_Obj->Object[i].stRect.s16Y1;
    stObjRct.s16X2 = (HI_S16)pstInitPara->pstCur_Frame_Obj->Object[i].stRect.s16X2;
    stObjRct.s16Y2 = (HI_S16)pstInitPara->pstCur_Frame_Obj->Object[i].stRect.s16Y2;

    //参数过滤
    if (pstInitPara->pstCur_Frame_Obj->Object[i].CCLRegionArea < pstInitPara->iObjMinArea)
    {
        return 0;
    }

    iInFlag = xmRctInRct(stObjRct, stDetectionRct);
    if (0 == iInFlag)
    {
        return 0;
    }
#ifdef _XM_IA_lINUX_
    HI_U64 pu64PixelStartPts = 0, pu64PixelEndPts = 0;

    //统计前景像素点
    HI_MPI_SYS_GetCurPts(&pu64PixelStartPts);
#endif 

    iFgPixelNumInDiff = xmStatPixelInFgImg(stObjRct, &pstInitPara->pstRemnantDetectionInfo->stImageDiffFg);
    //printf("----------------------Diff : %d\n", iFgPixelNumInDiff);
#ifdef _XM_IA_lINUX_
    HI_MPI_SYS_GetCurPts(&pu64PixelEndPts);

    //printf("DiffPixel: %lld ", pu64PixelEndPts - pu64PixelStartPts);

    HI_MPI_SYS_GetCurPts(&pu64PixelStartPts);
#endif 

    iFgPixelNumInGmm = xmStatPixelInFgImg(stObjRct, pstImageGmmFg);//fRatio
    //printf("---------------------Gmm : %d\n", iFgPixelNumInGmm);

#ifdef _XM_IA_lINUX_
    HI_MPI_SYS_GetCurPts(&pu64PixelEndPts);
#endif 

    fRatio = (float)iFgPixelNumInDiff / (float)iFgPixelNumInGmm;

    if (iFgPixelNumInDiff > 40 || fRatio > 0.2 || iFgPixelNumInGmm < 150)
    {
        return 0;
    }

    pstInitPara->pstRemnantDetectionRes->stDetectedRct[pstInitPara->pstRemnantDetectionRes->u16DetectRes] = stObjRct;
    pstInitPara->pstRemnantDetectionRes->u16DetectRes++;

    return 0;
}
//前景位置判断
HI_S32 xmBlob(XM_INIT_PARA *pstInitPara,IVE_CCBLOB_S *pstBlob)
{
    HI_U16  usiSpeedArry[MAX_OBJ_NUM];
    HI_U16  usiLastTimeArry[MAX_OBJ_NUM];
    IVE_RECT_S stDetectionRct = pstInitPara->pstRemnantDetectionInfo->stDetectionRct;
    IMP_RECT_S stObj;
    HI_S32 iGonMark,i;
    HI_S32 iGonNum = 0;
    int iWidth,iHeight;

    memset(pstInitPara->pstRemnantDetectionRes, 0, sizeof(XM_REMNANTDETECTION_RES_S));
    memcpy(usiSpeedArry, pstInitPara->pstRemnantDetectionInfo->usiSpeedArry, MAX_OBJ_NUM *sizeof(HI_U16));
    memcpy(usiLastTimeArry, pstInitPara->pstRemnantDetectionInfo->usiLastTimeArry, MAX_OBJ_NUM *sizeof(HI_U16));

    //前景CCL区域位置判别
    for (i = 0; i < 254; i++)
    {    
        iWidth = (pstBlob->astRegion[i].u16Bottom - pstBlob->astRegion[i].u16Top);
        iHeight = (pstBlob->astRegion[i].u16Right - pstBlob->astRegion[i].u16Left);
        if((iWidth * iHeight) >= ((stDetectionRct.u16Width*stDetectionRct.u16Height)/3))
        {
            stObj.s16Y1 = pstBlob->astRegion[i].u16Top;
            stObj.s16Y2 = pstBlob->astRegion[i].u16Bottom;
            stObj.s16X1 = pstBlob->astRegion[i].u16Left;
            stObj.s16X2 = pstBlob->astRegion[i].u16Right;
            if(1 == xmRctInRct(stObj,stDetectionRct))
            {
                iGonNum++;
            }
        }
    }
    //1有前景 2没有前景
    if(iGonNum != 0)
    {
        iGonMark = 1; 
    }
    else
    {
        iGonMark = 0; 
    }
    
    return iGonMark;
}

HI_S32 xmRctInRct(IMP_RECT_S stObjRct, IVE_RECT_S stDetectionRct)
{
    HI_S32 iInFlag = 0;
    HI_S32 iTop1 = stObjRct.s16Y1;
    HI_S32 iBottom1 = stObjRct.s16Y2;
    HI_S32 iLeft1 = stObjRct.s16X1;
    HI_S32 iRight1 = stObjRct.s16X2;

    HI_S32 iTop2 = stDetectionRct.u16Y;
    HI_S32 iBottom2 = stDetectionRct.u16Y + stDetectionRct.u16Height;
    HI_S32 iLeft2 = stDetectionRct.u16X;
    HI_S32 iRight2 = stDetectionRct.u16X + stDetectionRct.u16Width;

    HI_S32 iTop = 0;
    HI_S32 iBottom = 0;
    HI_S32 iLeft = 0;
    HI_S32 iRight = 0;

    HI_S32 iArea = 0;
    HI_S32 iArea1 = 0;
    HI_S32 iArea2 = 0;

    //一个矩形在另一个矩形内
    if ((iTop2 < iTop1) && (iBottom2 > iBottom1) && (iRight2 > iRight1) && (iLeft2 < iLeft1))
    {
        iInFlag = 1;
        return iInFlag;
    }

    //异常框
    if ((iTop1 < iTop2) && (iBottom1 > iBottom2) && (iRight1 > iRight2) && (iLeft1 < iLeft2))
    {
        iInFlag = 0;
        return iInFlag;
    }

    //不相交
    if ((iTop2 > iBottom1) || (iRight2 < iLeft1) || (iTop1 > iBottom2) || (iRight1 < iLeft2))
    {
        iInFlag = 0;
        return iInFlag;
    }

    //相交
    iTop = iTop1 > iTop2 ? iTop1 : iTop2;
    iBottom = iBottom1 > iBottom2 ? iBottom2 : iBottom1;
    iRight = iRight1 > iRight2 ? iRight2 : iRight1;
    iLeft = iLeft1 > iLeft2 ? iLeft1 : iLeft2;
    iArea = (iRight - iLeft) * (iBottom - iTop);
    iArea1 = (iRight1 - iLeft1) * (iBottom1 - iTop1);
    iArea2 = (iRight2 - iLeft2) * (iBottom2 - iTop2);

    iInFlag = 1;
    if ((float)iArea / iArea1 < 0.25)
    {
        iInFlag = 0;
    }

    if ((float)iArea / iArea2 < 0.05)
    {
        iInFlag = 0;
    }

    return iInFlag;
}

HI_S32 xmStatPixelInFgImg(IMP_RECT_S stObjRct, IVE_IMAGE_S *pstFgImg)
{
    HI_S32 iPixelNum = 1;
    HI_U8 *pu8ImgAddr= pstFgImg->pu8VirAddr[0];
    HI_U16 u16Stride = pstFgImg->u16Stride[0];

    HI_U16 u16StartX = stObjRct.s16X1;
    HI_U16 u16EndX = stObjRct.s16X2 - 1;
    HI_U16 u16StartY = stObjRct.s16Y1;
    HI_U16 u16EndY = stObjRct.s16Y2 - 1;
      
    HI_U32 iPosStart = 0;
    HI_U32 iPos = 0;
    HI_U16 i = 0;
    HI_U16 j = 0;

    if(u16StartX > pstFgImg->u16Width)
    {
        u16StartX = pstFgImg->u16Width - 1;
    }
    if(u16EndX > pstFgImg->u16Width)
    {
        u16EndX = pstFgImg->u16Width - 1;
    }
    if(u16StartY > pstFgImg->u16Height)
    {
        u16StartY = pstFgImg->u16Height - 1;
    }
    if(u16EndY > pstFgImg->u16Height)
    {
        u16EndY = pstFgImg->u16Height - 1;
    }
    
    if (IVE_IMAGE_TYPE_U8C1 != pstFgImg->enType)
    {
        printf("xmStatPixelInFgImg：Bad Image format Input\n");
        return iPixelNum;
    }   

    iPosStart = u16StartY*u16Stride + u16StartX;
    for (i = u16StartY; i < u16EndY; i++)
    {
        iPos = iPosStart;
        for (j = u16StartX; j < u16EndX; j++)
        {
            if (*(pu8ImgAddr + iPos) > 0)
            {
                iPixelNum++;
            }
            iPos++;
        }
        iPosStart += u16Stride;
    }

    return iPixelNum;
}

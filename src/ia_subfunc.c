#include "ia_subfunc.h"
#include "mpi_ive.h"
#include <math.h>
#include <unistd.h>

#ifdef _XM_IA_DEBUG_
extern int framenum;
#endif

#ifdef _XM_IA_MEM_
extern int size;
#endif

HI_U16 XM_IA_IVE_CalcStride(HI_U16 u16Width, HI_U16 u16Align)
{
    return (u16Width + ((u16Align - u16Width % u16Align) % u16Align));
}

FILE* ReadTxtSequence(char* filepath, int filenum, char* filetype, FILE* frp)
{
    /*1、将顺序int转为char*/
    char num[5];
    sprintf(num, "%d", filenum);
    /*2、组合成文件路径和文件名*/
    filepath = strcat(strcat(filepath, num), filetype);
#ifdef _XM_IA_DEBUG_
    printf("open: %s\n", filepath);
#endif
    /*3、打开文件*/
    frp = fopen(filepath, "rb");
    if (frp == NULL)
        printf("open %s fail\n", filepath);
    return frp;
}

void GMM_DES_IMAGE_S_TO_TXT(char* Fg_Output_Filepath, char* Bg_Output_Filepath,
    int output_filenum, char* filetype,
    IVE_GMM_S* Gmm)
{
    FILE* Fg_fwp = NULL;
    //FILE* Bg_fwp = NULL;

    /*1、将顺序int转为char*/
    char num[10];
    sprintf(num, "%d", output_filenum);

    /*2、组合成文件路径和文件名*/
    Fg_Output_Filepath = strcat(strcat(Fg_Output_Filepath, num), filetype);
    //Bg_Output_Filepath = strcat(strcat(Bg_Output_Filepath, num), filetype);

    /*3、以"wb"写入方式打开文件*/
    Fg_fwp = fopen(Fg_Output_Filepath, "wb");
    //Bg_fwp = fopen(Bg_Output_Filepath, "wb");

    //fwrite(Gmm->stFg.pu8VirAddr[0], sizeof(HI_U8), ImageWidth*ImageHeight, Fg_fwp);
    /********************************膨胀腐蚀（CCL）之后，最终结果是stERODEImg*****************/
    fwrite(Gmm->stERODEImg.pu8VirAddr[0], sizeof(HI_U8), Gmm->stERODEImg.u16Stride[0]*Gmm->stERODEImg.u16Height, Fg_fwp);
    //fwrite(Gmm->stBg.pu8VirAddr[0], sizeof(HI_U8), ImageWidth*ImageHeight, Bg_fwp);
    fclose(Fg_fwp);
    //fclose(Bg_fwp);
    printf("%s has done!\n", Fg_Output_Filepath);
    //printf("%s has done!\n", Bg_Output_Filepath);

}

HI_S32 IVE_CREATE_IMAGE_S(IVE_IMAGE_S *pstImg,
    IVE_IMAGE_TYPE_E enType, HI_U16 u16Width, HI_U16 u16Height)
{
    HI_U32 u32Size = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    if (NULL == pstImg)
    {
        printf("pstImg is null\n");
        return HI_FAILURE;
    }
    pstImg->enType = enType;
    pstImg->u16Width = u16Width;
    pstImg->u16Height = u16Height;
    pstImg->u16Stride[0] = XM_IA_IVE_CalcStride(pstImg->u16Width, IVE_ALIGN);

    if (enType == IVE_IMAGE_TYPE_U8C1)
    {
        u32Size = pstImg->u16Stride[0] * pstImg->u16Height;
    
#ifdef _XM_IA_lINUX_
     
        s32Ret = HI_MPI_SYS_MmzAlloc(&(pstImg->u32PhyAddr[0]), (void**)&pstImg->pu8VirAddr[0], NULL, HI_NULL, u32Size);
#endif
     
#ifdef _XM_IA_WINDOWS_
        pstImg->pu8VirAddr[0] = (HI_U8*)malloc(u32Size);
        pstImg->u32PhyAddr[0] = (HI_U32)pstImg->pu8VirAddr[0];
        if(pstImg->pu8VirAddr[0] == NULL)
        {
            printf("pstImg->pu8VirAddr[0] Failed\n");
        }
#endif

        if (s32Ret != HI_SUCCESS)
        {
            printf("Mmz Alloc fail,Error(%#x) size %d\n", s32Ret, u32Size);
            return s32Ret;
        }
        memset(pstImg->pu8VirAddr[0], 0, u32Size);
    }

    if (enType == IVE_IMAGE_TYPE_S16C1)
    {
        u32Size = pstImg->u16Stride[0] * pstImg->u16Height * sizeof(HI_S16);

#ifdef _XM_IA_lINUX_
        s32Ret = HI_MPI_SYS_MmzAlloc(&(pstImg->u32PhyAddr[0]), (void**)&pstImg->pu8VirAddr[0], NULL, HI_NULL, u32Size);
#endif

#ifdef _XM_IA_WINDOWS_
        pstImg->pu8VirAddr[0] = (HI_U8*)malloc(u32Size);
        pstImg->u32PhyAddr[0] = (HI_U32)pstImg->pu8VirAddr[0];
        if(pstImg->pu8VirAddr[0] == NULL)
        {
            printf("pstImg->pu8VirAddr[0] Failed\n");
        }
#endif

        if (s32Ret != HI_SUCCESS)
        {
            printf("Mmz Alloc fail,Error(%#x) size %d\n", s32Ret, u32Size);
            return s32Ret;
        }
        memset(pstImg->pu8VirAddr[0], 0, u32Size);
    }

#ifdef _XM_IA_MEM_
    size += u32Size;
#endif

    return s32Ret;
}

HI_VOID COMM_IVE_BlobToRect(IVE_CCBLOB_S *pstBlob, RECT_ARRAY_S *pstRect,
    HI_U16 u16RectMaxNum, HI_U16 u16AreaThrStep, int imageWid, int imageHgt)
{
    HI_U16 u16Num;
    HI_U16 i, j, k;
    int iIndex = 0;
    HI_U16 u16Thr = 0;
    HI_BOOL bValid;

    if (pstBlob->u8RegionNum > u16RectMaxNum)
    {
        u16Thr = pstBlob->u16CurAreaThr;
        do
        {
            u16Num = 0;
            u16Thr += u16AreaThrStep;
            for (i = 0; i < 254; i++)
            {
                if (pstBlob->astRegion[i].u32Area > u16Thr)
                {
                    u16Num++;
                }
            }
        } while (u16Num > u16RectMaxNum);
    }

    u16Num = 0;

    for (i = 0; i < 254; i++)
    {
        if (pstBlob->astRegion[i].u32Area > u16Thr)
        {
            HI_S32 usWid, usHgt;

            usWid = (HI_S32)(pstBlob->astRegion[i].u16Right - pstBlob->astRegion[i].u16Left);
            usHgt = (HI_S32)(pstBlob->astRegion[i].u16Bottom - pstBlob->astRegion[i].u16Top);

            //width height 最小尺寸
            if ((usWid * 30 < imageWid) || (usHgt * 30 < imageHgt))
            {
                continue;
            }
             //比例异常
            if (usWid > usHgt * 10 || usHgt > usWid * 10)
            {
                continue;
            }

            //占空比异常
            if (pstBlob->astRegion[i].u32Area < usWid * usHgt * 0.15)
            {
                continue;
            }

            pstRect->astRect[u16Num].u32Area = (HI_S32)(pstBlob->astRegion[i].u32Area);

            /********************************0-左上 1-右上 2-右下 3-左下*****************************/
            pstRect->astRect[u16Num].astPoint[0].s32X = (HI_S32)(pstBlob->astRegion[i].u16Left) & (~1);
            pstRect->astRect[u16Num].astPoint[0].s32Y = (HI_S32)(pstBlob->astRegion[i].u16Top) & (~1);

            pstRect->astRect[u16Num].astPoint[1].s32X = (HI_S32)(pstBlob->astRegion[i].u16Right) & (~1);
            pstRect->astRect[u16Num].astPoint[1].s32Y = (HI_S32)(pstBlob->astRegion[i].u16Top) & (~1);

            pstRect->astRect[u16Num].astPoint[2].s32X = (HI_S32)(pstBlob->astRegion[i].u16Right) & (~1);
            pstRect->astRect[u16Num].astPoint[2].s32Y = (HI_S32)(pstBlob->astRegion[i].u16Bottom) & (~1);

            pstRect->astRect[u16Num].astPoint[3].s32X = (HI_S32)(pstBlob->astRegion[i].u16Left) & (~1);
            pstRect->astRect[u16Num].astPoint[3].s32Y = (HI_S32)(pstBlob->astRegion[i].u16Bottom) & (~1);

            bValid = HI_TRUE;
            for (j = 0; j < 3; j++)
            {
                for (k = j + 1; k < 4; k++)
                {
                    if ((pstRect->astRect[u16Num].astPoint[j].s32X == pstRect->astRect[u16Num].astPoint[k].s32X)
                        && (pstRect->astRect[u16Num].astPoint[j].s32Y == pstRect->astRect[u16Num].astPoint[k].s32Y))
                    {
                        bValid = HI_FALSE;
                        break;
                    }

                }
            }
            if (HI_TRUE == bValid)
            {
                u16Num++;
            }
        }
    }

    /********************************CCL区域后期处理***************************/
    for (j = 0; j < u16Num - 1; j++)
    {
        if (0 != pstRect->astRect[j].u32Area)
        {
            for (k = j + 1; k < u16Num; k++)
            {
                if (0 == pstRect->astRect[k].u32Area)
                {
                    continue;
                }

                int iWidj = (int)pstRect->astRect[j].astPoint[1].s32X - (int)pstRect->astRect[j].astPoint[0].s32X;
                int iHgtj = (int)pstRect->astRect[j].astPoint[2].s32Y - (int)pstRect->astRect[j].astPoint[0].s32Y;
                int iWidk = (int)pstRect->astRect[k].astPoint[1].s32X - (int)pstRect->astRect[k].astPoint[0].s32X;
                int iHgtk = (int)pstRect->astRect[k].astPoint[2].s32Y - (int)pstRect->astRect[k].astPoint[0].s32Y;
                //中心点坐标的两倍，和j、k宽高的两倍做比较，就不除以2了
                int icenterjx = (int)pstRect->astRect[j].astPoint[1].s32X + (int)pstRect->astRect[j].astPoint[0].s32X;
                int icenterjy = (int)pstRect->astRect[j].astPoint[2].s32Y + (int)pstRect->astRect[j].astPoint[0].s32Y;
                int icenterkx = (int)pstRect->astRect[k].astPoint[1].s32X + (int)pstRect->astRect[k].astPoint[0].s32X;
                int icenterky = (int)pstRect->astRect[k].astPoint[2].s32Y + (int)pstRect->astRect[k].astPoint[0].s32Y;

                if (((abs(icenterjx - icenterkx)) <= (iWidj + iWidk)) && (abs(icenterjy - icenterky) <= (iHgtj + iHgtk)))
                {
                    pstRect->astRect[j].u32Area = pstRect->astRect[j].u32Area + pstRect->astRect[k].u32Area;
                    pstRect->astRect[j].astPoint[0].s32X = MIN(pstRect->astRect[k].astPoint[0].s32X, pstRect->astRect[j].astPoint[0].s32X);
                    pstRect->astRect[j].astPoint[0].s32Y = MIN(pstRect->astRect[k].astPoint[0].s32Y, pstRect->astRect[j].astPoint[0].s32Y);
                    pstRect->astRect[j].astPoint[1].s32X = MAX(pstRect->astRect[k].astPoint[1].s32X, pstRect->astRect[j].astPoint[1].s32X);
                    pstRect->astRect[j].astPoint[1].s32Y = MIN(pstRect->astRect[k].astPoint[1].s32Y, pstRect->astRect[j].astPoint[1].s32Y);
                    pstRect->astRect[j].astPoint[2].s32X = MAX(pstRect->astRect[k].astPoint[2].s32X, pstRect->astRect[j].astPoint[2].s32X);
                    pstRect->astRect[j].astPoint[2].s32Y = MAX(pstRect->astRect[k].astPoint[2].s32Y, pstRect->astRect[j].astPoint[2].s32Y);
                    pstRect->astRect[j].astPoint[3].s32X = MIN(pstRect->astRect[k].astPoint[3].s32X, pstRect->astRect[j].astPoint[3].s32X);
                    pstRect->astRect[j].astPoint[3].s32Y = MAX(pstRect->astRect[k].astPoint[3].s32Y, pstRect->astRect[j].astPoint[3].s32Y);
                    pstRect->astRect[k].u32Area = 0;
                }
            }
        }
    }

    for (j = 0; j < u16Num; j++)
    {
        if (pstRect->astRect[j].u32Area > 0)
        {
            if (iIndex != j)
            {
                memcpy(pstRect->astRect + iIndex, pstRect->astRect + j, sizeof(IVE_CCLRECT_S));
            }
            iIndex++;
        }
    }

    memset(pstRect->astRect + iIndex, 0, sizeof(IVE_CCLRECT_S)*(MAX_OBJ_NUM - iIndex));
    pstRect->u16Num = iIndex;

    return;
}

void RectToTxt(char* Rect_Output_Filepath, int output_filenum, char* filetype, RECT_ARRAY_S stRegion)
{
    char num[10];
    sprintf(num, "%d", output_filenum);
    /*组合成文件路径和文件名*/
    Rect_Output_Filepath = strcat(strcat(Rect_Output_Filepath, num), filetype);
    printf("%s\n", Rect_Output_Filepath);
    FILE* Rect_fwp = NULL;
    Rect_fwp = fopen(Rect_Output_Filepath, "wb");
    int w;
    for (w = 0; w < stRegion.u16Num; w++)//MAX_OBJ_NUM
    {

        fprintf(Rect_fwp, "%d", (HI_S32)stRegion.astRect[w].astPoint[0].s32X);//左边的坐标
        fprintf(Rect_fwp, "%s", " ");//插入空格
        fprintf(Rect_fwp, "%d", (HI_S32)stRegion.astRect[w].astPoint[0].s32Y);//上边的坐标
        fprintf(Rect_fwp, "%s", " ");//插入空格
        fprintf(Rect_fwp, "%d", (HI_S32)stRegion.astRect[w].astPoint[2].s32X);//右边的坐标
        fprintf(Rect_fwp, "%s", " ");//插入空格
        fprintf(Rect_fwp, "%d", (HI_S32)stRegion.astRect[w].astPoint[2].s32Y);//下边的坐标
        fprintf(Rect_fwp, "%s", " ");//插入空格
        //  printf("X:　%d Y: %d \n", (HI_S32)(stGmm.stRegion.astRect[0].astPoint[i].s32X) & (~1), (HI_S32)(stGmm.stRegion.astRect[0].astPoint[i].s32Y) & (~1));
    }
    fclose(Rect_fwp);
}

//初始化
HI_S32 IVE_GMM_LK_INIT(IVE_GMM_LK_S *pstGmmLk, HI_U16 u16Width, HI_U16 u16Height, ALGO_MODULE_E eAlgType)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32Size = 0;
    HI_U16 i;
    memset(pstGmmLk, 0, sizeof(IVE_GMM_LK_S));
	
    s32Ret = IVE_CREATE_IMAGE_S(&(pstGmmLk->stGMM.stResizeSrc), IVE_IMAGE_TYPE_U8C1, u16Width, u16Height);
    if (s32Ret != HI_SUCCESS)
    {
        printf("Mmz Alloc stSrc fail,Error(%#x)\n", s32Ret);
        return HISI_IVE_FAIL;
    }

    /*********************************************创建源图像***************************************/
    s32Ret = IVE_CREATE_IMAGE_S(&(pstGmmLk->stGMM.stSrc), IVE_IMAGE_TYPE_U8C1, u16Width, u16Height);
    if (s32Ret != HI_SUCCESS)
    {
        printf("Mmz Alloc stSrc fail,Error(%#x)\n", s32Ret);
        return HISI_IVE_FAIL;
    }

    /********************************************创建输出前景图像************************************/
    s32Ret = IVE_CREATE_IMAGE_S(&(pstGmmLk->stGMM.stFg), IVE_IMAGE_TYPE_U8C1, u16Width, u16Height);
    if (s32Ret != HI_SUCCESS)
    {
        printf("Mmz Alloc stFg fail,Error(%#x)\n", s32Ret);
        return HISI_IVE_FAIL;
    }
    /*******************************************创建输出背景图像************************************/
    s32Ret = IVE_CREATE_IMAGE_S(&(pstGmmLk->stGMM.stBg), IVE_IMAGE_TYPE_U8C1, u16Width, u16Height);
    if (s32Ret != HI_SUCCESS)
    {
        printf("Mmz Alloc stBg fail,Error(%#x)\n", s32Ret);
        return HISI_IVE_FAIL;
    }
    /*****************************************创建膨胀处理图像****************************************/
    s32Ret = IVE_CREATE_IMAGE_S(&(pstGmmLk->stGMM.stDILATEImg), IVE_IMAGE_TYPE_U8C1, u16Width, u16Height);
    if (s32Ret != HI_SUCCESS)
    {
        printf("Mmz Alloc stDILATEImg fail,Error(%#x)\n", s32Ret);
        return HISI_IVE_FAIL;
    }
    /**************************************创建腐蚀处理图像******************************************/
    s32Ret = IVE_CREATE_IMAGE_S(&(pstGmmLk->stGMM.stERODEImg), IVE_IMAGE_TYPE_U8C1, u16Width, u16Height);
    if (s32Ret != HI_SUCCESS)
    {
        printf("Mmz Alloc stERODEImg fail,Error(%#x)\n", s32Ret);
        return HISI_IVE_FAIL;
    }
	
    /***********************************设置膨胀、腐蚀参数******************************************/
    memset(pstGmmLk->stGMM.stDilateCtrl.au8Mask, 255, 25);
    memset(pstGmmLk->stGMM.stErodeCtrl.au8Mask, 255, 25);

    /************************************设置GMM模型控制参数*****************************************/
    pstGmmLk->stGMM.stGmmCtrl.u0q16InitWeight = 3276; //0.05
    pstGmmLk->stGMM.stGmmCtrl.u0q16BgRatio = 52428;   //0.8
    pstGmmLk->stGMM.stGmmCtrl.u22q10MaxVar = (2000 << 10);
    pstGmmLk->stGMM.stGmmCtrl.u22q10MinVar = (200 << 10);
    pstGmmLk->stGMM.stGmmCtrl.u22q10NoiseVar = (225 << 10);
    pstGmmLk->stGMM.stGmmCtrl.u8q8VarThr = 500;
    pstGmmLk->stGMM.stGmmCtrl.u8ModelNum = 3;//5;//WZG = 5
    if(IMP_OSC_AGLO_MODULE == eAlgType)
    {
        pstGmmLk->stGMM.stGmmCtrl.u0q16LearnRate = 100;//280;
    }
    else
    {
        pstGmmLk->stGMM.stGmmCtrl.u0q16LearnRate = 240;//327;
    }

    /************************************设置Sobel 模型 控制参数***************************************/
    pstGmmLk->stGMM.stSobelCtrl.enOutCtrl= IVE_SOBEL_OUT_CTRL_HOR;
    HI_S8 gasVCLTemplateGs_Tab[25]={0,0,0,0,0,0,-1,-2,-1,0,0,0,0,0,0,0,1,2,1,0,0,0,0,0,0};

    int s =0;
    for(s=0;s<25;s++){
        pstGmmLk->stGMM.stSobelCtrl.as8Mask[s] = gasVCLTemplateGs_Tab[s];
    }

    /************************************设置S16 转U8  控制参数*****************************************/
    pstGmmLk->stGMM.st16to8Ctrl.enMode = IVE_16BIT_TO_8BIT_MODE_S16_TO_U8_ABS;
    pstGmmLk->stGMM.st16to8Ctrl.u16Denominator = 1;
    pstGmmLk->stGMM.st16to8Ctrl.u8Numerator = 1;


    /************************************为GMM模型分配地址空间***************************************/
    u32Size = pstGmmLk->stGMM.stSrc.u16Width * pstGmmLk->stGMM.stSrc.u16Height * pstGmmLk->stGMM.stGmmCtrl.u8ModelNum * 7;
    pstGmmLk->stGMM.stModel.u32Size = u32Size;
    
#ifdef _XM_IA_lINUX_
    s32Ret = HI_MPI_SYS_MmzAlloc(&pstGmmLk->stGMM.stModel.u32PhyAddr, (void**)&pstGmmLk->stGMM.stModel.pu8VirAddr, NULL, HI_NULL, u32Size);
#endif

#ifdef _XM_IA_WINDOWS_
    pstGmmLk->stGMM.stModel.pu8VirAddr = (HI_U8*)malloc(u32Size);
    pstGmmLk->stGMM.stModel.u32PhyAddr = (HI_U32)pstGmmLk->stGMM.stModel.pu8VirAddr;
    if(pstGmmLk->stGMM.stModel.pu8VirAddr == NULL)
    {
        printf("pstGmmLk->stGMM.stModel.pu8VirAddr\n");
    }
#endif


    if (s32Ret != HI_SUCCESS)
    {
        printf("Mmz Alloc GMM Model fail,Error(%#x)\n", s32Ret);
        return HISI_IVE_FAIL;
    }
    memset(pstGmmLk->stGMM.stModel.pu8VirAddr, 0, u32Size);
#ifdef _XM_IA_MEM_
    size += u32Size;
#endif

    /*************************************初始化CCL模型**********************************************/
    u32Size = sizeof(IVE_CCBLOB_S);

#ifdef _XM_IA_lINUX_
    s32Ret = HI_MPI_SYS_MmzAlloc(&pstGmmLk->stGMM.stBlob.u32PhyAddr, (void**)&pstGmmLk->stGMM.stBlob.pu8VirAddr, NULL, HI_NULL, u32Size);
#endif

#ifdef _XM_IA_WINDOWS_
    pstGmmLk->stGMM.stBlob.u32PhyAddr = (HI_U32*)malloc(u32Size);;
    pstGmmLk->stGMM.stBlob.pu8VirAddr = (HI_U8*)malloc(u32Size);
    pstGmmLk->stGMM.stBlob.u32PhyAddr = (HI_U32)pstGmmLk->stGMM.stBlob.pu8VirAddr;
#endif
    if(pstGmmLk->stGMM.stBlob.pu8VirAddr == NULL)
    {
       printf("pstGmmLk->stGMM.stBlob.pu8VirAddr Failed\n");
    } 
    pstGmmLk->stGMM.stBlob.u32Size = u32Size;
    if (s32Ret != HI_SUCCESS)
    {
        printf("Mmz Alloc CCL Model fail,Error(%#x)\n", s32Ret);
        return HISI_IVE_FAIL;
    }
    memset(pstGmmLk->stGMM.stBlob.pu8VirAddr, 0, u32Size);
#ifdef _XM_IA_MEM_
        size += u32Size;
#endif

    /***********************************初始化CCL控制参数**********************************************/
//    pstGmmLk->stGMM.stCclCtrl.enMode = IVE_CCL_MODE_8C;//Only for Hi3516CV300 or Hi3519
    pstGmmLk->stGMM.stCclCtrl.u16InitAreaThr = 16;
    pstGmmLk->stGMM.stCclCtrl.u16Step = 4;

    /*********************************************创建角点检测源图像***************************************/
    s32Ret = IVE_CREATE_IMAGE_S(&pstGmmLk->stLK.stSrc, IVE_IMAGE_TYPE_U8C1, u16Width, u16Height);
    if (s32Ret != HI_SUCCESS)
    {
        printf("Mmz Alloc LK->stSrc fail,Error(%#x)\n", s32Ret);
        return HISI_IVE_FAIL;
    }

    /*************************************创建第一次角点检测输出图像***************************************/
    s32Ret = IVE_CREATE_IMAGE_S(&pstGmmLk->stLK.stDst, IVE_IMAGE_TYPE_U8C1, u16Width, u16Height);
    if (s32Ret != HI_SUCCESS)
    {
        printf("Mmz Alloc LK->stDst fail,Error(%#x)\n", s32Ret);
        return HISI_IVE_FAIL;
    }

    /**************************************创建停留图像******************************************/
    s32Ret = IVE_CREATE_IMAGE_S(&pstGmmLk->stLK.stPoly, IVE_IMAGE_TYPE_U8C1, u16Width, u16Height);
    if (s32Ret != HI_SUCCESS)
    {
        printf("Mmz Alloc LK->stDst fail,Error(%#x)\n", s32Ret);
        return HISI_IVE_FAIL;
    }

    s32Ret = IVE_CREATE_IMAGE_S(&pstGmmLk->stLK.stPolyDst, IVE_IMAGE_TYPE_U8C1, u16Width, u16Height);
    if (s32Ret != HI_SUCCESS)
    {
        printf("Mmz Alloc LK->stDst fail,Error(%#x)\n", s32Ret);
        return HISI_IVE_FAIL;
    }


    s32Ret = IVE_CREATE_IMAGE_S(&pstGmmLk->stLK.stPilfer, IVE_IMAGE_TYPE_U8C1, u16Width, u16Height);
    if (s32Ret != HI_SUCCESS)
    {
        printf("Mmz Alloc LK->stPilfer fail,Error(%#x)\n", s32Ret);
        return HISI_IVE_FAIL;
    }

    s32Ret = IVE_CREATE_IMAGE_S(&pstGmmLk->stLK.stPilferDst, IVE_IMAGE_TYPE_U8C1, u16Width, u16Height);
    if (s32Ret != HI_SUCCESS)
    {
        printf("Mmz Alloc LK->stPilferDst fail,Error(%#x)\n", s32Ret);
        return HISI_IVE_FAIL;
    }

    s32Ret = IVE_CREATE_IMAGE_S(&pstGmmLk->stLK.stSceneCh, IVE_IMAGE_TYPE_U8C1, u16Width, u16Height);
    if (s32Ret != HI_SUCCESS)
    {
        printf("Mmz Alloc LK->stPilfer fail,Error(%#x)\n", s32Ret);
        return HISI_IVE_FAIL;
    }

    s32Ret = IVE_CREATE_IMAGE_S(&pstGmmLk->stLK.stSceneChDst, IVE_IMAGE_TYPE_U8C1, u16Width, u16Height);
    if (s32Ret != HI_SUCCESS)
    {
        printf("Mmz Alloc LK->stPilferDst fail,Error(%#x)\n", s32Ret);
        return HISI_IVE_FAIL;
    }

    s32Ret = IVE_CREATE_IMAGE_S(&pstGmmLk->stLK.stNCCObj, IVE_IMAGE_TYPE_U8C1, u16Width, u16Height);
    if (s32Ret != HI_SUCCESS)
    {
        printf("Mmz Alloc LK->stNCC fail,Error(%#x)\n", s32Ret);
        return HISI_IVE_FAIL;
    }
    s32Ret = IVE_CREATE_IMAGE_S(&pstGmmLk->stLK.stNCC, IVE_IMAGE_TYPE_U8C1, u16Width, u16Height);
    if (s32Ret != HI_SUCCESS)
    {
        printf("Mmz Alloc LK->stNCC fail,Error(%#x)\n", s32Ret);
        return HISI_IVE_FAIL;
    }
    s32Ret = IVE_CREATE_IMAGE_S(&pstGmmLk->stGMM.stSobel, IVE_IMAGE_TYPE_S16C1, u16Width, u16Height);
    if (s32Ret != HI_SUCCESS)
    {
        printf("Mmz Alloc LK->stNCC fail,Error(%#x)\n", s32Ret);
        return HISI_IVE_FAIL;
    }

    s32Ret = IVE_CREATE_IMAGE_S(&pstGmmLk->stGMM.stSobelDst, IVE_IMAGE_TYPE_U8C1, u16Width, u16Height);
    if (s32Ret != HI_SUCCESS)
    {
        printf("Mmz Alloc LK->stNCC fail,Error(%#x)\n", s32Ret);
        return HISI_IVE_FAIL;
    }

    s32Ret = IVE_CREATE_IMAGE_S(&pstGmmLk->stGMM.stNCCSrc1, IVE_IMAGE_TYPE_U8C1, u16Width/2, u16Height/2);
    if (s32Ret != HI_SUCCESS)
    {
       printf("Mmz Alloc stNCCSrc fail,Error(%#x)\n", s32Ret);
       return HISI_IVE_FAIL;
    }
    s32Ret = IVE_CREATE_IMAGE_S(&pstGmmLk->stGMM.stNCCDst1, IVE_IMAGE_TYPE_U8C1, u16Width/2, u16Height/2);
    if (s32Ret != HI_SUCCESS)
    {
       printf("Mmz Alloc stNCCDst fail,Error(%#x)\n", s32Ret);
       return HISI_IVE_FAIL;
    }
       /* s32Ret = IVE_CREATE_IMAGE_S(&pstGmmLk->stGMM.stNCCSrc2, IVE_IMAGE_TYPE_U8C1, u16Width/2, u16Height/2);
    if (s32Ret != HI_SUCCESS)
    {
       printf("Mmz Alloc stNCCSrc fail,Error(%#x)\n", s32Ret);
       return HISI_IVE_FAIL;
    }
    s32Ret = IVE_CREATE_IMAGE_S(&pstGmmLk->stGMM.stNCCDst2, IVE_IMAGE_TYPE_U8C1, u16Width/2, u16Height/2);
    if (s32Ret != HI_SUCCESS)
    {
       printf("Mmz Alloc stNCCDst fail,Error(%#x)\n", s32Ret);
       return HISI_IVE_FAIL;
    }
        s32Ret = IVE_CREATE_IMAGE_S(&pstGmmLk->stGMM.stNCCSrc3, IVE_IMAGE_TYPE_U8C1, u16Width/2, u16Height/2);
    if (s32Ret != HI_SUCCESS)
    {
       printf("Mmz Alloc stNCCSrc fail,Error(%#x)\n", s32Ret);
       return HISI_IVE_FAIL;
    }
    s32Ret = IVE_CREATE_IMAGE_S(&pstGmmLk->stGMM.stNCCDst3, IVE_IMAGE_TYPE_U8C1, u16Width/2, u16Height/2);
    if (s32Ret != HI_SUCCESS)
    {
       printf("Mmz Alloc stNCCDst fail,Error(%#x)\n", s32Ret);
       return HISI_IVE_FAIL;
    }
        s32Ret = IVE_CREATE_IMAGE_S(&pstGmmLk->stGMM.stNCCSrc4, IVE_IMAGE_TYPE_U8C1, u16Width/2, u16Height/2);
    if (s32Ret != HI_SUCCESS)
    {
       printf("Mmz Alloc stNCCSrc fail,Error(%#x)\n", s32Ret);
       return HISI_IVE_FAIL;
    }
    s32Ret = IVE_CREATE_IMAGE_S(&pstGmmLk->stGMM.stNCCDst4, IVE_IMAGE_TYPE_U8C1, u16Width/2, u16Height/2);
    if (s32Ret != HI_SUCCESS)
    {
       printf("Mmz Alloc stNCCDst fail,Error(%#x)\n", s32Ret);
       return HISI_IVE_FAIL;
    }*/

    s32Ret = IVE_CREATE_IMAGE_S(&pstGmmLk->stGMM.stPolySrc, IVE_IMAGE_TYPE_U8C1, u16Width, u16Height);
    if (s32Ret != HI_SUCCESS)
    {
       printf("Mmz Alloc stNCCSrc fail,Error(%#x)\n", s32Ret);
       return HISI_IVE_FAIL;
    }
    s32Ret = IVE_CREATE_IMAGE_S(&pstGmmLk->stGMM.stPolyDst, IVE_IMAGE_TYPE_U8C1, u16Width, u16Height);
    if (s32Ret != HI_SUCCESS)
    {
       printf("Mmz Alloc stNCCDst fail,Error(%#x)\n", s32Ret);
       return HISI_IVE_FAIL;
    }

    u32Size = sizeof(IVE_NCC_DST_MEM_S);
    pstGmmLk->stLK.stNCCRDst.u32Size = u32Size;

#ifdef _XM_IA_lINUX_
	s32Ret = HI_MPI_SYS_MmzAlloc(&pstGmmLk->stLK.stNCCRDst.u32PhyAddr, (void**)&pstGmmLk->stLK.stNCCRDst.pu8VirAddr, NULL, HI_NULL, u32Size);
#endif
#ifdef _XM_IA_WINDOWS_
    pstGmmLk->stLK.stNCCRDst.pu8VirAddr = (HI_U8*)malloc(u32Size);
    pstGmmLk->stLK.stNCCRDst.u32PhyAddr = (HI_U32)pstGmmLk->stLK.stNCCRDst.pu8VirAddr;
    if(pstGmmLk->stLK.stNCCRDst.pu8VirAddr == NULL)
    {
        printf("pstGmmLk->stLK.stNCCRDst.pu8VirAddr Failed\n");
    }
#endif


    if (s32Ret != HI_SUCCESS)
    {
	    printf("Mmz Alloc pstStLk->stNCCDst fail,Error(%#x)\n", s32Ret);
	    return HISI_IVE_FAIL;
    }
    memset(pstGmmLk->stLK.stNCCRDst.pu8VirAddr, 0, u32Size);

    pstGmmLk->stLK.stNCCRObjDst.u32Size = u32Size;

#ifdef _XM_IA_lINUX_
	s32Ret = HI_MPI_SYS_MmzAlloc(&pstGmmLk->stLK.stNCCRObjDst.u32PhyAddr, (void**)&pstGmmLk->stLK.stNCCRObjDst.pu8VirAddr, NULL, HI_NULL, u32Size);
#endif
#ifdef _XM_IA_WINDOWS_
    pstGmmLk->stLK.stNCCRObjDst.pu8VirAddr = (HI_U8*)malloc(u32Size);
    pstGmmLk->stLK.stNCCRObjDst.u32PhyAddr = (HI_U32)pstGmmLk->stLK.stNCCRObjDst.pu8VirAddr;
#endif


    if (s32Ret != HI_SUCCESS)
    {
	    printf("Mmz Alloc pstStLk->stNCCRObjDst fail,Error(%#x)\n", s32Ret);
	    return HISI_IVE_FAIL;
    }
    memset(pstGmmLk->stLK.stNCCRObjDst.pu8VirAddr, 0, u32Size);


    pstGmmLk->stLK.stStCandiCornerCtrl.u0q8QualityLevel = CORNER_INIT_LEVEL;

    u32Size = 4 * (u16Width + (IVE_ALIGN - u16Width%IVE_ALIGN) % IVE_ALIGN) * u16Height + sizeof(IVE_ST_MAX_EIG_S);

#ifdef _XM_IA_lINUX_
    s32Ret = HI_MPI_SYS_MmzAlloc(&pstGmmLk->stLK.stStCandiCornerCtrl.stMem.u32PhyAddr, (void**)&pstGmmLk->stLK.stStCandiCornerCtrl.stMem.pu8VirAddr, NULL, HI_NULL, u32Size);
#endif
#ifdef _XM_IA_WINDOWS_
    //pstGmmLk->stLK.stStCandiCornerCtrl.stMem.u32PhyAddr = (HI_U32*)malloc(u32Size);;
    pstGmmLk->stLK.stStCandiCornerCtrl.stMem.pu8VirAddr = (HI_U8*)malloc(u32Size);
    pstGmmLk->stLK.stStCandiCornerCtrl.stMem.u32PhyAddr = (HI_U32)pstGmmLk->stLK.stStCandiCornerCtrl.stMem.pu8VirAddr;
#endif

    pstGmmLk->stLK.stStCandiCornerCtrl.stMem.u32Size = u32Size;
    if (s32Ret != HI_SUCCESS)
    {
        printf("Mmz Alloc stStCandiCornerCtrl.stMem fail,Error(%#x)\n", s32Ret);
        return HISI_IVE_FAIL;
    }
    memset(pstGmmLk->stLK.stStCandiCornerCtrl.stMem.pu8VirAddr, 0, u32Size);
#ifdef _XM_IA_MEM_
    size += u32Size;
#endif

    pstGmmLk->stLK.stStCornerCtrl.u16MaxCornerNum = MAX_POINT_NUM;
    pstGmmLk->stLK.stStCornerCtrl.u16MinDist = MIN_DIST;

    u32Size = sizeof(IVE_ST_CORNER_INFO_S);

#ifdef _XM_IA_lINUX_
    s32Ret = HI_MPI_SYS_MmzAlloc(&pstGmmLk->stLK.stDstCorner.u32PhyAddr, (void**)&pstGmmLk->stLK.stDstCorner.pu8VirAddr, NULL, HI_NULL, u32Size);
#endif
#ifdef _XM_IA_WINDOWS_
    //pstGmmLk->stLK.stDstCorner.u32PhyAddr = (HI_U32*)malloc(u32Size);
    pstGmmLk->stLK.stDstCorner.pu8VirAddr = (HI_U8*)malloc(u32Size);
    pstGmmLk->stLK.stDstCorner.u32PhyAddr = (HI_U32)pstGmmLk->stLK.stDstCorner.pu8VirAddr;
#endif

    pstGmmLk->stLK.stDstCorner.u32Size = u32Size;
    if (s32Ret != HI_SUCCESS)
    {
        printf("Mmz Alloc pstStLk->stDstCorner fail,Error(%#x)\n", s32Ret);
        return HISI_IVE_FAIL;
    }
    memset(pstGmmLk->stLK.stDstCorner.pu8VirAddr, 0, u32Size);
#ifdef _XM_IA_MEM_
    size += u32Size;
#endif
    pstGmmLk->stLK.stLkCtrl.u0q8Epsilon = 2;
    pstGmmLk->stLK.stLkCtrl.u0q8MinEigThr = 70;
    pstGmmLk->stLK.stLkCtrl.u16CornerNum = MAX_POINT_NUM *MAX_OBJ_NUM;
    pstGmmLk->stLK.stLkCtrl.u8IterCount = 5;

    s32Ret = IVE_CREATE_IMAGE_S(&pstGmmLk->stLK.stPyrTmp, IVE_IMAGE_TYPE_U8C1, u16Width, u16Height);
    if (s32Ret != HI_SUCCESS)
    {
        printf("Mmz Alloc LK->stPyrTmp fail,Error(%#x)\n", s32Ret);
        return HISI_IVE_FAIL;
    }

    HI_U16 u16PyrWidth = u16Width;
    HI_U16 u16PyrHeight = u16Height;
    u32Size = sizeof(IVE_POINT_S25Q7_S)* pstGmmLk->stLK.stLkCtrl.u16CornerNum;
	//WWWWWWWW
    for (i = 0; i < 2; i++)
    {
        if (i != 0)
        {
            u16PyrWidth /= 2;
            u16PyrHeight = (u16PyrHeight + 2 - 1) / 2;
        }
        s32Ret = IVE_CREATE_IMAGE_S(&pstGmmLk->stLK.astPrePyr[i], IVE_IMAGE_TYPE_U8C1, u16PyrWidth, u16PyrHeight);
        if (s32Ret != HI_SUCCESS)
        {
            printf("Mmz Alloc LK->astPrePyr fail,Error(%#x)\n", s32Ret);
            return HISI_IVE_FAIL;
        }
        s32Ret = IVE_CREATE_IMAGE_S(&pstGmmLk->stLK.astCurPyr[i], IVE_IMAGE_TYPE_U8C1, u16PyrWidth, u16PyrHeight);
        if (s32Ret != HI_SUCCESS)
        {
            printf("Mmz Alloc LK->astCurPyr fail,Error(%#x)\n", s32Ret);
            return HISI_IVE_FAIL;
        }

#ifdef _XM_IA_lINUX_
        s32Ret = HI_MPI_SYS_MmzAlloc(&pstGmmLk->stLK.astPoint[i].u32PhyAddr, (void**)&pstGmmLk->stLK.astPoint[i].pu8VirAddr, NULL, HI_NULL, u32Size);
#endif
#ifdef _XM_IA_WINDOWS_
        pstGmmLk->stLK.astPoint[i].pu8VirAddr = (HI_U8*)malloc(u32Size);
        pstGmmLk->stLK.astPoint[i].u32PhyAddr = (HI_U32)pstGmmLk->stLK.astPoint[i].pu8VirAddr;
#endif


        pstGmmLk->stLK.astPoint[i].u32Size = u32Size;
        if (s32Ret != HI_SUCCESS)
        {
            printf("Mmz Alloc LK->astPoint fail,Error(%#x)\n", s32Ret);
            return HISI_IVE_FAIL;
        }
        memset(pstGmmLk->stLK.astPoint[i].pu8VirAddr, 0, u32Size);
#ifdef _XM_IA_MEM_
    size += u32Size;
#endif
    }

    u32Size = sizeof(IVE_MV_S9Q7_S)* pstGmmLk->stLK.stLkCtrl.u16CornerNum;

#ifdef _XM_IA_lINUX_
    s32Ret = HI_MPI_SYS_MmzAlloc(&pstGmmLk->stLK.stMv.u32PhyAddr, (void**)&pstGmmLk->stLK.stMv.pu8VirAddr, NULL, HI_NULL, u32Size);
#endif
#ifdef _XM_IA_WINDOWS_
    pstGmmLk->stLK.stMv.pu8VirAddr = (HI_U8*)malloc(u32Size);
    pstGmmLk->stLK.stMv.u32PhyAddr = (HI_U32)pstGmmLk->stLK.stMv.pu8VirAddr;
#endif


    pstGmmLk->stLK.stMv.u32Size = u32Size;
    if (s32Ret != HI_SUCCESS)
    {
        printf("Mmz Alloc LK->stMv fail,Error(%#x)\n", s32Ret);
        return HISI_IVE_FAIL;
    }
    memset(pstGmmLk->stLK.stMv.pu8VirAddr, 0, pstGmmLk->stLK.stMv.u32Size);
#ifdef _XM_IA_MEM_
    size += u32Size;
#endif

    pstGmmLk->stLK.pstCornerInfo = (IVE_ST_CORNER_INFO_S *)pstGmmLk->stLK.stDstCorner.pu8VirAddr;
    pstGmmLk->stLK.pstPoint[0] = (IVE_POINT_S25Q7_S *)pstGmmLk->stLK.astPoint[0].pu8VirAddr;
    pstGmmLk->stLK.pstPoint[1] = (IVE_POINT_S25Q7_S *)pstGmmLk->stLK.astPoint[1].pu8VirAddr;
	//*************wwww
    //pstGmmLk->stLK.pstPoint[2] = (IVE_POINT_S25Q7_S *)pstGmmLk->stLK.astPoint[2].pu8VirAddr;
    pstGmmLk->stLK.pstMv = (IVE_MV_S9Q7_S *)pstGmmLk->stLK.stMv.pu8VirAddr;

    return s32Ret;

}


HI_S32 SAMPLE_IVE_PyrDown(IVE_ST_LK_S *pstStLk, IVE_SRC_IMAGE_S *pstSrc, IVE_DST_IMAGE_S *pstDst)
{
    HI_S32 s32Ret = HI_SUCCESS;
    IVE_HANDLE hIveHandle;
    HI_BOOL bInstant = HI_TRUE;
    IVE_SRC_DATA_S  stDataSrc;
    IVE_DST_DATA_S  stDataDst;
    IVE_DMA_CTRL_S  stDmaCtrl = { IVE_DMA_MODE_INTERVAL_COPY,
        0, 2, 1, 2 };
    //DMA间隔拷贝实现图像缩放


    pstStLk->stPyrTmp.u16Width = pstSrc->u16Width;
    pstStLk->stPyrTmp.u16Height = pstSrc->u16Height;
#if 0
    IVE_FILTER_CTRL_S stFltCtrl = { { 1, 2, 3, 2, 1,
        2, 5, 6, 5, 2,
        3, 6, 8, 6, 3,
        2, 5, 6, 5, 2,
        1, 2, 3, 2, 1 }, 7 };
    //高斯模板,直接缩小图像太尖锐，在缩放之前模糊化一下

    s32Ret = HI_MPI_IVE_Filter(&hIveHandle, pstSrc, &pstStLk->stPyrTmp, &stFltCtrl, bInstant);
    //做了一次过滤得到stPyrTmp，存在LK结构体里
    if (s32Ret != HI_SUCCESS)
    {
        printf("HI_MPI_IVE_Filter fail,Error(%#x)\n", s32Ret);
        return HISI_IVE_FAIL;
    }
#endif
    stDataSrc.pu8VirAddr = pstSrc->pu8VirAddr[0];
    stDataSrc.u32PhyAddr = pstSrc->u32PhyAddr[0];
    stDataSrc.u16Stride = pstSrc->u16Stride[0];
    stDataSrc.u16Width = pstSrc->u16Width;
    stDataSrc.u16Height = pstSrc->u16Height;

    stDataDst.pu8VirAddr = pstDst->pu8VirAddr[0];
    stDataDst.u32PhyAddr = pstDst->u32PhyAddr[0];
    stDataDst.u16Stride = pstDst->u16Stride[0];
    stDataDst.u16Width = pstDst->u16Width;
    stDataDst.u16Height = pstDst->u16Height;                //

#ifdef _XM_IA_lINUX_
    HI_U64 pu64ResizeStartPts = 0, pu64ResizeEndPts = 0;
    HI_MPI_SYS_GetCurPts(&pu64ResizeStartPts);
#endif

    s32Ret = HI_MPI_IVE_DMA(&hIveHandle, &stDataSrc, &stDataDst, &stDmaCtrl, bInstant);

#ifdef _XM_IA_lINUX_
    HI_MPI_SYS_GetCurPts(&pu64ResizeEndPts);
#endif
    //printf("IVE_PyrDown %lld\n", pu64ResizeEndPts - pu64ResizeStartPts);

    //上面进行了地址复制，实际上是将pstStLk->stPyrTmp的值复制给了pstDst
    if (s32Ret != HI_SUCCESS)
    {
        printf("HI_MPI_IVE_DMA fail,Error(%#x)", s32Ret);
        return HISI_IVE_FAIL;
    }
#if 0
    HI_BOOL bBlock = HI_TRUE;
    HI_BOOL bFinish = HI_FALSE;

    s32Ret = HI_MPI_IVE_Query(hIveHandle, &bFinish, bBlock);
    while (HI_ERR_IVE_QUERY_TIMEOUT == s32Ret)
    {
        usleep(200);
        s32Ret = HI_MPI_IVE_Query(hIveHandle, &bFinish, bBlock);
    }
    if (s32Ret != HI_SUCCESS)
    {
        printf("HI_MPI_IVE_Query fail,Error(%#x)\n", s32Ret);
        return HISI_IVE_FAIL;
    }
#endif

    return s32Ret;
}

HI_S32 SAMPLE_IVE_CopyPyr222(IVE_IMAGE_S *pstPyrSrc, IVE_IMAGE_S *pstPyrDst, HI_U32 u32Level)
{
    HI_U32 i;
    IVE_HANDLE hIveHandle;
    HI_BOOL bInstant = HI_FALSE;
    IVE_DMA_CTRL_S stDmaCtrl;
    IVE_DATA_S stSrcData;
    IVE_DST_DATA_S stDstData;
    HI_S32 s32Ret = HI_SUCCESS;

    for (i = 0; i < u32Level; i++)
    {
        stSrcData.pu8VirAddr = pstPyrSrc[i].pu8VirAddr[0];
        stSrcData.u32PhyAddr = pstPyrSrc[i].u32PhyAddr[0];
        stSrcData.u16Height = pstPyrSrc[i].u16Height;
        stSrcData.u16Width = pstPyrSrc[i].u16Width;
        stSrcData.u16Stride = pstPyrSrc[i].u16Stride[0];

        stDstData.pu8VirAddr = pstPyrDst[i].pu8VirAddr[0];
        stDstData.u32PhyAddr = pstPyrDst[i].u32PhyAddr[0];
        stDstData.u16Height = pstPyrDst[i].u16Height;
        stDstData.u16Width = pstPyrDst[i].u16Width;
        stDstData.u16Stride = pstPyrDst[i].u16Stride[0];

        stDmaCtrl.enMode = IVE_DMA_MODE_DIRECT_COPY;

#ifdef _XM_IA_lINUX_
 //       HI_MPI_SYS_MmzFlushCache(stSrcData.u32PhyAddr, stSrcData.pu8VirAddr, stSrcData.u16Height*stSrcData.u16Stride);
//        HI_MPI_SYS_MmzFlushCache(stDstData.u32PhyAddr, stDstData.pu8VirAddr, stDstData.u16Height*stDstData.u16Stride);
#endif

        s32Ret = HI_MPI_IVE_DMA(&hIveHandle, &stSrcData, &stDstData, &stDmaCtrl, bInstant);
        if (s32Ret != HI_SUCCESS)
        {
            printf("FUNC %s dma fail,Error(%#x)", __FUNCTION__, s32Ret);
            return HISI_IVE_FAIL;
        }
    }

    return s32Ret;
}

void ConstructFileName(char* Corner_Output_Filepath, int output_filenum, char* filetype)
{
    char num[10];
    sprintf(num, "%d", output_filenum);
    /*组合成文件路径和文件名*/
    Corner_Output_Filepath = strcat(strcat(Corner_Output_Filepath, num), filetype);
}

HI_S32 CclCorner(IVE_HANDLE* IveHandle, XM_INIT_PARA* pstInitPara)
{
    int j, k;
    int TotalCorner = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_BOOL bInstant = HI_FALSE;
    IVE_DMA_CTRL_S stDmaCtrl;
    IVE_DATA_S stSrcData;
    IVE_DST_DATA_S stDstData;
    IVE_ST_CORNER_INFO_S *pstCornerInfo = (IVE_ST_CORNER_INFO_S *)pstInitPara->pstGmmLk->stLK.stDstCorner.pu8VirAddr;
#ifdef _XM_IA_lINUX_
   HI_BOOL bBlock = HI_TRUE;
   HI_BOOL bFinish = HI_FALSE;
    HI_U64 pu64CandiStartPts = 0, pu64CandiEndPts = 0;
    HI_U64 pu64CornerStartPts = 0, pu64CornerEndPts = 0;
#endif
#ifdef _XM_IA_DEBUG_
    char corner_outpath[100];
    strcpy(corner_outpath, "../../../all/output/Corner_out/480x320/");
    ConstructFileName(corner_outpath, framenum, ".txt");
    FILE* Corner_fwp = NULL;
    Corner_fwp = fopen(corner_outpath, "a+");
#endif

    if ( pstInitPara->pstGmmLk->stGMM.stRegion.u16Num > 0)
    {
        for (j = 0; j < pstInitPara->pstGmmLk->stGMM.stRegion.u16Num; j++)//每一个CCL目标
        {
            if (pstInitPara->pstGmmLk->stGMM.stRegion.astRect[j].u32Area > pstInitPara->iObjMinArea &&
                pstInitPara->pstGmmLk->stGMM.stRegion.astRect[j].u32Area < pstInitPara->iObjMaxArea)
            {
                if (0 == pstInitPara->DoNewCorner[j])
                {
                    if (pstInitPara->pstGmmLk->stGMM.stRegion.astRect[j].astPoint[0].s32X >= ((HI_S32)pstInitPara->pstGmmLk->stGMM.stSrc.u16Width - 64))
                    {
                        pstInitPara->pstGmmLk->stGMM.stRegion.astRect[j].astPoint[0].s32X = pstInitPara->pstGmmLk->stGMM.stSrc.u16Width - 65;
                        pstInitPara->pstGmmLk->stGMM.stRegion.astRect[j].astPoint[2].s32X = pstInitPara->pstGmmLk->stGMM.stSrc.u16Width - 1;
                    }
                    if (pstInitPara->pstGmmLk->stGMM.stRegion.astRect[j].astPoint[0].s32Y >= ((HI_S32)pstInitPara->pstGmmLk->stGMM.stSrc.u16Height - 64))
                    {
                        pstInitPara->pstGmmLk->stGMM.stRegion.astRect[j].astPoint[0].s32Y = pstInitPara->pstGmmLk->stGMM.stSrc.u16Height - 65;
                        pstInitPara->pstGmmLk->stGMM.stRegion.astRect[j].astPoint[2].s32Y = pstInitPara->pstGmmLk->stGMM.stSrc.u16Height - 1;
                    }

                    int cclwidth = pstInitPara->pstGmmLk->stGMM.stRegion.astRect[j].astPoint[2].s32X -
                        pstInitPara->pstGmmLk->stGMM.stRegion.astRect[j].astPoint[0].s32X;
                    int cclheight = (pstInitPara->pstGmmLk->stGMM.stRegion.astRect[j].astPoint[2].s32Y -
                        pstInitPara->pstGmmLk->stGMM.stRegion.astRect[j].astPoint[0].s32Y) & 0x7ffffffe;

                    //stCorner支持的最小宽度
                    if (cclwidth < 64)
                    {
                        cclwidth = 64;
                    }
                    if (cclheight < 64)
                    {
                        cclheight = 64;
                    }

                    stSrcData.pu8VirAddr = pstInitPara->pstGmmLk->stGMM.stSrc.pu8VirAddr[0] +
                        pstInitPara->pstGmmLk->stGMM.stRegion.astRect[j].astPoint[0].s32Y * pstInitPara->pstGmmLk->stGMM.stSrc.u16Stride[0] +
                        pstInitPara->pstGmmLk->stGMM.stRegion.astRect[j].astPoint[0].s32X;
                    stSrcData.u32PhyAddr = pstInitPara->pstGmmLk->stGMM.stSrc.u32PhyAddr[0] +
                        pstInitPara->pstGmmLk->stGMM.stRegion.astRect[j].astPoint[0].s32Y * pstInitPara->pstGmmLk->stGMM.stSrc.u16Stride[0] +
                        pstInitPara->pstGmmLk->stGMM.stRegion.astRect[j].astPoint[0].s32X;
                    stSrcData.u16Height = cclheight;
                    stSrcData.u16Width = cclwidth;
                    stSrcData.u16Stride = XM_IA_IVE_CalcStride(cclwidth, IVE_ALIGN);

                    stDstData.pu8VirAddr = pstInitPara->pstGmmLk->stLK.stSrc.pu8VirAddr[0];
                    stDstData.u32PhyAddr = pstInitPara->pstGmmLk->stLK.stSrc.u32PhyAddr[0];
                    stDstData.u16Height = cclheight;
                    stDstData.u16Width = cclwidth;
                    stDstData.u16Stride = XM_IA_IVE_CalcStride(stSrcData.u16Width, IVE_ALIGN);

                    pstInitPara->pstGmmLk->stLK.stSrc.u16Height = stSrcData.u16Height;
                    pstInitPara->pstGmmLk->stLK.stSrc.u16Width = stSrcData.u16Width;
                    pstInitPara->pstGmmLk->stLK.stSrc.u16Stride[0] = stSrcData.u16Stride;

                    pstInitPara->pstGmmLk->stLK.stDst.u16Height = stSrcData.u16Height;
                    pstInitPara->pstGmmLk->stLK.stDst.u16Width = stSrcData.u16Width;
                    pstInitPara->pstGmmLk->stLK.stDst.u16Stride[0] = stDstData.u16Stride;

#ifdef _XM_IA_lINUX_
                    HI_MPI_SYS_GetCurPts(&pu64CandiStartPts);
#endif

                    stDmaCtrl.enMode = IVE_DMA_MODE_DIRECT_COPY;
#ifdef _XM_IA_lINUX_
 //                   HI_MPI_SYS_MmzFlushCache(stSrcData.u32PhyAddr, stSrcData.pu8VirAddr, stSrcData.u16Height * stSrcData.u16Stride);
 //                   HI_MPI_SYS_MmzFlushCache(stDstData.u32PhyAddr, stDstData.pu8VirAddr, stDstData.u16Height * stDstData.u16Stride);
#endif
                    s32Ret = HI_MPI_IVE_DMA(IveHandle, &stSrcData, &stDstData, &stDmaCtrl, bInstant);
                    if (s32Ret != HI_SUCCESS)
                    {
                        printf("FUNC %s Line %d fail,Error(%#x)\n", __FUNCTION__, __LINE__, s32Ret);
                        return HISI_IVE_FAIL;
                    }

                    s32Ret = HI_MPI_IVE_STCandiCorner(IveHandle, &pstInitPara->pstGmmLk->stLK.stSrc,
                        &pstInitPara->pstGmmLk->stLK.stDst,
                        &pstInitPara->pstGmmLk->stLK.stStCandiCornerCtrl, bInstant);
                    if (s32Ret != HI_SUCCESS)
                    {
                        printf("FUNC %s Line %d fail,Error(%#x)\n", __FUNCTION__, __LINE__, s32Ret);
                        return HISI_IVE_FAIL;
                    }

#if 0
                    s32Ret = HI_MPI_IVE_Query(IveHandle, &bFinish, bBlock);
                    while (HI_ERR_IVE_QUERY_TIMEOUT == s32Ret)
                    {
                        usleep(100);
                        s32Ret = HI_MPI_IVE_Query(IveHandle, &bFinish, bBlock);
                    }
                    if (s32Ret != HI_SUCCESS)
                    {
                        printf("HI_MPI_IVE_Query fail,Error(%#x)\n", s32Ret);
                        return HISI_IVE_FAIL;
                    }
#endif

#ifdef _XM_IA_lINUX_
                    HI_MPI_SYS_GetCurPts(&pu64CandiEndPts);

                    HI_MPI_SYS_GetCurPts(&pu64CornerStartPts);
#endif

                    s32Ret = HI_MPI_IVE_STCorner(&pstInitPara->pstGmmLk->stLK.stDst, &pstInitPara->pstGmmLk->stLK.stDstCorner,
                        &pstInitPara->pstGmmLk->stLK.stStCornerCtrl);
                    if (s32Ret != HI_SUCCESS)
                    {
                        printf("HI_MPI_IVE_STCorner fail,Error(%#x)\n", s32Ret);
                        return HISI_IVE_FAIL;
                    }
#ifdef _XM_IA_lINUX_
                    s32Ret = HI_MPI_IVE_Query(*IveHandle, &bFinish, bBlock);
                    while (HI_ERR_IVE_QUERY_TIMEOUT == s32Ret)
                    {

                        usleep(100);
                        s32Ret = HI_MPI_IVE_Query(*IveHandle, &bFinish, bBlock);
                    }
                    if (s32Ret != HI_SUCCESS)
                    {
                        printf("HI_MPI_IVE_Query fail,Error(%#x)\n", s32Ret);
                        return HISI_IVE_FAIL;
                    }
#endif
#ifdef _XM_IA_lINUX_
                    HI_MPI_SYS_GetCurPts(&pu64CornerEndPts);
#endif
                    //printf("CornerIndex %d candi %lld corner %lld\n", j, pu64CandiEndPts - pu64CandiStartPts, pu64CornerEndPts - pu64CornerStartPts);

                    pstInitPara->pstGmmLk->stLK.stLkCtrl.u16CornerNum = pstCornerInfo->u16CornerNum;
#ifdef _XM_IA_PRINT_
                    printf("u16CornerNum: %d TotalCorner : %d \n", pstInitPara->pstGmmLk->stLK.stLkCtrl.u16CornerNum, TotalCorner);
#endif

                    int count = 0;
                    for (k = 0; k < pstInitPara->pstGmmLk->stLK.stLkCtrl.u16CornerNum; k++)
                    {
                        HI_S32 X = (HI_S32)(pstCornerInfo->astCorner[k].u16X + pstInitPara->pstGmmLk->stGMM.stRegion.astRect[j].astPoint[0].s32X);
                        HI_S32 Y = (HI_S32)(pstCornerInfo->astCorner[k].u16Y + pstInitPara->pstGmmLk->stGMM.stRegion.astRect[j].astPoint[0].s32Y);

                        //判断找到的角点是不是在源CCL区域里（有可能会落在扩充的CCL背景区域里）
                        if (X >= pstInitPara->pstGmmLk->stGMM.stRegion.astRect[j].astPoint[0].s32X &&
                            X <= pstInitPara->pstGmmLk->stGMM.stRegion.astRect[j].astPoint[2].s32X &&
                            Y >= pstInitPara->pstGmmLk->stGMM.stRegion.astRect[j].astPoint[0].s32Y &&
                            Y <= pstInitPara->pstGmmLk->stGMM.stRegion.astRect[j].astPoint[2].s32Y)
                        {
                            pstInitPara->pstGmmLk->stLK.astPointTmp[TotalCorner + count].s25q7X = (HI_S32)((pstCornerInfo->astCorner[k].u16X +
                                pstInitPara->pstGmmLk->stGMM.stRegion.astRect[j].astPoint[0].s32X) << 7);
                            pstInitPara->pstGmmLk->stLK.astPointTmp[TotalCorner + count].s25q7Y = (HI_S32)((pstCornerInfo->astCorner[k].u16Y +
                                pstInitPara->pstGmmLk->stGMM.stRegion.astRect[j].astPoint[0].s32Y) << 7);
                            //printf("index %d  X: %d Y: %d ", TotalCorner + count, X, Y);
#ifdef _XM_IA_DEBUG_
                            fprintf(Corner_fwp, "%d", (HI_S32)(pstCornerInfo->astCorner[k].u16X + pstInitPara->pstGmmLk->stGMM.stRegion.astRect[j].astPoint[0].s32X));//X坐标
                            fprintf(Corner_fwp, "%s", " ");//插入空格
                            fprintf(Corner_fwp, "%d", (HI_S32)(pstCornerInfo->astCorner[k].u16Y + pstInitPara->pstGmmLk->stGMM.stRegion.astRect[j].astPoint[0].s32Y));//Y坐标
                            fprintf(Corner_fwp, "%s", " ");//插入空格
#endif
                            count++;
                        }
                    }

#ifdef _XM_IA_PRINT_
                    printf("Valid CornerNum: %d\n", count);
#endif
                    TotalCorner += count;
                }
            }
        }
#ifdef _XM_IA_DEBUG_
        if(Corner_fwp!=NULL)
        {
            fclose(Corner_fwp);
            Corner_fwp = NULL;
        }
#endif

    }
    return TotalCorner;
}

HI_S32 PointBelongCCL(IVE_POINT_S25Q7_S point, RECT_ARRAY_S stRegion)
{
    int k = 0,r = 100;
    for (k = 0; k < stRegion.u16Num; k++)
    {
        if (((point.s25q7X >> 7) >= (stRegion.astRect[k].astPoint[0].s32X - 5)) &&
            ((point.s25q7X >> 7) <= (stRegion.astRect[k].astPoint[2].s32X + 5)) &&
            ((point.s25q7Y >> 7) >= stRegion.astRect[k].astPoint[0].s32Y) &&
            ((point.s25q7Y >> 7) <= (stRegion.astRect[k].astPoint[2].s32Y)))
        {
            //printf(" PX: %d  PY: %d ", (point.s25q7X >> 7), (point.s25q7Y >> 7));
            //printf("left: %d  right: %d ", stRegion.astRect[k].astPoint[0].s32X, stRegion.astRect[k].astPoint[2].s32X);
            //printf("top: %d  bottom: %d ", stRegion.astRect[k].astPoint[0].s32Y, stRegion.astRect[k].astPoint[2].s32Y);
            r = k;
            break;
        }
    }
    return r;
}


HI_S32 LK_Pre_Proc(IVE_GMM_LK_S *pstGmmLk, int TotalCorner)
{
    //LK预处理
    HI_S32 s32Ret = HI_SUCCESS;
    HI_BOOL bInstant = HI_FALSE;
    IVE_HANDLE hIveHandle;
    IVE_DMA_CTRL_S stDmaCtrl;
    IVE_DATA_S stSrcData;
    IVE_DST_DATA_S stDstData;

    int k, j;
    if (TotalCorner <= MAX_POINT_NUM * MAX_OBJ_NUM)//只能有90个点
    {
        pstGmmLk->stLK.stLkCtrl.u16CornerNum = TotalCorner;
    }
    else
    {
        TotalCorner = MAX_POINT_NUM * MAX_OBJ_NUM;
        pstGmmLk->stLK.stLkCtrl.u16CornerNum = TotalCorner;
    }
    for (k = 0; k < TotalCorner; k++)
    {
		//MMMMMMMMMMM0223
		//for (j = 0; j < 3; j++)
	    for (j = 0; j < 2; j++)
        {
            if (j == 0)
            {
                pstGmmLk->stLK.pstPoint[0][k].s25q7X = pstGmmLk->stLK.astPointTmp[k].s25q7X;
                pstGmmLk->stLK.pstPoint[0][k].s25q7Y = pstGmmLk->stLK.astPointTmp[k].s25q7Y;
            }
            else
            {
                pstGmmLk->stLK.pstPoint[j][k].s25q7X = pstGmmLk->stLK.pstPoint[j - 1][k].s25q7X / 2;
                pstGmmLk->stLK.pstPoint[j][k].s25q7Y = pstGmmLk->stLK.pstPoint[j - 1][k].s25q7Y / 2;
            }
        }
    }
	//MMMMMMMMMMM0223
	//for (j = 0; j < 3; j++)
	for (j = 0; j < 2; j++)
    {
        if (j == 0)
        {
            //HI_MPI_SYS_GetCurPts(&pu64ResizeStartPts);
            //memcpy(pstGmmLk->stLK.astCurPyr[j].pu8VirAddr[0], pstGmmLk->stGMM.stSrc.pu8VirAddr[0],
            //pstGmmLk->stGMM.stSrc.u16Stride[0] * pstGmmLk->stGMM.stSrc.u16Height);//读入源图像存为当前帧的第0层
            //HI_MPI_SYS_GetCurPts(&pu64ResizeEndPts);
            //printf("LK copy %lld\n", pu64ResizeEndPts - pu64ResizeStartPts);
#ifdef _XM_IA_lINUX_
            HI_U64 pu64ResizeStartPts = 0, pu64ResizeEndPts = 0;
            HI_MPI_SYS_GetCurPts(&pu64ResizeStartPts);
#endif

            stSrcData.pu8VirAddr = pstGmmLk->stGMM.stSrc.pu8VirAddr[0];
            stSrcData.u32PhyAddr = pstGmmLk->stGMM.stSrc.u32PhyAddr[0];
            stSrcData.u16Height = pstGmmLk->stGMM.stSrc.u16Height;
            stSrcData.u16Width = pstGmmLk->stGMM.stSrc.u16Width;
            stSrcData.u16Stride = pstGmmLk->stGMM.stSrc.u16Stride[0];

            stDstData.pu8VirAddr = pstGmmLk->stLK.astCurPyr[j].pu8VirAddr[0];
            stDstData.u32PhyAddr = pstGmmLk->stLK.astCurPyr[j].u32PhyAddr[0];
            stDstData.u16Height = pstGmmLk->stLK.astCurPyr[j].u16Height;
            stDstData.u16Width = pstGmmLk->stLK.astCurPyr[j].u16Width;
            stDstData.u16Stride = pstGmmLk->stLK.astCurPyr[j].u16Stride[0];

            stDmaCtrl.enMode = IVE_DMA_MODE_DIRECT_COPY;

#ifdef _XM_IA_lINUX_
//            HI_MPI_SYS_MmzFlushCache(stSrcData.u32PhyAddr, stSrcData.pu8VirAddr, stSrcData.u16Height*stSrcData.u16Stride);
//            HI_MPI_SYS_MmzFlushCache(stDstData.u32PhyAddr, stDstData.pu8VirAddr, stDstData.u16Height*stDstData.u16Stride);
#endif

            s32Ret = HI_MPI_IVE_DMA(&hIveHandle, &stSrcData, &stDstData, &stDmaCtrl, bInstant);
            if (s32Ret != HI_SUCCESS)
            {
                printf("FUNC %s dma fail,Error(%#x)", __FUNCTION__, s32Ret);
                return HISI_IVE_FAIL;
            }
#ifdef _XM_IA_lINUX_
            HI_MPI_SYS_GetCurPts(&pu64ResizeEndPts);
#endif
            //printf("DMA copy %lld\n", pu64ResizeEndPts - pu64ResizeStartPts);
        }
        else
        {
            s32Ret = SAMPLE_IVE_PyrDown(&pstGmmLk->stLK, &pstGmmLk->stLK.astCurPyr[j - 1], &pstGmmLk->stLK.astCurPyr[j]);
            if (s32Ret != HI_SUCCESS)
            {
                printf("SAMPLE_IVE_PyrDown fail\n");
                return HISI_IVE_FAIL;
            }
        }
    }
    return s32Ret;
}

#if 0
HI_S32 LK_Follow_Proc(IVE_GMM_LK_S *pstGmmLk, int TotalCorner)
{
    int j;
    HI_BOOL bInstant = HI_FALSE;
    HI_S32 s32Ret = HI_SUCCESS;
    IVE_HANDLE IveHandle = 0;

    if (TotalCorner > 0)//LK跟踪部分
    {

#ifdef _XM_IA_lINUX_
    HI_BOOL bBlock = HI_TRUE;
    HI_BOOL bFinish = HI_FALSE;
    HI_U64 pu64iveLKStartPts = 0, pu64iveLKEndPts = 0;
    HI_MPI_SYS_GetCurPts(&pu64iveLKStartPts);
#endif

		//MMMMMMMMMMM0223
		//for (j = 0; j < 3; j++)
		for (j = 1; j >= 0; j--)
        {
            IVE_IMAGE_S stPreImg = pstGmmLk->stLK.astPrePyr[j];
            IVE_IMAGE_S stCurImg = pstGmmLk->stLK.astCurPyr[j];

            stPreImg.u16Height = (stPreImg.u16Height >> 1) << 1;
            stCurImg.u16Height = (stCurImg.u16Height >> 1) << 1;

            s32Ret = HI_MPI_IVE_LKOpticalFlow(&IveHandle, &stPreImg, &stCurImg, &pstGmmLk->stLK.astPoint[j],
                &pstGmmLk->stLK.stMv, &pstGmmLk->stLK.stLkCtrl, bInstant);
            if (s32Ret != HI_SUCCESS)
            {
                printf("HI_MPI_IVE_LKOpticalFlow fail,Error(%#x)\n", s32Ret);
                return HISI_IVE_FAIL;
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
                printf("HI_MPI_IVE_Query fail,Error(%#x)\n", s32Ret);
                return HISI_IVE_FAIL;
            }
#endif
#ifdef _XM_IA_lINUX_
            HI_MPI_SYS_GetCurPts(&pu64iveLKEndPts);
#endif

        //  printf("IVE LK: %lld\n", pu64iveLKEndPts - pu64iveLKStartPts);

        }

    }
    return s32Ret;
}
#endif

HI_VOID ObjectInit(Struct_Object* Object)
{
    memset(Object, 0, sizeof(Struct_Object));

    Object->CCLRegionNum = 100;
}

HI_VOID Corner_CCL_Distribute(IVE_GMM_LK_S *pstGmmLk, Frame_Object *Cur_Frame_Obj ,XM_INIT_PARA *pstInitPara,int* TotalCorner, int* TotalObjNum,
            IVE_POINT_S25Q7_S LKPoint[],IVE_POINT_S25Q7_S TempPoint[], IVE_MV_S9Q7_S TempMv[])
{
    int count;
    int j, k, s, u, r;
    int CurrentCCL[MAX_OBJ_NUM] = { 0, 0, 0, 0, 0, 0 };//当前CCL是否已被归为某目标

    memset(LKPoint, 0, sizeof(IVE_POINT_S25Q7_S)*MAX_OBJ_NUM * MAX_POINT_NUM);

    memset(CurrentCCL, 0, sizeof(int)* MAX_OBJ_NUM);
    for (k = (0+ pstInitPara->PolyGon); k < MAX_OBJ_NUM; k++)//对当前OBJ遍历
    {
        Cur_Frame_Obj->Object[k].PointInfo.PointNum = 0;
        if (Cur_Frame_Obj->Object[k].center.u16X != 0 && Cur_Frame_Obj->Object[k].center.u16Y != 0)//先用中心点判断OBJ的CCL区域
        {
            IVE_POINT_S25Q7_S Ptcenter;
            Ptcenter.s25q7X = Cur_Frame_Obj->Object[k].center.u16X << 7;
            Ptcenter.s25q7Y = Cur_Frame_Obj->Object[k].center.u16Y << 7;
            u = PointBelongCCL(Ptcenter, pstGmmLk->stGMM.stRegion);//做点与CCL判断时，需要将坐标左移7位

            Cur_Frame_Obj->Object[k].CCLRegionNum = u;

#ifdef _XM_IA_PRINT_
            printf(" Object[%d].center :%d  %d  result: %d \n", k, Cur_Frame_Obj->Object[k].center.u16X, Cur_Frame_Obj->Object[k].center.u16Y, u);
            printf("CurrentCCL[%d] = %d\n",u,CurrentCCL[u]);
#endif

            if (u != 100)//目标没有丢失,记录区域信息
            {
                if (CurrentCCL[u] == 0)
                {
                    Cur_Frame_Obj->Object[k].CCLRegionArea = pstGmmLk->stGMM.stRegion.astRect[u].u32Area;
                    Cur_Frame_Obj->Object[k].stRect.s16X1 = (IMP_S16)pstGmmLk->stGMM.stRegion.astRect[u].astPoint[0].s32X;
                    Cur_Frame_Obj->Object[k].stRect.s16Y1 = (IMP_S16)pstGmmLk->stGMM.stRegion.astRect[u].astPoint[0].s32Y;
                    Cur_Frame_Obj->Object[k].stRect.s16X2 = (IMP_S16)pstGmmLk->stGMM.stRegion.astRect[u].astPoint[2].s32X;
                    Cur_Frame_Obj->Object[k].stRect.s16Y2 = (IMP_S16)pstGmmLk->stGMM.stRegion.astRect[u].astPoint[2].s32Y;
                    int icenterx = (int)(pstGmmLk->stGMM.stRegion.astRect[u].astPoint[0].s32X + pstGmmLk->stGMM.stRegion.astRect[u].astPoint[2].s32X) / 2;
                    int icentery = (int)(pstGmmLk->stGMM.stRegion.astRect[u].astPoint[0].s32Y + pstGmmLk->stGMM.stRegion.astRect[u].astPoint[2].s32Y) / 2;
                    Cur_Frame_Obj->Object[k].CenterMotion.dx = (int)(icenterx - Cur_Frame_Obj->Object[k].center.u16X);
                    Cur_Frame_Obj->Object[k].CenterMotion.dy = (int)(icentery - Cur_Frame_Obj->Object[k].center.u16Y);
                    Cur_Frame_Obj->Object[k].center.u16X = icenterx;
                    Cur_Frame_Obj->Object[k].center.u16Y = icentery;

                    if (0 == Cur_Frame_Obj->Object[k].Origin_center.u16X)
                    {
                        Cur_Frame_Obj->Object[k].Origin_center.u16X = Cur_Frame_Obj->Object[k].center.u16X;
                    }
                    if (0 == Cur_Frame_Obj->Object[k].Origin_center.u16Y)
                    {
                        Cur_Frame_Obj->Object[k].Origin_center.u16Y = Cur_Frame_Obj->Object[k].center.u16Y;
                    }

                    CurrentCCL[u] = 1;//第u个区域已被分配给目标
                }
                else if (1 == CurrentCCL[u])//如果已经被分配，两个OBJ中心点在同一个CCL区域
                {
                    printf("WW Cur_Frame_Obj->ObjectNum %d\n", Cur_Frame_Obj->ObjectNum);
                    Cur_Frame_Obj->Object[k].CCLRegionNum = u;//先赋值，在判断，不然这个OBJ还存着上一帧的CCL编号
                    for (r = 0; r < k; r++)
                    {
                        if (Cur_Frame_Obj->Object[k].CCLRegionNum == Cur_Frame_Obj->Object[r].CCLRegionNum)
                            //因为都是对整个数组进行遍历，后面还会有排序，可以直接将重合区域的目标置为0
                        {
                            ObjectInit(Cur_Frame_Obj->Object + k);

                            Cur_Frame_Obj->ObjectNum--;
                            if (Cur_Frame_Obj->ObjectNum < 0)
                            {
                                printf("XX Cur_Frame_Obj->ObjectNum %d\n", Cur_Frame_Obj->ObjectNum);
                                Cur_Frame_Obj->ObjectNum = 0;
                            }
                        }
                    }
                }
            }
        }
    }

    memset(TempPoint, 0, sizeof(IVE_POINT_S25Q7_S)* MAX_OBJ_NUM * MAX_POINT_NUM);
    memset(TempMv, 0, sizeof(IVE_MV_S9Q7_S)*MAX_OBJ_NUM * MAX_POINT_NUM);
    count = 0;

    for (j = 0; j < (*TotalCorner); j++)
    {
        pstGmmLk->stLK.astPointTmp[j].s25q7X = pstGmmLk->stLK.pstPoint[0][j].s25q7X + pstGmmLk->stLK.pstMv[j].s9q7Dx;
        pstGmmLk->stLK.astPointTmp[j].s25q7Y = pstGmmLk->stLK.pstPoint[0][j].s25q7Y + pstGmmLk->stLK.pstMv[j].s9q7Dy;

        if (pstGmmLk->stLK.pstMv[j].s32Status == 0)//跟踪成功
        {
#ifdef _XM_IA_PRINT_
            printf(" X: %d  Y: %d ", (pstGmmLk->stLK.pstPoint[0][j].s25q7X + pstGmmLk->stLK.pstMv[j].s9q7Dx) >> 7,
                (pstGmmLk->stLK.pstPoint[0][j].s25q7Y + pstGmmLk->stLK.pstMv[j].s9q7Dy) >> 7);
            printf(" Dx: %d    Dy: %d    ", pstGmmLk->stLK.pstMv[j].s9q7Dx, pstGmmLk->stLK.pstMv[j].s9q7Dy);
#endif

            u = PointBelongCCL(pstGmmLk->stLK.astPointTmp[j], pstGmmLk->stGMM.stRegion);

#ifdef _XM_IA_PRINT_
            //↑当前点是否在此帧的CCL区域内。在返回CCL区域数组下标，不在返回100
            printf("Result: %d  \n", u);
#endif
            if (u != 100) //跟踪到的特征点在本帧CCL前景区域内
            {
                LKPoint[count].s25q7X = pstGmmLk->stLK.astPointTmp[j].s25q7X >> 7;
                LKPoint[count].s25q7Y = pstGmmLk->stLK.astPointTmp[j].s25q7Y >> 7;
                //              printf("CurrentCCL[%d]: %d  ", u, CurrentCCL[u]);

                if (0 == CurrentCCL[u])     //当前CCL未分配给任何目标,新建一个目标
                {
                    CurrentCCL[u] = 1;
                    ++(*TotalObjNum);

                    if (Cur_Frame_Obj->ObjectNum >= MAX_OBJ_NUM)
                    {
                        printf("ObjectNum over %d!!!!\n", MAX_OBJ_NUM);
                        Cur_Frame_Obj->ObjectNum = MAX_OBJ_NUM - 1;
                    }

                    Cur_Frame_Obj->Object[Cur_Frame_Obj->ObjectNum].ObjectID = *TotalObjNum;
                    Cur_Frame_Obj->Object[Cur_Frame_Obj->ObjectNum].PointInfo.PointNum = 0;
                    Cur_Frame_Obj->Object[Cur_Frame_Obj->ObjectNum].PointInfo.PointMotionX[0] = (int)pstGmmLk->stLK.pstMv[j].s9q7Dx;//左移过的
                    Cur_Frame_Obj->Object[Cur_Frame_Obj->ObjectNum].PointInfo.PointMotionY[0] = (int)pstGmmLk->stLK.pstMv[j].s9q7Dy;//左移过的
                    Cur_Frame_Obj->Object[Cur_Frame_Obj->ObjectNum].PointInfo.Point[0].s25q7X = pstGmmLk->stLK.astPointTmp[j].s25q7X; //左移过的
                    Cur_Frame_Obj->Object[Cur_Frame_Obj->ObjectNum].PointInfo.Point[0].s25q7Y = pstGmmLk->stLK.astPointTmp[j].s25q7Y;
                    Cur_Frame_Obj->Object[Cur_Frame_Obj->ObjectNum].PointInfo.PointNum++;
                    Cur_Frame_Obj->Object[Cur_Frame_Obj->ObjectNum].CCLRegionNum = u;
                    Cur_Frame_Obj->Object[Cur_Frame_Obj->ObjectNum].CCLRegionArea = pstGmmLk->stGMM.stRegion.astRect[u].u32Area;
                    Cur_Frame_Obj->Object[Cur_Frame_Obj->ObjectNum].stRect.s16X1 = (IMP_S16)pstGmmLk->stGMM.stRegion.astRect[u].astPoint[0].s32X;
                    Cur_Frame_Obj->Object[Cur_Frame_Obj->ObjectNum].stRect.s16Y1 = (IMP_S16)pstGmmLk->stGMM.stRegion.astRect[u].astPoint[0].s32Y;
                    Cur_Frame_Obj->Object[Cur_Frame_Obj->ObjectNum].stRect.s16X2 = (IMP_S16)pstGmmLk->stGMM.stRegion.astRect[u].astPoint[2].s32X;
                    Cur_Frame_Obj->Object[Cur_Frame_Obj->ObjectNum].stRect.s16Y2 = (IMP_S16)pstGmmLk->stGMM.stRegion.astRect[u].astPoint[2].s32Y;
                    Cur_Frame_Obj->Object[Cur_Frame_Obj->ObjectNum].CenterMotion.dx = 0;
                    Cur_Frame_Obj->Object[Cur_Frame_Obj->ObjectNum].CenterMotion.dy = 0;
                    Cur_Frame_Obj->Object[Cur_Frame_Obj->ObjectNum].center.u16X = (pstGmmLk->stGMM.stRegion.astRect[u].astPoint[0].s32X + pstGmmLk->stGMM.stRegion.astRect[u].astPoint[2].s32X) / 2;
                    Cur_Frame_Obj->Object[Cur_Frame_Obj->ObjectNum].center.u16Y = (pstGmmLk->stGMM.stRegion.astRect[u].astPoint[0].s32Y + pstGmmLk->stGMM.stRegion.astRect[u].astPoint[2].s32Y) / 2;
                    Cur_Frame_Obj->Object[Cur_Frame_Obj->ObjectNum].State = 1;
                    Cur_Frame_Obj->Object[Cur_Frame_Obj->ObjectNum].iTouchLine = 0;
                    Cur_Frame_Obj->Object[Cur_Frame_Obj->ObjectNum].iCount = 0;
                    Cur_Frame_Obj->Object[Cur_Frame_Obj->ObjectNum].iMark = 0;
                    Cur_Frame_Obj->Object[Cur_Frame_Obj->ObjectNum].iPolyMark = 0;
                    Cur_Frame_Obj->Object[Cur_Frame_Obj->ObjectNum].iPolyStart = 0;
                    Cur_Frame_Obj->Object[Cur_Frame_Obj->ObjectNum].iPolylock = 0;
                    Cur_Frame_Obj->Object[Cur_Frame_Obj->ObjectNum].iSceneClose = 0;
                    Cur_Frame_Obj->Object[Cur_Frame_Obj->ObjectNum].Origin_center.u16X = 0;
                    Cur_Frame_Obj->Object[Cur_Frame_Obj->ObjectNum].Origin_center.u16Y = 0;

                    Cur_Frame_Obj->ObjectNum++;

                    TempPoint[count].s25q7X = pstGmmLk->stLK.pstPoint[0][j].s25q7X + pstGmmLk->stLK.pstMv[j].s9q7Dx;
                    TempPoint[count].s25q7Y = pstGmmLk->stLK.pstPoint[0][j].s25q7Y + pstGmmLk->stLK.pstMv[j].s9q7Dy;
                    TempMv[count].s9q7Dx = pstGmmLk->stLK.pstMv[j].s9q7Dx;
                    TempMv[count].s9q7Dy = pstGmmLk->stLK.pstMv[j].s9q7Dy;
                    TempMv[count].s32Status = 0;
                    count++;

#ifdef _XM_IA_PRINT_
                    printf("000 Object[%d].PtNum:%d j :%d count %d \n",
                        Cur_Frame_Obj->ObjectNum, Cur_Frame_Obj->Object[Cur_Frame_Obj->ObjectNum].PointInfo.PointNum, j, count);
                    //  printf("center :%d  %d \n", Object[ObjNum].center.u16X, Object[ObjNum].center.u16Y);
#endif

                }
                else if (1 == CurrentCCL[u])
                {
                    for (k = 0; k < MAX_OBJ_NUM; k++)
                    {
                        if (u == Cur_Frame_Obj->Object[k].CCLRegionNum)
                        {
                            if(Cur_Frame_Obj->Object[k].PointInfo.PointNum < MAX_POINT_NUM)
                            {
                                s = 0;
                                for (r = 0; r < count; r++)     //与已存的TempPoint数组里面的点做比较，而不是对OBJ里面存的点做比较
                                {
                                    if ((pstGmmLk->stLK.astPointTmp[j].s25q7X >> 7) == (TempPoint[r].s25q7X >> 7)
                                        && (pstGmmLk->stLK.astPointTmp[j].s25q7Y >> 7) == (TempPoint[r].s25q7Y >> 7))
                                    {
                                        s = 1;
                                    }
                                }
                                if (s == 0)
                                {
                                    int index = Cur_Frame_Obj->Object[k].PointInfo.PointNum;

                                    Cur_Frame_Obj->Object[k].PointInfo.Point[index].s25q7X = pstGmmLk->stLK.astPointTmp[j].s25q7X;
                                    Cur_Frame_Obj->Object[k].PointInfo.Point[index].s25q7Y = pstGmmLk->stLK.astPointTmp[j].s25q7Y;
                                    Cur_Frame_Obj->Object[k].PointInfo.PointMotionX[index] = (int)pstGmmLk->stLK.pstMv[j].s9q7Dx;//左移过的
                                    Cur_Frame_Obj->Object[k].PointInfo.PointMotionY[index] = (int)pstGmmLk->stLK.pstMv[j].s9q7Dy;//左移过的
                                    Cur_Frame_Obj->Object[k].PointInfo.PointNum++;
                                    TempPoint[count].s25q7X = pstGmmLk->stLK.pstPoint[0][j].s25q7X + pstGmmLk->stLK.pstMv[j].s9q7Dx;
                                    TempPoint[count].s25q7Y = pstGmmLk->stLK.pstPoint[0][j].s25q7Y + pstGmmLk->stLK.pstMv[j].s9q7Dy;
                                    TempMv[count].s9q7Dx = pstGmmLk->stLK.pstMv[j].s9q7Dx;
                                    TempMv[count].s9q7Dy = pstGmmLk->stLK.pstMv[j].s9q7Dy;
                                    TempMv[count].s32Status = 0;
                                    count++;

                                    //printf("111 Object[%d].PtNum:%d j:%d count %d\n", k, Cur_Frame_Obj->Object[k].PointInfo.PointNum, j, count);

#ifdef _XM_IA_PRINT_
                                    printf("111 Object[%d].PtNum:%d j:%d count %d\n", k, Cur_Frame_Obj->Object[k].PointInfo.PointNum, j, count);
#endif
                                }
                            }
                            else
                            {
                                printf("Cur_Frame_Obj->Object-%d.PointNum reach MAX_POINT_NUM\n",Cur_Frame_Obj->Object[k].ObjectID);
                            }
                        }
                    }
                }
            }
        }
        else//跟踪失败
        {
#ifdef _XM_IA_PRINT_
            printf("LK Follow failed : %d  X: %d  Y: %d\n", j, pstGmmLk->stLK.pstPoint[0][j].s25q7X / 128,
                pstGmmLk->stLK.pstPoint[0][j].s25q7Y / 128);
#endif
        }
    }

    for (j = 0; j < pstGmmLk->stGMM.stRegion.u16Num; j++)
    {
        if (0 == CurrentCCL[j])
        {
             ++(*TotalObjNum);

             Cur_Frame_Obj->Object[Cur_Frame_Obj->ObjectNum].ObjectID = *TotalObjNum;
             Cur_Frame_Obj->Object[Cur_Frame_Obj->ObjectNum].PointInfo.PointMotionX[0] = 0;//左移过的
             Cur_Frame_Obj->Object[Cur_Frame_Obj->ObjectNum].PointInfo.PointMotionY[0] = 0;//左移过的
             Cur_Frame_Obj->Object[Cur_Frame_Obj->ObjectNum].PointInfo.Point[0].s25q7X = 0; //左移过的
             Cur_Frame_Obj->Object[Cur_Frame_Obj->ObjectNum].PointInfo.Point[0].s25q7Y = 0;
             Cur_Frame_Obj->Object[Cur_Frame_Obj->ObjectNum].PointInfo.PointNum = 1;
             Cur_Frame_Obj->Object[Cur_Frame_Obj->ObjectNum].CCLRegionNum = j;
             Cur_Frame_Obj->Object[Cur_Frame_Obj->ObjectNum].CCLRegionArea = pstGmmLk->stGMM.stRegion.astRect[j].u32Area;
             Cur_Frame_Obj->Object[Cur_Frame_Obj->ObjectNum].stRect.s16X1 = (IMP_S16)pstGmmLk->stGMM.stRegion.astRect[j].astPoint[0].s32X;
             Cur_Frame_Obj->Object[Cur_Frame_Obj->ObjectNum].stRect.s16Y1 = (IMP_S16)pstGmmLk->stGMM.stRegion.astRect[j].astPoint[0].s32Y;
             Cur_Frame_Obj->Object[Cur_Frame_Obj->ObjectNum].stRect.s16X2 = (IMP_S16)pstGmmLk->stGMM.stRegion.astRect[j].astPoint[2].s32X;
             Cur_Frame_Obj->Object[Cur_Frame_Obj->ObjectNum].stRect.s16Y2 = (IMP_S16)pstGmmLk->stGMM.stRegion.astRect[j].astPoint[2].s32Y;
             Cur_Frame_Obj->Object[Cur_Frame_Obj->ObjectNum].CenterMotion.dx = 0;
             Cur_Frame_Obj->Object[Cur_Frame_Obj->ObjectNum].CenterMotion.dy = 0;
             Cur_Frame_Obj->Object[Cur_Frame_Obj->ObjectNum].center.u16X = (pstGmmLk->stGMM.stRegion.astRect[j].astPoint[0].s32X +
                 pstGmmLk->stGMM.stRegion.astRect[j].astPoint[2].s32X) / 2;
             Cur_Frame_Obj->Object[Cur_Frame_Obj->ObjectNum].center.u16Y = (pstGmmLk->stGMM.stRegion.astRect[j].astPoint[0].s32Y +
                 pstGmmLk->stGMM.stRegion.astRect[j].astPoint[2].s32Y) / 2;
             Cur_Frame_Obj->Object[Cur_Frame_Obj->ObjectNum].State = 1;
             Cur_Frame_Obj->Object[Cur_Frame_Obj->ObjectNum].iTouchLine = 0;
             Cur_Frame_Obj->Object[Cur_Frame_Obj->ObjectNum].iMark = 0;
             Cur_Frame_Obj->Object[Cur_Frame_Obj->ObjectNum].iPolyMark = 0;
             Cur_Frame_Obj->Object[Cur_Frame_Obj->ObjectNum].iPolyStart = 0;
             Cur_Frame_Obj->Object[Cur_Frame_Obj->ObjectNum].iPolylock = 0;
             Cur_Frame_Obj->Object[Cur_Frame_Obj->ObjectNum].iCount = 0;
             Cur_Frame_Obj->Object[Cur_Frame_Obj->ObjectNum].iSceneClose = 0;
             Cur_Frame_Obj->Object[Cur_Frame_Obj->ObjectNum].Origin_center.u16X = 0;
             Cur_Frame_Obj->Object[Cur_Frame_Obj->ObjectNum].Origin_center.u16Y = 0;

             Cur_Frame_Obj->ObjectNum++;

             if (Cur_Frame_Obj->ObjectNum >= MAX_OBJ_NUM)
             {
                 printf("ObjectNum over %d!!!!\n", MAX_OBJ_NUM);
                 Cur_Frame_Obj->ObjectNum = MAX_OBJ_NUM - 1;
             }


            CurrentCCL[j] = 1;//第u个区域已被分配给目标
        }
    }

    (*TotalCorner) = count;
    memset(pstGmmLk->stLK.astPointTmp, 0, sizeof(IVE_POINT_S25Q7_S)*MAX_OBJ_NUM * MAX_POINT_NUM);
    memcpy(pstGmmLk->stLK.astPointTmp, TempPoint, sizeof(IVE_POINT_S25Q7_S)*MAX_OBJ_NUM * MAX_POINT_NUM);
    memset(pstGmmLk->stLK.pstMv, 0, sizeof(IVE_MV_S9Q7_S)* MAX_OBJ_NUM * MAX_POINT_NUM);
    memcpy(pstGmmLk->stLK.pstMv, TempMv, sizeof(IVE_MV_S9Q7_S)* MAX_OBJ_NUM * MAX_POINT_NUM);

}

HI_S32 Cur_Frame_Obj_Order(XM_INIT_PARA *pstInitPara)
{
    int j, iNewCorner = 0;
    //统计当前的活跃目标，还需要对目标数组进行排序，有效目标放在数组前段，因为之后遍历是从0-objNum
    pstInitPara->pstCur_Frame_Obj->ObjectNum = 0;
    memset(pstInitPara->DoNewCorner, 0, sizeof(int)*MAX_OBJ_NUM);

    for (j = 0; j < MAX_OBJ_NUM; j++)//先做一个有效目标排序
    {

        if ((pstInitPara->pstCur_Frame_Obj->Object[j].CCLRegionNum != 100) || (pstInitPara->pstCur_Frame_Obj->Object[j].iPolyMark != 0) ||
            (1 == pstInitPara->pstCur_Frame_Obj->Object[j].iPolyStart))
        {
            //为后面Action=2的Corner做准备
            if ((pstInitPara->pstCur_Frame_Obj->Object[j].PointInfo.PointNum < 5) && (pstInitPara->pstCur_Frame_Obj->Object[j].iPolyMark != -1))
            {
                //对于当前目标框定的CCL区域，只有点数少于5的时候新目标进来做新的一次角点
                pstInitPara->DoNewCorner[pstInitPara->pstCur_Frame_Obj->Object[j].CCLRegionNum] = 0;
            }
            else
            {
                pstInitPara->DoNewCorner[pstInitPara->pstCur_Frame_Obj->Object[j].CCLRegionNum] = 1;
            }
            //待修改的目标判定规则
            if ((((pstInitPara->pstCur_Frame_Obj->Object[j].PointInfo.PointNum >= 0 )||
				(1 == pstInitPara->pstCur_Frame_Obj->Object[j].iPolyStart) ||
                (pstInitPara->pstCur_Frame_Obj->Object[j].iPolyMark >= 1)) &&
                (pstInitPara->pstCur_Frame_Obj->Object[j].iPolyMark != -1)) &&
                ((pstInitPara->pstCur_Frame_Obj->Object[j].CCLRegionArea > pstInitPara->iObjMinArea) &&
                (pstInitPara->pstCur_Frame_Obj->Object[j].CCLRegionArea < pstInitPara->iObjMaxArea )))
                //如果没有点了但是还是有一定阈值且还在前景区域的话可以认为是目标，但是下一帧这边要做角点
            {
                pstInitPara->pstCur_Frame_Obj->Object[j].State = 1;
                //memcpy(&Object[CurObjNum], &Object[j], sizeof(Struct_Object));
                pstInitPara->pstCur_Frame_Obj->Object[pstInitPara->pstCur_Frame_Obj->ObjectNum] = pstInitPara->pstCur_Frame_Obj->Object[j];
                pstInitPara->pstCur_Frame_Obj->ObjectNum++;
                if ((0 == pstInitPara->pstCur_Frame_Obj->Object[j].PointInfo.PointNum) &&
                    (pstInitPara->pstCur_Frame_Obj->Object[j].u16RemnantFrameCount < 5))//如果没有了角点
                {
                    iNewCorner = 1;
                    pstInitPara->DoNewCorner[pstInitPara->pstCur_Frame_Obj->Object[j].CCLRegionNum] = 0;
                    memset(pstInitPara->pstCur_Frame_Obj->Object[j].History_MotionX, 0, sizeof(int) * HISTORY_FRAME);
                    memset(pstInitPara->pstCur_Frame_Obj->Object[j].History_MotionY, 0, sizeof(int) * HISTORY_FRAME);
                }
            }
            else
            {
                pstInitPara->pstCur_Frame_Obj->Object[j].State = 0;
            }
            //额外的目标擦除判断
            if((pstInitPara->pstCur_Frame_Obj->Object[j].iPolyMark == -1)&&(pstInitPara->pstCur_Frame_Obj->Object[j].u16RemnantFrameCount >= 4))
            {
                pstInitPara->pstCur_Frame_Obj->Object[j].State = 0;
                pstInitPara->pstCur_Frame_Obj->Object[j].iPolyMark = 0;
            }
        }
    }

    for (j = pstInitPara->pstCur_Frame_Obj->ObjectNum; j < MAX_OBJ_NUM; j++)
    {
        ObjectInit(pstInitPara->pstCur_Frame_Obj->Object + j);
    }

#ifdef _XM_IA_PRINT_
    int k;
    for (j = 0; j < MAX_OBJ_NUM; j++)
    {
        if (1 == pstInitPara->pstCur_Frame_Obj->Object[j].State)
        {
            printf("*****Object[%d].PointInfo.PointNum: %d  CCLNUM:%d   AREA: %d  state:%d  ObjectID: %d ****\n", j,
                pstInitPara->pstCur_Frame_Obj->Object[j].PointInfo.PointNum, pstInitPara->pstCur_Frame_Obj->Object[j].CCLRegionNum,
                pstInitPara->pstCur_Frame_Obj->Object[j].CCLRegionArea, pstInitPara->pstCur_Frame_Obj->Object[j].State,
                pstInitPara->pstCur_Frame_Obj->Object[j].ObjectID);
            for (k = 0; k < pstInitPara->pstCur_Frame_Obj->Object[j].PointInfo.PointNum; k++)
            {
                printf("------------Object[%d].Point[%d]: X:%d Y:%d Dx:%d Dy:%d -------------------\n", j, k,
                    pstInitPara->pstCur_Frame_Obj->Object[j].PointInfo.Point[k].s25q7X >> 7,
                    pstInitPara->pstCur_Frame_Obj->Object[j].PointInfo.Point[k].s25q7Y >> 7,
                    pstInitPara->pstCur_Frame_Obj->Object[j].PointInfo.PointMotionX[k],
                    pstInitPara->pstCur_Frame_Obj->Object[j].PointInfo.PointMotionY[k]);
            }
        }
    }
    printf("*********CurObjNum:   %d   *************\n", pstInitPara->pstCur_Frame_Obj->ObjectNum);
#endif

    return iNewCorner;
}

HI_S32  New_Corner(XM_INIT_PARA *pstInitPara)
{
    IVE_HANDLE IveHandle;
    int count,j,k;
    pstInitPara->pstGmmLk->stLK.stStCandiCornerCtrl.u0q8QualityLevel = CORNER_INIT_LEVEL;


    memset(pstInitPara->TempPoint, 0, sizeof(IVE_POINT_S25Q7_S)* MAX_OBJ_NUM * MAX_POINT_NUM);
    memset(pstInitPara->TempMv, 0, sizeof(IVE_MV_S9Q7_S)*MAX_OBJ_NUM * MAX_POINT_NUM);
    memcpy(pstInitPara->TempPoint, pstInitPara->pstGmmLk->stLK.astPointTmp, sizeof(IVE_POINT_S25Q7_S)*MAX_OBJ_NUM * MAX_POINT_NUM);   //  将当前的坐标和运动信息存在中间数组
    memcpy(pstInitPara->TempMv, pstInitPara->pstGmmLk->stLK.pstMv, sizeof(IVE_MV_S9Q7_S)* MAX_OBJ_NUM * MAX_POINT_NUM);
    count = (pstInitPara->TotalCorner);

#ifdef _XM_IA_PRINT_
    for (j = 0;j < pstInitPara->pstGmmLk->stGMM.stRegion.u16Num; j++)
    {
        printf("CCL [%d] Left %d Right %d Top %d Bottom %d Area: %d \n", j, pstInitPara->pstGmmLk->stGMM.stRegion.astRect[j].astPoint[0].s32X,
            pstInitPara->pstGmmLk->stGMM.stRegion.astRect[j].astPoint[2].s32X, pstInitPara->pstGmmLk->stGMM.stRegion.astRect[j].astPoint[0].s32Y,
            pstInitPara->pstGmmLk->stGMM.stRegion.astRect[j].astPoint[2].s32Y, pstInitPara->pstGmmLk->stGMM.stRegion.astRect[j].u32Area);
    }
#endif

#ifdef _XM_IA_lINUX_
    HI_U64 pu64CCLCornerStartPts = 0, pu64CCLCornerEndPts = 0;
    HI_MPI_SYS_GetCurPts(&pu64CCLCornerStartPts);
#endif

    ////对区域进行角点检测
    (pstInitPara->TotalCorner) = CclCorner(&IveHandle, pstInitPara);

#ifdef _XM_IA_lINUX_
    HI_MPI_SYS_GetCurPts(&pu64CCLCornerEndPts);
#endif
    //printf("CCLCorner cost : %lld \n", pu64CCLCornerEndPts - pu64CCLCornerStartPts);

    memset(pstInitPara->pstGmmLk->stLK.stMv.pu8VirAddr, 0, pstInitPara->pstGmmLk->stLK.stMv.u32Size); //将Mv初始化为0
    for (j = 0; j < count; j++)
    {
        pstInitPara->pstGmmLk->stLK.astPointTmp[(pstInitPara->TotalCorner) + j].s25q7X = (HI_S32)pstInitPara->TempPoint[j].s25q7X;
        pstInitPara->pstGmmLk->stLK.astPointTmp[(pstInitPara->TotalCorner) + j].s25q7Y = (HI_S32)pstInitPara->TempPoint[j].s25q7Y;
        pstInitPara->pstGmmLk->stLK.pstMv[(pstInitPara->TotalCorner) + j].s9q7Dx = pstInitPara->TempMv[j].s9q7Dx;
        pstInitPara->pstGmmLk->stLK.pstMv[(pstInitPara->TotalCorner) + j].s9q7Dy = pstInitPara->TempMv[j].s9q7Dy;
        //printf("   old OBJ : X : %d  Y : %d   ", pstGmmLk->stLK.astPointTmp[(*TotalCorner) + j].s25q7X / 128, pstGmmLk->stLK.astPointTmp[(*TotalCorner) + j].s25q7Y / 128);
    }
    (pstInitPara->TotalCorner) += count;

    for (j = 0; j < (pstInitPara->TotalCorner) - 1; j++)
    {
        for (k = j + 1; k < (pstInitPara->TotalCorner); k++)
        {
            if (pstInitPara->pstGmmLk->stLK.astPointTmp[j].s25q7X == pstInitPara->pstGmmLk->stLK.astPointTmp[k].s25q7X &&
                pstInitPara->pstGmmLk->stLK.astPointTmp[j].s25q7Y == pstInitPara->pstGmmLk->stLK.astPointTmp[k].s25q7Y)
            {
            //printf("Filter POint index:j %d k %d X: %d Y: %d\n",j,k,pstGmmLk->stLK.astPointTmp[j].s25q7X,
   //                     pstGmmLk->stLK.astPointTmp[j].s25q7Y);
                pstInitPara->pstGmmLk->stLK.astPointTmp[k].s25q7X = pstInitPara->pstGmmLk->stLK.astPointTmp[(pstInitPara->TotalCorner) - 1].s25q7X;
                pstInitPara->pstGmmLk->stLK.astPointTmp[k].s25q7Y = pstInitPara->pstGmmLk->stLK.astPointTmp[(pstInitPara->TotalCorner) - 1].s25q7Y;
                pstInitPara->pstGmmLk->stLK.pstMv[k].s9q7Dx = pstInitPara->pstGmmLk->stLK.pstMv[(pstInitPara->TotalCorner) - 1].s9q7Dx;
                pstInitPara->pstGmmLk->stLK.pstMv[k].s9q7Dy = pstInitPara->pstGmmLk->stLK.pstMv[(pstInitPara->TotalCorner) - 1].s9q7Dy;
                --(pstInitPara->TotalCorner);
            }
        }
    }
//  printf("NewObj (*TotalCorner)s: %d \n", (*TotalCorner));
    return 0;
}



static void sort(int *c, int sum)  //  排序
{
    int i, j;
    int v;
    for (i = 0; i < sum - 1; i++)
    {
        for (j = 0; j < sum - i - 1; j++)
        {
            if (c[j] < c[j + 1])
            {
                v = c[j];
                c[j] = c[j + 1];
                c[j + 1] = v;
            }
        }
    }
}

int mean(int *a, int count)
{
    int i, sum = 0;
    for (i = 1; i <= count; i++)
        sum += a[i];
    int mean = sum / count;
    return mean;
}
//void Orientation1(int *PointMotionX, int *PointMotionY, int PointNum, int CCLRegionNum, int *DifFrameX, int *DifFrameY)
void Orientation1(XM_INIT_PARA *pstInitPara)
{
    int i;

    for (i = 0; i < pstInitPara->pstCur_Frame_Obj->ObjectNum; i++)//对每一个目标进行遍历
    {
        //对同一 区域 （目标）的角点排序,选取X方向
        int Point_Num = pstInitPara->pstCur_Frame_Obj->Object[i].PointInfo.PointNum;
        sort(pstInitPara->pstCur_Frame_Obj->Object[i].PointInfo.PointMotionX, Point_Num);

        //以目标所有角点的dx/dy计算整个目标的dx/dy
        if (0 == Point_Num)
        {
            //pstInitPara->pstCur_Frame_Obj->Object[i].Motion.dx = (int)pstInitPara->pstCur_Frame_Obj->Object[i].CenterMotion.dx;
            //pstInitPara->pstCur_Frame_Obj->Object[i].Motion.dy = (int)pstInitPara->pstCur_Frame_Obj->Object[i].CenterMotion.dy;
        }
        else if ((Point_Num % 2) != 0)
        {
            pstInitPara->pstCur_Frame_Obj->Object[i].Motion.dx = (int)pstInitPara->pstCur_Frame_Obj->Object[i].PointInfo.PointMotionX[(Point_Num - 1) / 2];
            pstInitPara->pstCur_Frame_Obj->Object[i].Motion.dy = (int)pstInitPara->pstCur_Frame_Obj->Object[i].PointInfo.PointMotionY[(Point_Num - 1) / 2];
        }
        else
        {
            pstInitPara->pstCur_Frame_Obj->Object[i].Motion.dx = (int)(pstInitPara->pstCur_Frame_Obj->Object[i].PointInfo.PointMotionX[Point_Num / 2] +
                pstInitPara->pstCur_Frame_Obj->Object[i].PointInfo.PointMotionX[1 + Point_Num / 2]) / 2;
            pstInitPara->pstCur_Frame_Obj->Object[i].Motion.dy = (int)(pstInitPara->pstCur_Frame_Obj->Object[i].PointInfo.PointMotionY[Point_Num / 2] +
                pstInitPara->pstCur_Frame_Obj->Object[i].PointInfo.PointMotionY[1 + Point_Num / 2]) / 2;
        }

        pstInitPara->pstCur_Frame_Obj->Object[i].History_MotionX[pstInitPara->framenum % HISTORY_FRAME] = pstInitPara->pstCur_Frame_Obj->Object[i].Motion.dx;
        pstInitPara->pstCur_Frame_Obj->Object[i].History_MotionY[pstInitPara->framenum % HISTORY_FRAME] = pstInitPara->pstCur_Frame_Obj->Object[i].Motion.dy;


    }
}
//void Orientation2(int *DifFrameX, int *DifFrameY)
void Orientation2(Frame_Object* Cur_Frame_Obj)
{
    int i;
    for (i = 0; i < Cur_Frame_Obj->ObjectNum; i++)
    {
        Cur_Frame_Obj->Object[i].PreditMotion.dx = THETA * mean(Cur_Frame_Obj->Object[i].History_MotionX, HISTORY_FRAME) + (1 - THETA) * Cur_Frame_Obj->Object[i].Motion.dx;//预测信息
        Cur_Frame_Obj->Object[i].PreditMotion.dy = THETA * mean(Cur_Frame_Obj->Object[i].History_MotionY, HISTORY_FRAME) + (1 - THETA) * Cur_Frame_Obj->Object[i].Motion.dy;//预测信息
    }
}

HI_U32 Frame_Sub_Thr(XM_INIT_PARA *pstInitPara)
{
    HI_S32 s32Ret = HI_SUCCESS;
    IVE_HANDLE IveHandle;
    HI_BOOL bInstant = HI_FALSE;

    s32Ret = HI_MPI_IVE_Sub(&IveHandle, &pstInitPara->pstGmmLk->stGMM.stSrc, &pstInitPara->pstGmmLk->stLK.astPrePyr[0],
        &pstInitPara->pstGmmLk->stGMM.stDILATEImg, pstInitPara->pstSubCtrl, bInstant);//相减之后，阈值二值化之前的图像存到膨胀结果那里
    if (s32Ret != HI_SUCCESS)
    {
        printf("HI_MPI_IVE_Sub fail,Error(%#x)\n", s32Ret);
        return HISI_IVE_FAIL;
    }
#if 0
    HI_BOOL bBlock = HI_TRUE;
    HI_BOOL bFinish = HI_FALSE;

    s32Ret = HI_MPI_IVE_Query(IveHandle, &bFinish, bBlock);
    while (HI_ERR_IVE_QUERY_TIMEOUT == s32Ret)
    {
        //usleep(200);
        s32Ret = HI_MPI_IVE_Query(IveHandle, &bFinish, bBlock);
    }
    if (s32Ret != HI_SUCCESS)
    {
        printf("HI_MPI_IVE_Query fail,Error(%#x)\n", s32Ret);
        return HISI_IVE_FAIL;
    }
#endif

    s32Ret = HI_MPI_IVE_Thresh(&IveHandle, &pstInitPara->pstGmmLk->stGMM.stDILATEImg,
                &pstInitPara->pstRemnantDetectionInfo->stImageDiffFg, pstInitPara->pstThrCtrl, bInstant);
    if (s32Ret != HI_SUCCESS)
    {
        printf("Frame_Sub_Thr fail,Error(%#x)\n", s32Ret);
        return HISI_IVE_FAIL;
    }
#if 0
    s32Ret = HI_MPI_IVE_Query(IveHandle, &bFinish, bBlock);
    while (HI_ERR_IVE_QUERY_TIMEOUT == s32Ret)
    {
        //usleep(200);
        s32Ret = HI_MPI_IVE_Query(IveHandle, &bFinish, bBlock);
    }
    if (s32Ret != HI_SUCCESS)
    {
        printf("HI_MPI_IVE_Query fail,Error(%#x)\n", s32Ret);

            return HISI_IVE_FAIL;
    }
#endif

    return s32Ret;
}


// 返回三个点组成三角形的面积
float triangleArea(URP_IMP_POINT_S a, URP_IMP_POINT_S b, URP_IMP_POINT_S c)
{
    float result = fabs((a.s16X * b.s16Y + b.s16X * c.s16Y + c.s16X * a.s16Y - b.s16X * a.s16Y
            - c.s16X * b.s16Y - a.s16X * c.s16Y) / 2.0);

    return result;
}

/**
 * 判断p是否在abcd组成的四边形内
 * @return 如果p在四边形内返回1,否则返回2.
 **/
int pInQuadrangle_Init(URP_IMP_POINT_S a, URP_IMP_POINT_S b, URP_IMP_POINT_S c, URP_IMP_POINT_S d, URP_IMP_POINT_S p)
{

    float dTriangle = triangleArea(a, b, p) + triangleArea(b, c, p)
        + triangleArea(c, d, p) + triangleArea(d, a, p);

    float dQuadrangle = triangleArea(a, b, c) + triangleArea(c, d, a);
    float fTmp = fabs(dTriangle - dQuadrangle);

    return fTmp > 0.5 ? 1 : 2;
}

//点在周界外部的判断        Mode = 1 为点在周界外部的判断    Mode = 2 为点在周界内部的判断
int pInQuadrangle_InOut(URP_IMP_POINT_S a, URP_IMP_POINT_S b, URP_IMP_POINT_S c, URP_IMP_POINT_S d, URP_IMP_POINT_S p, URP_IMP_POINT_S rect, int level, int Mode)
{
    URP_IMP_POINT_S p1, p2, p3, p4;
    int iNum = 0, iPo1, iPo2, iPo3, iPo4, iGet = 0;


    //警戒等级
    if (level == 0)
    {
        iNum = (rect.s16X + rect.s16Y) / 40;
    }
    if (level == 1)
    {
        iNum = (rect.s16X + rect.s16Y) / 80;
    }
    if (level == 2)
    {
        iNum = 0;
    }
    p1.s16X = p.s16X;
    p1.s16Y = p.s16Y - iNum;
    p2.s16X = p.s16X + iNum;
    p2.s16Y = p.s16Y;
    p3.s16X = p.s16X;
    p3.s16Y = p.s16Y + iNum;
    p4.s16X = p.s16X - iNum;
    p4.s16Y = p.s16Y;

    //四个点判断是否在四边形内
    iPo1 = pInQuadrangle_Init( a,  b,  c,  d,  p1);
    iPo2 = pInQuadrangle_Init( a,  b,  c,  d,  p2);
    iPo3 = pInQuadrangle_Init( a,  b,  c,  d,  p3);
    iPo4 = pInQuadrangle_Init( a,  b,  c,  d,  p4);

    if (Mode == 1)
    {
        if ((iPo1 + iPo2 + iPo3 + iPo4) == 4)
        {
            iGet = -1;
        }
        else
        {
            iGet = 1;
        }
    }
    else if (Mode == 2)
    {
        if ((iPo1 + iPo2 + iPo3 + iPo4) == 8)
        {
            iGet = 1;
        }
        else
        {
            iGet = -1;
        }
    }
    // -1 内 1 外
    return  iGet;

}

 //s1,s2,c为两个线头和一个中心点。警戒等级
 int LineQuadrangleJudge(URP_IMP_POINT_S s1, URP_IMP_POINT_S s2, URP_IMP_POINT_S c, int level)
 {
     int iRange = 100, iGet = 0;

     //警戒等级
     if (level == 0)
     {
         iRange = 20;
     }
     if (level == 1)
     {
         iRange = 50;
     }
     if (level == 2)
     {
         iRange = 100;
     }

     //点线围成面积
     float triangArea = triangleArea(s1, s2, c);

     float linelength = sqrt(pow((s1.s16X - s2.s16X), 2) + pow((s1.s16Y - s2.s16Y), 2));

     //对比面积
     float contrastArea = linelength * linelength / iRange;

     //面积对比
     float contrastResult = triangArea / contrastArea;

     if (contrastResult < 1)
     {
         iGet = 1;
     }
     else
     {
         iGet = -1;
     }

     return iGet;
 }

 //点移动的距离
 float CentDistance(URP_IMP_POINT_S oCent, URP_IMP_POINT_S sCent)
 {

     float CentDistance = sqrt(pow((sCent.s16X - oCent.s16X), 2) + pow((sCent.s16Y - oCent.s16Y), 2));

     return CentDistance;

 }

//场景变换判定----------------------------------------------------------------------------
void SceneChange_Judge(XM_INIT_PARA *pstInitPara)
{
    HI_S32 s32Ret = HI_SUCCESS;
    IVE_HANDLE IveHandle;
    HI_BOOL bInstant = HI_FALSE;
	float fSem[4] = { 0 };
	float fSemAll, fCom;
    //float fSem1,fSem2,fSem3,fSem4,fSemAll,fCom;

     s32Ret = HI_MPI_IVE_16BitTo8Bit(&IveHandle, &pstInitPara->pstGmmLk->stGMM.stSobel,
        &pstInitPara->pstGmmLk->stGMM.stSobelDst, &pstInitPara->pstGmmLk->stGMM.st16to8Ctrl,bInstant);
     if (s32Ret != HI_SUCCESS)
     {
        IA_Printf("SceneCh 16BitTo8Bit fail,Error(%#x)\n", s32Ret);
     }


    if(1 == pstInitPara->stAvdPara->stChangePara.u32Enable)
    {

        pstInitPara->iSceneGo = 0;
        //pstInitPara->iSceneMark = 0;

        if(21 == pstInitPara->framenum)
        {
            s32Ret = IveImageCopy( &pstInitPara->pstGmmLk->stGMM.stSobelDst, &pstInitPara->pstGmmLk->stLK.stNCC);
            if (s32Ret != HI_SUCCESS)
            {
                IA_Printf("SceneCh ImageCopy fail,Error(%#x)\n", s32Ret);
            }
        }


       /* IMP_RECT_S stCut1;
        IMP_RECT_S stCut2;
        IMP_RECT_S stCut3;*/
        IMP_RECT_S stCut;
				int i,j;
		for (i = 0; i < 2; i++)
		{
			for (j = 0; j < 2; j++)
			{
				stCut.s16X1 = i * (pstInitPara->iImgWidth / 2);
				stCut.s16Y1 = j * (pstInitPara->iImgHeigth / 2);
				stCut.s16X2 = (i + 1) * (pstInitPara->iImgWidth / 2);
				stCut.s16Y2 = (j + 1) * (pstInitPara->iImgHeigth / 2);

				fSem[i * 2 + j] = SceneChange_NCC(pstInitPara,
					stCut,
					&pstInitPara->pstGmmLk->stGMM.stNCCSrc1,
					&pstInitPara->pstGmmLk->stGMM.stNCCDst1);
			}
		}

		fSemAll = (1 - fSem[0]) + (1 - fSem[1]) + (1 - fSem[2]) + (1 - fSem[3]);

        /*stCut1.s16X1 = 0;
        stCut1.s16Y1 = 0;
        stCut1.s16X2 = pstInitPara->iImgWidth/2;
        stCut1.s16Y2 = pstInitPara->iImgHeigth/2;

        stCut2.s16X1 = pstInitPara->iImgWidth/2;
        stCut2.s16Y1 = 0;
        stCut2.s16X2 = pstInitPara->iImgWidth;
        stCut2.s16Y2 = pstInitPara->iImgHeigth/2;

        stCut3.s16X1 = 0;
        stCut3.s16Y1 = pstInitPara->iImgHeigth/2;
        stCut3.s16X2 = pstInitPara->iImgWidth/2;
        stCut3.s16Y2 = pstInitPara->iImgHeigth;

        stCut4.s16X1 = pstInitPara->iImgWidth/2;
        stCut4.s16Y1 = pstInitPara->iImgHeigth/2;
        stCut4.s16X2 = pstInitPara->iImgWidth;
        stCut4.s16Y2 = pstInitPara->iImgHeigth;

        fSem1 = SceneChange_NCC(pstInitPara,stCut1,
            &pstInitPara->pstGmmLk->stGMM.stNCCSrc1,&pstInitPara->pstGmmLk->stGMM.stNCCDst1);
        fSem2 = SceneChange_NCC(pstInitPara,stCut2,
            &pstInitPara->pstGmmLk->stGMM.stNCCSrc2,&pstInitPara->pstGmmLk->stGMM.stNCCDst2);
        fSem3 = SceneChange_NCC(pstInitPara,stCut3,
            &pstInitPara->pstGmmLk->stGMM.stNCCSrc3,&pstInitPara->pstGmmLk->stGMM.stNCCDst3);
        fSem4 = SceneChange_NCC(pstInitPara,stCut4,
            &pstInitPara->pstGmmLk->stGMM.stNCCSrc4,&pstInitPara->pstGmmLk->stGMM.stNCCDst4);

        fSemAll = (1-fSem1) + (1-fSem2) + (1-fSem3) + (1-fSem4);*/

        fCom = 1.6 - (pstInitPara->stAvdPara->stChangePara.u32AlarmLevel)*0.20;


        if(fSemAll > fCom)
        {
            pstInitPara->iSceneGo = 1;
        }

        if((1 == pstInitPara->iSceneGo)||(1 == pstInitPara->iSceneMark))
        {
            pstInitPara->iSceneCount++;
        }
        else
        {
            pstInitPara->iSceneCount = 0;
        }

        if(pstInitPara->iSceneCount >= 15)
        {
            pstInitPara->iSceneMark = 1;
        }

        if(pstInitPara->iSceneCount >= 18)
        {
            pstInitPara->iSceneMark = 0;
            pstInitPara->iSceneCount = 0;
        }

#ifdef _XM_IA_PRINT_
        printf("fSemAll:%f\n",fSemAll);
        printf("fCom:%f\n",fCom);
#endif

        if(0 == pstInitPara->iSceneCount)
        {
            s32Ret = IveImageCopy( &pstInitPara->pstGmmLk->stGMM.stSobelDst, &pstInitPara->pstGmmLk->stLK.stNCC);
            if (s32Ret != HI_SUCCESS)
            {
                IA_Printf("SceneCh ImageCopy fail,Error(%#x)\n", s32Ret);
            }
        }
    }
}


//------------------------------------------------------------------------------------------

 //遗留物判定
 void Polygon_Judge(XM_INIT_PARA *pstInitPara)
{
	int l;
     if(IMP_FUNC_OSC == (pstInitPara->pstURPpara->stRuleSet.astRule[0].u32Mode & IMP_FUNC_OSC))
     {
        pstInitPara->PolyGon = 0;
        int iOpenTime = pstInitPara->pstURPpara->stRuleSet.astRule[0].stPara.stOscRulePara.stOscPara.s32TimeMin * FRAME_RATE;
        int iCloseTime = (pstInitPara->pstURPpara->stRuleSet.astRule[0].stPara.stOscRulePara.stOscPara.s32TimeMin * FRAME_RATE) + 30;
        for (l = 0; l < pstInitPara->pstCur_Frame_Obj->ObjectNum; l++)
        {
            pstInitPara->pstCur_Frame_Obj->Object[l].iPolyStart = 0;
            //保留遗留物最后一刻图像区域
            if((pstInitPara->pstCur_Frame_Obj->Object[l].u16RemnantFrameCount == 4)&&
                (pstInitPara->pstCur_Frame_Obj->Object[l].stPolyRect.s16X1 == 0)&&
                (pstInitPara->pstCur_Frame_Obj->Object[l].stPolyRect.s16X2 == 0)&&
                (pstInitPara->pstCur_Frame_Obj->Object[l].stPolyRect.s16Y1 == 0)&&
                (pstInitPara->pstCur_Frame_Obj->Object[l].stPolyRect.s16Y2 == 0))
            {
                pstInitPara->pstCur_Frame_Obj->Object[l].stPolyRect.s16X1 = pstInitPara->pstCur_Frame_Obj->Object[l].stRect.s16X1;
                pstInitPara->pstCur_Frame_Obj->Object[l].stPolyRect.s16X2 = pstInitPara->pstCur_Frame_Obj->Object[l].stRect.s16X2;
                pstInitPara->pstCur_Frame_Obj->Object[l].stPolyRect.s16Y1 = pstInitPara->pstCur_Frame_Obj->Object[l].stRect.s16Y1;
                pstInitPara->pstCur_Frame_Obj->Object[l].stPolyRect.s16Y2 = pstInitPara->pstCur_Frame_Obj->Object[l].stRect.s16Y2;
            }

            //遗留物区域框定
            if ((pstInitPara->pstCur_Frame_Obj->Object[l].u16RemnantFrameCount >= 4)&&(pstInitPara->pstCur_Frame_Obj->Object[l].iPolyMark > -1))
            {
                pstInitPara->pstGmmLk->stGMM.stRegion.astRect[l].astPoint[0].s32X = pstInitPara->pstCur_Frame_Obj->Object[l].stPolyRect.s16X1;
                pstInitPara->pstGmmLk->stGMM.stRegion.astRect[l].astPoint[0].s32Y = pstInitPara->pstCur_Frame_Obj->Object[l].stPolyRect.s16Y1;
                pstInitPara->pstGmmLk->stGMM.stRegion.astRect[l].astPoint[2].s32X = pstInitPara->pstCur_Frame_Obj->Object[l].stPolyRect.s16X2;
                pstInitPara->pstGmmLk->stGMM.stRegion.astRect[l].astPoint[2].s32Y = pstInitPara->pstCur_Frame_Obj->Object[l].stPolyRect.s16Y2;

                pstInitPara->pstCur_Frame_Obj->Object[l].stRect.s16X1 = pstInitPara->pstCur_Frame_Obj->Object[l].stPolyRect.s16X1;
                pstInitPara->pstCur_Frame_Obj->Object[l].stRect.s16X2 = pstInitPara->pstCur_Frame_Obj->Object[l].stPolyRect.s16X2;
                pstInitPara->pstCur_Frame_Obj->Object[l].stRect.s16Y1 = pstInitPara->pstCur_Frame_Obj->Object[l].stPolyRect.s16Y1;
                pstInitPara->pstCur_Frame_Obj->Object[l].stRect.s16Y2 = pstInitPara->pstCur_Frame_Obj->Object[l].stPolyRect.s16Y2;
            }

            if (pstInitPara->pstCur_Frame_Obj->Object[l].u16RemnantFrameCount == 4)
            {
                IveImageCopy( &pstInitPara->pstGmmLk->stGMM.stSrc, &pstInitPara->pstGmmLk->stLK.stPoly);
            }

            if((FrameSub_Objects(pstInitPara,pstInitPara->pstGmmLk->stLK.stPoly,
                    pstInitPara->pstGmmLk->stLK.stPolyDst, pstInitPara->pstCur_Frame_Obj->Object[l].stPolyRect, 10) == 1) &&
                    (pstInitPara->pstCur_Frame_Obj->Object[l].u16RemnantFrameCount >= 4))
            {
                pstInitPara->pstCur_Frame_Obj->Object[l].iPolyStart = 1;
            }

            //  修改 增加判断 转入 帧差
            if (pstInitPara->pstCur_Frame_Obj->Object[l].u16RemnantFrameCount >= iOpenTime)
            {
                pstInitPara->pstCur_Frame_Obj->Object[l].iPolyMark = 2;//中间态

                if(FrameSub_Objects(pstInitPara,pstInitPara->pstGmmLk->stLK.stPoly,
                    pstInitPara->pstGmmLk->stLK.stPolyDst, pstInitPara->pstCur_Frame_Obj->Object[l].stPolyRect, 10) == 1)
                {
                    pstInitPara->pstCur_Frame_Obj->Object[l].iPolyMark = 1;

                }

                pstInitPara->pstCur_Frame_Obj->Object[l].iPolyClose ++;

                if(pstInitPara->pstCur_Frame_Obj->Object[l].iPolyClose > iCloseTime)
                {
                    pstInitPara->pstCur_Frame_Obj->Object[l].iPolyMark = -1;
                    pstInitPara->PolyGon = 1;
                }

            }
#ifdef _XM_IA_PRINT_
            printf("iPolyStart:%d\n",pstInitPara->pstCur_Frame_Obj->Object[l].iPolyStart);
            printf("Opentime:%d  Closetime:%d\n",iOpenTime,iCloseTime);
            printf("Count:%d\n",pstInitPara->pstCur_Frame_Obj->Object[l].u16RemnantFrameCount);
            printf("iPolyClose:%d\n", pstInitPara->pstCur_Frame_Obj->Object[l].iPolyClose);
            printf("Poly:%d\n", pstInitPara->pstCur_Frame_Obj->Object[l].iPolyMark);
#endif

        }
    }
 }

//物品盗移判定
 void Pilfer_Judge(XM_INIT_PARA *pstInitPara)
 {
     HI_S32 s32Ret = HI_SUCCESS;

     if(IMP_FUNC_OBJSTOLEN == (pstInitPara->pstURPpara->stRuleSet.astRule[0].u32Mode & IMP_FUNC_OBJSTOLEN))
     {


        if(21 == pstInitPara->framenum)
        {
            s32Ret = IveImageCopy( &pstInitPara->pstGmmLk->stGMM.stSrc, &pstInitPara->pstGmmLk->stLK.stNCCObj);
            if (s32Ret != HI_SUCCESS)
            {
                IA_Printf("SceneCh ImageCopy fail,Error(%#x)\n", s32Ret);
            }
        }

        pstInitPara->PolyGon = 0;
        IMP_RECT_S stCut;
        float iSubMrak = 0;
        int iOpenTime = pstInitPara->pstURPpara->stRuleSet.astRule[0].stPara.stOscRulePara.stOscPara.s32TimeMin * FRAME_RATE;
        int iCloseTime = (pstInitPara->pstURPpara->stRuleSet.astRule[0].stPara.stOscRulePara.stOscPara.s32TimeMin * FRAME_RATE) + 10;
				int l;
        for (l = 0; l < pstInitPara->pstCur_Frame_Obj->ObjectNum; l++)
        {
            pstInitPara->pstCur_Frame_Obj->Object[l].iPolyStart = 0;

            //保留盗移物画框区域
            if((pstInitPara->pstCur_Frame_Obj->Object[l].u16RemnantFrameCount == 3)&&
                (pstInitPara->pstCur_Frame_Obj->Object[l].stPilferRect.s16X1 == 0)&&
                (pstInitPara->pstCur_Frame_Obj->Object[l].stPilferRect.s16X2 == 0)&&
                (pstInitPara->pstCur_Frame_Obj->Object[l].stPilferRect.s16Y1 == 0)&&
                (pstInitPara->pstCur_Frame_Obj->Object[l].stPilferRect.s16Y2 == 0))
            {
                pstInitPara->pstCur_Frame_Obj->Object[l].stPilferRect.s16X1 = pstInitPara->pstRemnantDetectionInfo->stDetectionRct.u16X;
                pstInitPara->pstCur_Frame_Obj->Object[l].stPilferRect.s16Y1 = pstInitPara->pstRemnantDetectionInfo->stDetectionRct.u16Y;
                pstInitPara->pstCur_Frame_Obj->Object[l].stPilferRect.s16X2=
                    pstInitPara->pstRemnantDetectionInfo->stDetectionRct.u16X + pstInitPara->pstRemnantDetectionInfo->stDetectionRct.u16Width;
                pstInitPara->pstCur_Frame_Obj->Object[l].stPilferRect.s16Y2=
                    pstInitPara->pstRemnantDetectionInfo->stDetectionRct.u16Y+ pstInitPara->pstRemnantDetectionInfo->stDetectionRct.u16Height;
            }

            //物品盗移区域框定
            if ((pstInitPara->pstCur_Frame_Obj->Object[l].u16RemnantFrameCount >= 3)&&(pstInitPara->pstCur_Frame_Obj->Object[l].iPolyMark > -1))
            {
                pstInitPara->pstGmmLk->stGMM.stRegion.astRect[l].astPoint[0].s32X = pstInitPara->pstCur_Frame_Obj->Object[l].stPilferRect.s16X1;
                pstInitPara->pstGmmLk->stGMM.stRegion.astRect[l].astPoint[0].s32Y = pstInitPara->pstCur_Frame_Obj->Object[l].stPilferRect.s16Y1;
                pstInitPara->pstGmmLk->stGMM.stRegion.astRect[l].astPoint[2].s32X = pstInitPara->pstCur_Frame_Obj->Object[l].stPilferRect.s16X2;
                pstInitPara->pstGmmLk->stGMM.stRegion.astRect[l].astPoint[2].s32Y = pstInitPara->pstCur_Frame_Obj->Object[l].stPilferRect.s16Y2;

                pstInitPara->pstCur_Frame_Obj->Object[l].stRect.s16X1 = pstInitPara->pstCur_Frame_Obj->Object[l].stPilferRect.s16X1;
                pstInitPara->pstCur_Frame_Obj->Object[l].stRect.s16X2 = pstInitPara->pstCur_Frame_Obj->Object[l].stPilferRect.s16X2;
                pstInitPara->pstCur_Frame_Obj->Object[l].stRect.s16Y1 = pstInitPara->pstCur_Frame_Obj->Object[l].stPilferRect.s16Y1;
                pstInitPara->pstCur_Frame_Obj->Object[l].stRect.s16Y2 = pstInitPara->pstCur_Frame_Obj->Object[l].stPilferRect.s16Y2;

                stCut.s16X1 = pstInitPara->pstCur_Frame_Obj->Object[l].stPilferRect.s16X1;
                stCut.s16Y1 = pstInitPara->pstCur_Frame_Obj->Object[l].stPilferRect.s16Y1;
                stCut.s16X2 = pstInitPara->pstCur_Frame_Obj->Object[l].stPilferRect.s16X2;
                stCut.s16Y2 = pstInitPara->pstCur_Frame_Obj->Object[l].stPilferRect.s16Y2;

            }

            if(pstInitPara->pstCur_Frame_Obj->Object[l].u16RemnantFrameCount >= 3)
            {
                iSubMrak = FrameObj_NCC(pstInitPara,stCut,
                    &pstInitPara->pstGmmLk->stGMM.stPolySrc,&pstInitPara->pstGmmLk->stGMM.stPolyDst);
            }

            //和最初始图像作对比，帧差大于一定值触发开关
            if((iSubMrak < 0.96) && (pstInitPara->pstCur_Frame_Obj->Object[l].u16RemnantFrameCount >= 3))
            {
                pstInitPara->pstCur_Frame_Obj->Object[l].iPolyStart = 1;
            }

            if((iSubMrak > 0.96) && (pstInitPara->pstCur_Frame_Obj->Object[l].u16RemnantFrameCount >= 3))
            {
                pstInitPara->pstCur_Frame_Obj->Object[l].u16RemnantFrameCount = 0;
                pstInitPara->pstCur_Frame_Obj->Object[l].iPolyMark = -1;
                pstInitPara->PolyGon = 1;
            }

            //增加判断 转入 帧差
            if (pstInitPara->pstCur_Frame_Obj->Object[l].u16RemnantFrameCount >= iOpenTime)
            {
                pstInitPara->pstCur_Frame_Obj->Object[l].iPolyMark = 2;//中间态

                if(iSubMrak < 0.96)
                {
                    pstInitPara->pstCur_Frame_Obj->Object[l].iPolyMark = 1;
                }

                pstInitPara->pstCur_Frame_Obj->Object[l].iPolyClose ++;

                //持续一定时间后关闭报警，并且保留此帧为对比基准图像
                if(pstInitPara->pstCur_Frame_Obj->Object[l].iPolyClose > iCloseTime)
                {
                    pstInitPara->pstCur_Frame_Obj->Object[l].iPolyMark = -1;
                    pstInitPara->PolyGon = 1;
                    s32Ret = IveImageCopy( &pstInitPara->pstGmmLk->stGMM.stSrc, &pstInitPara->pstGmmLk->stLK.stNCCObj);
                    if (s32Ret != HI_SUCCESS)
                    {
                        IA_Printf("SceneCh ImageCopy fail,Error(%#x)\n", s32Ret);
                    }
                }

            }

#ifdef _XM_IA_PRINT_
                printf("Opentime:%d  Closetime:%d\n",iOpenTime,iCloseTime);
                printf("Count:%d\n",pstInitPara->pstCur_Frame_Obj->Object[l].u16RemnantFrameCount);
                printf("iPolyClose:%d\n", pstInitPara->pstCur_Frame_Obj->Object[l].iPolyClose);
                printf("Poly:%d\n", pstInitPara->pstCur_Frame_Obj->Object[l].iPolyMark);
                printf("PolyGon:%d\n",pstInitPara->PolyGon);
                printf("iSubMrak:%f\n",iSubMrak);
#endif

        }
     }
 }
//场景变换NCC--------------------------------------------------------------------------
float SceneChange_NCC(XM_INIT_PARA *pstInitPara, IMP_RECT_S stCut,IVE_SRC_IMAGE_S *stNCCSrc,IVE_DST_IMAGE_S *stNCCDst)
{
    HI_S32 s32Ret = HI_SUCCESS;
    IVE_HANDLE IveHandle;
    HI_BOOL bInstant = HI_FALSE;
    int cclwidth,cclheight,i;
    float fSem;
    HI_U64 pu64NCCStartPts = 0,pu64NCCEndPts = 0;
#ifdef _XM_IA_lINUX_
    HI_BOOL bBlock = HI_TRUE;
    HI_BOOL bFinish = HI_FALSE;
#endif
    cclwidth = stCut.s16X2 - stCut.s16X1;
    cclheight = stCut.s16Y2 - stCut.s16Y1;

     //支持的最小长宽度
     if (cclwidth < 32)
     {
         cclwidth = 32;
     }
     if (cclheight < 32)
     {
         cclheight = 32;
     }

     if((stCut.s16Y1 + cclheight) > pstInitPara->pstGmmLk->stGMM.stSobelDst.u16Height)
     {
            stCut.s16Y1 = pstInitPara->pstGmmLk->stGMM.stSobelDst.u16Height - cclheight ;
     }

     if((stCut.s16X1 + cclwidth) > pstInitPara->pstGmmLk->stGMM.stSrc.u16Width)
     {
            stCut.s16X1 = pstInitPara->pstGmmLk->stGMM.stSobelDst.u16Width - cclwidth ;
     }

     //
     stNCCSrc->u16Stride[0] = XM_IA_IVE_CalcStride(cclwidth, IVE_ALIGN);
     stNCCSrc->u16Width = (cclwidth >> 1) << 1;
     stNCCSrc->u16Height = (cclheight >> 1) << 1 ;
     for(i=0;i<cclheight;i++)
     {
            memcpy( stNCCSrc->pu8VirAddr[0] + i * stNCCSrc->u16Stride[0] ,
              pstInitPara->pstGmmLk->stLK.stNCC.pu8VirAddr[0] + (stCut.s16Y1+i) * pstInitPara->pstGmmLk->stLK.stNCC.u16Stride[0] + stCut.s16X1,
               cclwidth);
     }


     stNCCDst->u16Stride[0] = XM_IA_IVE_CalcStride(cclwidth, IVE_ALIGN);
     stNCCDst->u16Width = (cclwidth >> 1) << 1;
     stNCCDst->u16Height = (cclheight >> 1) << 1 ;
     for(i=0;i<cclheight;i++)
     {
            memcpy(stNCCDst->pu8VirAddr[0] + i * stNCCDst->u16Stride[0],
             pstInitPara->pstGmmLk->stGMM.stSobelDst.pu8VirAddr[0] + (stCut.s16Y1+i) * pstInitPara->pstGmmLk->stGMM.stSobelDst.u16Stride[0] + stCut.s16X1,
              cclwidth);
     }

#ifdef _XM_IA_lINUX_
//     HI_MPI_SYS_MmzFlushCache(stNCCSrc->u32PhyAddr[0], stNCCSrc->pu8VirAddr[0],
 //        stNCCSrc->u16Height*stNCCSrc->u16Stride[0]);
 //    HI_MPI_SYS_MmzFlushCache(stNCCDst->u32PhyAddr[0], stNCCDst->pu8VirAddr[0],
 //        stNCCDst->u16Height*stNCCDst->u16Stride[0]);
#endif

#ifdef _XM_IA_lINUX_
    HI_MPI_SYS_GetCurPts(&pu64NCCStartPts);
#endif

    s32Ret =
    HI_MPI_IVE_NCC(&IveHandle,stNCCDst,stNCCSrc,&pstInitPara->pstGmmLk->stLK.stNCCRDst, bInstant);
    if (s32Ret != HI_SUCCESS)
    {
        IA_Printf("NCC fail,Error(%#x)\n", s32Ret);
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
    HI_MPI_SYS_GetCurPts(&pu64NCCEndPts);
#endif

    IVE_NCC_DST_MEM_S* pstNccResult=(IVE_NCC_DST_MEM_S*)pstInitPara->pstGmmLk->stLK.stNCCRDst.pu8VirAddr;

    fSem = (pstNccResult->u64Numerator) / (sqrt(pstNccResult->u64QuadSum1) * sqrt(pstNccResult->u64QuadSum2));

#ifdef _XM_IA_PRINT_
    printf("NCC use time:%lld\n",pu64NCCEndPts - pu64NCCStartPts);
    printf("fSem:%f\n",fSem);
#endif
    return fSem;
}

//物品NCC
float FrameObj_NCC(XM_INIT_PARA *pstInitPara, IMP_RECT_S stCut,IVE_SRC_IMAGE_S *stNCCSrc,IVE_DST_IMAGE_S *stNCCDst)
{
    HI_S32 s32Ret = HI_SUCCESS;
    IVE_HANDLE IveHandle;
    HI_BOOL bInstant = HI_FALSE;
    HI_U64 pu64NCCStartPts = 0,pu64NCCEndPts = 0;
    int cclwidth,cclheight,i;
    float fSem;

    cclwidth = stCut.s16X2 - stCut.s16X1;
    cclheight = stCut.s16Y2 - stCut.s16Y1;

     //支持的最小长宽度
     if (cclwidth < 32)
     {
         cclwidth = 32;
     }
     if (cclheight < 32)
     {
         cclheight = 32;
     }

     if((stCut.s16Y1 + cclheight) > pstInitPara->pstGmmLk->stGMM.stSrc.u16Height)
     {
            stCut.s16Y1 = pstInitPara->pstGmmLk->stGMM.stSrc.u16Height - cclheight ;
     }

     if((stCut.s16X1 + cclwidth) > pstInitPara->pstGmmLk->stGMM.stSrc.u16Width)
     {
            stCut.s16X1 = pstInitPara->pstGmmLk->stGMM.stSrc.u16Width - cclwidth ;
     }

     //
     stNCCSrc->u16Stride[0] = XM_IA_IVE_CalcStride(cclwidth, IVE_ALIGN);
     stNCCSrc->u16Width = (cclwidth >> 1) << 1;
     stNCCSrc->u16Height = (cclheight >> 1) << 1 ;
     for(i=0;i<cclheight;i++)
     {
            memcpy( stNCCSrc->pu8VirAddr[0] + i * stNCCSrc->u16Stride[0] ,
              pstInitPara->pstGmmLk->stLK.stNCCObj.pu8VirAddr[0] + (stCut.s16Y1+i) * pstInitPara->pstGmmLk->stLK.stNCCObj.u16Stride[0] + stCut.s16X1,
               cclwidth);
     }

    stNCCDst->u16Stride[0] = XM_IA_IVE_CalcStride(cclwidth, IVE_ALIGN);
    stNCCDst->u16Width = (cclwidth >> 1) << 1;
    stNCCDst->u16Height = (cclheight >> 1) << 1 ;
    for(i=0;i<cclheight;i++)
    {
           memcpy(stNCCDst->pu8VirAddr[0] + i * stNCCDst->u16Stride[0],
            pstInitPara->pstGmmLk->stGMM.stSrc.pu8VirAddr[0] + (stCut.s16Y1+i) * pstInitPara->pstGmmLk->stGMM.stSrc.u16Stride[0] + stCut.s16X1,
             cclwidth);
    }

#ifdef _XM_IA_lINUX_
    HI_BOOL bBlock = HI_TRUE;
    HI_BOOL bFinish = HI_FALSE;
 //   HI_MPI_SYS_MmzFlushCache(stNCCSrc->u32PhyAddr[0], stNCCSrc->pu8VirAddr[0],
 //       stNCCSrc->u16Height*stNCCSrc->u16Stride[0]);
 //   HI_MPI_SYS_MmzFlushCache(stNCCDst->u32PhyAddr[0], stNCCDst->pu8VirAddr[0],
  //      stNCCDst->u16Height*stNCCDst->u16Stride[0]);
#endif

#ifdef _XM_IA_lINUX_
    HI_MPI_SYS_GetCurPts(&pu64NCCStartPts);
#endif

    s32Ret =
    HI_MPI_IVE_NCC(&IveHandle,stNCCDst,stNCCSrc,&pstInitPara->pstGmmLk->stLK.stNCCRObjDst, bInstant);
    if (s32Ret != HI_SUCCESS)
    {
        IA_Printf("NCC fail,Error(%#x)\n", s32Ret);
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
    HI_MPI_SYS_GetCurPts(&pu64NCCEndPts);
#endif

    IVE_NCC_DST_MEM_S* pstNccResult=(IVE_NCC_DST_MEM_S*)pstInitPara->pstGmmLk->stLK.stNCCRObjDst.pu8VirAddr;

    fSem = (pstNccResult->u64Numerator) / (sqrt(pstNccResult->u64QuadSum1) * sqrt(pstNccResult->u64QuadSum2));

#ifdef _XM_IA_PRINT_
    printf("NCC use time:%lld\n",pu64NCCEndPts - pu64NCCStartPts);
    printf("fSem:%f\n",fSem);
#endif
    return fSem;
}


 //物品帧差函数
 int FrameSub_Objects(XM_INIT_PARA *pstInitPara, IVE_SRC_IMAGE_S stOriginal, IVE_DST_IMAGE_S stOriginalDst, IMP_RECT_S stRect, int iXsub)
 {

     HI_S32 s32Ret = HI_SUCCESS;
     IVE_HANDLE IveHandle;
     HI_BOOL bInstant = HI_FALSE;
     IVE_SRC_DATA_S  stDataSrc;
     int iMark_x = 0;
     //IVE_DMA_CTRL_S stDmaCtrl;
     //stDmaCtrl.enMode = IVE_DMA_MODE_SET_3BYTE  ;

#ifdef _XM_IA_lINUX_
     HI_U64 pu64objpolyStartPts = 0,pu64objpolyEndPts = 0;
     HI_BOOL bBlock = HI_TRUE;
     HI_BOOL bFinish = HI_FALSE;
     HI_MPI_SYS_GetCurPts(&pu64objpolyStartPts);
#endif

     //计算图像侦差
     s32Ret = HI_MPI_IVE_Sub(&IveHandle, &stOriginal, &pstInitPara->pstGmmLk->stGMM.stSrc, &stOriginalDst, pstInitPara->pstSubCtrl, bInstant);
          if (s32Ret != HI_SUCCESS)
     {
         printf("HI_MPI_IVE_Sub fail,Error(%#x)\n", s32Ret);
         return HISI_IVE_FAIL;
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
     HI_MPI_SYS_GetCurPts(&pu64objpolyEndPts);
#endif

     int cclwidth = stRect.s16X2 - stRect.s16X1;
     int cclheight = stRect.s16Y2 - stRect.s16Y1;

     //支持的最小长宽度
     if (cclwidth < 10)
     {
        cclwidth = 10;
     }
     if (cclheight < 10)
     {
        cclheight = 10;
     }

     //截取遗留物区域
     stDataSrc.pu8VirAddr = stOriginalDst.pu8VirAddr[0] + stRect.s16Y1 * stOriginalDst.u16Stride[0] + stRect.s16X1;
     stDataSrc.u32PhyAddr = stOriginalDst.u32PhyAddr[0] + stRect.s16Y1 * stOriginalDst.u16Stride[0] + stRect.s16X1;

     stDataSrc.u16Stride = XM_IA_IVE_CalcStride(cclwidth, IVE_ALIGN);
     stDataSrc.u16Width = cclwidth;
     stDataSrc.u16Height = cclheight;


     float fSumSub = 0;
		int i;
     //算法 验证 图像 差值
     for (i = 0 ; i<(cclwidth*cclheight); i++)
     {
         fSumSub = fSumSub + (stDataSrc.pu8VirAddr[i]);
     }

     //计算平均帧差
     float fAve = fabs(fSumSub/(cclwidth*cclheight));

     //1为有遗留物 -1为无遗留物
     if (fAve < iXsub)
     {
         iMark_x = 1;
     }
     if (fAve >= iXsub)
     {
         iMark_x = -1;
     }

     return iMark_x;
 }

int IveImageCopy(IVE_IMAGE_S *pstSrc, IVE_IMAGE_S *pstDst)
{

    HI_S32 s32Ret = HI_SUCCESS;
    HI_BOOL bInstant = HI_FALSE;
    IVE_HANDLE IveHandle;
    IVE_DMA_CTRL_S stDmaCtrl;
    IVE_DATA_S stSrcData;
    IVE_DST_DATA_S stDstData;


    stSrcData.pu8VirAddr = pstSrc->pu8VirAddr[0];
    stSrcData.u32PhyAddr = pstSrc->u32PhyAddr[0];
    stSrcData.u16Height = pstSrc->u16Height;
    stSrcData.u16Width = pstSrc->u16Width;
    stSrcData.u16Stride = XM_IA_IVE_CalcStride(pstSrc->u16Width, IVE_ALIGN);

    stDstData.pu8VirAddr = pstDst->pu8VirAddr[0];
    stDstData.u32PhyAddr = pstDst->u32PhyAddr[0];
    stDstData.u16Height = pstDst->u16Height;
    stDstData.u16Width = pstDst->u16Width;
    stDstData.u16Stride = XM_IA_IVE_CalcStride(pstDst->u16Width, IVE_ALIGN);

    stDmaCtrl.enMode = IVE_DMA_MODE_DIRECT_COPY;

#ifdef _XM_IA_lINUX_
//    HI_MPI_SYS_MmzFlushCache(stSrcData.u32PhyAddr, stSrcData.pu8VirAddr, stSrcData.u16Height*stSrcData.u16Stride);
//    HI_MPI_SYS_MmzFlushCache(stDstData.u32PhyAddr, stDstData.pu8VirAddr, stDstData.u16Height*stDstData.u16Stride);
#endif

    s32Ret = HI_MPI_IVE_DMA(&IveHandle, &stSrcData, &stDstData, &stDmaCtrl, bInstant);
    if (s32Ret != HI_SUCCESS)
    {
        printf("FUNC %s Line %d fail,Error(%#x)\n", __FUNCTION__, __LINE__, s32Ret);
        return HISI_IVE_FAIL;
    }
#ifdef  _XM_IA_lINUX_
    HI_BOOL bBlock = HI_TRUE;
    HI_BOOL bFinish = HI_FALSE;

    s32Ret = HI_MPI_IVE_Query(IveHandle, &bFinish, bBlock);
	while (HI_ERR_IVE_QUERY_TIMEOUT == s32Ret)
	{
		usleep(200);
		s32Ret = HI_MPI_IVE_Query(IveHandle, &bFinish, bBlock);
	}
    if(s32Ret != HI_SUCCESS)
    {
        printf("HI_MPI_IVE_Query fail,Error(%#x)\n",s32Ret);
        return s32Ret;
    }
#endif
    return s32Ret;
}
//时间打印函数
void IA_Printf(const char *format, ...)
{


    char* pArg=NULL;
    char c;

    pArg = (char*) &format;
    pArg += sizeof(format);

    do
    {
        c =*format;
        if (c != '%')
        {
            putchar(c); //照原样输出字符
        }
        else
        {
            switch(*++format)
            {
                case 'd':
                    printf( "%d",*((int*)pArg));
                break;
                case 'f':
                    printf( "%f",*((double*)pArg));
                break;
                default:
                break;
            }
        pArg += sizeof(int);
        }
    ++format;
    }while (*format != '\0');

    pArg = NULL;

    time_t rawtime;
    struct tm *timeinfo;
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    printf (format);
    printf ("The warning time is: %s \n", asctime(timeinfo));

    return;
}



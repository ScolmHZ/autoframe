#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "blurecheck.h"

#define WIDTHBYTES(a) ((a<<2)>>2)
#define MAX(a, b) ((a) > (b) ? (a) : (b)) 
#define MIN(a, b) ((a) > (b) ? (a) : (b)) 
S32 ImgYuv2Rgb(IN IMGYUV_S *pstImgYuv, OUT IMGINFO_S *pstImgRgb)
{
	U32 *  pucImgY = pstImgYuv->pucImgY;
	U32 *  pucImgU = pstImgYuv->pucImgU;
	U32 *  pucImgV = pstImgYuv->pucImgV;
	U8 *  pucImgO = pstImgRgb->pucImage; /* B G R */

	S32 iImgWid = pstImgYuv->iImgWid;
	S32 iImgHgt = pstImgYuv->iImgHgt;
	S32 i = 0, iUnitByte  = 3;
	S32 j = 0, iStrideRgb = pstImgYuv->iStride;
	S32 iStrideY = WIDTHBYTES(pstImgYuv->iStride);
	S32 iStrideC = iStrideY;
	S32 iR,iG,iB;
	for (i = 0; i < iImgHgt; i++)
	{
		for (j = 0; j < iImgWid; j++)
		{
			S32 iRgbIdx = (i * iStrideRgb + j )* iUnitByte;
			S32 iY = pucImgY[i * iStrideY + j];
			S32 iOffSet = j >> 1 << 1;
			S32 iU = pucImgU[(i >> 1) * iStrideC + iOffSet];
			S32 iV = pucImgV[(i >> 1) * iStrideC + iOffSet];

			iR = ((iY << 14) + 1402 * (iV - 128)/1000 +
				(1 << (14 - 1))) >> 14;
			iG = ((iY << 14) - 34414 * (iU - 128)/100000 -
				71414 * (iV - 128)/100000 +
				(1 << (14 - 1))) >> 14;
			iB = ((iY << 14) + 1772 * (iU - 128)/1000 +
				(1 << (14 - 1))) >> 14;
			
			pucImgO[iRgbIdx + 2] = (U8)MIN(255, MAX(0, iB)); /* B */
			pucImgO[iRgbIdx + 1] = (U8)MIN(255, MAX(0, iG)); /* G */
			pucImgO[iRgbIdx + 0] = (U8)MIN(255, MAX(0, iR)); /* R */
		}
	}
	return 0;
}

#if 1
S32 ImgRgb2Grey( IN IMGINFO_S *pstImgC, OUT IMGINFO_S *pstImgG,int datalen)
{	
	S32 i = 0, j = 0;
	pU8 pucImgSrc = pstImgC->pucImage;
	pU8 pucImgDst = pstImgG->pucImage;
	S32 iSrcStride = 0, iDstStride = 0;

	pstImgG->iImgHgt = pstImgC->iImgHgt;
	pstImgG->iImgWid = pstImgC->iImgWid;
	pstImgG->iStride = pstImgC->iImgWid;
	pstImgG->iBitCnt = 8;
	FILE*fp;
	fp = fopen ("./CAI.jpg","w");
	fwrite (pstImgC->pucImage,1280*720*3,1,fp);
	fclose (fp);

	iSrcStride = WIDTHBYTES(pstImgC->iStride);
	iDstStride = WIDTHBYTES(pstImgG->iStride);
	pstImgG->iStride = iDstStride;
	pucImgDst = pucImgDst+210;
	for (i = 0; i < pstImgC->iImgHgt; i++)     
	{
		for (j = 0; j < pstImgC->iImgWid; j++) 
		{
			pU8 pucTmpSrc = pucImgSrc + j * (pstImgC->iBitCnt >> 3);
			S32 iPixTmp =  (pucTmpSrc[0] * 3736  + pucTmpSrc[1] * 19236 + pucTmpSrc[2] * 9795) >> 15;
			pucImgDst[j] = (U8)iPixTmp;
		}
	pucImgSrc += iSrcStride;
	pucImgDst += iDstStride;
	}

	FILE*PIC;
	PIC = fopen ("./pic.jpg","w");
	fwrite (pstImgG->pucImage,1280*720*3,1,PIC);
	fclose (PIC);
	return 0;
}
#endif

/*****************************************************************************
  函 数 名: VD_BlurCheck
  创建日期: 
  作    者: 
  函数描述: 模糊检测函数
  输入参数:IN VD_RGB_IMG_S * pstRGB : 彩色图像结构体指针  
           IN pU8 pucYData : 灰度图像数据指针  
               
  输出参数: INOUT pS32 piBlurRes  评分输出
    返回值: 是否调用成功
    注意点: 
------------------------------------------------------------------------------
修改历史
日期        姓名             描述
--------------------------------------------------------------------------

*****************************************************************************/
S32 VD_BlurCheck(IN VD_RGB_IMG_S * pstRGB, IN pU32 pucYData, INOUT pS32 piBlurRes)
{   
    U32 m    = 0, n    = 0;
    U32 uixs = 0, uixe = 0;
    U32 uiys = 0, uiye = 0;
    U32 k   = 0, l   = 0;
    U32 uiRows = 0, uiCols  = 0;
    S32 iLefW  = 0, iRigW  = 0;
    S32 iLFlag = 0, iRFlag = 0;
    
    U32 uiTotalByte = 0;
    S32 iGradThre = 0;
    U32 uiBSize   = 0;
    U32 uiNumx    = 0, uiNumy    = 0;
    S32 iWeeCnt   = 0, iCnt      = 0;
    
    S32 iWeeGradThre   = 0;
    S32 iEdgeWidCnt    = 0;
    S32 iGrayMeanBlock = 0;
    
    F32 fColorVar   = 0, fWeeGradMean = 0;
    F32 fWeeGradVar = 0, fWeeGradMax  = 0;
    F32 fBlocRatio  = 0, fEdgeWidSum  = 0;
    F32 fEdgeWidVer = 0, fEdgeWidHor  = 0, fEdgeWidth = 0;
    F32 fGradVer    = 0, fgradVerL    = 0, fgradVerR  = 0;
    F32 fGradHor    = 0, fGradHorU    = 0, fGradHorD  = 0;
    F32 fWeeSum     = 0, fSum         = 0, fTotalWeeGradMean = 0;
    F32 fWeeGrad[4];
    

    pU8  pucDat       = NULL, pucDatColor    = NULL, pucMask = NULL;
    pF32 pfUnionGrad  = NULL, pfGradx        = NULL;
    pF32 pfGrady      = NULL, pfGrayMean     = NULL;
    pF32 pfUionGradMax= NULL, pfUionGradMean = NULL;
    pV   pBuffer      = NULL, pBufferOffset  = NULL;

    /*入参检查*/
    if ( NULL == pstRGB || NULL == pucYData || NULL == piBlurRes)
    {
       printf("NULL Error\n");
        return DSP_VD_PARAPTR_NULL;
    }
    if ( NULL == pstRGB->pucRGB )
    {
        printf("no data\n");
        return DSP_VD_PARAPTR_NULL;
    }

    uiRows = pstRGB->uiHeight;
    uiCols = pstRGB->uiWidth;

    /*图像分块大小*/
    if ( uiRows <= 300 )
    {
        uiBSize = 8;
        iWeeGradThre = 180;
    }
    else if ( uiRows <= 900 &&  uiRows > 300 )
    {
        uiBSize = 16;
        iWeeGradThre = 75;
    }
    else
    {
        uiBSize = 20;
        iWeeGradThre = 75;
    }
#if 1 
    /*去除OSD*/
    if ( uiRows > 500 )
    {
        uixs = 0 + 5;                            /*图像周围有黑边，去除*/
        uixe = uiCols - 5;
        uiys =  20 * uiRows / 100  - 1;   /*左开右闭*/
        uiye =  70 * uiRows / 100;
    } 
    else
    {
        uixs = 0 + 5;
        uixe = uiCols - 5;
        uiys = 15 * uiRows / 100 - 1;
        uiye = 70 * uiRows / 100;
    }
#endif
    uiRows = uiye - uiys;   /*ROI 图像高度*/
    uiCols = uixe - uixs;   /*ROI 图像宽度*/
    /*预开辟内存空间*/
    uiTotalByte = uiRows * uiCols * sizeof(F32) * 5;
    pBuffer = (void *) malloc( uiTotalByte );
    if ( NULL == pBuffer )
    {
        printf("malloc failed\n");
        return DSP_VD_PARAPTR_NULL;
    }
    memset(pBuffer, 0, uiTotalByte);
    
    if ( DSP_VD_SUCCESS != VD_GetROIDat(pstRGB, pucYData, uixs, uixe, uiys, uiye, pBuffer,uiTotalByte) )
    {
        printf("VD_GetROIDat failed\n");
        if(NULL != pBuffer)
        {
            free( pBuffer );
            pBuffer = NULL;
        }
        else
        {
            printf("empty buff\n");
        }
        return DSP_VD_FAIL;
    }
    pucDatColor   = (pU8) pBuffer;                                       /*彩色图像数据*/
    pucDat        = pucDatColor + uiRows * uiCols * 3;                     /*灰度图像数据*/
    pBufferOffset = (void *) (pucDat + uiRows * uiCols);                   /*当前缓存指针偏移指针*/

    //由灰度获取四方向梯度、水平竖直梯度(绝对值)、灰度均值
    if(DSP_VD_SUCCESS != VD_BlurGetDat( pucDat, pBufferOffset, uiRows, uiCols ))
    {
        if(NULL != pBuffer)
        {
            free( pBuffer );
            pBuffer = NULL;
        }
        else
        {
            printf("empty buff1\n");
        }
        printf("VD_BlurGetDat\n");
        return DSP_VD_FAIL;
    }

    pfUnionGrad    = (pF32) pBufferOffset;               /*联合梯度*/
    pfGradx        = pfUnionGrad + uiRows * uiCols;        /*水平梯度*/
    pfGrady        = pfUnionGrad + uiRows * uiCols * 2;    /*垂直梯度*/
    pfGrayMean     = pfUnionGrad + uiRows * uiCols * 3;    /*全局灰度均值*/
    pfUionGradMax  = pfGrayMean + 1;                     /*全局联合梯度最大值*/
    pfUionGradMean = pfUionGradMax + 1;                  /*全局联合梯度均值*/
    pucMask = (pU8) (pfUionGradMean + 1);                /*全局模板*/

    if ( uiRows <= 300 )   /*CIF格式*/
    {
        iGradThre = 90;   /*联合梯度阈值*/
    }
    else
    {
        iGradThre = 50;
    }
    if ( *pfUionGradMean > iGradThre )                         /*明显清晰*/
    {
        *piBlurRes = 90;                                      /*C5*/
        if(NULL != pBuffer)
        {
            free( pBuffer );
            pBuffer = NULL;
        }
        else
        {
            printf("empty buff2\n");
        }
        return DSP_VD_SUCCESS;
    }
    else if ( *pfUionGradMean < 15 && *pfUionGradMax <= 200 )  /*明显模糊*/
    {
        *piBlurRes = 20;                                       /*B1*/
        if(NULL != pBuffer)
        {
            free( pBuffer );
            pBuffer = NULL;
        }
        else
        {
            printf("empty buff3\n");
        }
        return DSP_VD_SUCCESS;
    }

    /*将整帧图像进行划分，划分为变长为iBSize的大块*/
    uiNumx = (U32)(uiCols / uiBSize);
    uiNumy = (U32)(uiRows / uiBSize);
    iCnt  = 0;
    
    /*对整帧图像以大块为单位，计算精细梯度*/
    for ( m = 1; m <= uiNumy; m++ )
    {
        for ( n = 1; n < uiNumx; n++ )
        {
            uixs = (n-1)*uiBSize;
            uixe = n*uiBSize - 1;
            uiys = (m-1)*uiBSize;
            uiye = m*uiBSize - 1;
            fColorVar = VD_ColorVar(pucDatColor, uixs, uixe, uiys, uiye, uiCols, (S32*)iGrayMeanBlock);
            /*fColorVar越大，当前大块中的细节越丰富*/
            if ( fColorVar > 10 )
            {
                if ( *pfGrayMean < 80 && iGrayMeanBlock >= 220 ) /*暗图像中亮块不处理*/
                {
                    continue;
                }
                else
                {                           
                    for ( k = uiys; k <= uiye; k++ )
                    {
                        for ( l = uixs; l <= uixe; l++ )
                        {
                            *(pucMask + k*uiCols + l) = 1;   /*暗图像中亮块部分不会被赋值为1，可以加以区别*/
                        }
                    } /*判别块模板赋值结束*/
                    iWeeCnt = 0;
                    fWeeSum = 0;
                    /*将大块再次进行划分，分成2x2的小块，计算小块中四个像素点梯度的相关数据*/
                    for ( k = uiys; k <= uiye; k+=2 )
                    {
                        for ( l = uixs; l <= uixe; l+=2 )
                        {
                            fWeeGrad[0] = *(pfUnionGrad + k*uiCols + l);
                            fWeeGrad[1] = *(pfUnionGrad + k*uiCols + l + 1);
                            fWeeGrad[2] = *(pfUnionGrad + (k+1)*uiCols + l);
                            fWeeGrad[3] = *(pfUnionGrad + (k+1)*uiCols + l + 1);

                            fWeeGradMean = ( fWeeGrad[0] + fWeeGrad[1] + fWeeGrad[2] + fWeeGrad[3] ) / 4;
                            fWeeGradMax  = MAX( MAX( MAX(fWeeGrad[0], fWeeGrad[1]), fWeeGrad[2] ), fWeeGrad[3]);

                            /*计算方差*/
                            fWeeGradVar  = ( fWeeGrad[0] - fWeeGradMean) * ( fWeeGrad[0] - fWeeGradMean )
                                + ( fWeeGrad[1] - fWeeGradMean) * ( fWeeGrad[1] - fWeeGradMean )
                                + ( fWeeGrad[2] - fWeeGradMean) * ( fWeeGrad[2] - fWeeGradMean )
                                + ( fWeeGrad[3] - fWeeGradMean) * ( fWeeGrad[3] - fWeeGradMean );

                            fWeeGradVar /= 3; 

                            /*fWeeGradVar足够大时，图像细节才多，才有可能是清晰的图像*/
                            if ( fWeeGradVar > 6 )
                            {
                                iWeeCnt++;               /*统计当前大块中有多少个2x2的小块满足这个条件*/
                                fWeeSum += fWeeGradMax;  /*所有满足上述条件的小块中，四个像素点中最大梯度之和*/
                            }                               
                        }
                    } /*细化梯度计算结束*/
                    if ( 0 != iWeeCnt )
                    {
                        iCnt++;                            /*当大块中有小块满足 fWeeGradVar>6 的条件，此大块有可能是清晰图像，统计这种大块的数目*/
                        fSum += (fWeeSum / (F32)iWeeCnt);  /*统计所有满足上述条件的大块中 fWeeSum/(F32)iWeeCnt 的和，此数据越大，意味着图像越清晰*/
                    }
                }
            }
        }
    } /*分块处理结束*/
    if(0 != iCnt)
    {
        fTotalWeeGradMean = (F32)((F32)(fSum / (F32)iCnt));  /*图像整体分块精细梯度*/
    }
    else
    {
        printf("iCnt error\n");
        if(NULL != pBuffer)
        {
            free( pBuffer );
            pBuffer = NULL;
        }
        else
        {
            printf("empty buff4\n");
        }
        return DSP_VD_FAIL;
    } 
    if(0 != uiNumy)
    {
        fBlocRatio = (F32)iCnt / (F32)(S32)(uiNumx*uiNumy);     /*判别块比率*/
    }
    else
    {
        printf("uiNumy error\n");
        if(NULL != pBuffer)
        {
            free( pBuffer );
            pBuffer = NULL;
        }
        else
        {
            printf("empty buff5\n");
        }
        return DSP_VD_FAIL;
    } 

    if ( fBlocRatio < 0.01 )
    {
        *piBlurRes = 15;                                          /*B2*/
        if(NULL != pBuffer)
        {
            free( pBuffer );
            pBuffer = NULL;
        }
        else
        {
            printf("empty buff6\n");
        }
        return DSP_VD_SUCCESS;
    }

    if ( fTotalWeeGradMean > iWeeGradThre )
    {
        if ( fBlocRatio <= 0.2 )
        {
            *piBlurRes = 85;                                       /*C1*/
            if(NULL != pBuffer)
            {
                free( pBuffer );
                pBuffer = NULL;
            }
            else
            {
                printf("empty buff7\n");
            }
            return DSP_VD_SUCCESS;
        }
        else
        {
            *piBlurRes = 86;                                       /*C4*/
            if(NULL != pBuffer)
            {
                free( pBuffer );
                pBuffer = NULL;
            }
            else
            {
                printf("empty buff8\n");
            }
            return DSP_VD_SUCCESS;
        }
    }
    else if ( fTotalWeeGradMean < 35 )
    {
        if ( fBlocRatio <= 0.2 )
        {
            *piBlurRes = 30;                                       /*B5*/
            if(NULL != pBuffer)
            {
                free( pBuffer );
                pBuffer = NULL;
            }
            else
            {
                printf("empty buff9\n");
            }
            return DSP_VD_SUCCESS;
        }
        else
        {
            *piBlurRes = 31;                                       /*B1*/
            if(NULL != pBuffer)
            {
                free( pBuffer );
                pBuffer = NULL;
            }
            else
            {
                printf("empty buff10\n");
            }
            return DSP_VD_SUCCESS;
        }
    }

    /*垂直边缘宽度*/
    fEdgeWidSum = 0;
    iEdgeWidCnt = 0;

    for ( m = 0; m < uiRows; m++ )
    {
        for ( n = 10; n < uiCols - 10; n++ )
        {
            if ( 1 == *( pucMask + m*uiCols + n ) )                              /*判别点*/
            {
                iLefW=-1;
                iRigW=-1;
                iLFlag=0;
                iRFlag=0;
                fGradVer  = *( pfGrady + m*uiCols + n );                         /*该位置垂直梯度*/
                fgradVerL = *( pfGrady + m*uiCols + n-1 );
                fgradVerR = *( pfGrady + m*uiCols + n+1 );
                if ( fGradVer>40 && fGradVer>fgradVerL && fGradVer>fgradVerR )  /*局部极大值*/
                {
                    /*寻找左右起点，计算边缘宽度*/
                    for ( k = 1; k <= 10; k++ )
                    {
                        if ( 1 == iLFlag && 1 == iRFlag )
                        {
                            break;
                        }
                        if ( 0 == iLFlag )
                        {
                            if ( *( pfGrady + m*uiCols + n-k ) < 15 )     /*在当前极大值的左边寻找梯度值小于15的点，作为左边缘*/
                            {
                                iLefW = (S32)k; /*记录左边缘点距离极大值点的水平具体*/
                                iLFlag = 1;
                            }
                        }
                        if ( 0 == iRFlag )
                        {
                            if ( *( pfGrady + m*uiCols + n+k ) < 15 )     /*在当前极大值的右边寻找梯度值小于15的点，作为右边缘*/
                            {
                                iRigW = (S32)k; /*记录右边缘点距离极大值点的水平具体*/
                                iRFlag = 1;
                            }
                        }
                    } /*垂直梯度左右边缘宽度求解结束*/
                    if ( -1 == iLefW && -1 != iRigW )
                    {
                        fEdgeWidSum += iRigW;
                        iEdgeWidCnt++;
                    }
                    else if ( -1 != iLefW && -1 == iRigW )
                    {
                        fEdgeWidSum += iLefW;
                        iEdgeWidCnt++;
                    }
                    else if ( -1 != iLefW && -1 != iRigW )
                    {
                        fEdgeWidSum += (F32) ( iLefW + iRigW ) / 2;
                        iEdgeWidCnt++;
                    }
                    else
                    {
                        continue;
                    }
                }
            }
        }
    } /*垂直边缘宽度遍历结束*/
    if(0 != iEdgeWidCnt)
    {
        fEdgeWidVer = (F32)(fEdgeWidSum / iEdgeWidCnt);       /*整体垂直边缘宽度*/
    }
    else
    {
        printf("iEdgeWidCnt error\n");
        if(NULL != pBuffer)
        {
            free( pBuffer );
            pBuffer = NULL;
        }
        else
        {
            printf("empty buff11\n");
        }
        return DSP_VD_FAIL;
    } 

    /*水平边缘宽度*/
    fEdgeWidSum = 0;
    iEdgeWidCnt = 0;
    for ( n = 0; n < uiCols; n++ )
    {
        for ( m = 10; m < uiRows - 10; m++ )
        {
            if ( 1 == *( pucMask + m*uiCols + n ) )                              /*判别点*/
            {
                iLefW=-1; iRigW=-1; iLFlag=0; iRFlag=0;
                fGradHor  = *( pfGradx + m*uiCols + n );                         /*该位置垂直梯度*/
                fGradHorU = *( pfGradx + (m-1)*uiCols + n );
                fGradHorD = *( pfGradx + (m+1)*uiCols + n );
                if ( fGradHor>40 && fGradHor>fGradHorU && fGradHor>fGradHorD )  /*局部极大值*/
                {
                    for ( k = 1; k <= 10; k++ )
                    {
                        if ( 1 == iLFlag && 1 == iRFlag )
                        {
                            break;
                        }
                        if ( 0 == iLFlag )
                        {
                            if ( *( pfGradx + (m-k)*uiCols + n ) < 15 )
                            {
                                iLefW = (S32)k;
                                iLFlag = 1;
                            }
                        }
                        if ( 0 == iRFlag )
                        {
                            if ( *( pfGradx + (m+k)*uiCols + n ) < 15 )
                            {
                                iRigW = (S32)k;
                                iRFlag = 1;
                            }
                        }
                    } /*水平梯度上下边缘宽度求解结束*/
                    if( -1 == iLefW && -1 != iRigW )
                    {
                        fEdgeWidSum += iRigW;
                        iEdgeWidCnt++;
                    }
                    else if( -1 != iLefW && -1 == iRigW )
                    {
                        fEdgeWidSum += iLefW;
                        iEdgeWidCnt++;
                    }
                    else if( -1 != iLefW && -1 != iRigW )
                    {
                        fEdgeWidSum += (F32) ( iLefW + iRigW ) / 2;
                        iEdgeWidCnt++;
                    }
                    else
                    {
                        continue;
                    }
                }
            }
        }
    } /*垂直边缘宽度遍历结束*/
    if(0 != iEdgeWidCnt)
    {
        fEdgeWidHor = (F32)(fEdgeWidSum / iEdgeWidCnt);       /*整体水平边缘宽度*/
    }
    else
    {
        printf("iEdgeWidCnt error\n");
        if(NULL != pBuffer)
        {
            free( pBuffer );
            pBuffer = NULL;
        }
        else
        {
            printf("empty buff12\n");
        }
        return DSP_VD_FAIL;
    }                
    fEdgeWidth  = (F32)((fEdgeWidHor + fEdgeWidVer) / 2);     /*整体边缘宽度*/
    
    if ( fEdgeWidth < 4.15 )    /*清晰*/
    {
        if ( fBlocRatio <= 0.2 )
        {
            *piBlurRes = 80;                                       /*C2*/
            if(NULL != pBuffer)
            {
                free( pBuffer );
                pBuffer = NULL;
            }
            else
            {
                printf("empty buff13\n");
            }
            return DSP_VD_SUCCESS;
        }
        else
        {
            *piBlurRes = 81;                                       /*C3*/
            if(NULL != pBuffer)
            {
                free( pBuffer );
                pBuffer = NULL;
            }
            else
            {
                printf("empty buff14\n");
            }
            return DSP_VD_SUCCESS;
        }
    }
    else if ( fEdgeWidth >= 4.15 )
    {
        if ( fBlocRatio <= 0.4 )
        {
            *piBlurRes = 40;                                       /*B6*/
            if(NULL != pBuffer)
            {
                free( pBuffer );
                pBuffer = NULL;
            }
            else
            {
                printf("empty buff15\n");
            }
            return DSP_VD_SUCCESS;
        }
        else
        {
            *piBlurRes = 41;                                       /*B4*/
            if(NULL != pBuffer)
            {
                free( pBuffer );
                pBuffer = NULL;
            }
            else
            {
                printf("empty buff16\n");
            }
            return DSP_VD_SUCCESS;
        }
    }
    else if ( fEdgeWidHor < 3.85 || fEdgeWidVer < 3.85 )
    {
        if ( fBlocRatio <= 0.2 )
        {
            *piBlurRes = 80;                                       /*C2*/
            if(NULL != pBuffer)
            {
                free( pBuffer );
                pBuffer = NULL;
            }
            else
            {
               printf("empty buff17\n");
            }
            return DSP_VD_SUCCESS;
        }
        else
        {
            *piBlurRes = 81;                                       /*C3*/
            if(NULL != pBuffer)
            {
                free( pBuffer );
                pBuffer = NULL;
            }
            else
            {
                printf("empty buff18\n");
            }
            return DSP_VD_SUCCESS;
        }
    }
    else
    {
        *piBlurRes = 82;                                       /*C6*/
    }


    if(NULL != pBuffer)
    {
        free( pBuffer );
        pBuffer = NULL;
    }
    else
    {
        printf("empty buff19\n");
    }
    return DSP_VD_SUCCESS;
}

/*****************************************************************************
  函 数 名: VD_GetROIDat
  创建日期:  
  作    者: 
  函数描述: 获取感兴趣区域图像数据
  输入参数: IN VD_RGB_IMG_S * pstRGB : 彩色图像结构体指针  
            IN pU8 pucYData : 灰度图像数据指针           
            IN S32 ixs : 水平起始坐标                   
            IN S32 ixe : 水平终止坐标                              
            IN S32 iys : 垂直起始坐标                              
            IN S32 iye : 垂直终止坐标                             
            OUT void *pBuffer : 输出内存区             
  输出参数: 无
  返回值: 函数调用是否成功
  注意点: 
------------------------------------------------------------------------------
  修改历史                                                                  
  日期        姓名             描述                                         
  --------------------------------------------------------------------------
                                                                        
*****************************************************************************/
S32 VD_GetROIDat(IN VD_RGB_IMG_S * pstRGB, IN pU32 pucYData, IN U32 uixs, IN U32 uixe, 
                 IN U32 uiys, IN U32 uiye, INOUT void *pBuffer,IN U32 size)
{
    // pBuffer: 彩色图像数据+灰度数据
    // 坐标范围: 左开右闭
    U32 i = 0, j = 0;
    U32 uiCols = 0, uiRows;
    pU8 pucRGB = NULL;
    pU8 pucDat = NULL;
    pU8 pucDatColor = NULL;
    
    if ( NULL == pstRGB || NULL == pucYData || NULL == pBuffer)
    {
        printf("add buf to list error\n");
        return DSP_VD_PARAPTR_NULL;
    }
    if ( NULL == pstRGB->pucRGB )
    {
        printf("add buf to list error\n");
        return DSP_VD_PARAPTR_NULL;
    }

    uiCols      = pstRGB->uiWidth;
    uiRows = pstRGB->uiHeight;
    pucRGB      = pstRGB->pucRGB;
    pucDatColor = (pU8) pBuffer;                                    /*彩色图像数据*/
    pucDat      = pucDatColor + ( uixe - uixs ) * ( uiye - uiys ) * 3;  /*灰度图像数据*/

    if ( NULL == pucDat )
    {
        printf("pucDat error\n");
        return DSP_VD_PARAPTR_NULL;
    }

    

    if ( (1280*720 > 3 * uiRows * uiCols) || ( size < (uixe - uixs) * (uiye - uiys) * 5 * sizeof(F32) ) )
    {
        printf("size failed \n");
        return DSP_VD_PARAPTR_NULL;
    }
    for ( i = uiys+1; i <= uiye; i++ )                                /*左开右闭: (iys, iye]*/
    {
        for ( j = uixs+1; j <= uixe; j++ )
        {
            *( pucDatColor + (i-uiys-1)*3*(uixe-uixs) + (j-uixs-1)*3 )     = *( pucRGB + i*uiCols*3 + j*3 );      // B
            *( pucDatColor + (i-uiys-1)*3*(uixe-uixs) + (j-uixs-1)*3 + 1 ) = *( pucRGB + i*uiCols*3 + j*3 + 1);   // G
            *( pucDatColor + (i-uiys-1)*3*(uixe-uixs) + (j-uixs-1)*3 + 2 ) = *( pucRGB + i*uiCols*3 + j*3 + 2 );  // R
            *( pucDat + (i-uiys-1)*(uixe-uixs) + (j-uixs-1) ) = *( pucYData + i*uiCols + j );
        }
    }
    return DSP_VD_SUCCESS;
}

/*****************************************************************************
  函 数 名: VD_BlurGetDat
  创建日期: 
  作    者: 
  函数描述: 获取图像数据运算结果
  输入参数: IN pU8 pucDat : 输入灰度数据       
            INOUT void *pBuffer : 输出计算结果 
            IN S32 iRows : 行        
            IN S32 iCols : 列        
  输出参数: 无
    返回值: 
    注意点: 
------------------------------------------------------------------------------
    修改历史                                                                  
    日期        姓名             描述                                         
  --------------------------------------------------------------------------
                                                                              
*****************************************************************************/
S32 VD_BlurGetDat(IN pU8 pucDat, INOUT void *pBuffer, IN U32 uiRows, IN U32 uiCols)
{
    S32 i = 0, j = 0;
    S32 m = 0, n = 0;
    S32 iRows = (S32)uiRows;
    S32 iCols = (S32)uiCols;
    F32 fGraySum = 0;
    F32 fSobelx  = 0, fSobely  = 0;
    F32 fSobelxy = 0, fSobelyx = 0;
    
    F32 fUionGradMax   = 0;
    F32 fUionGradSum   = 0;
    pF32 pfUnionGrad   = NULL;
    pF32 pfGradx       = NULL;
    pF32 pfGrady       = NULL;
    pF32 pfGrayMean    = NULL;
    pF32 pfUionGradMax = NULL;
    pF32 pfUionGradMean= NULL;

    /*Sobel四方向模板*/
    F32 afMx[3][3]  = { {-1, -2, -1}, {0, 0, 0}, {1, 2, 1} }; 
    F32 afMy[3][3]  = { {-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1} };
    F32 afMxy[3][3] = { {-2, -1, 0}, {-1, 0, 1}, {0, 1, 2} };  /*45度方向*/
    F32 afMyx[3][3] = { {0, 1, 2,}, {-1, 0, 1}, {-2, -1, 0} };  /*135度方向*/
    F32 afPix[3][3];
    
    if ( NULL == pucDat )
    {
        printf("add buf to list error\n");
        return DSP_VD_PARAPTR_NULL;
    }

    pfUnionGrad    = (pF32) pBuffer;
    pfGradx        = pfUnionGrad + iRows * iCols;
    pfGrady        = pfUnionGrad + iRows * iCols * 2;
    pfGrayMean     = pfUnionGrad + iRows * iCols * 3;
    pfUionGradMax  = pfGrayMean + 1;
    pfUionGradMean = pfUionGradMax + 1;

    for ( i = 0; i < iRows; i++ ) 
    {
        for ( j = 0; j < iCols; j++ )
        {
            fGraySum += (F32) *( pucDat + i*iCols + j );   /*顺便计算灰度总和*/
            /*图像模板赋值*/
            for ( m = -1; m <= 1; m++ ) /*行偏移*/
            { 
                for ( n = -1; n <=1; n++ ) /*列偏移*/
                {
                    if ( j+n < 0 ) /*左边界复制扩展*/
                    {
                        if ( i+m < 0 ) /*扩展边界左上角*/
                        {
                            afPix[m+1][n+1] = (F32) *( pucDat + (i+m+1)*iCols + (j+n+1) );
                        }
                        else if ( i+m >= iRows ) /*扩展边界左下角*/
                        {
                            afPix[m+1][n+1] = (F32) *( pucDat + (i+m-1)*iCols + (j+n+1) );
                        }
                        else  /*扩展左边界*/
                        {
                            afPix[m+1][n+1] = (F32) *( pucDat + (i+m)*iCols + (j+n+1) );
                        }
                    }
                    else if ( j+n >= iCols ) /*右边界复制扩展*/
                    {
                        if ( i+m < 0 ) /*扩展边界右上角*/
                        {
                            afPix[m+1][n+1] = (F32) *( pucDat + (i+m+1)*iCols + (j+n-1) );
                        }
                        else if ( i+m >= iRows ) /*扩展边界右下角*/
                        {
                            afPix[m+1][n+1] = (F32) *( pucDat + (i+m-1)*iCols + (j+n-1) );
                        }
                        else  /*扩展右边界*/
                        {
                            afPix[m+1][n+1] = (F32) *( pucDat + (i+m)*iCols + (j+n-1) );
                        }
                    }
                    else if ( i+m < 0 ) /*上边界复制扩展*/
                    {
                        afPix[m+1][n+1] = (F32) *( pucDat + (i+m+1)*iCols + (j+n) );
                    }
                    else if ( i+m >= iRows ) /*下边界复制扩展*/
                    {
                        afPix[m+1][n+1] = (F32) *( pucDat + (i+m-1)*iCols + (j+n) );
                    }
                    else /*边界内*/
                    {
                        afPix[m+1][n+1] = (F32) *( pucDat + (i+m)*iCols + (j+n) );                  
                    }
                }
            } /*图像模板赋值结束*/

            fSobelx  = 0;
            fSobely  = 0;
            fSobelxy = 0;
            fSobelyx = 0;
            for ( m = 0; m < 3; m++ )
            {
                for ( n = 0; n < 3; n++ )
                {
                    fSobelx  += afPix[m][n] * afMx[m][n];
                    fSobely  += afPix[m][n] * afMy[m][n];
                    fSobelxy += afPix[m][n] * afMxy[m][n];
                    fSobelyx += afPix[m][n] * afMyx[m][n];
                }
            } /*模板计算结束*/

            *(pfUnionGrad + i*iCols + j) = sqrt( fSobelx*fSobelx + fSobely*fSobely + fSobelxy*fSobelxy + fSobelyx*fSobelyx );
            *(pfGradx + i*iCols + j) = abs( fSobelx );
            *(pfGrady + i*iCols + j) = abs( fSobely );
            if ( *(pfUnionGrad + i*iCols + j) > fUionGradMax )  /*最大联合梯度*/
            {
                fUionGradMax = *(pfUnionGrad + i*iCols + j);
            }
            fUionGradSum += *(pfUnionGrad + i*iCols + j);       /*联合梯度和*/
        }
    } /*图像遍历结束*/

    if(0 != iRows*iCols)
    {
        *(pfGrayMean) = fGraySum / ((F32)iRows * (F32)iCols);
        *(pfUionGradMean) = fUionGradSum / ((F32)iRows * (F32)iCols);
    }
    *(pfUionGradMax)  = fUionGradMax;
    

    return DSP_VD_SUCCESS;
}

/*****************************************************************************
  函 数 名: VD_ColorVar
  创建日期:  
  作    者: 
  函数描述: 彩色图像块颜色方差
  输入参数: IN pU8 pucDatColor : 彩色图像起始地址         
            IN S32 ixs : 水平起始坐标                
            IN S32 ixe : 水平终止坐标                       
            IN S32 iys : 垂直起始坐标                       
            IN S32 iye : 垂直起始坐标                       
            IN S32 iCols : 图像宽               
            INOUT S32 &iGrayMeanBlock : 图像灰度均值 
  输出参数: 无
    返回值: 
    注意点: 
------------------------------------------------------------------------------
    修改历史                                                                  
    日期        姓名             描述                                         
  --------------------------------------------------------------------------
                                                                              
*****************************************************************************/
F32 VD_ColorVar(pU8 pucDatColor, U32 uixs, U32 uixe, U32 uiys, U32 uiye, U32 uiCols, S32 *iGrayMeanBlock)
{
    U32 i = 0, j = 0;
    F32 fSumb = 0, fSumg = 0;
    F32 fSumr = 0, fVarR = 0;
    F32 fVarG = 0, fVarB = 0;
    F32 fR    = 0, fG    = 0, fB = 0;

    /*入参检查*/
    if ( NULL == pucDatColor )
    {
        printf("add buf to list error\n");
        return DSP_VD_PARAPTR_NULL;
    }

    /*将当前大块中所有点的RGB分量各自对应累加*/
    for ( i = uiys; i <= uiye; i++ )
    {
        for ( j = uixs; j <= uixe; j++ )
        {
            fSumb += (F32) ( *(pucDatColor + i*uiCols*3 + j*3) );
            fSumg += (F32) ( *(pucDatColor + i*uiCols*3 + j*3 + 1) );
            fSumr += (F32) ( *(pucDatColor + i*uiCols*3 + j*3 + 2) );
        }
    }

    /*求取三个分量的平均值*/

    fB = fSumb / ( (F32)(uixe-uixs+1) * (F32)(uiye-uiys+1) );
    fG = fSumg / ( (F32)(uixe-uixs+1) * (F32)(uiye-uiys+1) );
    fR = fSumr / ( (F32)(uixe-uixs+1) * (F32)(uiye-uiys+1) );
    
    /*当前大块的灰度均值*/
    *iGrayMeanBlock = ( (S32)fR*299 + (S32)fG*587 + (S32)fB*114 + 500) / 1000;  

    fSumb = 0;
    fSumg = 0;
    fSumr = 0;
    /*计算当前大块中RGB三个分量的方差*/
    for ( i = uiys; i <= uiye; i++ )
    {
        for ( j = uixs; j <= uixe; j++ )
        {
            fSumb += ((F32) *(pucDatColor + i*uiCols*3 + j*3) - fB) * ((F32) *(pucDatColor + i*uiCols*3 + j*3) - fB);
            fSumg += ((F32) *(pucDatColor + i*uiCols*3 + j*3 + 1) - fG) * ((F32) *(pucDatColor + i*uiCols*3 + j*3 + 1) - fG);
            fSumr += ((F32) *(pucDatColor + i*uiCols*3 + j*3 + 2) - fR) * ((F32) *(pucDatColor + i*uiCols*3 + j*3 + 2) - fR);
        }
    }
    if(0 != (uixe-uixs+1)*(uiye-uiys+1)-1)
    {
        fVarB = fSumb / ( (F32)(uixe-uixs+1) * (F32)(uiye-uiys+1) - 1 );
        fVarG = fSumg / ( (F32)(uixe-uixs+1) * (F32)(uiye-uiys+1) - 1 );
        fVarR = fSumr / ( (F32)(uixe-uixs+1) * (F32)(uiye-uiys+1) - 1 );
    }

    return sqrt( (F32) (fVarR+fVarG+fVarB) );
}

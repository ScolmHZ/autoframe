#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ia_interface.h"
#include "ia_subfunc.h"
#include "imp_algo_type.h"
#include "imp_algo_urp_param.h"
#include "imp_avd_para.h"
#include <time.h>
#ifdef _XM_IA_lINUX_
#include <math.h>
#include <unistd.h>
#include <time.h>
#endif

#define iImageWid (352)
#define iImageHgt (288)

#define MAX_IMAGE_WIDTH   1280
#define MAX_IMAGE_HEIGHT  720

//int framenum = 0;
void *pvHandle;
//IVE_IMAGE_S stImgTmp;
//IVE_IMAGE_S stIveImage;
URP_PARA_S stURPpara;
//IMP_AVD_PARA_S stAvdPara;


int QPMapInit()
{
    HI_S32 s32Ret;

    //s32Ret = XM_IA_Init(iImageWid, iImageHgt, IMP_OSC_AGLO_MODULE, &pvHandle);
    s32Ret = XM_IA_Init(iImageWid, iImageHgt, IMP_PEA_AGLO_MODULE, &pvHandle);
	
    if (s32Ret != XM_SUCCESS)
    {
	    printf ("XM_IA_Init Failed!\n");
    }
    //printf("a:%d pvHandle: %x\n", s32Ret, *((int*)pvHandle));

    memset(&stURPpara, 0, sizeof(URP_PARA_S));
    //memset(&stAvdPara, 0, sizeof(IMP_AVD_PARA_S));

    //stAvdPara.stChangePara.u32AlarmLevel = 5;
    //stAvdPara.stChangePara.u32AlarmLevel = 5;

#if 0
    stURPpara.stRuleSet.astRule[0].u32Mode = IMP_FUNC_OBJSTOLEN;
    stURPpara.stRuleSet.astRule[0].stPara.stOscRulePara.astSpclRgs[0].stOscRg.astPoint[0].s16X = 180 * 8192 / iImageWid;
    stURPpara.stRuleSet.astRule[0].stPara.stOscRulePara.astSpclRgs[0].stOscRg.astPoint[0].s16Y = 240 * 8192 / iImageHgt;
    stURPpara.stRuleSet.astRule[0].stPara.stOscRulePara.astSpclRgs[0].stOscRg.astPoint[2].s16X = 228 * 8192 / iImageWid;
    stURPpara.stRuleSet.astRule[0].stPara.stOscRulePara.astSpclRgs[0].stOscRg.astPoint[2].s16Y = 280 * 8192 / iImageHgt;
    stURPpara.stRuleSet.astRule[0].stPara.stOscRulePara.stOscPara.s32SizeMin = 0;
    stURPpara.stRuleSet.astRule[0].stPara.stOscRulePara.stOscPara.s32SizeMax = 100;
#endif
    
#if 0
    stURPpara.stRuleSet.astRule[0].u32Mode = IMP_FUNC_OSC;
    stURPpara.stRuleSet.astRule[0].stPara.stOscRulePara.astSpclRgs[0].stOscRg.astPoint[0].s16X = 147 * 8192 / iImageWid;
    stURPpara.stRuleSet.astRule[0].stPara.stOscRulePara.astSpclRgs[0].stOscRg.astPoint[0].s16Y = 89 * 8192 / iImageHgt;
    stURPpara.stRuleSet.astRule[0].stPara.stOscRulePara.astSpclRgs[0].stOscRg.astPoint[2].s16X = 228 * 8192 / iImageWid;
    stURPpara.stRuleSet.astRule[0].stPara.stOscRulePara.astSpclRgs[0].stOscRg.astPoint[2].s16Y = 217 * 8192 / iImageHgt;
    stURPpara.stRuleSet.astRule[0].stPara.stOscRulePara.stOscPara.s32SizeMin = 0;
    stURPpara.stRuleSet.astRule[0].stPara.stOscRulePara.stOscPara.s32SizeMax = 90;
    stURPpara.stRuleSet.astRule[0].stPara.stOscRulePara.stOscPara.s32TimeMin = 1;
#endif
    
#if 0
    stURPpara.stRuleSet.astRule[0].u32Mode = IMP_FUNC_PERIMETER;
    stURPpara.stRuleSet.astRule[0].u32Level = 0;//警戒等级
    stURPpara.stRuleSet.astRule[0].stPara.stPerimeterRulePara.stLimitPara.s32MinDist = 0;//移动判定距离百分比
    stURPpara.stRuleSet.astRule[0].stPara.stPerimeterRulePara.s32Mode = 2;
    stURPpara.stRuleSet.astRule[0].stPara.stPerimeterRulePara.stLimitPara.stBoundary.astBoundaryPts[0].s16X = 99 * 8192 / iImageWid;
    stURPpara.stRuleSet.astRule[0].stPara.stPerimeterRulePara.stLimitPara.stBoundary.astBoundaryPts[0].s16Y = 84 * 8192 / iImageHgt;
    
    stURPpara.stRuleSet.astRule[0].stPara.stPerimeterRulePara.stLimitPara.stBoundary.astBoundaryPts[1].s16X = 218 * 8192 / iImageWid;
    stURPpara.stRuleSet.astRule[0].stPara.stPerimeterRulePara.stLimitPara.stBoundary.astBoundaryPts[1].s16Y = 83 * 8192 / iImageHgt;
    
    stURPpara.stRuleSet.astRule[0].stPara.stPerimeterRulePara.stLimitPara.stBoundary.astBoundaryPts[2].s16X = 219 * 8192 / iImageWid;
    stURPpara.stRuleSet.astRule[0].stPara.stPerimeterRulePara.stLimitPara.stBoundary.astBoundaryPts[2].s16Y = 269 * 8192 / iImageHgt;
    
    stURPpara.stRuleSet.astRule[0].stPara.stPerimeterRulePara.stLimitPara.stBoundary.astBoundaryPts[3].s16X = 98 * 8192 / iImageWid;
    stURPpara.stRuleSet.astRule[0].stPara.stPerimeterRulePara.stLimitPara.stBoundary.astBoundaryPts[3].s16Y = 268 * 8192 / iImageHgt;
#endif
    
#if 1
    stURPpara.stRuleSet.astRule[0].u32Mode = IMP_FUNC_TRIPWIRE;
    stURPpara.stRuleSet.astRule[0].stPara.stTripwireRulePara.astLines[0].s32Valid = 1;
    
    stURPpara.stRuleSet.astRule[0].u32Level = 0;//警戒等级
    //stURPpara.stRuleSet.astRule[0].stPara.stTripwireRulePara.astLines[0].s32ForbiddenDirection = 3;
    stURPpara.stRuleSet.astRule[0].stPara.stTripwireRulePara.astLines[0].s32IsDoubleDirection = 1;
    //stURPpara.stRuleSet.astRule[0].stPara.stTripwireRulePara.astLines[0].s32ForbiddenDirection = 1;
    stURPpara.stRuleSet.astRule[0].stPara.stTripwireRulePara.stLimitPara.s32MinDist = 0;//最小移动百分比
    
    stURPpara.stRuleSet.astRule[0].stPara.stTripwireRulePara.astLines[0].stLine.stStartPt.s16X = 150 * 8192 / iImageWid;
    stURPpara.stRuleSet.astRule[0].stPara.stTripwireRulePara.astLines[0].stLine.stStartPt.s16Y = 150 * 8192 / iImageHgt;
    stURPpara.stRuleSet.astRule[0].stPara.stTripwireRulePara.astLines[0].stLine.stEndPt.s16X = 260 * 8192 / iImageWid;
    stURPpara.stRuleSet.astRule[0].stPara.stTripwireRulePara.astLines[0].stLine.stEndPt.s16Y = 150 * 8192 / iImageHgt;//office3拌线坐标2
#endif

    //XM_IA_AvdConfig(pvHandle, &stAvdPara);
    s32Ret = XM_IA_Config(pvHandle, &stURPpara);
	
    if (s32Ret != XM_SUCCESS)
    {
	    printf("XM_IA_Config Failed!\n");
    }

    //(&stIveImage, IVE_IMAGE_TYPE_U8C1, iImageWid, iImageHgt);
	
	  //IVE_CREATE_IMAGE_S(&stImgTmp, IVE_IMAGE_TYPE_U8C1, MAX_IMAGE_WIDTH, MAX_IMAGE_HEIGHT);
	
    return 0;
}

int GetQPMap(INTEL_CODING *pstMapCfg, QP_RESULT* pstQPResult)
{

    HI_S32 s32Ret;
    
    IVE_IMAGE_S stIveImage;

    memcpy (&stIveImage,&pstMapCfg->stIveImage,sizeof (IVE_IMAGE_S));
	
    //XM_Image_Resize(&stImgTmp, &stIveImage);
#ifdef _XM_IA_lINUX_
	  HI_U64 pu64WorkStartPts = 0, pu64WorkEndPts = 0;
	
	  HI_MPI_SYS_GetCurPts(&pu64WorkStartPts);
#endif

    s32Ret = XM_IA_Work(pvHandle, &stIveImage, pstMapCfg, pstQPResult);
	 
	  if (s32Ret != XM_SUCCESS)
    {
	     printf ("XM_IA_Work Failed!\n");
    }
	
	
#ifdef _XM_IA_lINUX_
	HI_MPI_SYS_GetCurPts(&pu64WorkEndPts);
	
//	printf("Work_Time = %ld\n", (long int)(pu64WorkEndPts - pu64WorkStartPts));
#endif	
	
    return 0;
}

int QPMapFinish()
{
    HI_S32 s32Ret;
	
    s32Ret = XM_IA_Destroy(&pvHandle);
	
    if (s32Ret != XM_SUCCESS)
    {
 	    printf ("XM_IA_Destory Failed!\n");
    }
	
    return 0;
}

//
//  "$Id: Imp.h 4 2009-08-17 14:09:00Z liwj $"
//
//  Copyright (c)2008-2008, ZheJiang JuFeng Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//
#ifndef __XM_IA_INTERFACE_H__
#define __XM_IA_INTERFACE_H__

#include "hi_comm_ive.h"
#include "hi_type.h"
#include "hi_ive.h"
#include "imp_algo_type.h"
#include "imp_algo_urp_param.h"
#include "imp_avd_para.h"
#include "ia_struct_para.h"

#ifdef _XM_IA_lINUX_
#include "hi_comm_sys.h"
#include "mpi_sys.h"
#endif


#ifdef __cplusplus
extern "C" {
#endif


#define XM_MALLOC_FAIL (1)
#define XM_SUCCESS (0)
#define XM_FREE_FAIL (2)
#define XM_NULL_POINTER (3)
#define XM_HISI_IVE_FAIL (4)
#define XM_PARA_FAULT (5)

int QPMapInit();

int GetQPMap(INTEL_CODING *pstMapCfg, QP_RESULT* pstQPResult);

int QPMapFinish();

int XM_Video_ENDEC(XM_INIT_PARA *pstInitPara, INTEL_CODING *pstMapCfg, QP_RESULT* stQPResult);

int XM_Image_Resize(IVE_IMAGE_S *pstImgI, IVE_IMAGE_S *pstImgO);

/// 初始化智能算法
/// \param [in] iImageWid 图像宽度
/// \param [in] iImageHgt 图像高度
/// \param [in] eAlgType  算法类型
/// \param [out] pvHandle 输出句柄
/// \return 0 初始化成功
/// \return >0初始化失败
int XM_IA_Init(int iImageWid, int iImageHgt, ALGO_MODULE_E eAlgType, void **pvHandle);


/// 智能算法规则配置函数
/// \param [in] pvHandle   输出句柄
/// \param [in] pstURPpara 规则参数
/// \return 0  成功
/// \return >0 失败
int XM_IA_Config(void *pvHandle, URP_PARA_S *pstURPpara);


/// 智能算法工作主函数
/// \param [in] pvHandle  输出句柄
/// \param [in] pstImage   图像宽度
/// \return 0  成功
/// \return >0 失败
int XM_IA_AvdConfig(void *pvHandle, IMP_AVD_PARA_S *stAvdPara);

int XM_IA_Work(void *pvHandle, IVE_IMAGE_S *pstImage, INTEL_CODING *pstMapCfg, QP_RESULT* stQPResult);

/// 智能算法获取结果函数
/// \param [in]  pvHandle  输出句柄
/// \param [out] pstResult 结果参数
/// \return 0  成功
/// \return >0 失败
int XM_IA_GetResults(void *pvHandle, RESULT_S *pstResult);

/// 智能算法销毁函数
/// \param [in] pvHandle  输出句柄
/// \return 0  成功
/// \return >0 失败
int XM_IA_Destroy(void **pvHandle);


#ifdef __cplusplus
}
#endif

#endif //__XM_IA_INTERFACE_H__
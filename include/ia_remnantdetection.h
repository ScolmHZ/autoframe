
#ifndef _IA_REMNANT
#define _IA_REMNANT

#include "hi_comm_ive.h"
#include "hi_type.h"
#include "hi_ive.h"
#include "ia_struct_para.h"

#ifdef _XM_IA_lINUX_
#include "hi_comm_sys.h"
#include "mpi_sys.h"
#endif


#ifdef __cplusplus
extern "C" {
#endif


HI_S32 xmRctInRct(IMP_RECT_S stObjRct, IVE_RECT_S stDetectionRct);

HI_S32 xmStatPixelInFgImg(IMP_RECT_S stObjRct, IVE_IMAGE_S *pstFgImg);

HI_S32 xmBlob(XM_INIT_PARA *pstInitPara,IVE_CCBLOB_S *pstBlob);

HI_S32 xmRemnantDetection(XM_INIT_PARA *pstInitPara, IVE_CCLRECT_S stCclRect, IVE_IMAGE_S *pstImageGmmFg,int i);


#ifdef __cplusplus
}
#endif


#endif
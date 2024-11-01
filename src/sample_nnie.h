#ifndef __SAMPLE_NNIE_H__
#define __SAMPLE_NNIE_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#include "hi_type.h"
#include "sample_comm_nnie.h"
#include "sample_svp_nnie_software.h"


/******************************************************************************
* function : Cnn engine interface
******************************************************************************/
HI_S32 SVP_NNIE_CnnInit(SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam, YOLOV3_SOFTWARE_PARAM_S *pstSoftWareParam, SAMPLE_SVP_NNIE_MODEL_S *pstNnieModel, HI_CHAR *pu8Model);
HI_S32 SVP_NNIE_CnnProcess(SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam, YOLOV3_SOFTWARE_PARAM_S *pstSoftWareParam, SAMPLE_SVP_NNIE_MODEL_S *pstNnieModel, HI_CHAR *pu8Data);
HI_S32 SVP_NNIE_CnnDeinit(SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam, YOLOV3_SOFTWARE_PARAM_S *pstSoftWareParam, SAMPLE_SVP_NNIE_MODEL_S *pstNnieModel);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __SAMPLE_NNIE_H__ */


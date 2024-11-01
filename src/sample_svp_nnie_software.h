#ifndef _SAMPLE_SVP_USER_KERNEL_H_
#define _SAMPLE_SVP_USER_KERNEL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "hi_comm_svp.h"
#include "hi_nnie.h"
#include "mpi_nnie.h"
#include "sample_comm_svp.h"
#include "sample_comm_nnie.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


#define MAX(a,b)           (((a)>(b))?(a):(b))
#define MIN(a,b)           (((a)<(b))?(a):(b))
#define SIGMOID(x)         (HI_FLOAT)(1.0f/(1+exp(-x)))

#define QUANT_BASE         (4096)


/*stack for sort*/
typedef struct hiSAMPLE_SVP_NNIE_STACK_S
{
    HI_S32 s32Min;
    HI_S32 s32Max;
}SAMPLE_SVP_NNIE_STACK_S;

typedef struct hiYOLOV3_BOX_S
{
    HI_FLOAT f32Xmin;
    HI_FLOAT f32Xmax;
    HI_FLOAT f32Ymin;
    HI_FLOAT f32Ymax;
    HI_FLOAT f32ClsScore;
    HI_U32 u32MaxScoreIndex;
    HI_U32 u32Mask;
}YOLOV3_BOX_S;

typedef struct hiYOLOV3_BOX_RESULT_INFO_S
{
    HI_U32 u32OriImgHeight;
    HI_U32 u32OriImgWidth;
    HI_U32 u32BoxResultNum;
    YOLOV3_BOX_S stYoloV3Boxs[100];
}YOLOV3_BOX_RESULT_INFO_S;

// Detection initialization parameters
typedef enum agDETECT_TYPE_E
{
    DETECT_XXX_YOLOV3      = 0,
    DETECT_TYPE_MAX_NUM
}DETECT_TYPE_E;

/*Yolov3 software parameter*/
typedef struct hiYOLOV3_SOFTWARE_PARAM_S
{
    HI_U32 u32NetHeight;
    HI_U32 u32NetWidth;
    HI_U32 u32BoxNumOfGrid; //3
    HI_U32 u32ClassNum;
    HI_U32 u32ParamNum;
    HI_U32 u32ChannelNum;
    HI_U32 u32MaxBoxNumOfScale; //100
    HI_U32 u32StepNum; //no more than 3
    HI_FLOAT f32ConfThres; //0.5
    HI_FLOAT f32NmsThres; //0.5
    HI_FLOAT af32Anchors[3][6];
    HI_U32 au32Steps[3];
    //memory needed by post-process of getting detection result
	SVP_MEM_INFO_S stResultMem;
    YOLOV3_BOX_RESULT_INFO_S stBoxResultInfo;
    DETECT_TYPE_E enDetectType;
}YOLOV3_SOFTWARE_PARAM_S;

/*YOLOV3*/
HI_S32 YoloV3GetResultMem(YOLOV3_SOFTWARE_PARAM_S* pstSoftwareParam);
HI_S32 YoloV3GetResult(SAMPLE_SVP_NNIE_PARAM_S* pstNnieParam, YOLOV3_SOFTWARE_PARAM_S* pstSoftwareParam);

#ifdef __cplusplus
}
#endif

#endif /* _SAMPLE_SVP_USER_KERNEL_H_ */


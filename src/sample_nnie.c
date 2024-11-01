#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <math.h>

#include "hi_common.h"
#include "hi_comm_sys.h"
#include "hi_comm_svp.h"
#include "sample_comm.h"
#include "sample_comm_svp.h"
#include "sample_comm_nnie.h"
#include "sample_comm_ive.h"

#include "sample_nnie.h"
#include "sample_svp_nnie_software.h"

#if MSGINFO
#include "MyTimeC.h"
#endif


#ifdef SAMPLE_SVP_NNIE_PERF_STAT
#define SAMPLE_SVP_NNIE_PERF_STAT_OP_FORWARD_CLREAR()  memset(&s_stOpForwardPerfTmp,0,sizeof(s_stOpForwardPerfTmp));
#define SAMPLE_SVP_NNIE_PERF_STAT_OP_FORWARD_SRC_FLUSH_TIME() SAMPLE_SVP_NNIE_PERF_STAT_GET_DIFF_TIME(s_stOpForwardPerfTmp.u64SrcFlushTime)
#define SAMPLE_SVP_NNIE_PERF_STAT_OP_FORWARD_PRE_DST_FLUSH_TIME() SAMPLE_SVP_NNIE_PERF_STAT_GET_DIFF_TIME(s_stOpForwardPerfTmp.u64PreDstFulshTime)
#define SAMPLE_SVP_NNIE_PERF_STAT_OP_FORWARD_AFTER_DST_FLUSH_TIME() SAMPLE_SVP_NNIE_PERF_STAT_GET_DIFF_TIME(s_stOpForwardPerfTmp.u64AferDstFulshTime)
#define SAMPLE_SVP_NNIE_PERF_STAT_OP_FORWARD_OP_TIME() SAMPLE_SVP_NNIE_PERF_STAT_GET_DIFF_TIME(s_stOpForwardPerfTmp.u64OPTime)
static SAMPLE_SVP_NNIE_OP_PERF_STAT_S   s_stOpForwardPerfTmp = {0};
extern SAMPLE_SVP_NNIE_OP_PERF_STAT_S   g_stOpRpnPerfTmp;
#else
#define SAMPLE_SVP_NNIE_PERF_STAT_OP_FORWARD_CLREAR()
#define SAMPLE_SVP_NNIE_PERF_STAT_OP_FORWARD_SRC_FLUSH_TIME()
#define SAMPLE_SVP_NNIE_PERF_STAT_OP_FORWARD_PRE_DST_FLUSH_TIME()
#define SAMPLE_SVP_NNIE_PERF_STAT_OP_FORWARD_AFTER_DST_FLUSH_TIME()
#define SAMPLE_SVP_NNIE_PERF_STAT_OP_FORWARD_OP_TIME()
#endif

/******************************************************************************
* function : NNIE Forward
******************************************************************************/
static HI_S32 SAMPLE_SVP_NNIE_Forward(SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam,
    SAMPLE_SVP_NNIE_INPUT_DATA_INDEX_S* pstInputDataIdx,
    SAMPLE_SVP_NNIE_PROCESS_SEG_INDEX_S* pstProcSegIdx,HI_BOOL bInstant)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 i = 0, j = 0;
    HI_BOOL bFinish = HI_FALSE;
    SVP_NNIE_HANDLE hSvpNnieHandle = 0;
    HI_U32 u32TotalStepNum = 0;
    SAMPLE_SVP_NIE_PERF_STAT_DEF_VAR()

    SAMPLE_SVP_NNIE_PERF_STAT_OP_FORWARD_CLREAR()

    SAMPLE_COMM_SVP_FlushCache(pstNnieParam->astForwardCtrl[pstProcSegIdx->u32SegIdx].stTskBuf.u64PhyAddr,
        SAMPLE_SVP_NNIE_CONVERT_64BIT_ADDR(HI_VOID,pstNnieParam->astForwardCtrl[pstProcSegIdx->u32SegIdx].stTskBuf.u64VirAddr),
        pstNnieParam->astForwardCtrl[pstProcSegIdx->u32SegIdx].stTskBuf.u32Size);

    SAMPLE_SVP_NNIE_PERF_STAT_BEGIN()
    for(i = 0; i < pstNnieParam->astForwardCtrl[pstProcSegIdx->u32SegIdx].u32DstNum; i++)
    {
        if(SVP_BLOB_TYPE_SEQ_S32 == pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].enType)
        {
            for(j = 0; j < pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u32Num; j++)
            {
                u32TotalStepNum += *(SAMPLE_SVP_NNIE_CONVERT_64BIT_ADDR(HI_U32,pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].unShape.stSeq.u64VirAddrStep)+j);
            }
            SAMPLE_COMM_SVP_FlushCache(pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u64PhyAddr,
                SAMPLE_SVP_NNIE_CONVERT_64BIT_ADDR(HI_VOID,pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u64VirAddr),
                u32TotalStepNum*pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u32Stride);

        }
        else
        {
            SAMPLE_COMM_SVP_FlushCache(pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u64PhyAddr,
                SAMPLE_SVP_NNIE_CONVERT_64BIT_ADDR(HI_VOID,pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u64VirAddr),
                pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u32Num*
                pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].unShape.stWhc.u32Chn*
                pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].unShape.stWhc.u32Height*
                pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u32Stride);
        }
    }
    SAMPLE_SVP_NNIE_PERF_STAT_END()
    SAMPLE_SVP_NNIE_PERF_STAT_OP_FORWARD_PRE_DST_FLUSH_TIME()

    /*set input blob according to node name*/
    if(pstInputDataIdx->u32SegIdx != pstProcSegIdx->u32SegIdx)
    {
        for(i = 0; i < pstNnieParam->pstModel->astSeg[pstProcSegIdx->u32SegIdx].u16SrcNum; i++)
        {
            for(j = 0; j < pstNnieParam->pstModel->astSeg[pstInputDataIdx->u32SegIdx].u16DstNum; j++)
            {
                if(0 == strncmp(pstNnieParam->pstModel->astSeg[pstInputDataIdx->u32SegIdx].astDstNode[j].szName,
                    pstNnieParam->pstModel->astSeg[pstProcSegIdx->u32SegIdx].astSrcNode[i].szName,
                    SVP_NNIE_NODE_NAME_LEN))
                {
                    pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astSrc[i] =
                        pstNnieParam->astSegData[pstInputDataIdx->u32SegIdx].astDst[j];
                    break;
                }
            }
            SAMPLE_SVP_CHECK_EXPR_RET((j == pstNnieParam->pstModel->astSeg[pstInputDataIdx->u32SegIdx].u16DstNum),
                HI_FAILURE,SAMPLE_SVP_ERR_LEVEL_ERROR,"Error,can't find %d-th seg's %d-th src blob!\n",
                pstProcSegIdx->u32SegIdx,i);
        }
    }

    /*NNIE_Forward*/
    SAMPLE_SVP_NNIE_PERF_STAT_BEGIN()
    s32Ret = HI_MPI_SVP_NNIE_Forward(&hSvpNnieHandle,
        pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astSrc,
        pstNnieParam->pstModel, pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst,
        &pstNnieParam->astForwardCtrl[pstProcSegIdx->u32SegIdx], bInstant);
    SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
        "Error,HI_MPI_SVP_NNIE_Forward failed!\n");

    if(bInstant)
    {
        /*Wait NNIE finish*/
        while(HI_ERR_SVP_NNIE_QUERY_TIMEOUT == (s32Ret = HI_MPI_SVP_NNIE_Query(pstNnieParam->astForwardCtrl[pstProcSegIdx->u32SegIdx].enNnieId,
            hSvpNnieHandle, &bFinish, HI_TRUE)))
        {
            usleep(100);
            SAMPLE_SVP_TRACE(SAMPLE_SVP_ERR_LEVEL_INFO,
                "HI_MPI_SVP_NNIE_Query Query timeout!\n");
        }
    }
    SAMPLE_SVP_NNIE_PERF_STAT_END()
    SAMPLE_SVP_NNIE_PERF_STAT_OP_FORWARD_OP_TIME()
    u32TotalStepNum = 0;

    SAMPLE_SVP_NNIE_PERF_STAT_BEGIN()
    for(i = 0; i < pstNnieParam->astForwardCtrl[pstProcSegIdx->u32SegIdx].u32DstNum; i++)
    {
        if(SVP_BLOB_TYPE_SEQ_S32 == pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].enType)
        {
            for(j = 0; j < pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u32Num; j++)
            {
                u32TotalStepNum += *(SAMPLE_SVP_NNIE_CONVERT_64BIT_ADDR(HI_U32,pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].unShape.stSeq.u64VirAddrStep)+j);
            }
            SAMPLE_COMM_SVP_FlushCache(pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u64PhyAddr,
                SAMPLE_SVP_NNIE_CONVERT_64BIT_ADDR(HI_VOID,pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u64VirAddr),
                u32TotalStepNum*pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u32Stride);

        }
        else
        {
            SAMPLE_COMM_SVP_FlushCache(pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u64PhyAddr,
                SAMPLE_SVP_NNIE_CONVERT_64BIT_ADDR(HI_VOID,pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u64VirAddr),
                pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u32Num*
                pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].unShape.stWhc.u32Chn*
                pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].unShape.stWhc.u32Height*
                pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u32Stride);
        }
    }
    SAMPLE_SVP_NNIE_PERF_STAT_END()
    SAMPLE_SVP_NNIE_PERF_STAT_OP_FORWARD_AFTER_DST_FLUSH_TIME()

    return s32Ret;
}

/******************************************************************************
* function : Fill Src Data
******************************************************************************/
static HI_S32 SAMPLE_SVP_NNIE_FillSrcData(HI_CHAR *pu8Data,
    SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam, SAMPLE_SVP_NNIE_INPUT_DATA_INDEX_S* pstInputDataIdx)
{
    HI_U32 i = 0, j = 0, n = 0;
    HI_U32 u32Height = 0, u32Width = 0, u32Chn = 0, u32Stride = 0, u32Dim = 0;
    HI_U32 u32VarSize = 0;
    HI_U8  *pu8PicAddr = NULL;
    HI_U32 *pu32StepAddr = NULL;
    HI_U32 u32SegIdx = pstInputDataIdx->u32SegIdx;
    HI_U32 u32NodeIdx = pstInputDataIdx->u32NodeIdx;
    HI_U32 u32TotalStepNum = 0;

    /*check data*/
    SAMPLE_SVP_CHECK_EXPR_RET(NULL == pu8Data,HI_INVALID_VALUE,SAMPLE_SVP_ERR_LEVEL_ERROR,
        "Error, src data null!\n");

    /*get data size*/
    if(SVP_BLOB_TYPE_U8 <= pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].enType &&
        SVP_BLOB_TYPE_YVU422SP >= pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].enType)
    {
        u32VarSize = sizeof(HI_U8);
    }
    else
    {
        u32VarSize = sizeof(HI_U32);
    }

    /*fill src data*/
    if(SVP_BLOB_TYPE_SEQ_S32 == pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].enType)
    {
        u32Dim = pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].unShape.stSeq.u32Dim;
        u32Stride = pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u32Stride;
        pu32StepAddr = SAMPLE_SVP_NNIE_CONVERT_64BIT_ADDR(HI_U32,pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].unShape.stSeq.u64VirAddrStep);
        pu8PicAddr = SAMPLE_SVP_NNIE_CONVERT_64BIT_ADDR(HI_U8,pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u64VirAddr);
        for(n = 0; n < pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u32Num; n++)
        {
            for(i = 0;i < *(pu32StepAddr+n); i++)
            {
                memcpy(pu8PicAddr, pu8Data, u32Dim * u32VarSize);
                pu8PicAddr += u32Stride;
                pu8Data += u32Dim * u32VarSize;
            }
            u32TotalStepNum += *(pu32StepAddr+n);
        }
        SAMPLE_COMM_SVP_FlushCache(pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u64PhyAddr,
            SAMPLE_SVP_NNIE_CONVERT_64BIT_ADDR(HI_VOID,pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u64VirAddr),
            u32TotalStepNum*u32Stride);
    }
    else
    {
        u32Height = pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].unShape.stWhc.u32Height;
        u32Width = pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].unShape.stWhc.u32Width;
        u32Chn = pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].unShape.stWhc.u32Chn;
        u32Stride = pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u32Stride;
        pu8PicAddr = SAMPLE_SVP_NNIE_CONVERT_64BIT_ADDR(HI_U8,pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u64VirAddr);
        if(SVP_BLOB_TYPE_YVU420SP== pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].enType)
        {
            for(n = 0; n < pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u32Num; n++)
            {
                for(i = 0; i < u32Chn*u32Height/2; i++)
                {
                    memcpy(pu8PicAddr, pu8Data, u32Width * u32VarSize);
                    pu8PicAddr += u32Stride;
                    pu8Data += u32Width * u32VarSize;
                }
            }
        }
        else if(SVP_BLOB_TYPE_YVU422SP== pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].enType)
        {
            for(n = 0; n < pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u32Num; n++)
            {
                for(i = 0; i < u32Height*2; i++)
                {
                    memcpy(pu8PicAddr, pu8Data, u32Width * u32VarSize);
                    pu8PicAddr += u32Stride;
                    pu8Data += u32Width * u32VarSize;
                }
            }
        }
        else
        {
            for(n = 0; n < pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u32Num; n++)
            {
                for(i = 0;i < u32Chn; i++)
                {
                    for(j = 0; j < u32Height; j++)
                    {
                        memcpy(pu8PicAddr, pu8Data, u32Width * u32VarSize);
                        pu8PicAddr += u32Stride;
                        pu8Data += u32Width * u32VarSize;
                    }
                }
            }
        }
        SAMPLE_COMM_SVP_FlushCache(pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u64PhyAddr,
            SAMPLE_SVP_NNIE_CONVERT_64BIT_ADDR(HI_VOID,pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u64VirAddr),
            pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u32Num*u32Chn*u32Height*u32Stride);
    }

    return HI_SUCCESS;
}

/******************************************************************************
* function : Cnn Deinit
******************************************************************************/
static HI_S32 SAMPLE_SVP_NNIE_Cnn_Deinit(SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam,
    YOLOV3_SOFTWARE_PARAM_S* pstSoftWareParam,SAMPLE_SVP_NNIE_MODEL_S* pstNnieModel)
{
    HI_S32 s32Ret = HI_SUCCESS;
    /*hardware para deinit*/
    if(pstNnieParam!=NULL)
    {
        s32Ret = SAMPLE_COMM_SVP_NNIE_ParamDeinit(pstNnieParam);
        SAMPLE_SVP_CHECK_EXPR_TRACE(HI_SUCCESS != s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
            "Error,SAMPLE_COMM_SVP_NNIE_ParamDeinit failed!\n");
    }
    /*software para deinit*/
    if(pstSoftWareParam!=NULL)
    {
        if(0 != pstSoftWareParam->stResultMem.u64PhyAddr && 0 != pstSoftWareParam->stResultMem.u64VirAddr)
        {
            SAMPLE_SVP_MMZ_FREE(pstSoftWareParam->stResultMem.u64PhyAddr, pstSoftWareParam->stResultMem.u64VirAddr);
            pstSoftWareParam->stResultMem.u64PhyAddr = 0;
            pstSoftWareParam->stResultMem.u64VirAddr = 0;
            pstSoftWareParam->stResultMem.u32Size = 0;
        }
    }
    /*model deinit*/
    if(pstNnieModel!=NULL)
    {
        s32Ret = SAMPLE_COMM_SVP_NNIE_UnloadModel(pstNnieModel);
        SAMPLE_SVP_CHECK_EXPR_TRACE(HI_SUCCESS != s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
            "Error,SAMPLE_COMM_SVP_NNIE_UnloadModel failed!\n");
    }

    return s32Ret;
}

/******************************************************************************
* function : Cnn init
******************************************************************************/
static HI_S32 SAMPLE_SVP_NNIE_Cnn_ParamInit(SAMPLE_SVP_NNIE_CFG_S* pstNnieCfg,
    SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam, YOLOV3_SOFTWARE_PARAM_S* pstSoftWareParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    /*init hardware para*/
    s32Ret = SAMPLE_COMM_SVP_NNIE_ParamInit(pstNnieCfg,pstNnieParam);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,INIT_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
        "Error(%#x),SAMPLE_COMM_SVP_NNIE_ParamInit failed!\n",s32Ret);

    /*init software para*/
    if(pstSoftWareParam!=NULL)
    {
        pstSoftWareParam->u32NetHeight = pstNnieParam->astSegData[0].astSrc[0].unShape.stWhc.u32Height;
        pstSoftWareParam->u32NetWidth = pstNnieParam->astSegData[0].astSrc[0].unShape.stWhc.u32Width;
        pstSoftWareParam->u32StepNum = pstNnieParam->pstModel->astSeg[0].u16DstNum;
        pstSoftWareParam->u32MaxBoxNumOfScale = 100;
        pstSoftWareParam->f32ConfThres = 0.5;
        pstSoftWareParam->f32NmsThres = 0.5;
        if(pstSoftWareParam->enDetectType == DETECT_XXX_YOLOV3)
        {
            pstSoftWareParam->u32BoxNumOfGrid = 3;
            pstSoftWareParam->u32ClassNum = 1;
            pstSoftWareParam->u32ParamNum = 5 + pstSoftWareParam->u32ClassNum;
            pstSoftWareParam->u32ChannelNum = pstSoftWareParam->u32BoxNumOfGrid * pstSoftWareParam->u32ParamNum;
            HI_FLOAT af32Anchors[3][6] = {
                {88, 88, 88, 88, 88, 88},
                {88, 88, 88, 88, 88, 88},
                {88, 88, 88, 88, 88, 88}
            };
            memcpy(pstSoftWareParam->af32Anchors, af32Anchors, sizeof(af32Anchors));
            pstSoftWareParam->au32Steps[0] = 32;
            pstSoftWareParam->au32Steps[1] = 16;
            pstSoftWareParam->au32Steps[1] = 8;
            pstSoftWareParam->stBoxResultInfo.u32OriImgHeight = 1080;
            pstSoftWareParam->stBoxResultInfo.u32OriImgWidth = 1920;
        }
        else
        {
            fprintf(stderr, "unknown detect type (%s:%d)!\n", __FUNCTION__, __LINE__);
            exit(-1);
        }

        s32Ret = YoloV3GetResultMem(pstSoftWareParam);
    }

    return s32Ret;

INIT_FAIL_0:
    s32Ret = SAMPLE_SVP_NNIE_Cnn_Deinit(pstNnieParam,pstSoftWareParam,NULL);
    SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
            "Error(%#x),SAMPLE_SVP_NNIE_Cnn_Deinit failed!\n",s32Ret);
    return HI_FAILURE;
}



HI_S32 SVP_NNIE_CnnInit(SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam, YOLOV3_SOFTWARE_PARAM_S *pstSoftWareParam, SAMPLE_SVP_NNIE_MODEL_S *pstNnieModel, HI_CHAR *pu8Model)
{
#if MSGINFO
    MyTimeC myTimeC;
    init(&myTimeC);
#endif
    HI_S32 s32Ret = HI_SUCCESS;

    /*Set configuration parameter*/
    SAMPLE_SVP_NNIE_CFG_S stNnieCfg = {0};
    stNnieCfg.u32MaxInputNum = 1; //max input image num in each batch
    stNnieCfg.u32MaxRoiNum = 0;
    stNnieCfg.aenNnieCoreId[0] = SVP_NNIE_ID_0; //set NNIE core

    /*Sys init*/
    //SAMPLE_COMM_SVP_CheckSysInit();

    /*CNN Load model*/
#if MSGINFO
    SAMPLE_SVP_TRACE_INFO("Cnn load model!\n");
    myTimeC.reset(&myTimeC, NULL);
#endif
    s32Ret = SAMPLE_COMM_SVP_NNIE_LoadModel(pu8Model,pstNnieModel);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,CNN_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                               "Error,SAMPLE_COMM_SVP_NNIE_LoadModel failed!\n");
#if MSGINFO
    myTimeC.reset(&myTimeC, "Cnn load model");
#endif

    /*CNN parameter initialization*/
#if MSGINFO
    SAMPLE_SVP_TRACE_INFO("Cnn parameter initialization!\n");
    myTimeC.reset(&myTimeC, NULL);
#endif
    pstNnieParam->pstModel = &pstNnieModel->stModel;
    s32Ret = SAMPLE_SVP_NNIE_Cnn_ParamInit(&stNnieCfg,pstNnieParam,pstSoftWareParam);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,CNN_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                               "Error,SAMPLE_SVP_NNIE_Cnn_ParamInit failed!\n");
#if MSGINFO
    myTimeC.reset(&myTimeC, "Cnn parameter initialization");
#endif

    return s32Ret;

CNN_FAIL_0:
    SAMPLE_SVP_NNIE_Cnn_Deinit(pstNnieParam,pstSoftWareParam,pstNnieModel);
    //SAMPLE_COMM_SVP_CheckSysExit();
    return HI_FAILURE;
}

HI_S32 SVP_NNIE_CnnProcess(SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam, YOLOV3_SOFTWARE_PARAM_S *pstSoftWareParam, SAMPLE_SVP_NNIE_MODEL_S *pstNnieModel, HI_CHAR *pu8Data)
{
#if MSGINFO
    MyTimeC myTimeC;
    init(&myTimeC);
#endif
    HI_S32 s32Ret = HI_SUCCESS;

    /*Sys init*/
    //SAMPLE_COMM_SVP_CheckSysInit();

    /*Fill src data*/
#if MSGINFO
    SAMPLE_SVP_TRACE_INFO("Cnn fill src data!\n");
    myTimeC.reset(&myTimeC, NULL);
#endif
    SAMPLE_SVP_NNIE_INPUT_DATA_INDEX_S stInputDataIdx = {0};
    stInputDataIdx.u32SegIdx = 0;
    stInputDataIdx.u32NodeIdx = 0;
    s32Ret = SAMPLE_SVP_NNIE_FillSrcData(pu8Data,pstNnieParam,&stInputDataIdx);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,CNN_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                               "Error,SAMPLE_SVP_NNIE_FillSrcData failed!\n");
#if MSGINFO
    myTimeC.reset(&myTimeC, "Cnn fill src data");
#endif

    /*NNIE process(process the 0-th segment)*/
#if MSGINFO
    SAMPLE_SVP_TRACE_INFO("Cnn forward!\n");
    myTimeC.reset(&myTimeC, NULL);
#endif
    SAMPLE_SVP_NNIE_PROCESS_SEG_INDEX_S stProcSegIdx = {0};
    stProcSegIdx.u32SegIdx = 0;
    s32Ret = SAMPLE_SVP_NNIE_Forward(pstNnieParam,&stInputDataIdx,&stProcSegIdx,HI_TRUE);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,CNN_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                               "Error,SAMPLE_SVP_NNIE_Forward failed!\n");
#if MSGINFO
    myTimeC.reset(&myTimeC, "Cnn forward");
#endif

    YoloV3GetResult(pstNnieParam, pstSoftWareParam);
#if MSGINFO
    myTimeC.reset(&myTimeC, "Cnn post");
#endif

    return s32Ret;

CNN_FAIL_0:
    SAMPLE_SVP_NNIE_Cnn_Deinit(pstNnieParam,pstSoftWareParam,pstNnieModel);
    //SAMPLE_COMM_SVP_CheckSysExit();
    return HI_FAILURE;
}

HI_S32 SVP_NNIE_CnnDeinit(SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam, YOLOV3_SOFTWARE_PARAM_S *pstSoftWareParam, SAMPLE_SVP_NNIE_MODEL_S *pstNnieModel)
{
#if MSGINFO
    MyTimeC myTimeC;
    init(&myTimeC);
#endif
    HI_S32 s32Ret = HI_SUCCESS;

#if MSGINFO
    SAMPLE_SVP_TRACE_INFO("Cnn unload model and parameter!\n");
    myTimeC.reset(&myTimeC, NULL);
#endif
    SAMPLE_SVP_NNIE_Cnn_Deinit(pstNnieParam,pstSoftWareParam,pstNnieModel);
    //SAMPLE_COMM_SVP_CheckSysExit();
#if MSGINFO
    myTimeC.reset(&myTimeC, "Cnn unload model and parameter");
#endif

    return s32Ret;
}


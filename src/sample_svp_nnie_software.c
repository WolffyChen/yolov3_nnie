#include "sample_svp_nnie_software.h"

#ifdef __cplusplus    // If used by C++ code,
extern "C" {          // we need to export the C interface
#endif


HI_FLOAT GetMaxVal(HI_S32 *ps32Val, HI_U32 h, HI_U32 w, HI_U32 u32Stride, HI_U32 u32OneCSize, HI_U32 u32Num, HI_U32 *pu32MaxValueIndex)
{
    HI_FLOAT f32MaxTmp = (HI_FLOAT)ps32Val[5 * u32OneCSize + h * u32Stride + w] / QUANT_BASE;
    *pu32MaxValueIndex = 0;
    for (HI_U32 i = 1; i < u32Num; i++)
    {
        if ((HI_FLOAT)ps32Val[(i + 5) * u32OneCSize + h * u32Stride + w] / QUANT_BASE > f32MaxTmp)
        {
            f32MaxTmp = (HI_FLOAT)ps32Val[(i + 5) * u32OneCSize + h * u32Stride + w] / QUANT_BASE;
            *pu32MaxValueIndex = i;
        }
    }

    return f32MaxTmp;
}

HI_S32 BoxArgswap(YOLOV3_BOX_S *pstBox1, YOLOV3_BOX_S *pstBox2)
{
    YOLOV3_BOX_S stBoxTmp = { 0 };

    memcpy(&stBoxTmp, pstBox1, sizeof(stBoxTmp));
    memcpy(pstBox1, pstBox2, sizeof(stBoxTmp));
    memcpy(pstBox2, &stBoxTmp, sizeof(stBoxTmp));

    return HI_SUCCESS;
}

HI_S32 NonRecursiveArgQuickSortWithBox(YOLOV3_BOX_S *pstBoxs, HI_S32 s32Low, HI_S32 s32High, SAMPLE_SVP_NNIE_STACK_S *pstStack)
{
    HI_S32 i = s32Low;
    HI_S32 j = s32High;
    HI_S32 s32Top = 0;

    pstStack[s32Top].s32Min = s32Low;
    pstStack[s32Top].s32Max = s32High;

    HI_FLOAT f32KeyConfidence = pstBoxs[s32Low].f32ClsScore;

    while (s32Top > -1)
    {
        s32Low = pstStack[s32Top].s32Min;
        s32High = pstStack[s32Top].s32Max;
        i = s32Low;
        j = s32High;
        s32Top--;

        f32KeyConfidence = pstBoxs[s32Low].f32ClsScore;

        while (i < j)
        {
            while ((i < j) && (f32KeyConfidence > pstBoxs[j].f32ClsScore))
            {
                j--;
            }
            if (i < j)
            {
                BoxArgswap(&pstBoxs[i], &pstBoxs[j]);
                i++;
            }

            while ((i < j) && (f32KeyConfidence < pstBoxs[i].f32ClsScore))
            {
                i++;
            }
            if (i < j)
            {
                BoxArgswap(&pstBoxs[i], &pstBoxs[j]);
                j--;
            }
        }

        if (s32Low < i - 1)
        {
            s32Top++;
            pstStack[s32Top].s32Min = s32Low;
            pstStack[s32Top].s32Max = i - 1;
        }

        if (s32High > i + 1)
        {
            s32Top++;
            pstStack[s32Top].s32Min = i + 1;
            pstStack[s32Top].s32Max = s32High;
        }
    }

    return HI_SUCCESS;
}

HI_DOUBLE SvpDetYoloCalIou(YOLOV3_BOX_S *pstBox1, YOLOV3_BOX_S *pstBox2)
{
    // check the input
    HI_FLOAT f32XMin = MAX(pstBox1->f32Xmin, pstBox2->f32Xmin);
    HI_FLOAT f32YMin = MAX(pstBox1->f32Ymin, pstBox2->f32Ymin);
    HI_FLOAT f32XMax = MIN(pstBox1->f32Xmax, pstBox2->f32Xmax);
    HI_FLOAT f32YMax = MIN(pstBox1->f32Ymax, pstBox2->f32Ymax);

    HI_FLOAT InterWidth = f32XMax - f32XMin;
    HI_FLOAT InterHeight = f32YMax - f32YMin;

    if (InterWidth <= 0 || InterHeight <= 0) return HI_SUCCESS;

    HI_DOUBLE f64InterArea = InterWidth * InterHeight;
    HI_DOUBLE f64Box1Area = (pstBox1->f32Xmax - pstBox1->f32Xmin) * (pstBox1->f32Ymax - pstBox1->f32Ymin);
    HI_DOUBLE f64Box2Area = (pstBox2->f32Xmax - pstBox2->f32Xmin) * (pstBox2->f32Ymax - pstBox2->f32Ymin);

    HI_DOUBLE f64UnionArea = f64Box1Area + f64Box2Area - f64InterArea;

    return f64InterArea / f64UnionArea;
}

HI_S32 SvpDetYoloNonMaxSuppression(YOLOV3_BOX_S *pstBoxs, HI_U32 u32BoxNum, HI_FLOAT f32NmsThresh, HI_U32 u32MaxRoiNum)
{
    for (HI_U32 i = 0, u32Num = 0; i < u32BoxNum && u32Num < u32MaxRoiNum; i++)
    {
        if (0 == pstBoxs[i].u32Mask)
        {
            u32Num++;
            for (HI_U32 j = i + 1; j < u32BoxNum; j++)
            {
                if (0 == pstBoxs[j].u32Mask)
                {
                    HI_DOUBLE f64Iou = SvpDetYoloCalIou(&pstBoxs[i], &pstBoxs[j]);
                    if (f64Iou >= (HI_DOUBLE)f32NmsThresh)
                    {
                        pstBoxs[j].u32Mask = 1;
                    }
                }
            }
        }
    }

    return HI_SUCCESS;
}

HI_S32 YoloV3GetResultAllBlobs(SVP_BLOB_S *pstDstBlob, YOLOV3_SOFTWARE_PARAM_S *pstSoftwareParam)
{
    HI_S32 s32Ret = HI_SUCCESS;

    // result calc para config
    HI_FLOAT f32ScoreFilterThresh = pstSoftwareParam->f32ConfThres;
    HI_FLOAT f32NmsThresh = pstSoftwareParam->f32NmsThres;

    // assist para config
    HI_U32 u32SrcWidth  = pstSoftwareParam->stBoxResultInfo.u32OriImgWidth;
    HI_U32 u32SrcHeight = pstSoftwareParam->stBoxResultInfo.u32OriImgHeight;
    HI_U32 u32MaxBoxNum = pstSoftwareParam->u32MaxBoxNumOfScale;
    HI_U32 u32GridBoxNum = pstSoftwareParam->u32BoxNumOfGrid;

    HI_U32 u32AssistBoxNum = 0;
    HI_U32 u32AssistStackNum = 0;
    for (HI_U32 u32Id = 0; u32Id < pstSoftwareParam->u32StepNum; u32Id++)
    {
        HI_U32 u32GridNumH = pstSoftwareParam->u32NetHeight / pstSoftwareParam->au32Steps[u32Id];
        HI_U32 u32GridNumW = pstSoftwareParam->u32NetWidth / pstSoftwareParam->au32Steps[u32Id];
        u32AssistBoxNum += u32GridNumH * u32GridNumW * u32GridBoxNum;
        u32AssistStackNum += u32GridNumH * u32GridNumW * u32GridBoxNum;
    }

    HI_S32 *ps32ResultMem = (HI_S32*)(pstSoftwareParam->stResultMem.u64VirAddr);
    YOLOV3_BOX_S* pstBox = (YOLOV3_BOX_S*)(ps32ResultMem); //assit_box_size
    SAMPLE_SVP_NNIE_STACK_S* pstAssistStack = (SAMPLE_SVP_NNIE_STACK_S*)(pstBox + u32AssistBoxNum); //assit_size
    YOLOV3_BOX_S *pstBoxResult = (YOLOV3_BOX_S*)(pstAssistStack + u32AssistStackNum); //result_box_size

    HI_U32 u32BoxResultNum = 0;
    HI_U32 u32BoxsNum = 0;
    for (HI_U32 u32Id = 0; u32Id < pstSoftwareParam->u32StepNum; u32Id++)
    {
        for (HI_U32 u32NumIndex = 0; u32NumIndex < pstDstBlob->u32Num; u32NumIndex++)
        {
            HI_U32 u32GridNumH = pstSoftwareParam->u32NetHeight / pstSoftwareParam->au32Steps[u32Id];
            HI_U32 u32GridNumW = pstSoftwareParam->u32NetWidth / pstSoftwareParam->au32Steps[u32Id];

            //printf("n:%u, c:%u, h:%u, w:%u\n", pstDstBlob->u32Num, pstDstBlob->unShape.stWhc.u32Chn, pstDstBlob->unShape.stWhc.u32Height, pstDstBlob->unShape.stWhc.u32Width);
            if ((u32GridNumH != pstDstBlob->unShape.stWhc.u32Height) ||
                (u32GridNumW != pstDstBlob->unShape.stWhc.u32Width)  ||
                (pstSoftwareParam->u32ChannelNum != pstDstBlob->unShape.stWhc.u32Chn))
            {
                printf("error grid number!\n");
                return HI_FAILURE;
            }

            HI_U32 u32Stride = pstDstBlob->u32Stride / 4; // Align16(u32Width)
            HI_U32 u32OneCSize = u32Stride * pstDstBlob->unShape.stWhc.u32Height;
            HI_U32 u32FrameStride = u32OneCSize * pstDstBlob->unShape.stWhc.u32Chn;

            HI_S32* ps32InputData = (HI_S32*)(pstDstBlob->u64VirAddr) + u32NumIndex * u32FrameStride;
            for (HI_U32 n = 0; n < u32GridNumH * u32GridNumW; n++)
            {
                // grid
                HI_U32 w = n % u32GridNumW;
                HI_U32 h = n / u32GridNumW;
                for (HI_U32 k = 0; k < pstSoftwareParam->u32BoxNumOfGrid; k++)
                {
                    HI_U32 u32Index = k * pstSoftwareParam->u32ParamNum * u32OneCSize;

                    HI_FLOAT f32ObjScore = SIGMOID((HI_FLOAT)ps32InputData[u32Index + 4 * u32OneCSize + h * u32Stride + w] / QUANT_BASE); //objscore;

                    HI_U32 u32MaxValueIndex = 0;
                    HI_FLOAT f32MaxScore = 1.0;
                    if (pstSoftwareParam->u32ClassNum > 1)
                    {
                        HI_FLOAT f32MaxVal = GetMaxVal(&ps32InputData[u32Index], h, w, u32Stride, u32OneCSize, pstSoftwareParam->u32ClassNum, &u32MaxValueIndex);
                        f32MaxScore = SIGMOID(f32MaxVal);
                    }

                    HI_FLOAT f32ClassScore = f32MaxScore * f32ObjScore;
                    if (f32ClassScore > f32ScoreFilterThresh) // && width != 0 && height != 0) // filter the low score box
                    {
                        HI_FLOAT x = ((HI_FLOAT)w + SIGMOID((HI_FLOAT)ps32InputData[u32Index + 0 * u32OneCSize + h * u32Stride + w] / QUANT_BASE)) / u32GridNumW;
                        HI_FLOAT y = ((HI_FLOAT)h + SIGMOID((HI_FLOAT)ps32InputData[u32Index + 1 * u32OneCSize + h * u32Stride + w] / QUANT_BASE)) / u32GridNumH;
                        HI_FLOAT f32Width = (HI_FLOAT)(exp((HI_FLOAT)ps32InputData[u32Index + 2 * u32OneCSize + h * u32Stride + w] / QUANT_BASE) * pstSoftwareParam->af32Anchors[u32Id][2 * k]) / pstSoftwareParam->u32NetWidth;
                        HI_FLOAT f32Height = (HI_FLOAT)(exp((HI_FLOAT)ps32InputData[u32Index + 3 * u32OneCSize + h * u32Stride + w] / QUANT_BASE) * pstSoftwareParam->af32Anchors[u32Id][2 * k + 1]) / pstSoftwareParam->u32NetHeight;
                        pstBox[u32BoxsNum].f32Xmin          = x - f32Width * 0.5f;  // xmin
                        pstBox[u32BoxsNum].f32Xmax          = x + f32Width * 0.5f;  // xmax
                        pstBox[u32BoxsNum].f32Ymin          = y - f32Height * 0.5f; // ymin
                        pstBox[u32BoxsNum].f32Ymax          = y + f32Height * 0.5f; // ymax
                        pstBox[u32BoxsNum].f32ClsScore      = f32ClassScore;        // class score
                        pstBox[u32BoxsNum].u32MaxScoreIndex = u32MaxValueIndex;     // max class score index
                        pstBox[u32BoxsNum].u32Mask          = 0;                    // suppression mask

                        u32BoxsNum++;
                    }
                }
            }
        }
        // next step
        pstDstBlob += 1;
    }

    // quick sort
    s32Ret = NonRecursiveArgQuickSortWithBox(pstBox, 0, u32BoxsNum - 1, pstAssistStack);

    // nms
    s32Ret = SvpDetYoloNonMaxSuppression(pstBox, u32BoxsNum, f32NmsThresh, u32MaxBoxNum);

    // get the result
    for (HI_U32 n = 0; (n < u32BoxsNum) && (u32BoxResultNum < u32MaxBoxNum); n++)
    {
        if (0 == pstBox[n].u32Mask)
        {
            pstBoxResult[u32BoxResultNum].f32Xmin = MAX(pstBox[n].f32Xmin * u32SrcWidth, 0);
            pstBoxResult[u32BoxResultNum].f32Xmax = MIN(pstBox[n].f32Xmax * u32SrcWidth, u32SrcWidth - 1);
            pstBoxResult[u32BoxResultNum].f32Ymin = MAX(pstBox[n].f32Ymin * u32SrcHeight, 0);
            pstBoxResult[u32BoxResultNum].f32Ymax = MIN(pstBox[n].f32Ymax * u32SrcHeight, u32SrcHeight - 1);
            pstBoxResult[u32BoxResultNum].f32ClsScore = pstBox[n].f32ClsScore;
            pstBoxResult[u32BoxResultNum].u32MaxScoreIndex = pstBox[n].u32MaxScoreIndex;

            u32BoxResultNum++;
        }
    }

    pstSoftwareParam->stBoxResultInfo.u32BoxResultNum = u32BoxResultNum;
    memcpy(pstSoftwareParam->stBoxResultInfo.stYoloV3Boxs, pstBoxResult, sizeof(YOLOV3_BOX_S) * u32BoxResultNum);

    return s32Ret;
}

HI_U32 YoloV3GetResultMemSize(HI_U32 u32Id, YOLOV3_SOFTWARE_PARAM_S* pstSoftwareParam)
{
    HI_U32 u32GridNumH = pstSoftwareParam->u32NetHeight / pstSoftwareParam->au32Steps[u32Id];
    HI_U32 u32GridNumW = pstSoftwareParam->u32NetWidth / pstSoftwareParam->au32Steps[u32Id];
    HI_U32 u32BoxSize = u32GridNumH * u32GridNumW * pstSoftwareParam->u32BoxNumOfGrid  * sizeof(YOLOV3_BOX_S);
    HI_U32 u32StackSize = u32GridNumH * u32GridNumW * pstSoftwareParam->u32BoxNumOfGrid * sizeof(SAMPLE_SVP_NNIE_STACK_S);
    HI_U32 u32ResultBoxSize = pstSoftwareParam->u32MaxBoxNumOfScale * sizeof(YOLOV3_BOX_S);

    return (u32BoxSize + u32StackSize + u32ResultBoxSize);
}

HI_S32 YoloV3GetResultMem(YOLOV3_SOFTWARE_PARAM_S* pstSoftwareParam)
{
    HI_S32 s32Ret = HI_SUCCESS;

    // yolo v3 special mem init method
    HI_U32 u32ResultMemSize = 0;
    for (HI_U32 i = 0; i < pstSoftwareParam->u32StepNum; i++)
    {
        u32ResultMemSize += YoloV3GetResultMemSize(i, pstSoftwareParam);
    }
    if (0 != u32ResultMemSize)
    {
        HI_U64 u64PhyAddr = 0;
        HI_U8* pu8VirAddr = NULL;
        s32Ret = SAMPLE_COMM_SVP_MallocCached("SAMPLE_YOLOV3_INIT", NULL, (HI_U64*)&u64PhyAddr, (void**)&pu8VirAddr, u32ResultMemSize);
        memset(pu8VirAddr, 0, u32ResultMemSize);
        SAMPLE_COMM_SVP_FlushCache(u64PhyAddr, (void*)pu8VirAddr, u32ResultMemSize);
        pstSoftwareParam->stResultMem.u64PhyAddr = u64PhyAddr;
        pstSoftwareParam->stResultMem.u64VirAddr = (HI_U64)((HI_UL)pu8VirAddr);
        pstSoftwareParam->stResultMem.u32Size = u32ResultMemSize;
    }

    return s32Ret;
}

HI_S32 YoloV3GetResult(SAMPLE_SVP_NNIE_PARAM_S* pstNnieParam, YOLOV3_SOFTWARE_PARAM_S* pstSoftwareParam)
{
    HI_S32 s32Ret = HI_SUCCESS;

    SVP_BLOB_S *pstDstBlob = pstNnieParam->astSegData[0].astDst;
    s32Ret = YoloV3GetResultAllBlobs(pstDstBlob, pstSoftwareParam);

    return s32Ret;
}

#ifdef __cplusplus
}
#endif


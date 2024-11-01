#include <iostream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "sample_nnie.h"
#include "MyTimeC.h"

#include <fstream>
#include <string>

int main()
{
    MyTimeC myTimeC;
    init(&myTimeC);

    HI_S32 s32Ret = HI_SUCCESS;

    /*cnn para*/
    SAMPLE_SVP_NNIE_MODEL_S stCnnModel = {0};
    SAMPLE_SVP_NNIE_PARAM_S stCnnNnieParam = {0};
    YOLOV3_SOFTWARE_PARAM_S stYoloV3SoftwareParam = {0};
    stYoloV3SoftwareParam.enDetectType = DETECT_XXX_YOLOV3;

    FILE *fin_model = fopen("./yolov3-1cls.wk", "rb");
    fseek(fin_model, 0L, SEEK_END);
    long model_len = ftell(fin_model);
    fseek(fin_model, 0L, SEEK_SET);
    char *model = (char*)malloc(model_len * sizeof(char));
    fread(model, model_len, 1, fin_model);
    fclose(fin_model);

    myTimeC.reset(&myTimeC, NULL);
    stCnnModel.stModelBuf.u32Size = model_len;
    s32Ret = SVP_NNIE_CnnInit(&stCnnNnieParam, &stYoloV3SoftwareParam, &stCnnModel, model);
    myTimeC.reset(&myTimeC, "init");
    free(model);

    FILE *fin_data = fopen("./data.bin", "rb");
    fseek(fin_data, 0L, SEEK_END);
    long data_len = ftell(fin_data);
    fseek(fin_data, 0L, SEEK_SET);
    char *data = (char*)malloc(data_len * sizeof(char));
    fread(data, data_len, 1, fin_data);
    fclose(fin_data);

    myTimeC.reset(&myTimeC, NULL);
    s32Ret = SVP_NNIE_CnnProcess(&stCnnNnieParam, &stYoloV3SoftwareParam, &stCnnModel, data);
    myTimeC.reset(&myTimeC, "process");
    free(data);

    int input_num = stCnnNnieParam.pstModel->astSeg[0].u16SrcNum;
    int output_num = stCnnNnieParam.pstModel->astSeg[0].u16DstNum;
    std::cout << input_num << std::endl;
    std::cout << output_num << std::endl;
    char *input_0_name = stCnnNnieParam.pstModel->astSeg[0].astSrcNode[0].szName;
    char *output_0_name = stCnnNnieParam.pstModel->astSeg[0].astDstNode[0].szName;
    char *output_1_name = stCnnNnieParam.pstModel->astSeg[0].astDstNode[1].szName;
    std::cout << input_0_name << std::endl;
    std::cout << output_0_name << std::endl;
    printf("n:%u, c:%u, h:%u, w:%u\n", stCnnNnieParam.astSegData[0].astDst[0].u32Num,
                                        stCnnNnieParam.astSegData[0].astDst[0].unShape.stWhc.u32Chn,
                                        stCnnNnieParam.astSegData[0].astDst[0].unShape.stWhc.u32Height,
                                        stCnnNnieParam.astSegData[0].astDst[0].unShape.stWhc.u32Width);
    std::cout << output_1_name << std::endl;
    printf("n:%u, c:%u, h:%u, w:%u\n", stCnnNnieParam.astSegData[0].astDst[1].u32Num,
                                        stCnnNnieParam.astSegData[0].astDst[1].unShape.stWhc.u32Chn,
                                        stCnnNnieParam.astSegData[0].astDst[1].unShape.stWhc.u32Height,
                                        stCnnNnieParam.astSegData[0].astDst[1].unShape.stWhc.u32Width);

    for(int i = 0; i < 1; ++i)
    {
        printf("*****************%d*****************\n", i);
        printf("%d, %d\n", stYoloV3SoftwareParam.stBoxResultInfo.u32OriImgWidth, stYoloV3SoftwareParam.stBoxResultInfo.u32OriImgHeight);
        printf("%d\n", stYoloV3SoftwareParam.stBoxResultInfo.u32BoxResultNum);
        for (int j = 0; j < (int)stYoloV3SoftwareParam.stBoxResultInfo.u32BoxResultNum; ++j)
        {
            printf("%d  %5.4f  %4.2f  %4.2f  %4.2f  %4.2f\n",
                    stYoloV3SoftwareParam.stBoxResultInfo.stYoloV3Boxs[j].u32MaxScoreIndex,
                    stYoloV3SoftwareParam.stBoxResultInfo.stYoloV3Boxs[j].f32ClsScore,
                    stYoloV3SoftwareParam.stBoxResultInfo.stYoloV3Boxs[j].f32Xmin, stYoloV3SoftwareParam.stBoxResultInfo.stYoloV3Boxs[j].f32Ymin,
                    stYoloV3SoftwareParam.stBoxResultInfo.stYoloV3Boxs[j].f32Xmax, stYoloV3SoftwareParam.stBoxResultInfo.stYoloV3Boxs[j].f32Ymax);
        }
        printf("==============================================================================\n");
    }

    myTimeC.reset(&myTimeC, NULL);
    s32Ret = SVP_NNIE_CnnDeinit(&stCnnNnieParam, &stYoloV3SoftwareParam, &stCnnModel);
    myTimeC.reset(&myTimeC, "deinit");

    return s32Ret;
}


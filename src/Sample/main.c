/*---------------------------------------------------------------------------*/
/* Copyright(C)  2018  OMRON Corporation                                     */
/*                                                                           */
/* Licensed under the Apache License, Version 2.0 (the "License");           */
/* you may not use this file except in compliance with the License.          */
/* You may obtain a copy of the License at                                   */
/*                                                                           */
/*     http://www.apache.org/licenses/LICENSE-2.0                            */
/*                                                                           */
/* Unless required by applicable law or agreed to in writing, software       */
/* distributed under the License is distributed on an "AS IS" BASIS,         */
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  */
/* See the License for the specific language governing permissions and       */
/* limitations under the License.                                            */
/*---------------------------------------------------------------------------*/

#define _CRT_SECURE_NO_WARNINGS
#ifdef WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "uart.h"
#include "HVCApi.h"
#include "HVCDef.h"
#include "HVCExtraUartFunc.h"
#include "STBWrap.h"

#define LOGBUFFERSIZE   16384

#define UART_GENERAL_TIMEOUT              1000
#define UART_EXECUTE_TIMEOUT              ((10+10+6+3+15+15+1+1+15+10)*1000)
                                                          /* HVC execute command signal timeout period */
#define UART_WRITE_ALBUM_TIMEOUT          5000            /* HVC write album command signal timeout period */
#define UART_REFORMAT_ALBUM_TIMEOUT      10000            /* HVC reformat album command signal timeout period */
#define UART_REGIST_COUNT_TIMEOUT        (UART_REFORMAT_ALBUM_TIMEOUT + 1000)            
                                                          /* HVC set regist count command signal timeout period */

#define SENSOR_ROLL_ANGLE_DEFAULT            0            /* Camera angle setting (0Åã) */

#define BODY_THRESHOLD_DEFAULT             500            /* Threshold for Human Body Detection */
#define FACE_THRESHOLD_DEFAULT             500            /* Threshold for Face Detection */
#define HAND_THRESHOLD_DEFAULT             500            /* Threshold for Hand Detection */
#define REC_THRESHOLD_DEFAULT              500            /* Threshold for Face Recognition */
#define VERIFY_THRESHOLD_DEFAULT           500            /* Threshold for Face Detection(Verify) */

#define BODY_SIZE_RANGE_MIN_DEFAULT         30            /* Human Body Detection minimum detection size */
#define BODY_SIZE_RANGE_MAX_DEFAULT       8192            /* Human Body Detection maximum detection size */
#define HAND_SIZE_RANGE_MIN_DEFAULT         40            /* Hand Detection minimum detection size */
#define HAND_SIZE_RANGE_MAX_DEFAULT       8192            /* Hand Detection maximum detection size */
#define FACE_SIZE_RANGE_MIN_DEFAULT         64            /* Face Detection minimum detection size */
#define FACE_SIZE_RANGE_MAX_DEFAULT       8192            /* Face Detection maximum detection size */

#define FACE_POSE_DEFAULT                    0            /* Face Detection facial pose (frontal face) */
#define FACE_ANGLE_DEFAULT                   0            /* Face Detection roll angle (Å}15Åã) */

#define STB_RETRYCOUNT_DEFAULT               2            /* Retry Count for STB */
#define STB_POSSTEADINESS_DEFAULT           30            /* Position Steadiness for STB */
#define STB_SIZESTEADINESS_DEFAULT          30            /* Size Steadiness for STB */
#define STB_OFF                              0            /* Not Use STB */
#define STB_ON                               1            /* Use STB */

#define STB_PE_FRAME_DEFAULT                10            /* Complete Frame Count for property estimation in STB */
#define STB_PE_ANGLEUDMIN_DEFAULT          -15            /* Up/Down face angle minimum value for property estimation in STB */
#define STB_PE_ANGLEUDMAX_DEFAULT           20            /* Up/Down face angle maximum value for property estimation in STB */
#define STB_PE_ANGLELRMIN_DEFAULT          -20            /* Left/Right face angle minimum value for property estimation in STB */
#define STB_PE_ANGLELRMAX_DEFAULT           20            /* Left/Right face angle maximum value for property estimation in STB */
#define STB_PE_THRESHOLD_DEFAULT           300            /* Threshold for property estimation in STB */

#define STB_FR_FRAME_DEFAULT                 5            /* Complete Frame Count for recognition in STB */
#define STB_FR_RATIO_DEFAULT                60            /* Account Ratio for recognition in STB */
#define STB_FR_ANGLEUDMIN_DEFAULT          -15            /* Up/Down face angle minimum value for recognition in STB */
#define STB_FR_ANGLEUDMAX_DEFAULT           20            /* Up/Down face angle maximum value for recognition in STB */
#define STB_FR_ANGLELRMIN_DEFAULT          -20            /* Left/Right face angle maximum value for recognition in STB */
#define STB_FR_ANGLELRMAX_DEFAULT           20            /* Left/Right face angle minimum value for recognition in STB */
#define STB_FR_THRESHOLD_DEFAULT           300            /* Threshold for recognition in STB */


int SaveAlbumData(const char *inFileName, int inDataSize, unsigned char *inAlbumData);
int LoadAlbumData(const char *inFileName, int *outDataSize, unsigned char *outAlbumData);
#ifdef WIN32
void SaveBitmapFile(int nWidth, int nHeight, UINT8 *unImageBuffer, const char *szFileName);
#else
void SaveBitmapFile(int nWidth, int nHeight, unsigned char *unImageBuffer, const char *szFileName);
#define sprintf_s(buf, num, ...) sprintf(buf, __VA_ARGS__)
#define printf_s(...) printf(__VA_ARGS__)
int kbhit(void);
#endif

void SampleFuncExecution(char *pStr, int stb_use);              /* Detection/Estimation         */
void SampleFuncRecognitionIdentify(char *pStr, int stb_use);    /* Recognition(Identify)        */
void SampleFuncRecognitionVerify(char *pStr);                   /* Recognition(Verify)          */
void SampleFuncRegisterData(char *pStr);                        /* Register data                */
void SampleFuncDeleteData(char *pStr);                          /* Delete specified data        */
void SampleFuncDeleteUser(char *pStr);                          /* Delete specified User        */
void SampleFuncDeleteAll(char *pStr);                           /* Delete all data              */
void SampleFuncSaveAlbum(char *pStr);                           /* Save Album                   */
void SampleFuncLoadAlbum(char *pStr);                           /* Load Album                   */
void SampleFuncWriteAlbum(char *pStr);                          /* Save Album on Flash ROM      */
void SampleFuncReformatAlbum(char *pStr);                       /* Reformat Flash ROM           */
void SampleFuncSetRegistCount(char *pStr);                      /* Set regist user count max    */
void SampleFuncGetRegistCount(char *pStr);                      /* Get regist user count max    */


/*----------------------------------------------------------------------------*/
/* UART send signal                                                           */
/* param    : int   inDataSize  send signal data                              */
/*          : UINT8 *inData     data length                                   */
/* return   : int               send signal complete data number              */
/*----------------------------------------------------------------------------*/
int UART_SendData(int inDataSize, UINT8 *inData)
{
    /* Send Data */
    int ret = com_send(inData, inDataSize);
    return ret;
}

/*----------------------------------------------------------------------------*/
/* UART receive signal                                                        */
/* param    : int   inTimeOutTime   timeout time (ms)                         */
/*          : int   *inDataSize     receive signal data size                  */
/*          : UINT8 *outResult      receive signal data                       */
/* return   : int                   receive signal complete data number       */
/*----------------------------------------------------------------------------*/
int UART_ReceiveData(int inTimeOutTime, int inDataSize, UINT8 *outResult)
{
    /* Receive Data */
    int ret = com_recv(inTimeOutTime, outResult, inDataSize);
    return ret;
}

/* Print Log Message */
static void PrintLog(char *pStr)
{
    puts(pStr);
}


/* HVC Execute Processing  */
int main(int argc, char *argv[])
{
    INT32 ret = 0;  /* Return code */

    INT32 inRate;
    int listBaudRate[] = {
                              9600,
                             38400,
                            115200,
                            230400,
                            460800,
                            921600
                         };

    UINT8 status;
    HVC_VERSION version;

    INT32 angleNo;
    HVC_THRESHOLD threshold;
    HVC_SIZERANGE sizeRange;
    INT32 pose;
    INT32 angle;
    INT32 verifyTh;

    int i;
    int revision;
    int stb_use = STB_ON;
    int funcNo;
    int exit = 0;
#ifndef WIN32
    int ch = -1;
#endif

    char *pStr;         /* String Buffer for logging output */

    S_STAT serialStat;  /* Serial port set value*/


    /******************************/
    /* Get command line arguments */
    /******************************/
    serialStat.com_num = 0;
    serialStat.BaudRate = 0;    /* Default Baudrate = 9600 */

    if ( argc < 3 || argc > 4) {
    PrintLog("Usage: sample.exe <com_port> <baudrate> [STB_ON|STB_OFF]\n");
        return (-1);
    }

    serialStat.com_num  = atoi(argv[1]);
    if ( com_init(&serialStat) == 0 ) {
        PrintLog("Failed to open COM port.\n");
        return (-1);
    }

    serialStat.BaudRate = atoi(argv[2]);
    for ( inRate = 0; inRate<(int)(sizeof(listBaudRate)/sizeof(int)); inRate++ ) {
        if ( listBaudRate[inRate] == (int)serialStat.BaudRate ) {
            break;
        }
    }
    if ( inRate >= (int)(sizeof(listBaudRate)/sizeof(int)) ) {
        PrintLog("Failed to set baudrate.\n");
        return (-1);
    }

    /* Change Baudrate */
    ret = HVC_SetBaudRate(UART_GENERAL_TIMEOUT, inRate, &status);
    if ( (ret != 0) || (status != 0) ) {
        PrintLog("HVCApi(HVC_SetBaudRate) Error.\n");
        return (-1);
    }

    if ( com_init(&serialStat) == 0 ) {
        PrintLog("Failed to open COM port.\n");
        return (-1);
    }

    if ( argc == 4 ) {        
        /* STB_ON/STB_OFF */
        if (strcmp (argv[3] ,"STB_ON") == 0) {
            stb_use = STB_ON;
        } else if (strcmp (argv[3] ,"STB_OFF") == 0) {
            stb_use = STB_OFF;
        } else {
            PrintLog("Please Set STB_ON or STB_OFF.\n");
            return (-1);
        }
    }

    /*****************************/
    /* Logging Buffer allocation */
    /*****************************/
    pStr = (char *)malloc(LOGBUFFERSIZE);
    if ( pStr == NULL ) {
        PrintLog("Failed to allocate Logging Buffer.\n");
        return (-1);
    }
    memset(pStr, 0, LOGBUFFERSIZE);

    do {
        /*********************************/
        /* Get Model and Version         */
        /*********************************/
        ret = HVC_GetVersion(UART_GENERAL_TIMEOUT, &version, &status);
        if ( ret != 0 ) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVCApi(HVC_GetVersion) Error : %d\n", ret);
            break;
        }
        if ( status != 0 ) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVC_GetVersion Response Error : 0x%02X\n", status);
            break;
        }

        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVC_GetVersion : ");

        for(i = 0; i < 12; i++){
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "%c", version.string[i] );
        }

        revision = version.revision[0] + (version.revision[1]<<8) + (version.revision[2]<<16) + (version.revision[3]<<24);
        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "%d.%d.%d.%d", version.major, version.minor, version.relese, revision);

        /*********************************/
        /* Set Camera Angle              */
        /*********************************/
        angleNo = SENSOR_ROLL_ANGLE_DEFAULT;

        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVC_SetCameraAngle : 0x%02x", angleNo);

        ret = HVC_SetCameraAngle(UART_GENERAL_TIMEOUT, angleNo, &status);
        if ( ret != 0 ) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVCApi(HVC_SetCameraAngle) Error : %d", ret);
        }
        else if ( status != 0 ) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVC_SetCameraAngle Response Error : 0x%02X", status);
        }

        angleNo = 0xff;

        ret = HVC_GetCameraAngle(UART_GENERAL_TIMEOUT, &angleNo, &status);
        if ( ret != 0 ) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVCApi(HVC_GetCameraAngle) Error : %d", ret);
        }
        else if ( status != 0 ) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVC_GetCameraAngle Response Error : 0x%02X", status);
        }
        else {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVC_GetCameraAngle : 0x%02x", angleNo);
        }

        /*********************************/
        /* Set Threshold Values          */
        /*********************************/
        threshold.bdThreshold = BODY_THRESHOLD_DEFAULT;
        threshold.hdThreshold = HAND_THRESHOLD_DEFAULT;
        threshold.dtThreshold = FACE_THRESHOLD_DEFAULT;
        threshold.rsThreshold = REC_THRESHOLD_DEFAULT;

        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVC_SetThreshold : Body=%4d Hand=%4d Face=%4d Recognition=%4d",
                         threshold.bdThreshold, threshold.hdThreshold, threshold.dtThreshold, threshold.rsThreshold);

        ret = HVC_SetThreshold(UART_GENERAL_TIMEOUT, &threshold, &status);
        if ( ret != 0 ) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVCApi(HVC_SetThreshold) Error : %d", ret);
        }
        else if ( status != 0 ) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVC_SetThreshold Response Error : 0x%02X", status);
        }

        threshold.bdThreshold = 0;
        threshold.hdThreshold = 0;
        threshold.dtThreshold = 0;
        threshold.rsThreshold = 0;

        ret = HVC_GetThreshold(UART_GENERAL_TIMEOUT, &threshold, &status);
        if ( ret != 0 ) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVCApi(HVC_GetThreshold) Error : %d", ret);
        }
        else if ( status != 0 ) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVC_GetThreshold Response Error : 0x%02X", status);
        }
        else {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVC_GetThreshold : Body=%4d Hand=%4d Face=%4d Recognition=%4d",
                             threshold.bdThreshold, threshold.hdThreshold, threshold.dtThreshold, threshold.rsThreshold);
        }

        /*********************************/
        /* Set Detection Size            */
        /*********************************/
        sizeRange.bdMinSize = BODY_SIZE_RANGE_MIN_DEFAULT;
        sizeRange.bdMaxSize = BODY_SIZE_RANGE_MAX_DEFAULT;
        sizeRange.hdMinSize = HAND_SIZE_RANGE_MIN_DEFAULT;
        sizeRange.hdMaxSize = HAND_SIZE_RANGE_MAX_DEFAULT;
        sizeRange.dtMinSize = FACE_SIZE_RANGE_MIN_DEFAULT;
        sizeRange.dtMaxSize = FACE_SIZE_RANGE_MAX_DEFAULT;

        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVC_SetSizeRange : Body=(%4d,%4d) Hand=(%4d,%4d) Face=(%4d,%4d)",
                                                            sizeRange.bdMinSize, sizeRange.bdMaxSize,
                                                            sizeRange.hdMinSize, sizeRange.hdMaxSize,
                                                            sizeRange.dtMinSize, sizeRange.dtMaxSize);

        ret = HVC_SetSizeRange(UART_GENERAL_TIMEOUT, &sizeRange, &status);
        if ( ret != 0 ) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVCApi(HVC_SetSizeRange) Error : %d", ret);
        }
        else if ( status != 0 ) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVC_SetSizeRange Response Error : 0x%02X", status);
        }

        sizeRange.bdMinSize = 0;
        sizeRange.bdMaxSize = 0;
        sizeRange.hdMinSize = 0;
        sizeRange.hdMaxSize = 0;
        sizeRange.dtMinSize = 0;
        sizeRange.dtMaxSize = 0;

        ret = HVC_GetSizeRange(UART_GENERAL_TIMEOUT, &sizeRange, &status);
        if ( ret != 0 ) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVCApi(HVC_GetSizeRange) Error : %d", ret);
        }
        else if ( status != 0 ) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVC_GetSizeRange Response Error : 0x%02X", status);
        }
        else {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVC_GetSizeRange : Body=(%4d,%4d) Hand=(%4d,%4d) Face=(%4d,%4d)",
                                                                sizeRange.bdMinSize, sizeRange.bdMaxSize,
                                                                sizeRange.hdMinSize, sizeRange.hdMaxSize,
                                                                sizeRange.dtMinSize, sizeRange.dtMaxSize);
        }

        /*********************************/
        /* Set Face Angle                */
        /*********************************/
        pose = FACE_POSE_DEFAULT;
        angle = FACE_ANGLE_DEFAULT;

        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVC_SetFaceDetectionAngle : Pose = 0x%02x Angle = 0x%02x", pose, angle);

        ret = HVC_SetFaceDetectionAngle(UART_GENERAL_TIMEOUT, pose, angle, &status);
        if ( ret != 0 ) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVCApi(HVC_SetFaceDetectionAngle) Error : %d", ret);
        }
        else if ( status != 0 ) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVC_SetFaceDetectionAngle Response Error : 0x%02X", status);
        }

        pose = 0xff;
        angle = 0xff;

        ret = HVC_GetFaceDetectionAngle(UART_GENERAL_TIMEOUT, &pose, &angle, &status);
        if ( ret != 0 ) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVCApi(HVC_GetFaceDetectionAngle) Error : %d", ret);
        }
        else if ( status != 0 ) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVC_GetFaceDetectionAngle Response Error : 0x%02X", status);
        }
        else{
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVC_GetFaceDetectionAngle : Pose = 0x%02x Angle = 0x%02x", pose, angle);
        }

        /*********************************/
        /* Set Verify Threshold          */
        /*********************************/
        verifyTh = VERIFY_THRESHOLD_DEFAULT;

        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVC_SetVerifyThreshold : Threshold = 0x%02x", verifyTh);

        ret = HVC_SetVerifyThreshold(UART_GENERAL_TIMEOUT, verifyTh, &status);
        if ( ret != 0 ) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVCApi(HVC_SetVerifyThreshold) Error : %d", ret);
        }
        else if ( status != 0 ) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVC_SetVerifyThreshold Response Error : 0x%02X", status);
        }

        verifyTh = 0xff;

        ret = HVC_GetVerifyThreshold(UART_GENERAL_TIMEOUT, &verifyTh, &status);
        if ( ret != 0 ) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVCApi(HVC_GetVerifyThreshold) Error : %d", ret);
        }
        else if ( status != 0 ) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVC_GetVerifyThreshold Response Error : 0x%02X", status);
        }
        else {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVC_GetVerifyThreshold : Threshold = 0x%02x", verifyTh);
        }

        /******************/
        /* Log Output     */
        /******************/
        PrintLog(pStr);
        memset(pStr, 0, LOGBUFFERSIZE);

        do {
            /*********************************/
            /* Select function               */
            /*********************************/
            funcNo = -1;

            PrintLog("\n");
            PrintLog("Execute function");
            PrintLog("");
            PrintLog("  1 : Detection/Estimation");
            PrintLog("  2 : Recognition(Identify)");
            PrintLog("  3 : Recognition(Verify)");
            PrintLog("  4 : Register data");
            PrintLog("  5 : Delete specified data");
            PrintLog("  6 : Delete specified user");
            PrintLog("  7 : Delete all data");
            PrintLog("  8 : Save Album");
            PrintLog("  9 : Load Album");
            PrintLog(" 10 : Save Album on Flash ROM");
            PrintLog(" 11 : Reformat Flash ROM");
            PrintLog(" 12 : Set Number of registered people in album");
            PrintLog(" 13 : Get Number of registered people in album");
            PrintLog("");
            PrintLog("  0 : Exit");

            printf_s("\nSelect function : ");
            scanf("%d", &funcNo);
            scanf("%*c");

            /*********************************/
            /* Execute function              */
            /*********************************/
            switch (funcNo) {
            case 1:
                /* Detection/Estimation */
                SampleFuncExecution(pStr, stb_use);
                break;
            case 2:
                /* Recognition(Identify) */
                SampleFuncRecognitionIdentify(pStr, stb_use);
                break;
            case 3:
                /* Recognition(Verify) */
                SampleFuncRecognitionVerify(pStr);
                break;
            case 4:
                /* Register data */
                SampleFuncRegisterData(pStr);
                break;
            case 5:
                /* Delete specified data */
                SampleFuncDeleteData(pStr);
                break;
            case 6:
                /* Delete specified User */
                SampleFuncDeleteUser(pStr);
                break;
            case 7:
                /* Delete all data */
                SampleFuncDeleteAll(pStr);
                break;
            case 8:
                /* Save Album */
                SampleFuncSaveAlbum(pStr);
                break;
            case 9:
                /* Load Album */
                SampleFuncLoadAlbum(pStr);
                break;
            case 10:
                /* Save Album on Flash ROM */
                SampleFuncWriteAlbum(pStr);
                break;
            case 11:
                /* Reformat Flash ROM */
                SampleFuncReformatAlbum(pStr);
                break;
            case 12:
                /* Set Number of registered people in album */
                SampleFuncSetRegistCount(pStr);
                break;
            case 13:
                /* Get Number of registered people in album */
                SampleFuncGetRegistCount(pStr);
                break;
            case 0:
                /* Exit */
                exit = 1;
                break;
            default:
                sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "Invalid number");
                break;
            }

            /******************/
            /* Log Output     */
            /******************/
            PrintLog(pStr);
            memset(pStr, 0, LOGBUFFERSIZE);

            if (funcNo >= 4) {
#ifdef WIN32
                while ( _kbhit() ) {
                    _getch();
                }
#else
                while (  kbhit()  !=  -1  );
#endif
                PrintLog("\n< Press any key to return to menu. >\n");
#ifdef WIN32
                _getch();
#else
                do {
                    ch = kbhit();
                } while(ch == -1);
#endif
            }

        } while (exit == 0);

    } while (0);

    /******************/
    /* Log Output     */
    /******************/
    PrintLog(pStr);

    com_close();

    /* Free Logging Buffer */
    if ( pStr != NULL ) {
        free(pStr);
    }
    return (0);
}


/*----------------------------------------------------------------------------*/
/* SampleFuncExecution                                                        */
/* param    : char  *pStr       log strings                                   */
/*          : int   stb_use     STB use ON/OFF                                */
/* return   : void                                                            */
/*----------------------------------------------------------------------------*/
void SampleFuncExecution(char *pStr, int stb_use)
{
    INT32 ret = 0;
    INT32 execFlag;
    INT32 imageNo;
    UINT8 status;

    HVC_RESULT *pHVCResult = NULL;

    int nSTBFaceCount;
    int nSTBBodyCount;
    STB_FACE *pSTBFaceResult;
    STB_BODY *pSTBBodyResult;

    int nIndex;
    int i;
    int ch = 0;
   
    char *pExStr[] = {"?", "Neutral", "Happiness", "Surprise", "Anger", "Sadness"};

    if (stb_use == STB_ON) {
        /*********************************/
        /* STB Initialize                */
        /*********************************/
        ret = STB_Init(STB_FUNC_BD | STB_FUNC_DT | STB_FUNC_PT | STB_FUNC_AG | STB_FUNC_GN);
        if ( ret != 0 ) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nSTB_Init Error : %d\n", ret);
            return;
        }

        /*********************************/
        /* Set STB Parameters            */
        /*********************************/
        ret = STB_SetTrParam(STB_RETRYCOUNT_DEFAULT, STB_POSSTEADINESS_DEFAULT, STB_SIZESTEADINESS_DEFAULT);
        if ( ret != 0 ) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVCApi(STB_SetTrParam) Error : %d\n", ret);
            return;
        }

        ret = STB_SetPeParam(STB_PE_THRESHOLD_DEFAULT, STB_PE_ANGLEUDMIN_DEFAULT, STB_PE_ANGLEUDMAX_DEFAULT, STB_PE_ANGLELRMIN_DEFAULT, STB_PE_ANGLELRMAX_DEFAULT, STB_PE_FRAME_DEFAULT);
        if ( ret != 0 ) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVCApi(STB_SetPeParam) Error : %d\n", ret);
            return;
        }
        }

    /*********************************/
    /* Result Structure Allocation   */
    /*********************************/
    pHVCResult = (HVC_RESULT *)malloc(sizeof(HVC_RESULT));
    if ( pHVCResult == NULL ) { /* Error processing */
        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nMemory Allocation Error : %08lx\n", sizeof(HVC_RESULT));
        return;
    }

    do {
        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nPress Space Key to end: ");

        /******************/
        /* Log Output     */
        /******************/
        PrintLog(pStr);
        memset(pStr, 0, LOGBUFFERSIZE);

        /*********************************/
        /* Execute Detection             */
        /*********************************/
        execFlag = HVC_ACTIV_BODY_DETECTION | HVC_ACTIV_HAND_DETECTION | HVC_ACTIV_FACE_DETECTION | HVC_ACTIV_FACE_DIRECTION |
                   HVC_ACTIV_AGE_ESTIMATION | HVC_ACTIV_GENDER_ESTIMATION | HVC_ACTIV_GAZE_ESTIMATION | HVC_ACTIV_BLINK_ESTIMATION |
                   HVC_ACTIV_EXPRESSION_ESTIMATION;
        imageNo = HVC_EXECUTE_IMAGE_QVGA_HALF; /* HVC_EXECUTE_IMAGE_NONE; */


        ret = HVC_ExecuteEx(UART_EXECUTE_TIMEOUT, execFlag, imageNo, pHVCResult, &status);

        if ( ret != 0 ) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVCApi(HVC_ExecuteEx) Error : %d\n", ret);
            continue;
        }
        if ( status != 0 ) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVC_ExecuteEx Response Error : 0x%02X\n", status);
            continue;
        }

        if ( imageNo != HVC_EXECUTE_IMAGE_NONE ) {
            SaveBitmapFile(pHVCResult->image.width, pHVCResult->image.height, pHVCResult->image.image, "DetectionImage.bmp");
        }

        if (stb_use == STB_ON) {
            if ( STB_Exec(pHVCResult->executedFunc, pHVCResult, &nSTBFaceCount, &pSTBFaceResult, &nSTBBodyCount, &pSTBBodyResult) == 0 ) {
                for ( i = 0; i < nSTBBodyCount; i++ )
                {
                    if ( pHVCResult->bdResult.num <= i ) break;

                    nIndex = pSTBBodyResult[i].nDetectID;
                             pHVCResult->bdResult.bdResult[nIndex].posX = (short)pSTBBodyResult[i].center.x;
                             pHVCResult->bdResult.bdResult[nIndex].posY = (short)pSTBBodyResult[i].center.y;
                             pHVCResult->bdResult.bdResult[nIndex].size = pSTBBodyResult[i].nSize;
                }
                for ( i = 0; i < nSTBFaceCount; i++ )
                {
                    if ( pHVCResult->fdResult.num <= i ) break;

                    nIndex = pSTBFaceResult[i].nDetectID;
                    pHVCResult->fdResult.fcResult[nIndex].dtResult.posX = (short)pSTBFaceResult[i].center.x;
                    pHVCResult->fdResult.fcResult[nIndex].dtResult.posY = (short)pSTBFaceResult[i].center.y;
                    pHVCResult->fdResult.fcResult[nIndex].dtResult.size = pSTBFaceResult[i].nSize;

                    if (pHVCResult->executedFunc & HVC_ACTIV_AGE_ESTIMATION) {
                        pHVCResult->fdResult.fcResult[nIndex].ageResult.confidence += 10000; /* During */
                        if ( pSTBFaceResult[i].age.status >= STB_STATUS_COMPLETE ) {
                            pHVCResult->fdResult.fcResult[nIndex].ageResult.age = pSTBFaceResult[i].age.value;
                            pHVCResult->fdResult.fcResult[nIndex].ageResult.confidence += 10000; /* Complete */
                        }
                    }
                    if (pHVCResult->executedFunc & HVC_ACTIV_GENDER_ESTIMATION) {
                        pHVCResult->fdResult.fcResult[nIndex].genderResult.confidence += 10000; /* During */
                        if ( pSTBFaceResult[i].gender.status >= STB_STATUS_COMPLETE ) {
                            pHVCResult->fdResult.fcResult[nIndex].genderResult.gender = pSTBFaceResult[i].gender.value;
                            pHVCResult->fdResult.fcResult[nIndex].genderResult.confidence += 10000; /* Complete */
                        }
                    }
                }
            }

            if (pHVCResult->executedFunc & HVC_ACTIV_BODY_DETECTION) {
                /* Body Detection result string */
                sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n Body result count:%d", pHVCResult->bdResult.num);
                for (i = 0; i < pHVCResult->bdResult.num; i++) {
                    sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Index:%d TR_ID:%d X:%d Y:%d Size:%d Confidence:%d", i,
                                            pSTBBodyResult[i].nTrackingID,
                                            pHVCResult->bdResult.bdResult[i].posX, pHVCResult->bdResult.bdResult[i].posY,
                                            pHVCResult->bdResult.bdResult[i].size, pHVCResult->bdResult.bdResult[i].confidence);
                }
            }
            if (pHVCResult->executedFunc & HVC_ACTIV_HAND_DETECTION) {
                /* Hand Detection result string */
                sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n Hand result count:%d", pHVCResult->hdResult.num);
                for (i = 0; i < pHVCResult->hdResult.num; i++) {
                    sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Index:%d \t\tX:%d Y:%d Size:%d Confidence:%d", i,
                                            pHVCResult->hdResult.hdResult[i].posX, pHVCResult->hdResult.hdResult[i].posY,
                                            pHVCResult->hdResult.hdResult[i].size, pHVCResult->hdResult.hdResult[i].confidence);
                }
            }

            /* Face Detection result string */
            if (pHVCResult->executedFunc & (HVC_ACTIV_FACE_DETECTION        | HVC_ACTIV_FACE_DIRECTION    |
                                            HVC_ACTIV_AGE_ESTIMATION        | HVC_ACTIV_GENDER_ESTIMATION |
                                            HVC_ACTIV_GAZE_ESTIMATION       | HVC_ACTIV_BLINK_ESTIMATION  |
                                            HVC_ACTIV_EXPRESSION_ESTIMATION | HVC_ACTIV_FACE_RECOGNITION)) {
                sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n Face result count:%d", pHVCResult->fdResult.num);
                for (i = 0; i < pHVCResult->fdResult.num; i++) {
                    if (pHVCResult->executedFunc & HVC_ACTIV_FACE_DETECTION) {
                        /* Detection */
                        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Index:%d TR_ID:%d X:%d Y:%d Size:%d Confidence:%d", i,
                                                pSTBFaceResult[i].nTrackingID,
                                                pHVCResult->fdResult.fcResult[i].dtResult.posX, pHVCResult->fdResult.fcResult[i].dtResult.posY,
                                                pHVCResult->fdResult.fcResult[i].dtResult.size, pHVCResult->fdResult.fcResult[i].dtResult.confidence);
                    }
                    if (pHVCResult->executedFunc & HVC_ACTIV_FACE_DIRECTION) {
                        /* Face Direction */
                        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Face Direction\tLR:%d UD:%d Roll:%d Confidence:%d",
                                                pHVCResult->fdResult.fcResult[i].dirResult.yaw, pHVCResult->fdResult.fcResult[i].dirResult.pitch,
                                                pHVCResult->fdResult.fcResult[i].dirResult.roll, pHVCResult->fdResult.fcResult[i].dirResult.confidence);
                    }
                    if (pHVCResult->executedFunc & HVC_ACTIV_AGE_ESTIMATION) {
                        /* Age */
                        if (-128 == pHVCResult->fdResult.fcResult[i].ageResult.age){
                            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Age\t\tEstimation not possible");
                        } else {
                            if ( pHVCResult->fdResult.fcResult[i].ageResult.confidence >= 20000 ) { /* Complete */
                                sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Age\t\tAge:%d Confidence:%d (*)",
                                                        pHVCResult->fdResult.fcResult[i].ageResult.age, pHVCResult->fdResult.fcResult[i].ageResult.confidence - 20000);
                            }
                            else if ( pHVCResult->fdResult.fcResult[i].ageResult.confidence >= 10000 ) { /* During */
                                sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Age\t\tAge:%d Confidence:%d (-)",
                                                        pHVCResult->fdResult.fcResult[i].ageResult.age, pHVCResult->fdResult.fcResult[i].ageResult.confidence - 10000);
                            } 
                            else {
                                sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Age\t\tAge:%d Confidence:%d (x)",
                                                        pHVCResult->fdResult.fcResult[i].ageResult.age, pHVCResult->fdResult.fcResult[i].ageResult.confidence);
                            }
                        }
                    }
                    if (pHVCResult->executedFunc & HVC_ACTIV_GENDER_ESTIMATION) {
                        /* Gender */
                        if (-128 == pHVCResult->fdResult.fcResult[i].genderResult.gender) {
                            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Gender\t\tEstimation not possible");
                        }
                        else {
                            if ( pHVCResult->fdResult.fcResult[i].genderResult.confidence >= 20000 ) { /* Complete */
                                if (1 == pHVCResult->fdResult.fcResult[i].genderResult.gender) {
                                    sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Gender\t\tGender:%s Confidence:%d (*)",
                                                            "Male", pHVCResult->fdResult.fcResult[i].genderResult.confidence - 20000);
                                }
                                else {
                                    sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Gender\t\tGender:%s Confidence:%d (*)",
                                                            "Female", pHVCResult->fdResult.fcResult[i].genderResult.confidence - 20000);
                                }
                            } 
                            else if ( pHVCResult->fdResult.fcResult[i].genderResult.confidence >= 10000 ) { /* During */
                                if (1 == pHVCResult->fdResult.fcResult[i].genderResult.gender) {
                                    sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Gender\t\tGender:%s Confidence:%d (-)",
                                                            "Male", pHVCResult->fdResult.fcResult[i].genderResult.confidence - 10000);
                                }
                                else {
                                    sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Gender\t\tGender:%s Confidence:%d (-)",
                                                            "Female", pHVCResult->fdResult.fcResult[i].genderResult.confidence - 10000);
                                }
                            } else {
                                if (1 == pHVCResult->fdResult.fcResult[i].genderResult.gender) {
                                    sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Gender\t\tGender:%s Confidence:%d (x)",
                                                            "Male", pHVCResult->fdResult.fcResult[i].genderResult.confidence);
                                }
                                else {
                                    sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Gender\t\tGender:%s Confidence:%d (x)",
                                                            "Female", pHVCResult->fdResult.fcResult[i].genderResult.confidence);
                                }
                            }
                        }
                    }
                    if (pHVCResult->executedFunc & HVC_ACTIV_GAZE_ESTIMATION) {
                        /* Gaze */
                        if ((-128 == pHVCResult->fdResult.fcResult[i].gazeResult.gazeLR) ||
                            (-128 == pHVCResult->fdResult.fcResult[i].gazeResult.gazeUD)) {
                            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Gaze\t\tEstimation not possible");
                        }
                        else {
                                sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Gaze\t\tLR:%d UD:%d",
                                                        pHVCResult->fdResult.fcResult[i].gazeResult.gazeLR, pHVCResult->fdResult.fcResult[i].gazeResult.gazeUD);
                        }
                    }
                    if (pHVCResult->executedFunc & HVC_ACTIV_BLINK_ESTIMATION) {
                        /* Blink */
                        if ((-128 == pHVCResult->fdResult.fcResult[i].blinkResult.ratioL) ||
                            (-128 == pHVCResult->fdResult.fcResult[i].blinkResult.ratioR)){
                            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Blink\t\tEstimation not possible");
                        }
                        else {
                            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Blink\t\tLeft:%d Right:%d",
                                                    pHVCResult->fdResult.fcResult[i].blinkResult.ratioL, pHVCResult->fdResult.fcResult[i].blinkResult.ratioR);
                        }
                    }
                    if (pHVCResult->executedFunc & HVC_ACTIV_EXPRESSION_ESTIMATION) {
                        /* Expression */
                        if (-128 == pHVCResult->fdResult.fcResult[i].expressionResult.score[0]) {
                            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Expression\tEstimation not possible");
                        }
                        else{
                            if (pHVCResult->fdResult.fcResult[i].expressionResult.topExpression > EX_SADNESS) {
                                pHVCResult->fdResult.fcResult[i].expressionResult.topExpression = 0;
                            }
                            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Expression\tExpression:%s Score:%d, %d, %d, %d, %d Degree:%d",
                                                    pExStr[pHVCResult->fdResult.fcResult[i].expressionResult.topExpression],
                                                    pHVCResult->fdResult.fcResult[i].expressionResult.score[0],
                                                    pHVCResult->fdResult.fcResult[i].expressionResult.score[1],
                                                    pHVCResult->fdResult.fcResult[i].expressionResult.score[2],
                                                    pHVCResult->fdResult.fcResult[i].expressionResult.score[3],
                                                    pHVCResult->fdResult.fcResult[i].expressionResult.score[4],
                                                    pHVCResult->fdResult.fcResult[i].expressionResult.degree);
                        }
                    }
                }
            }
        } else {
            if (pHVCResult->executedFunc & HVC_ACTIV_BODY_DETECTION) {
                /* Body Detection result string */
                sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n Body result count:%d", pHVCResult->bdResult.num);
                for (i = 0; i < pHVCResult->bdResult.num; i++) {
                    sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Index:%d \t\tX:%d Y:%d Size:%d Confidence:%d", i,
                                            pHVCResult->bdResult.bdResult[i].posX, pHVCResult->bdResult.bdResult[i].posY,
                                            pHVCResult->bdResult.bdResult[i].size, pHVCResult->bdResult.bdResult[i].confidence);
                }
            }
            if (pHVCResult->executedFunc & HVC_ACTIV_HAND_DETECTION) {
                /* Hand Detection result string */
                sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n Hand result count:%d", pHVCResult->hdResult.num);
                for (i = 0; i < pHVCResult->hdResult.num; i++) {
                    sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Index:%d \t\tX:%d Y:%d Size:%d Confidence:%d", i,
                                            pHVCResult->hdResult.hdResult[i].posX, pHVCResult->hdResult.hdResult[i].posY,
                                            pHVCResult->hdResult.hdResult[i].size, pHVCResult->hdResult.hdResult[i].confidence);
                }
            }

            /* Face Detection result string */
            if (pHVCResult->executedFunc & (HVC_ACTIV_FACE_DETECTION        | HVC_ACTIV_FACE_DIRECTION    |
                                            HVC_ACTIV_AGE_ESTIMATION        | HVC_ACTIV_GENDER_ESTIMATION |
                                            HVC_ACTIV_GAZE_ESTIMATION       | HVC_ACTIV_BLINK_ESTIMATION  |
                                            HVC_ACTIV_EXPRESSION_ESTIMATION | HVC_ACTIV_FACE_RECOGNITION)) {
                sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n Face result count:%d", pHVCResult->fdResult.num);
                for (i = 0; i < pHVCResult->fdResult.num; i++) {
                    if (pHVCResult->executedFunc & HVC_ACTIV_FACE_DETECTION) {
                        /* Detection */
                        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Index:%d \t\tX:%d Y:%d Size:%d Confidence:%d", i,
                                                pHVCResult->fdResult.fcResult[i].dtResult.posX, pHVCResult->fdResult.fcResult[i].dtResult.posY,
                                                pHVCResult->fdResult.fcResult[i].dtResult.size, pHVCResult->fdResult.fcResult[i].dtResult.confidence);
                    }
                    if (pHVCResult->executedFunc & HVC_ACTIV_FACE_DIRECTION) {
                        /* Face Direction */
                        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Face Direction\tLR:%d UD:%d Roll:%d Confidence:%d",
                                                pHVCResult->fdResult.fcResult[i].dirResult.yaw, pHVCResult->fdResult.fcResult[i].dirResult.pitch,
                                                pHVCResult->fdResult.fcResult[i].dirResult.roll, pHVCResult->fdResult.fcResult[i].dirResult.confidence);
                    }
                    if (pHVCResult->executedFunc & HVC_ACTIV_AGE_ESTIMATION) {
                        /* Age */
                        if (-128 == pHVCResult->fdResult.fcResult[i].ageResult.age) {
                            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Age\t\tEstimation not possible");
                        } else {
                            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Age\t\tAge:%d Confidence:%d",
                                                    pHVCResult->fdResult.fcResult[i].ageResult.age, pHVCResult->fdResult.fcResult[i].ageResult.confidence);
                        }
                    }
                    if (pHVCResult->executedFunc & HVC_ACTIV_GENDER_ESTIMATION) {
                        /* Gender */
                        if(-128 == pHVCResult->fdResult.fcResult[i].genderResult.gender){
                            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Gender\t\tEstimation not possible");
                        }
                        else{
                            if(1 == pHVCResult->fdResult.fcResult[i].genderResult.gender){
                                sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Gender\t\tGender:%s Confidence:%d",
                                                        "Male", pHVCResult->fdResult.fcResult[i].genderResult.confidence);
                            }
                            else{
                                sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Gender\t\tGender:%s Confidence:%d",
                                                        "Female", pHVCResult->fdResult.fcResult[i].genderResult.confidence);
                            }
                        }
                    }
                    if (pHVCResult->executedFunc & HVC_ACTIV_GAZE_ESTIMATION) {
                        /* Gaze */
                        if ((-128 == pHVCResult->fdResult.fcResult[i].gazeResult.gazeLR) ||
                            (-128 == pHVCResult->fdResult.fcResult[i].gazeResult.gazeUD)) {
                            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Gaze\t\tEstimation not possible");
                        }
                        else {
                            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Gaze\t\tLR:%d UD:%d",
                                                    pHVCResult->fdResult.fcResult[i].gazeResult.gazeLR, pHVCResult->fdResult.fcResult[i].gazeResult.gazeUD);
                        }
                    }
                    if (pHVCResult->executedFunc & HVC_ACTIV_BLINK_ESTIMATION) {
                        /* Blink */
                        if ((-128 == pHVCResult->fdResult.fcResult[i].blinkResult.ratioL) ||
                            (-128 == pHVCResult->fdResult.fcResult[i].blinkResult.ratioR)) {
                            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Blink\t\tEstimation not possible");
                        }
                        else {
                            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Blink\t\tLeft:%d Right:%d",
                                                    pHVCResult->fdResult.fcResult[i].blinkResult.ratioL, pHVCResult->fdResult.fcResult[i].blinkResult.ratioR);
                        }
                    }
                    if (pHVCResult->executedFunc & HVC_ACTIV_EXPRESSION_ESTIMATION) {
                        /* Expression */
                        if (-128 == pHVCResult->fdResult.fcResult[i].expressionResult.score[0]) {
                            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Expression\tEstimation not possible");
                        }
                        else {
                            if (pHVCResult->fdResult.fcResult[i].expressionResult.topExpression > EX_SADNESS) {
                                    pHVCResult->fdResult.fcResult[i].expressionResult.topExpression = 0;
                            }
                            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Expression\tExpression:%s Score:%d, %d, %d, %d, %d Degree:%d",
                                                    pExStr[pHVCResult->fdResult.fcResult[i].expressionResult.topExpression],
                                                    pHVCResult->fdResult.fcResult[i].expressionResult.score[0],
                                                    pHVCResult->fdResult.fcResult[i].expressionResult.score[1],
                                                    pHVCResult->fdResult.fcResult[i].expressionResult.score[2],
                                                    pHVCResult->fdResult.fcResult[i].expressionResult.score[3],
                                                    pHVCResult->fdResult.fcResult[i].expressionResult.score[4],
                                                    pHVCResult->fdResult.fcResult[i].expressionResult.degree);
                        }
                    }
                }
            }
        }

        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n");

#ifdef WIN32
        if ( _kbhit() ) {
            ch = _getch();
            ch = toupper( ch );
        }
#else
        ch = kbhit();
#endif
    } while( ch != ' ' );

    /********************************/
    /* Free result area             */
    /********************************/
    if ( pHVCResult != NULL ) {
        free(pHVCResult);
    }

    /*********************************/
    /* STB Finalize                  */
    /*********************************/
    if (stb_use == STB_ON) {
        STB_Final();
    }
}


/*----------------------------------------------------------------------------*/
/* SampleFuncRecognitionIdentify                                              */
/* param    : char  *pStr       log strings                                   */
/*          : int   stb_use     STB use ON/OFF                                */
/* return   : void                                                            */
/*----------------------------------------------------------------------------*/
void SampleFuncRecognitionIdentify(char *pStr, int stb_use)
{
    INT32 ret = 0;
    UINT8 status;

    int i;
    int ch = 0;

    INT32 execFlag;
    INT32 imageNo;
    INT32 index;

    HVC_RESULT *pHVCResult = NULL;

    int nSTBFaceCount;
    int nSTBBodyCount;
    STB_FACE *pSTBFaceResult;
    STB_BODY *pSTBBodyResult;


    if (stb_use == STB_ON) {
        /*********************************/
        /* STB Initialize                */
        /*********************************/
        ret = STB_Init(STB_FUNC_DT | STB_FUNC_PT | STB_FUNC_FR);
        if ( ret != 0 ) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nSTB_Init Error : %d\n", ret);
            return;
        }

        /*********************************/
        /* Set STB Parameters            */
        /*********************************/
        ret = STB_SetTrParam(STB_RETRYCOUNT_DEFAULT, STB_POSSTEADINESS_DEFAULT, STB_SIZESTEADINESS_DEFAULT);
        if ( ret != 0 ) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVCApi(STB_SetTrParam) Error : %d\n", ret);
            return;
        }

        ret = STB_SetFrParam(STB_FR_THRESHOLD_DEFAULT, STB_FR_ANGLEUDMIN_DEFAULT, STB_FR_ANGLEUDMAX_DEFAULT, STB_FR_ANGLELRMIN_DEFAULT, STB_FR_ANGLELRMAX_DEFAULT, STB_FR_FRAME_DEFAULT, STB_FR_RATIO_DEFAULT);
        if ( ret != 0 ) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVCApi(STB_SetFrParam) Error : %d\n", ret);
            return;
        }
    }

    /*********************************/
    /* Result Structure Allocation   */
    /*********************************/
    pHVCResult = (HVC_RESULT *)malloc(sizeof(HVC_RESULT));
    if ( pHVCResult == NULL ) { /* Error processing */
        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nMemory Allocation Error : %08lx\n", sizeof(HVC_RESULT));
        return;
    }

    while (1) {
        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nPress the Enter Key to Detection and the Space Key to end: ");

        /******************/
        /* Log Output     */
        /******************/
        PrintLog(pStr);
        memset(pStr, 0, LOGBUFFERSIZE);

        /******************/
        /* Input          */
        /******************/
#ifdef WIN32
        do {
            ch = _getch();
            ch = toupper( ch );
        } while(ch != '\r' && ch != ' ');
#else
        do {
            ch = kbhit();
        } while(ch != '\n' && ch != ' ');
#endif

        if (ch == ' ') break;   /* space to end */

        /*********************************/
        /* Execute Detection             */
        /*********************************/
        if (stb_use == STB_ON) {
            execFlag = HVC_ACTIV_FACE_DETECTION | HVC_ACTIV_FACE_DIRECTION | HVC_ACTIV_FACE_RECOGNITION;
        } else {
            execFlag = HVC_ACTIV_FACE_DETECTION | HVC_ACTIV_FACE_RECOGNITION;
        }
        imageNo = HVC_EXECUTE_IMAGE_QVGA_HALF; /* HVC_EXECUTE_IMAGE_NONE; */

        ret = HVC_ExecuteEx(UART_EXECUTE_TIMEOUT, execFlag, imageNo, pHVCResult, &status);
        if ( ret != 0 ) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVCApi(HVC_ExecuteEx) Error : %d\n", ret);
            continue;
        }
        if ( status != 0 ) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVC_ExecuteEx Response Error : 0x%02X\n", status);
            continue;
        }

        if ( imageNo != HVC_EXECUTE_IMAGE_NONE ) {
            SaveBitmapFile(pHVCResult->image.width, pHVCResult->image.height, pHVCResult->image.image, "IdentifyImage.bmp");
        }

        if (stb_use == STB_ON) {
            if ( STB_Exec(pHVCResult->executedFunc, pHVCResult, &nSTBFaceCount, &pSTBFaceResult, &nSTBBodyCount, &pSTBBodyResult) == 0 ) {
                for ( i = 0; i < nSTBFaceCount; i++ )
                {
                    if ( pHVCResult->fdResult.num <= i ) break;

                    index = pSTBFaceResult[i].nDetectID;
                    pHVCResult->fdResult.fcResult[index].dtResult.posX = (short)pSTBFaceResult[i].center.x;
                    pHVCResult->fdResult.fcResult[index].dtResult.posY = (short)pSTBFaceResult[i].center.y;
                    pHVCResult->fdResult.fcResult[index].dtResult.size = pSTBFaceResult[i].nSize;

                    if (pHVCResult->executedFunc & HVC_ACTIV_FACE_RECOGNITION) {
                        pHVCResult->fdResult.fcResult[index].recognitionResult.confidence += 10000; /* During */
                        if ( pSTBFaceResult[i].recognition.status >= STB_STATUS_COMPLETE ) {
                            pHVCResult->fdResult.fcResult[index].recognitionResult.uid = pSTBFaceResult[i].recognition.value;
                            pHVCResult->fdResult.fcResult[index].recognitionResult.confidence += 10000; /* Complete */
                        }
                    }
                }
            }
        }

        /* Face Detection result string */
        if (pHVCResult->executedFunc & HVC_ACTIV_FACE_DETECTION) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n Face result count:%d", pHVCResult->fdResult.num);
            for (i = 0; i < pHVCResult->fdResult.num; i++) {
                /* Detection */
                sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Index:%d \tX:%d Y:%d Size:%d Confidence:%d", i,
                                        pHVCResult->fdResult.fcResult[i].dtResult.posX, pHVCResult->fdResult.fcResult[i].dtResult.posY,
                                        pHVCResult->fdResult.fcResult[i].dtResult.size, pHVCResult->fdResult.fcResult[i].dtResult.confidence);
                if (pHVCResult->executedFunc & HVC_ACTIV_FACE_RECOGNITION) {
                    /* Recognition */
                    if (-128 == pHVCResult->fdResult.fcResult[i].recognitionResult.uid) {
                        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Recognition\tRecognition not possible");
                    }
                    else if (-127 == pHVCResult->fdResult.fcResult[i].recognitionResult.uid) {
                        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Recognition\tNot registered");
                    }
                    else {
                        if (stb_use == STB_ON) {
                            if ( pHVCResult->fdResult.fcResult[i].recognitionResult.confidence >= 20000 ) { /* Complete */
                                sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Recognition\tID:%d Confidence:%d (*)",
                                                        pHVCResult->fdResult.fcResult[i].recognitionResult.uid,
                                                        pHVCResult->fdResult.fcResult[i].recognitionResult.confidence - 20000);
                            } else if ( pHVCResult->fdResult.fcResult[i].recognitionResult.confidence >= 10000 ) { /* During */
                                sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Recognition\tID:%d Confidence:%d (-)",
                                                        pHVCResult->fdResult.fcResult[i].recognitionResult.uid,
                                                        pHVCResult->fdResult.fcResult[i].recognitionResult.confidence - 10000);
                            } else {
                                sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Recognition\tID:%d Confidence:%d (x)",
                                                        pHVCResult->fdResult.fcResult[i].recognitionResult.uid,
                                                        pHVCResult->fdResult.fcResult[i].recognitionResult.confidence);
                            }
                        }
                        else {
                            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Recognition\tID:%d Confidence:%d",
                                                    pHVCResult->fdResult.fcResult[i].recognitionResult.uid,
                                                    pHVCResult->fdResult.fcResult[i].recognitionResult.confidence);
                        }
                    }
                }
            }
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n");
        }
    }

    /********************************/
    /* Free result area             */
    /********************************/
    if( pHVCResult != NULL ){
        free(pHVCResult);
    }

    /*********************************/
    /* STB Finalize                  */
    /*********************************/
    if (stb_use == STB_ON) {
        STB_Final();
    }
}


/*----------------------------------------------------------------------------*/
/* SampleFuncRecognitionVerify                                                */
/* param    : char  *pStr       log strings                                   */
/*          : int   stb_use     STB use ON/OFF                                */
/* return   : void                                                            */
/*----------------------------------------------------------------------------*/
void SampleFuncRecognitionVerify(char *pStr)
{
    INT32 ret = 0;
    UINT8 status;

    int i;
    int ch = 0;

    INT32 execFlag;
    INT32 imageNo;

    HVC_RESULT *pHVCResult = NULL;

    INT32 userId = -1;


    /******************/
    /* Input          */
    /******************/
    printf_s("\nInput user ID : ");
    scanf("%d", &userId);
    scanf("%*c");

    if (userId < 0 || userId > 999) {
        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nInvalid user ID\n");
        return;
    }

    /*********************************/
    /* Set Verify user ID            */
    /*********************************/
    ret = HVC_SetVerifyUser(UART_GENERAL_TIMEOUT, userId, &status);
    if ( ret != 0 ) {
        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVCApi(HVC_SetVerifyUser) Error : %d\n", ret);
        return;
    }
    if ( status != 0 ) {
        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVC_SetVerifyUser Response Error : 0x%02X\n", status);
        return;
    }
    userId = 0xff;
    ret = HVC_GetVerifyUser(UART_GENERAL_TIMEOUT, &userId, &status);
    if ( ret != 0 ) {
        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVCApi(HVC_SetVerifyUser) Error : %d\n", ret);
        return;
    }
    if ( status != 0 ) {
        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVC_SetVerifyUser Response Error : 0x%02X\n", status);
        return;
    }
    sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nVerify user ID = %d\n", userId);

    /*********************************/
    /* Result Structure Allocation   */
    /*********************************/
    pHVCResult = (HVC_RESULT *)malloc(sizeof(HVC_RESULT));
    if ( pHVCResult == NULL ) { /* Error processing */
        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nMemory Allocation Error : %08lx\n", sizeof(HVC_RESULT));
        return;
    }

    while (1) {
        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nPress the Enter Key to Detection and the Space Key to end: ");

        /******************/
        /* Log Output     */
        /******************/
        PrintLog(pStr);
        memset(pStr, 0, LOGBUFFERSIZE);

        /******************/
        /* Input          */
        /******************/
#ifdef WIN32
        do {
            ch = _getch();
            ch = toupper( ch );
        } while(ch != '\r' && ch != ' ');
#else
        do {
            ch = kbhit();
        } while(ch != '\n' && ch != ' ');
#endif

        if (ch == ' ') break;   /* space to end */

        /*********************************/
        /* Execute Detection             */
        /*********************************/
        execFlag = HVC_ACTIV_FACE_DETECTION | HVC_ACTIV_FACE_VERIFY;
        imageNo = HVC_EXECUTE_IMAGE_QVGA_HALF; /* HVC_EXECUTE_IMAGE_NONE; */

        ret = HVC_ExecuteEx(UART_EXECUTE_TIMEOUT, execFlag, HVC_EXECUTE_IMAGE_QVGA_HALF, pHVCResult, &status);
        if ( ret != 0 ) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVCApi(HVC_ExecuteEx) Error : %d\n", ret);
            continue;
        }
        if ( status != 0 ) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVC_ExecuteEx Response Error : 0x%02X\n", status);
            continue;
        }

        if ( imageNo != HVC_EXECUTE_IMAGE_NONE ) {
            SaveBitmapFile(pHVCResult->image.width, pHVCResult->image.height, pHVCResult->image.image, "VerifyImage.bmp");
        }
    
        /* Face Detection result string */
        if (pHVCResult->executedFunc & HVC_ACTIV_FACE_DETECTION) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n Face result count:%d", pHVCResult->fdResult.num);
            for (i = 0; i < pHVCResult->fdResult.num; i++) {
                /* Detection */
                sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Index:%d \tX:%d Y:%d Size:%d Confidence:%d", i,
                                        pHVCResult->fdResult.fcResult[i].dtResult.posX, pHVCResult->fdResult.fcResult[i].dtResult.posY,
                                        pHVCResult->fdResult.fcResult[i].dtResult.size, pHVCResult->fdResult.fcResult[i].dtResult.confidence);
                if (pHVCResult->executedFunc & HVC_ACTIV_FACE_VERIFY) {
                    /* Verify */
                    if (-128 == pHVCResult->fdResult.fcResult[i].authResult.auth) {
                        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Verify\tVerify not possible");
                    }
                    else if (-127 == pHVCResult->fdResult.fcResult[i].authResult.auth) {
                        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Verify\tNot registered");
                    }
                    else {
                        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n      Verify\tResult:0x%04X Confidence:%d",
                                                pHVCResult->fdResult.fcResult[i].authResult.auth & 0x0000FFFF,
                                                pHVCResult->fdResult.fcResult[i].authResult.confidence);
                    }
                }
            }
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\n");
        }
    }

    /********************************/
    /* Free result area             */
    /********************************/
    if( pHVCResult != NULL ){
        free(pHVCResult);
    }
}


/*----------------------------------------------------------------------------*/
/* SampleFuncRegisterData                                                     */
/* param    : char  *pStr       log strings                                   */
/* return   : void                                                            */
/*----------------------------------------------------------------------------*/
void SampleFuncRegisterData(char *pStr)
{
    INT32 ret = 0;
    UINT8 status;

    int ch = 0;

    HVC_IMAGE *pImage = NULL;

        INT32 userId = -1;
    INT32 dataId = -1;


    /******************/
    /* Input          */
    /******************/
    printf_s("\nInput user ID : ");
    scanf("%d", &userId);
    scanf("%*c");

    if (userId < 0 || userId > 999) {
        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nInvalid user ID\n");
        return;
    }

    printf_s("Input data ID : ");
    scanf("%d", &dataId);
    scanf("%*c");

    if (dataId < 0 || dataId > 9) {
        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nInvalid data ID\n");
        return;
    }

    do {
        /*********************************/
        /* Image Structure allocation    */
        /*********************************/
        pImage = (HVC_IMAGE *)malloc(sizeof(HVC_IMAGE));
        if ( pImage == NULL ) { /* Error processing */
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nMemory Allocation Error : %08lx\n", sizeof(HVC_IMAGE));
            break;
        }

        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nPress the Enter Key to register a face : ");

        /******************/
        /* Log Output     */
        /******************/
        PrintLog(pStr);
        memset(pStr, 0, LOGBUFFERSIZE);

        /******************/
        /* Input          */
        /******************/
#ifdef WIN32
        do {
            ch = _getch();
            ch = toupper( ch );
        } while(ch != '\r' && ch != ' ');
#else
        do {
            ch = kbhit();
        } while(ch != '\n' && ch != ' ');
#endif

        /*********************************/
        /* Execute Registration          */
        /*********************************/
        ret = HVC_Registration(UART_GENERAL_TIMEOUT, userId, dataId, pImage, &status);
        if ( ret != 0 ) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVCApi(HVC_Registration) Error : %d\n", ret);
            continue;
        }
        if ( status != 0 ) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVC_Registration Error : 0x%02X\n", status);
            continue;
        }

        SaveBitmapFile(pImage->width, pImage->height, pImage->image, "RegisterImage.bmp");
        
        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nRegistration complete.\n");

    } while (0);

    /********************************/
    /* Free image area             */
    /********************************/
    if( pImage != NULL ){
        free(pImage);
    }
}


/*----------------------------------------------------------------------------*/
/* SampleFuncDeleteData                                                       */
/* param    : char  *pStr       log strings                                   */
/* return   : void                                                            */
/*----------------------------------------------------------------------------*/
void SampleFuncDeleteData(char *pStr)
{
    INT32 ret = 0;
    UINT8 status;

    INT32 userId = -1;
    INT32 dataId = -1;


    /******************/
    /* Input          */
    /******************/
    printf_s("\nInput user ID : ");
    scanf("%d", &userId);
    scanf("%*c");

    if (userId < 0 || userId > 999) {
        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nInvalid user ID\n");
        return;
    }

    printf_s("Input data ID : ");
    scanf("%d", &dataId);
    scanf("%*c");

    if (dataId < 0 || dataId > 9) {
        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nInvalid data ID\n");
        return;
    }

    /*********************************/
    /* Delete data                   */
    /*********************************/
    ret = HVC_DeleteData(UART_GENERAL_TIMEOUT, userId, dataId, &status);
    if ( ret != 0 ) {
        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVCApi(HVC_DeleteData) Error : %d\n", ret);
        return;
    }
    if ( status != 0 ) {
        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVC_DeleteData Error : 0x%02X\n", status);
        return;
    }

    sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nDelete specified data complete.\n");
}


/*----------------------------------------------------------------------------*/
/* SampleFuncDeleteUser                                                       */
/* param    : char  *pStr       log strings                                   */
/* return   : void                                                            */
/*----------------------------------------------------------------------------*/
void SampleFuncDeleteUser(char *pStr)
{
    INT32 ret = 0;
    UINT8 status;

    INT32 userId = -1;


    /******************/
    /* Input          */
    /******************/
    printf_s("\nInput user ID : ");
    scanf("%d", &userId);
    scanf("%*c");

    if (userId < 0 || userId > 999) {
        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nInvalid user ID\n");
        return;
    }

    /*********************************/
    /* Delete user                   */
    /*********************************/
    ret = HVC_DeleteUser(UART_GENERAL_TIMEOUT, userId, &status);
    if ( ret != 0 ) {
        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVCApi(HVC_DeleteUser) Error : %d\n", ret);
        return;
    }
    if ( status != 0 ) {
        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVC_DeleteUser Error : 0x%02X\n", status);
        return;
    }

    sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nDelete specified user complete.\n");
}


/*----------------------------------------------------------------------------*/
/* SampleFuncDeleteAll                                                        */
/* param    : char  *pStr       log strings                                   */
/* return   : void                                                            */
/*----------------------------------------------------------------------------*/
void SampleFuncDeleteAll(char *pStr)
{
    INT32 ret = 0;
    UINT8 status;


    /*********************************/
    /* Delete all data               */
    /*********************************/
    ret = HVC_DeleteAll(UART_GENERAL_TIMEOUT, &status);
    if ( ret != 0 ) {
        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVCApi(HVC_DeleteAll) Error : %d\n", ret);
        return;
    }
    if ( status != 0 ) {
        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVC_DeleteAll Error : 0x%02X\n", status);
        return;
    }

    sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nDelete all data complete.\n");
}


/*----------------------------------------------------------------------------*/
/* SampleFuncSaveAlbum                                                        */
/* param    : char  *pStr       log strings                                   */
/* return   : void                                                            */
/*----------------------------------------------------------------------------*/
void SampleFuncSaveAlbum(char *pStr)
{
    INT32 ret = 0;
    UINT8 status;

    UINT8 *pAlbumData = NULL;
    INT32 albumDataSize = 0;


    /*********************************/
    /* Album data allocation         */
    /*********************************/
    pAlbumData = (UINT8*)malloc(sizeof(UINT8) * (HVC_ALBUM_SIZE_MAX + 8));
    if ( pAlbumData == NULL ) { /* Error processing */
        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nMemory Allocation Error : %08lx\n", sizeof(UINT8) * (HVC_ALBUM_SIZE_MAX + 8));
        return;
    }

    do {
        /*********************************/
        /* Save Album                    */
        /*********************************/
        ret = HVC_SaveAlbum(UART_GENERAL_TIMEOUT, pAlbumData, &albumDataSize, &status);
        if ( ret != 0 ) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVCApi(HVC_SaveAlbum) Error : %d\n", ret);
            break;
        }
        if ( status != 0 ) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVC_SaveAlbum Response Error : 0x%02X\n", status);
            break;
        }

        ret = SaveAlbumData("HVCAlbum.alb", albumDataSize, pAlbumData);
        if (ret != 0) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nFailed to output the album data file : HVCAlbum.alb\n");
        }

        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nSave Album complete.\n");

    } while (0);

        /********************************/
    /* Free album data area         */
    /********************************/
    if ( pAlbumData != NULL ){
        free(pAlbumData);
        pAlbumData = NULL;
    }
}


/*----------------------------------------------------------------------------*/
/* SampleFuncLoadAlbum                                                        */
/* param    : char  *pStr       log strings                                   */
/* return   : void                                                            */
/*----------------------------------------------------------------------------*/
void SampleFuncLoadAlbum(char *pStr)
{
    INT32 ret = 0;
    UINT8 status;

    UINT8 *pAlbumData = NULL;
    INT32 albumDataSize = 0;


    /*********************************/
    /* Album data allocation         */
    /*********************************/
    pAlbumData = (UINT8*)malloc(sizeof(UINT8) * (HVC_ALBUM_SIZE_MAX + 8));
    if ( pAlbumData == NULL ) { /* Error processing */
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nMemory Allocation Error : %08lx\n", sizeof(UINT8) * (HVC_ALBUM_SIZE_MAX + 8));
            return;
    }

    do {
        /*********************************/
        /* Load Album                    */
        /*********************************/
        ret = LoadAlbumData("HVCAlbum.alb", &albumDataSize, pAlbumData);
        if (ret != 0) {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nFaile to open the album data faile : HVCAlbum.alb\n");
            break;
        }

        if ( albumDataSize != 0 ) {
            ret = HVC_LoadAlbum(UART_GENERAL_TIMEOUT, pAlbumData, albumDataSize, &status);
            if ( ret != 0 ) {
                sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVCApi(HVC_LoadAlbum) Error : %d\n", ret);
                break;
            }
            if ( status != 0 ) {
                sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVC_LoadAlbum Response Error : 0x%02X\n", status);
                break;
            }
        }
        else {
            sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nThe album data could not be read : HVCAlbum.alb\n");
            break;
        }

        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nLoad Album complete.\n");

    } while (0);

    /********************************/
    /* Free album data area         */
    /********************************/
    if ( pAlbumData != NULL ){
        free(pAlbumData);
        pAlbumData = NULL;
    }
}


/*----------------------------------------------------------------------------*/
/* SampleFuncWriteAlbum                                                       */
/* param    : char  *pStr       log strings                                   */
/* return   : void                                                            */
/*----------------------------------------------------------------------------*/
void SampleFuncWriteAlbum(char *pStr)
{
    INT32 ret = 0;
    UINT8 status;


    /*********************************/
    /* Write Album                   */
    /*********************************/
    ret = HVC_WriteAlbum(UART_WRITE_ALBUM_TIMEOUT, &status);
    if ( ret != 0 ) {
        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVCApi(HVC_WriteAlbum) Error : %d\n", ret);
        return;
    }
    if ( status != 0 ) {
        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVC_WriteAlbum Response Error : 0x%02X\n", status);
        return;
    }

    sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nSave Album on Flash ROM complete.\n");
}


/*----------------------------------------------------------------------------*/
/* SampleFuncReformatAlbum                                                    */
/* param    : char  *pStr       log strings                                   */
/* return   : void                                                            */
/*----------------------------------------------------------------------------*/
void SampleFuncReformatAlbum(char *pStr)
{
    INT32 ret = 0;
    UINT8 status;


    /*********************************/
    /* Write Album                   */
    /*********************************/
    ret = HVC_ReformatAlbum(UART_REFORMAT_ALBUM_TIMEOUT, &status);
    if ( ret != 0 ) {
        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVCApi(HVC_ReformatAlbum) Error : %d\n", ret);
        return;
    }
    if ( status != 0 ) {
        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVC_ReformatAlbum Response Error : 0x%02X\n", status);
        return;
    }

    sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nReformat Flash ROM complete.\n");
}


/*----------------------------------------------------------------------------*/
/* SampleFuncSetRegistCount                                                   */
/* param    : char  *pStr       log strings                                   */
/* return   : void                                                            */
/*----------------------------------------------------------------------------*/
void SampleFuncSetRegistCount(char *pStr)
{
    INT32 ret = 0;
    UINT8 status;

    INT32 usrCnt = 0;


    /******************/
    /* Input          */
    /******************/
    printf_s("\nInput user count [0:100 1:500 2:1000] : ");
    scanf("%d", &usrCnt);
    scanf("%*c");

    if (usrCnt < 0 || usrCnt > 2) {
        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nInvalid user count\n");
        return;
    }

    /*********************************/
    /* Write Album                   */
    /*********************************/
    ret = HVC_SetRegistCount(UART_REGIST_COUNT_TIMEOUT, usrCnt, &status);
    if ( ret != 0 ) {
        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVCApi(HVC_SetRegistCount) Error : %d\n", ret);
        return;
    }
    if ( status != 0 ) {
        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVC_SetRegistCount Response Error : 0x%02X\n", status);
        return;
    }

    sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nSet Number of registered people in album complete.\n");
}


/*----------------------------------------------------------------------------*/
/* SampleFuncGetRegistCount                                                   */
/* param    : char  *pStr       log strings                                   */
/* return   : void                                                            */
/*----------------------------------------------------------------------------*/
void SampleFuncGetRegistCount(char *pStr)
{
    INT32 ret = 0;
    UINT8 status;

    INT32 usrCnt = 0;
    int   regNum[3] = {100, 500, 1000};

    /*********************************/
    /* Write Album                   */
    /*********************************/
    ret = HVC_GetRegistCount(UART_GENERAL_TIMEOUT, &usrCnt, &status);
    if ( ret != 0 ) {
        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVCApi(SampleFuncGetRegistCount) Error : %d\n", ret);
        return;
    }
    if ( status != 0 ) {
        sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVC_GetRegistCount Response Error : 0x%02X\n", status);
        return;
    }

    sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nRegistration user max num : %d [ %d ]\n", usrCnt, regNum[usrCnt]);
    sprintf_s(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nGet Number of registered people in album complete.\n");
}


#ifndef WIN32
/*----------------------------------------------------------------------------*/
/* kbhit for Linux                                                            */
/* param    : void                                                            */
/* return   : int                    input keyboard character code            */
/*----------------------------------------------------------------------------*/
int kbhit(void)
{
    struct termios oldAttr, newAttr;
    int flg;
    int ch;
        int ret;

    tcgetattr(STDIN_FILENO, &oldAttr);
    newAttr = oldAttr;
    newAttr.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newAttr);
    flg = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flg | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldAttr);
    fcntl(STDIN_FILENO, F_SETFL, flg);

    if (ch == EOF) {
        ret = -1;
    }
    else {
        ret = ch;
    }

    return ret;
}
#endif

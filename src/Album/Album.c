/*---------------------------------------------------------------------------*/
/* Copyright(C)  2017  OMRON Corporation                                     */
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

#include <stdio.h>

#ifndef WIN32
#endif


int SaveAlbumData(const char *inFileName, int inDataSize, unsigned char *inAlbumData)
{
    FILE* pFile = NULL;

#ifdef WIN32
    fopen_s(&pFile, inFileName, "wb");
#else
    pFile = fopen(inFileName, "wb");
#endif
    if(NULL == pFile){
        return -1;
    }

    fwrite(inAlbumData, inDataSize, 1, pFile);

    fclose(pFile);

    return 0;
}

int LoadAlbumData(const char *inFileName, int *outDataSize, unsigned char *outAlbumData)
{
    FILE *pFile = NULL;

    *outDataSize = 0;

#ifdef WIN32
    fopen_s(&pFile, inFileName, "rb");
#else
    pFile = fopen(inFileName, "rb");
#endif
    if(NULL == pFile){
        return -1;
    }

    fseek(pFile, 0, SEEK_END);
    *outDataSize = ftell(pFile);

    fseek(pFile, 0, SEEK_SET);

    fread(outAlbumData, *outDataSize, 1, pFile);

    fclose(pFile);

    return 0;
}

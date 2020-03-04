/***************************************************************************
*                                                                         *
*                   SiRF Technology, Inc. Platform Software               *
*                                                                         *
*    Copyright (c) 2008 by SiRF Technology, Inc.  All rights reserved.    *
*                                                                         *
*    This Software is protected by United States copyright laws and       *
*    international treaties.  You may not reverse engineer, decompile     *
*    or disassemble this Software.                                        *
*                                                                         *
*    WARNING:                                                             *
*    This Software contains SiRF Technology, Inc.'s confidential and      *
*    proprietary information. UNAUTHORIZED COPYING, USE, DISTRIBUTION,    *
*    PUBLICATION, TRANSFER, SALE, RENTAL OR DISCLOSURE IS PROHIBITED      *
*    AND MAY RESULT IN SERIOUS LEGAL CONSEQUENCES.  Do not copy this      *
*    Software without SiRF Technology, Inc.'s  express written            *
*    permission.   Use of any portion of the contents of this Software    *
*    is subject to and restricted by your written agreement with          *
*    SiRF Technology, Inc.                                                *
*                                                                         *
***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <linux/fb.h>
#include <cutils/properties.h>

#include "sirfsoc_fb.h"

typedef long BOOL, LONG;
typedef unsigned char BYTE, *LPBYTE;
typedef unsigned short WORD, *LPWORD;
typedef unsigned long DWORD, *LPDWORD;
typedef char TCHAR,*LPTSTR;
typedef const TCHAR *LPCTSTR;

#define USE_FB 1

#define TRUE 1
#define FALSE 0
#define MAX_PATH 260
#define _T(x) x

#define DEBUG 1
#if DEBUG
#define _tprintf printf
#else
#define _tprintf
#endif

#define _stsprintf sprintf
#define _stscanf sscanf
#define _tcscpy strcpy
#define _tmain main
#define TAG "goodbye"

#define BITMAP_ID 0x4D42
#define DWORD_BYTE 4
#define BYTE_BIT 8
#define DWORD_BIT 32
#define ALPHA_TRANSPARENT 0
#define ALPHA_OPAQUE 255
#define BI_RGB 0
#define BI_BITFIELDS 3

typedef struct tagCOLORFORMAT
{
    LPCTSTR szName;
    int nId;
} COLORFORMAT,*LPCOLORFORMAT;

COLORFORMAT ColorFormatList[] =
{
    {"RGB565", FORMAT_RGB_565},
    {"RGBA8888", FORMAT_RGBA_8888},
    {"BGRA8888", FORMAT_BGRA_8888},
    {"", 0}
};

#pragma pack(2)
typedef struct tagBITMAPFILEHEADER
{
    WORD bfType;
    DWORD bfSize;
    WORD bfReserved1;
    WORD bfReserved2;
    DWORD bfOffBits;
} BITMAPFILEHEADER;
#pragma pack()

typedef struct tagBITMAPINFOHEADER
{
    DWORD biSize;
    LONG biWidth;
    LONG biHeight;
    WORD biPlanes;
    WORD biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    LONG biXPelsPerMeter;
    LONG biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
} BITMAPINFOHEADER;

typedef struct PALETTEENTRY
{
    BYTE peRed;
    BYTE peGreen;
    BYTE peBlue;
    BYTE peFlags;
} PALETTEENTRY, *LPPALETTEENTRY;

typedef struct tagRGBBITFIELD
{
    DWORD dwRedMask;
    DWORD dwGreenMask;
    DWORD dwBlueMask;
} RGBBITFIELD, *LPRGBBITFIELD;

typedef struct tagRGBQUAD
{
    BYTE rgbBlue;
    BYTE rgbGreen;
    BYTE rgbRed;
    BYTE rgbReserved;
} RGBQUAD, *LPRGBQUAD;

typedef struct tagIAMGEINFO
{
    int nWidth;
    int nHeight;
    int nFrameLength;
    LPRGBQUAD lpImageData;
} IAMGEINFO, *LPIAMGEINFO;

typedef struct tagOVERLAYPROPERTY
{
    int nColorFormat;
    BOOL bUseColorKey;
    DWORD dwColorKeyMin;
    DWORD dwColorKeyMax;
    BOOL bUseAlphaValue;
    int nAlphaValueMin;
    int nAlphaValueMax;
    TCHAR szFilePath[MAX_PATH];

    int nXPixelStride;
    int nYPixelStride;
    int nXByteStride;
    int nYByteStride;
    int nBufferNum;
    int nFd;
} OVERLAYPROPERTY, *LPOVERLAYPROPERTY;

typedef struct tagFBPROPERTY
{
    int nColorFormat;
    TCHAR szFilePath[MAX_PATH];
    struct fb_fix_screeninfo sFix;
    struct fb_var_screeninfo sVar;
    int nFd;
} FBPROPERTY, *LPFBPROPERTY;

typedef struct tagTESTPARAM
{
#ifdef USE_FB
    FBPROPERTY Fb;
#else
    OVERLAYPROPERTY Overlay;
#endif
} TESTPARAM, *LPTESTPARAM;

BOOL gHibernating = FALSE;

//Parse command line to get test parameters, test parameters are stored in lpTestParam.
BOOL GetParameters(int argc, TCHAR* argv[], LPTESTPARAM lpTestParam)
{
    LPTSTR pCurChar;
    BOOL bSuccess;
    int nPos;
    int i;

    if (!lpTestParam)
    {
        _tprintf(_T(TAG": Test Param Init Failed!\n"));
        goto ERROR_HANDLER;
    }

    for (nPos=1;nPos<argc;nPos++)
    {
        pCurChar = argv[nPos];
        if (*pCurChar == '&')//meet a option here
        {
            break;
        }
        if (*pCurChar != '-')//meet a option here
        {
            goto CMD_ERROR_HANDLER;
        }
        pCurChar++;
        switch (*pCurChar)
        {
        case _T('p'):
        case _T('P')://<File Path>
            _tprintf(_T(TAG": %s\n"), pCurChar);
            nPos++;
            if (nPos >= argc)
            {
                goto CMD_ERROR_HANDLER;
            }
            pCurChar = argv[nPos];
#ifdef USE_FB
            bSuccess = _stscanf(pCurChar, "%s", lpTestParam->Fb.szFilePath);
            _tprintf(_T(TAG": -P %s\n"), lpTestParam->Fb.szFilePath);
#else
            bSuccess = _stscanf(pCurChar, "%s", lpTestParam->Overlay.szFilePath);
            _tprintf(_T(TAG": -P %s\n"), lpTestParam->Overlay.szFilePath);
#endif
            if (bSuccess <= 0)
            {
                goto CMD_ERROR_HANDLER;
            }
            break;
        case _T('G'):
        case _T('g')://goodbye
            nPos++;
            gHibernating = TRUE;
            break;
        default:
            goto CMD_ERROR_HANDLER;
            break;
        }
    }
    _tprintf(_T("TestMemory: GetParameters has successfully completed!\n"));
    return TRUE;

CMD_ERROR_HANDLER:
    _tprintf(_T(TAG": Illegal Command Line!\n"));
    _tprintf(_T("Usage: goodbye -P <File Path> [-G]\n"));

ERROR_HANDLER:
    _tprintf(_T(TAG": GetParameters Failed!\n"));
    return FALSE;
}

inline BOOL CheckReadError(LPTSTR szFile, unsigned int uBytesToRead, int nBytesRead)
{
    if (nBytesRead == -1)
    {
        _tprintf(TAG": Error Reading File: %s!\n", szFile);
        return FALSE;
    }
    if (nBytesRead < (int)uBytesToRead)
    {
        _tprintf(TAG": Unexpected File End: %s!\n", szFile);
        return FALSE;
    }
    return TRUE;
}

DWORD GetFirstSetBitIndex(DWORD dwValue)
{
    DWORD dwBitIndex=0;
    int i;

    for (i=0;i<DWORD_BIT;i++)
    {
        if (dwValue&(1<<i))
        {
            break;
        }
        dwBitIndex++;
    }
    return dwBitIndex;
}

BOOL ReadBMPLine(const int hBMPFile, const LPTSTR szBMPFile, const int nRowPos, const int nDwordStride, const WORD wBitCount,
                 const LPPALETTEENTRY lpPalette, const LPRGBBITFIELD lpBitField, LPIAMGEINFO lpImageInfo)
{
    int nBytesRead;
    int nDwordPos;
    DWORD dwData;
    int nPixelPos=0;
    int nBitPos;
    DWORD dwColorMask=0;
    DWORD dwColorData;
    BOOL bReachLineEnd=FALSE;
    int nRGBIndex=0;
    int nByteIndex;

    for (nDwordPos=0;nDwordPos<nDwordStride;nDwordPos++)
    {
        nBytesRead = read(hBMPFile, &dwData, DWORD_BYTE);//BMP File Is DWORD Aligned
        if (!CheckReadError(szBMPFile, DWORD_BYTE, nBytesRead))
        {
            goto ERROR_HANDLER;
        }
        switch (wBitCount)
        {
        case 1:
        case 4:
        case 8:
            dwColorMask = ((DWORD)1<<wBitCount) - 1;
            for (nByteIndex=0;nByteIndex<DWORD_BYTE;nByteIndex++)
            {
                for (nBitPos=nByteIndex*BYTE_BIT+BYTE_BIT-wBitCount;nBitPos>=nByteIndex*8;nBitPos-=wBitCount)
                {
                    dwColorData = (dwData&(dwColorMask<<nBitPos))>>nBitPos;
                    lpImageInfo->lpImageData[nRowPos*lpImageInfo->nWidth+nPixelPos].rgbRed =
                        lpPalette[dwColorData].peBlue;
                    lpImageInfo->lpImageData[nRowPos*lpImageInfo->nWidth+nPixelPos].rgbGreen =
                        lpPalette[dwColorData].peGreen;
                    lpImageInfo->lpImageData[nRowPos*lpImageInfo->nWidth+nPixelPos].rgbBlue =
                        lpPalette[dwColorData].peRed;
                    lpImageInfo->lpImageData[nRowPos*lpImageInfo->nWidth+nPixelPos].rgbReserved = ALPHA_OPAQUE;
                    nPixelPos++;
                    if (nPixelPos >= lpImageInfo->nWidth)
                    {
                        bReachLineEnd = TRUE;
                        break;
                    }
                }
                if (bReachLineEnd)
                {
                    break;
                }
            }
            break;
        case 16:
            if (lpBitField)//BI_BITFIELDS
            {
                for (nBitPos=0;nBitPos<DWORD_BIT;nBitPos+=wBitCount)
                {
                    dwColorData = (dwData&(dwColorMask<<nBitPos))>>nBitPos;
                    lpImageInfo->lpImageData[nRowPos*lpImageInfo->nWidth+nPixelPos].rgbRed =
                        (BYTE)((dwColorData&lpBitField->dwRedMask)>>GetFirstSetBitIndex(lpBitField->dwRedMask));
                    lpImageInfo->lpImageData[nRowPos*lpImageInfo->nWidth+nPixelPos].rgbGreen =
                        (BYTE)((dwColorData&lpBitField->dwGreenMask)>>GetFirstSetBitIndex(lpBitField->dwGreenMask));
                    lpImageInfo->lpImageData[nRowPos*lpImageInfo->nWidth+nPixelPos].rgbBlue =
                        (BYTE)((dwColorData&lpBitField->dwBlueMask)>>GetFirstSetBitIndex(lpBitField->dwBlueMask));
                    lpImageInfo->lpImageData[nRowPos*lpImageInfo->nWidth+nPixelPos].rgbReserved = ALPHA_OPAQUE;
                    nPixelPos++;
                    if (nPixelPos >= lpImageInfo->nWidth)
                    {
                        bReachLineEnd = TRUE;
                        break;
                    }
                }
            }
            else//BI_RGB(RGB555)
            {
                dwColorMask = ((DWORD)1<<wBitCount) - 1;
                for (nBitPos=0;nBitPos<DWORD_BIT;nBitPos+=wBitCount)
                {
                    dwColorData = (dwData&(dwColorMask<<nBitPos))>>nBitPos;
                    lpImageInfo->lpImageData[nRowPos*lpImageInfo->nWidth+nPixelPos].rgbRed =
                        (BYTE)(((dwColorData&0x7C00)>>7)+7);
                    lpImageInfo->lpImageData[nRowPos*lpImageInfo->nWidth+nPixelPos].rgbGreen =
                        (BYTE)(((dwColorData&0x03E0)>>2)+7);
                    lpImageInfo->lpImageData[nRowPos*lpImageInfo->nWidth+nPixelPos].rgbBlue =
                        (BYTE)(((dwColorData&0x001F)<<3)+7);
                    lpImageInfo->lpImageData[nRowPos*lpImageInfo->nWidth+nPixelPos].rgbReserved = ALPHA_OPAQUE;
                    nPixelPos++;
                    if (nPixelPos >= lpImageInfo->nWidth)
                    {
                        bReachLineEnd = TRUE;
                        break;
                    }
                }
            }
            break;
        case 24:
            switch (nRGBIndex)
            {
            case 0:
                lpImageInfo->lpImageData[nRowPos*lpImageInfo->nWidth+nPixelPos].rgbBlue = (BYTE)(dwData&0x0000ff);
                lpImageInfo->lpImageData[nRowPos*lpImageInfo->nWidth+nPixelPos].rgbGreen = (BYTE)((dwData&0x00ff00)>>8);
                lpImageInfo->lpImageData[nRowPos*lpImageInfo->nWidth+nPixelPos].rgbRed = (BYTE)((dwData&0xff0000)>>16);
                lpImageInfo->lpImageData[nRowPos*lpImageInfo->nWidth+nPixelPos].rgbReserved = ALPHA_OPAQUE;
                nPixelPos++;
                if (nPixelPos >= lpImageInfo->nWidth)
                {
                    bReachLineEnd = TRUE;
                    break;
                }
                lpImageInfo->lpImageData[nRowPos*lpImageInfo->nWidth+nPixelPos].rgbBlue = (BYTE)((dwData&0xff000000)>>24);
                nRGBIndex = 1;
                break;
            case 1:
                lpImageInfo->lpImageData[nRowPos*lpImageInfo->nWidth+nPixelPos].rgbGreen = (BYTE)(dwData&0x0000ff);
                lpImageInfo->lpImageData[nRowPos*lpImageInfo->nWidth+nPixelPos].rgbRed = (BYTE)((dwData&0x00ff00)>>8);
                lpImageInfo->lpImageData[nRowPos*lpImageInfo->nWidth+nPixelPos].rgbReserved = ALPHA_OPAQUE;
                nPixelPos++;
                if (nPixelPos >= lpImageInfo->nWidth)
                {
                    bReachLineEnd = TRUE;
                    break;
                }
                lpImageInfo->lpImageData[nRowPos*lpImageInfo->nWidth+nPixelPos].rgbBlue = (BYTE)((dwData&0xff0000)>>16);
                lpImageInfo->lpImageData[nRowPos*lpImageInfo->nWidth+nPixelPos].rgbGreen = (BYTE)((dwData&0xff000000)>>24);
                nRGBIndex = 2;
                break;
            case 2:
                lpImageInfo->lpImageData[nRowPos*lpImageInfo->nWidth+nPixelPos].rgbRed = (BYTE)(dwData&0x0000ff);
                lpImageInfo->lpImageData[nRowPos*lpImageInfo->nWidth+nPixelPos].rgbReserved = ALPHA_OPAQUE;
                nPixelPos++;
                if (nPixelPos >= lpImageInfo->nWidth)
                {
                    bReachLineEnd = TRUE;
                    break;
                }
                lpImageInfo->lpImageData[nRowPos*lpImageInfo->nWidth+nPixelPos].rgbBlue = (BYTE)((dwData&0x00ff00)>>8);
                lpImageInfo->lpImageData[nRowPos*lpImageInfo->nWidth+nPixelPos].rgbGreen = (BYTE)((dwData&0xff0000)>>16);
                lpImageInfo->lpImageData[nRowPos*lpImageInfo->nWidth+nPixelPos].rgbRed = (BYTE)((dwData&0xff000000)>>24);
                lpImageInfo->lpImageData[nRowPos*lpImageInfo->nWidth+nPixelPos].rgbReserved = ALPHA_OPAQUE;
                nPixelPos++;
                if (nPixelPos >= lpImageInfo->nWidth)
                {
                    bReachLineEnd = TRUE;
                    break;
                }
                nRGBIndex = 0;
                break;
            }
            break;
        case 32:
            lpImageInfo->lpImageData[nRowPos*lpImageInfo->nWidth+nPixelPos].rgbBlue = (BYTE)(dwData&0x0000ff);
            lpImageInfo->lpImageData[nRowPos*lpImageInfo->nWidth+nPixelPos].rgbGreen = (BYTE)((dwData&0x00ff00)>>8);
            lpImageInfo->lpImageData[nRowPos*lpImageInfo->nWidth+nPixelPos].rgbRed = (BYTE)((dwData&0xff0000)>>16);
            lpImageInfo->lpImageData[nRowPos*lpImageInfo->nWidth+nPixelPos].rgbReserved = (BYTE)((dwData&0xff000000)>>24);
            nPixelPos++;
            if (nPixelPos >= lpImageInfo->nWidth)
            {
                bReachLineEnd = TRUE;
                break;
            }
            break;
        default:
            break;
        }
        if (bReachLineEnd)
        {
            break;
        }
    }
//    _tprintf(TAG": ReadBMPLine Has Successfully Completed!\n");
    return TRUE;

ERROR_HANDLER:
    _tprintf(TAG": ReadBMPLine Failed!\n");
    return FALSE;
}

BOOL LoadBitmapFile(LPTSTR szBMPFile, LPIAMGEINFO lpImageInfo)
{
    int hBMPFile;
    BITMAPFILEHEADER bmf;
    BITMAPINFOHEADER bmi;
    unsigned int uBytesToRead;
    int nBytesRead;
    int nPaletteSize;
    LPPALETTEENTRY lpPalette=NULL;
    RGBBITFIELD BitField;
    LPRGBBITFIELD lpBitField=NULL;
    int nByteLine;
    int nDwordStride;
    int nRowPos;

    memset(lpImageInfo, 0, sizeof(IAMGEINFO));
    //Open BMP File
    hBMPFile = open(szBMPFile, O_RDONLY);
    if (hBMPFile == -1)
    {
        _tprintf(TAG": Failed To Open Bitmap File: %s!\n", szBMPFile);
        goto ERROR_HANDLER;
    }
    //Read BMP Header
    uBytesToRead = sizeof(bmf);
    nBytesRead = read(hBMPFile, &bmf, uBytesToRead);
    if (!CheckReadError(szBMPFile, uBytesToRead, nBytesRead))
    {
        goto ERROR_HANDLER;
    }
    if (bmf.bfType != BITMAP_ID)
    {
        _tprintf(TAG": Unsupported Format: %s is not a bitmap!\n", szBMPFile);
        goto ERROR_HANDLER;
    }
    uBytesToRead = sizeof(bmi);
    nBytesRead = read(hBMPFile, &bmi, uBytesToRead);
    if (!CheckReadError(szBMPFile, uBytesToRead, nBytesRead))
    {
        goto ERROR_HANDLER;
    }
    //Read BMP Palette If Exist
    if(!bmi.biBitCount)
    {
        _tprintf(TAG": Unsupported Format: %s is not a bitmap!\n", szBMPFile);
        goto ERROR_HANDLER;
    }
    if (bmi.biBitCount < 16)//Have Palette
    {
        if (!bmi.biClrUsed)
        {
            nPaletteSize = 1 << bmi.biBitCount;
        }
        else
        {
            nPaletteSize = bmi.biClrUsed;
        }
        lpPalette = (LPPALETTEENTRY)malloc(nPaletteSize*sizeof(PALETTEENTRY));
        if (!lpPalette)
        {
            _tprintf(TAG": Allocate Palette Buffer Failed!\n");
            goto ERROR_HANDLER;
        }
        uBytesToRead = nPaletteSize*sizeof(PALETTEENTRY);
        nBytesRead = read(hBMPFile, lpPalette, uBytesToRead);
        if (!CheckReadError(szBMPFile, uBytesToRead, nBytesRead))
        {
            goto ERROR_HANDLER;
        }
    }
    //Read BMP Bit Field Mask If Exist
    if(bmi.biCompression != BI_RGB)
    {
        if (bmi.biCompression != BI_BITFIELDS)
        {
            _tprintf(TAG": Unsupported Format: %s is a compressed bitmap or is not a bitmap!\n", szBMPFile);
            goto ERROR_HANDLER;
        }
        else if (bmi.biBitCount==16 || bmi.biBitCount==32)//Have Bit Field Mask
        {
            uBytesToRead = sizeof(BitField);
            nBytesRead = read(hBMPFile, &BitField, uBytesToRead);
            if (!CheckReadError(szBMPFile, uBytesToRead, nBytesRead))
            {
                goto ERROR_HANDLER;
            }
            lpBitField = &BitField;
        }
    }
    //Get BMP Basic Info
    nByteLine = (bmi.biWidth*bmi.biBitCount+7)/8;
    nDwordStride = (nByteLine+3)/4;
    lpImageInfo->nWidth = bmi.biWidth;
    if (bmi.biHeight < 0)
    {
        lpImageInfo->nHeight = -bmi.biHeight;
    }
    else
    {
        lpImageInfo->nHeight = bmi.biHeight;
    }
    lpImageInfo->nFrameLength = lpImageInfo->nWidth * lpImageInfo->nHeight * sizeof(RGBQUAD);
    lpImageInfo->lpImageData = (LPRGBQUAD)malloc(lpImageInfo->nFrameLength);
    if (!lpImageInfo->lpImageData)
    {
        _tprintf(TAG": Allocate Image Data Buffer Failed!\n");
        goto ERROR_HANDLER;
    }
    //Read BMP Data
    if (lseek(hBMPFile, bmf.bfOffBits, SEEK_SET) == -1L)
    {
        _tprintf(TAG": Error Seeking File: %s!\n", szBMPFile);
    }
    if (bmi.biHeight < 0)//Up-bottom Bitmap
    {
        for(nRowPos=0;nRowPos<lpImageInfo->nHeight;nRowPos++)
        {
            if (!ReadBMPLine(hBMPFile, szBMPFile, nRowPos, nDwordStride, bmi.biBitCount, lpPalette, lpBitField, lpImageInfo))
            {
                goto ERROR_HANDLER;
            }
        }
    }
    else//Bottom-up Bitmap
    {
        for(nRowPos=lpImageInfo->nHeight-1;nRowPos>=0;nRowPos--)
        {
            if (!ReadBMPLine(hBMPFile, szBMPFile, nRowPos, nDwordStride, bmi.biBitCount, lpPalette, lpBitField, lpImageInfo))
            {
                goto ERROR_HANDLER;
            }
        }
    }
    //Free Resources
    free(lpPalette);
    close(hBMPFile);
    _tprintf(TAG": LoadBitmapFile Has Successfully Completed!\n");
    return TRUE;

ERROR_HANDLER:
    if (lpPalette)
    {
        free(lpPalette);
    }
    if (lpImageInfo->lpImageData)
    {
        free(lpImageInfo->lpImageData);
        lpImageInfo->lpImageData = NULL;
    }
    if (hBMPFile != -1)
    {
        close(hBMPFile);
    }
    _tprintf(TAG": LoadBitmapFile Failed!\n");
    return FALSE;
}

void DestroyBitmap(LPIAMGEINFO lpImageInfo)
{
    if (lpImageInfo->lpImageData)
    {
        free(lpImageInfo->lpImageData);
        lpImageInfo->lpImageData = NULL;
    }
}

void ConvertToRGB565(LPBYTE lpAddress, int nWidthStride, const LPIAMGEINFO lpImageInfo)
{
    int x,y,nOverlayOffset,nPixelOffset;
    WORD wData;

    for (y=0;y<lpImageInfo->nHeight;y++)
    {
        nPixelOffset = y*lpImageInfo->nWidth;
        nOverlayOffset = y*nWidthStride;
        for (x=0;x<lpImageInfo->nWidth;x++)
        {
            wData = ((WORD)lpImageInfo->lpImageData[nPixelOffset].rgbBlue>>3) |
                ((WORD)lpImageInfo->lpImageData[nPixelOffset].rgbGreen>>2<<5) |
                ((WORD)lpImageInfo->lpImageData[nPixelOffset].rgbRed>>3<<11);
            *((LPWORD)(lpAddress+nOverlayOffset)) = wData;
            nPixelOffset++;
            nOverlayOffset+=2;
        }
    }
}

void ConvertToRGBA8888(LPBYTE lpAddress, int nWidthStride, const LPIAMGEINFO lpImageInfo)
{
    int x,y,nOverlayOffset,nPixelOffset;

    for (y=0;y<lpImageInfo->nHeight;y++)
    {
        nPixelOffset = y*lpImageInfo->nWidth;
        nOverlayOffset = y*nWidthStride;
        for (x=0;x<lpImageInfo->nWidth;x++)
        {
            lpAddress[nOverlayOffset] = lpImageInfo->lpImageData[nPixelOffset].rgbRed;
            lpAddress[nOverlayOffset+1] = lpImageInfo->lpImageData[nPixelOffset].rgbGreen;
            lpAddress[nOverlayOffset+2] = lpImageInfo->lpImageData[nPixelOffset].rgbBlue;
            lpAddress[nOverlayOffset+3] = lpImageInfo->lpImageData[nPixelOffset].rgbReserved;
            nPixelOffset++;
            nOverlayOffset+=4;
        }
    }
}

void ConvertToBGRA8888(LPBYTE lpAddress, int nWidthStride, const LPIAMGEINFO lpImageInfo)
{
    int x,y,nOverlayOffset,nPixelOffset;

    for (y=0;y<lpImageInfo->nHeight;y++)
    {
        nPixelOffset = y*lpImageInfo->nWidth;
        nOverlayOffset = y*nWidthStride;
        for (x=0;x<lpImageInfo->nWidth;x++)
        {
            lpAddress[nOverlayOffset] = lpImageInfo->lpImageData[nPixelOffset].rgbBlue;
            lpAddress[nOverlayOffset+1] = lpImageInfo->lpImageData[nPixelOffset].rgbGreen;
            lpAddress[nOverlayOffset+2] = lpImageInfo->lpImageData[nPixelOffset].rgbRed;
            lpAddress[nOverlayOffset+3] = lpImageInfo->lpImageData[nPixelOffset].rgbReserved;
            nPixelOffset++;
            nOverlayOffset+=4;
        }
    }
}


BOOL ConvertColorFormat(int nColorFormat, LPBYTE lpAddress, int nWidthStride, int nHeightStride, const LPIAMGEINFO lpImageInfo)
{
    switch (nColorFormat)
    {
    case FORMAT_RGB_565:
        ConvertToRGB565(lpAddress, nWidthStride*2, lpImageInfo);
        break;

    case FORMAT_RGBA_8888:
        ConvertToRGBA8888(lpAddress, nWidthStride*4, lpImageInfo);
        break;

    case FORMAT_BGRA_8888:
        ConvertToBGRA8888(lpAddress, nWidthStride*4, lpImageInfo);
        break;

    default:
        _tprintf(TAG": Objective Color Format Not Supported!\n");
        goto ERROR_HANDLER;
        break;
    }
//    _tprintf(TAG": ConvertColorFormat Has Successfully Completed!\n");
    return TRUE;

ERROR_HANDLER:
    _tprintf(TAG": ConvertColorFormat Failed!\n");
    return FALSE;
}

BOOL SetColorKey(BOOL bUseColorKey, int nColorKey)
{
    goto ERROR_HANDLER;
    return TRUE;

ERROR_HANDLER:
    _tprintf(TAG": SetColorKey Not Supported Yet!\n");
    return FALSE;
}

BOOL SetAlphaValue(BOOL bUseAlphaValue, int nAlphaValue)
{
    goto ERROR_HANDLER;
    return TRUE;

ERROR_HANDLER:
    _tprintf(TAG": SetAlphaValue Not Supported Yet!\n");
    return FALSE;
}

#ifdef USE_FB
BOOL DeinitFb(const LPFBPROPERTY lpFb)
{
#if 0
    if (ioctl(lpFb->nFd, SIRFSOCFB_DISABLE_LAYER))
    {
        _tprintf("Fail to destroy layer\n");
    }
#endif
    close(lpFb->nFd);
    return TRUE;
}

BOOL InitFb(const LPFBPROPERTY lpFb, LPIAMGEINFO lpImageInfo)
{
    if (!LoadBitmapFile(lpFb->szFilePath, lpImageInfo))
    {
        goto ERROR_HANDLER;
    }
    int fd = open("/dev/graphics/fb0", O_RDWR);
    if (-1 == ioctl(fd, FBIOGET_FSCREENINFO, &lpFb->sFix))
    {
	_tprintf("ERROR: FBIOGET_FSCREENINFO");
	goto ERROR_HANDLER;
    }
    if (-1 == ioctl(fd, FBIOGET_VSCREENINFO, &lpFb->sVar))
    {
	_tprintf("ERROR: FBIOGET_VSCREENINFO");
        goto ERROR_HANDLER;
    }

    if (ioctl(fd, SIRFSOCFB_SET_TOPLAYER)) {
        _tprintf("ERROR: SIRFSOCFB_SET_TOPLAYER\n");
        return FALSE;
    }


    lpFb->nFd = fd;

    _tprintf(TAG": InitFb Has Successfully Completed!\n");
    return TRUE;

ERROR_HANDLER:
    _tprintf(TAG": InitFb Failed!\n");
    return FALSE;
}
#else
BOOL DeinitOverlay(const LPOVERLAYPROPERTY lpOverlay)
{
    if (ioctl(lpOverlay->nFd, SIRFSOCFB_DESTROY_LAYER))
    {
        _tprintf("Fail to destroy layer\n");
    }
    close(lpOverlay->nFd);
    return TRUE;
}


BOOL InitOverlay(const LPOVERLAYPROPERTY lpOverlay, LPIAMGEINFO lpImageInfo)
{
    if (!LoadBitmapFile(lpOverlay->szFilePath, lpImageInfo))
    {
        goto ERROR_HANDLER;
    }

    int fd = open("/dev/graphics/fb1", O_RDWR);
    struct sirfsocfb_createlayer create;

    create.format = lpOverlay->nColorFormat;
    create.width = lpImageInfo->nWidth;
    create.height = lpImageInfo->nHeight;

    if (ioctl(fd, SIRFSOCFB_CREATE_LAYER, &create))
    {
        _tprintf("Fail to create layer, Force to destroy layer\n");
        if (ioctl(fd, SIRFSOCFB_DESTROY_LAYER))
        {
            _tprintf("Fail to force destroying layer\n");
            return FALSE;
        }
        if (ioctl(fd, SIRFSOCFB_CREATE_LAYER, &create))
        {
            _tprintf("Fail to create layer\n");
            return FALSE;
        }
    }

        if (ioctl(fd, SIRFSOCFB_SET_TOPLAYER)) {
                _tprintf("ERROR: SIRFSOCFB_SET_TOPLAYER\n");
        return FALSE;
        }


    lpOverlay->nXPixelStride = create.wstride_pixel;
    lpOverlay->nYPixelStride = create.hstride_pixel;
    lpOverlay->nXByteStride = create.wstride_byte;
    lpOverlay->nYByteStride = create.hstride_byte;
    lpOverlay->nBufferNum = create.num_buffers;
    lpOverlay->nFd = fd;

    _tprintf(TAG": InitOverlay Has Successfully Completed!\n");
    return TRUE;

ERROR_HANDLER:
    _tprintf(TAG": InitOverlay Failed!\n");
    return FALSE;
}
#endif

BOOL PerformShow(LPTESTPARAM lpTestParam)
{
    unsigned int w_stride;
    unsigned int h_stride;
    LPBYTE lpAddress, lpBase;
    IAMGEINFO ImageInfo;
    int x,y,xSize,ySize;
#ifdef USE_FB
    LPFBPROPERTY pFb = &lpTestParam->Fb;

    if (!InitFb(pFb, &ImageInfo))
    {
        goto ERROR_HANDLER;
    }
    w_stride = pFb->sVar.xres;
    h_stride = pFb->sVar.yres;

    if (!LoadBitmapFile(pFb->szFilePath, &ImageInfo))
    {
        goto ERROR_HANDLER;
    }
#else
    LPOVERLAYPROPERTY pOverlay = &lpTestParam->Overlay;
    int nBuffer;

    if (!InitOverlay(pOverlay, &ImageInfo))
    {
        goto ERROR_HANDLER;
    }

    w_stride = pOverlay->nXPixelStride;
    h_stride = pOverlay->nYPixelStride;

    if (!LoadBitmapFile(pOverlay->szFilePath, &ImageInfo))
    {
        goto ERROR_HANDLER;
    }
#endif

    _tprintf("w_stride = %d\n", w_stride);
    _tprintf("h_stride = %d\n", h_stride);

#ifdef USE_FB
    if ((lpAddress = mmap(0, pFb->sFix.smem_len, PROT_READ|PROT_WRITE, MAP_SHARED, pFb->nFd, 0)) == (void *) -1)
    {
        _tprintf("Fail to mmap buffers\n");
        goto ERROR_HANDLER;
    }

    if (!ConvertColorFormat(pFb->nColorFormat, lpAddress, w_stride, h_stride, &ImageInfo))
    {
        goto ERROR_HANDLER;
    }

    pFb->sVar.xoffset = 0;
    pFb->sVar.yoffset = 0;
    pFb->sVar.activate = FB_ACTIVATE_FORCE;
    if (-1 == ioctl(pFb->nFd, FBIOPUT_VSCREENINFO, &pFb->sVar))
    {
        _tprintf("ERROR: FBIOPUT_VSCREENINFO");
        goto ERROR_HANDLER;
    }

    if (-1 == ioctl(pFb->nFd, SIRFSOCFB_ENABLE_LAYER))
    {
	_tprintf("ERROR: SIRFSOCFB_ENABLE_LAYER");
        goto ERROR_HANDLER;
    }
    if (-1 == ioctl(pFb->nFd, SIRFSOCFB_SET_TOPLAYER))
    {
	_tprintf("ERROR: SIRFSOCFB_SET_TOPLAYER");
        goto ERROR_HANDLER;
    }
    munmap(lpAddress, pFb->sFix.smem_len);

#else
    if ((lpBase = mmap(0, pOverlay->nBufferNum*pOverlay->nYByteStride,
            PROT_READ|PROT_WRITE, MAP_SHARED, pOverlay->nFd, 0)) == (void *) -1)
    {
        _tprintf("Fail to mmap buffers\n");
        goto ERROR_HANDLER;
    }

    struct sirfsocfb_screen scr;

    if (ioctl(pOverlay->nFd, SIRFSOCFB_DQ_BUFFER, &nBuffer))
    {
        _tprintf("Fail to dequeue buffers\n");
        goto ERROR_HANDLER;
    }

    lpAddress = (LPBYTE)lpBase+(pOverlay->nYByteStride*nBuffer);

    if (!ConvertColorFormat(pOverlay->nColorFormat, lpAddress, w_stride, h_stride, &ImageInfo))
    {
        goto ERROR_HANDLER;
    }

    if (ioctl(pOverlay->nFd, SIRFSOCFB_Q_BUFFER, &nBuffer))
    {
        _tprintf("Fail to queue buffer %d\n", nBuffer);
        goto ERROR_HANDLER;
    }

    scr.xstart = 0;
    scr.ystart = 0;
    scr.xsize = w_stride;
    scr.ysize = h_stride;

    if (ioctl(pOverlay->nFd, SIRFSOCFB_SET_SCRSIZE, &scr))
    {
        _tprintf("Fail to set screen size\n");
        goto ERROR_HANDLER;
    }

    munmap(lpBase, pOverlay->nBufferNum*pOverlay->nYByteStride);
#endif

    if (gHibernating == TRUE) {
        _tprintf(TAG": It is the time to say goodbye!\n");

        property_set("sys.goodbye.status", "done");
        char value[PROPERTY_VALUE_MAX];
        while (1) {
            property_get("sys.hibernating", value, "");
            if (value[0] == '0')
                break;
            sleep(1);
        }

        _tprintf(TAG": I am back again!\n");
    } else {
        getchar();
    }

    DestroyBitmap(&ImageInfo);
#ifdef USE_FB
    DeinitFb(pFb);
#else
    DeinitOverlay(pOverlay);
#endif
    _tprintf(TAG": PerformShow Has Successfully Completed!\n");
    return TRUE;

ERROR_HANDLER:
    _tprintf(TAG": PerformShow Failed!\n");
    return FALSE;
}

BOOL Goodbye(int argc, TCHAR* argv[])
{
    TESTPARAM stTestParam;

    int ret, fd, width = 0, height = 0;
    struct fb_var_screeninfo vi;

    fd = open("/dev/graphics/fb0", O_RDWR);
    if (fd < 0)
            return fd;

    ret = ioctl(fd, FBIOGET_VSCREENINFO, &vi);

    if (!ret) {
            width = vi.xres;
            height = vi.yres;
    }

    _tprintf(TAG": the screen width is %d, the height is %d\n", width, height);
    close(fd);

#ifdef USE_FB
    stTestParam.Fb.nColorFormat = FORMAT_BGRA_8888;

    _stsprintf(stTestParam.Fb.szFilePath, "/system/etc/goodbye_%dx%d.bmp", width, height);
    _tprintf(TAG": the stTestParam.Fb.szFilePath is %s\n", stTestParam.Fb.szFilePath);
#else
    stTestParam.Overlay.nColorFormat = FORMAT_BGRA_8888;
    stTestParam.Overlay.bUseColorKey = FALSE;
    stTestParam.Overlay.bUseAlphaValue = FALSE;

    _stsprintf(stTestParam.Overlay.szFilePath, "/system/etc/goodbye_%dx%d.bmp", width, height);
    _tprintf(TAG": the stTestParam.Overlay.szFilePath is %s\n", stTestParam.Overlay.szFilePath);
#endif
    if (!GetParameters(argc, argv, &stTestParam))
    {
        _tprintf(_T(TAG": failed to GetParameters!\r\n"));
        return FALSE;
    }

    if (!PerformShow(&stTestParam))
    {
        _tprintf(TAG": failed to PerformShow!\r\n");
        return FALSE;
    }

    _tprintf(TAG": enjoy yourself\n");
    return TRUE;
}

int _tmain(int argc, TCHAR* argv[])
{
    Goodbye(argc, argv);
    return 0;
}


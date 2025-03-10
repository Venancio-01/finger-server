#include"stdio.h"
#include"stdlib.h"
#include"unistd.h"
#include<string.h>
#include <memory.h>
#include <dirent.h>
#include"include/tool.h"


#ifndef BYTE
#define BYTE unsigned char
#endif 

#ifndef BOOL
#define BOOL BYTE
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE (!FALSE)
#endif 

#ifndef HANDLE
#define HANDLE void*
#endif

#ifndef NULL
#define NULL (void*)0
#endif

#ifndef WORD
#define WORD unsigned short
#endif

#ifndef DWORD
#define DWORD unsigned int
#endif

#pragma pack (1)
typedef struct tagBITMAPINFOHEADER{
        DWORD      biSize;
        int       biWidth;
        int       biHeight;
        WORD       biPlanes;
        WORD       biBitCount;
        DWORD      biCompression;
        DWORD      biSizeImage;
        int       biXPelsPerMeter;
        int       biYPelsPerMeter;
        DWORD      biClrUsed;
        DWORD      biClrImportant;
} BITMAPINFOHEADER, *LPBITMAPINFOHEADER, *PBITMAPINFOHEADER;


typedef struct tagRGBQUAD {
        BYTE    rgbBlue;
        BYTE    rgbGreen;
        BYTE    rgbRed;
        BYTE    rgbReserved;
} RGBQUAD;
typedef RGBQUAD *LPRGBQUAD;

typedef struct tagBITMAPFILEHEADER {
        WORD    bfType;
        DWORD   bfSize;
        WORD    bfReserved1;
        WORD    bfReserved2;
        DWORD   bfOffBits;
} BITMAPFILEHEADER, *LPBITMAPFILEHEADER, *PBITMAPFILEHEADER;


typedef struct tagBITMAPINFO {
        BITMAPINFOHEADER    bmiHeader;
        RGBQUAD             bmiColors[1];
} BITMAPINFO, *LPBITMAPINFO, *PBITMAPINFO;
#pragma pack()



static int WriteBitmapHeader(unsigned char *Buffer, int Width, int Height)
{
        BITMAPFILEHEADER *bmpfheader=(BITMAPFILEHEADER *)Buffer;
        BITMAPINFO *bmpinfo=(BITMAPINFO *)(((char*)bmpfheader)+14);
        int i,w;
        memset(bmpfheader,0,0x500);
        bmpfheader->bfType =19778;
        w = ((Width+3)/4)*4*Height+sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFO)+255*sizeof(RGBQUAD);
        memcpy((void*)(((char*)bmpfheader)+2), &w, 4);
        //bmpfheader->bfOffBits;
        w= sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFO)+255*sizeof(RGBQUAD);
        memcpy((void*)(((char*)bmpfheader)+10), &w, 4);
        bmpinfo->bmiHeader.biWidth=Width;
        bmpinfo->bmiHeader.biHeight=Height;
        bmpinfo->bmiHeader.biBitCount=8;
        bmpinfo->bmiHeader.biClrUsed=0;
        bmpinfo->bmiHeader.biSize=sizeof(bmpinfo->bmiHeader);
        bmpinfo->bmiHeader.biPlanes=1;
        bmpinfo->bmiHeader.biSizeImage=((Width+3)/4)*4*Height;
        for(i=1;i<256;i++)
        {
                bmpinfo->bmiColors[i].rgbBlue=i;
                bmpinfo->bmiColors[i].rgbGreen=i;
                bmpinfo->bmiColors[i].rgbRed=i;
        }
        return sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+256*sizeof(RGBQUAD);
}

 int WriteBitmap(unsigned char *buffer, int Width, int Height, unsigned char* outBuffer)
{
        unsigned char Buffer[0x500];
        int i, w;
        int nHeadLen = WriteBitmapHeader(Buffer, Width, Height);
        int nPos = 0;
        memcpy(outBuffer, Buffer, nHeadLen);
        nPos += nHeadLen;
        w = ((Width+3)/4)*4;
        buffer+=Width*(Height-1);
        unsigned char bufFill[4];
        memset(bufFill, 0xFF, 4);
        for(i=0; i<Height; i++)
        {
                memcpy(outBuffer+nPos, buffer, Width);
                nPos+= Width;
                if(w-Width)
                {
                        memcpy(outBuffer+nPos, bufFill, w-Width);
                        nPos+= w-Width;
                }
                buffer-=Width;
        }
        return nPos;
}

 int WriteBitmapFile(BYTE *buffer, int Width, int Height, char *file)
{
        FILE *f=fopen(file, "wb");
        if(f)
        {
                unsigned char Buffer[0x500];
                int i, w=WriteBitmapHeader(Buffer, Width, Height);
                fwrite(Buffer, w, 1, f);
                w = ((Width+3)/4)*4;
                buffer+=Width*(Height-1);
                for(i=0; i<Height; i++)
                {
                        fwrite(buffer, Width, 1, f);
                        if(w-Width)
                                fwrite(buffer, w-Width, 1, f);
                        buffer-=Width;
                }
                fclose(f);
                return Width*Height;
        }
        return 0;
}


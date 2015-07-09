/*
 * bmp2rgb.hpp
 *
 *  Created on: Jul 9, 2015
 *      Author: Peter
 */

#ifndef BMP2RGB_HPP_
#define BMP2RGB_HPP_

/*
 * BMP image loading
 */

#include <iostream>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

typedef unsigned short  WORD;
typedef unsigned int   	DWORD;
typedef long            LONG;

#pragma pack(push, 1)

typedef struct tagBITMAPFILEHEADER
{
    WORD bfType;  //specifies the file type
    DWORD bfSize;  //specifies the size in bytes of the bitmap file
    WORD bfReserved1;  //reserved; must be 0
    WORD bfReserved2;  //reserved; must be 0
    DWORD bOffBits;  //species the offset in bytes from the bitmapfileheader to the bitmap bits
}BITMAPFILEHEADER;

#pragma pack(pop)

#pragma pack(push, 1)

typedef struct tagBITMAPINFOHEADER
{
    DWORD biSize;  //specifies the number of bytes required by the struct
    LONG biWidth;  //specifies width in pixels
    LONG biHeight;  //species height in pixels
    WORD biPlanes; //specifies the number of color planes, must be 1
    WORD biBitCount; //specifies the number of bit per pixel
    DWORD biCompression;//spcifies the type of compression
    DWORD biSizeImage;  //size of image in bytes
    LONG biXPelsPerMeter;  //number of pixels per meter in x axis
    LONG biYPelsPerMeter;  //number of pixels per meter in y axis
    DWORD biClrUsed;  //number of colors used by th ebitmap
    DWORD biClrImportant;  //number of colors that are important
}BITMAPINFOHEADER;

#pragma pack(pop)

#pragma pack(push, 1)
typedef struct
{
    uint8_t  rgbtRed;
    uint8_t  rgbtBlue;
    uint8_t  rgbtGreen;
}
RGBTRIPLE;
#pragma pack(pop)

RGBTRIPLE *LoadBitmapFile(char *filename, BITMAPINFOHEADER *bitmapInfoHeader)
{
    FILE *filePtr; //our file pointer
    BITMAPFILEHEADER bitmapFileHeader; //our bitmap file header
    RGBTRIPLE *RGBbitmap;

    unsigned char *bitmapImage;  //store image data

    int imageIdx=0;  //image index counter
    unsigned char tempRGB;  //our swap variable


    //open filename in read binary mode
    filePtr = fopen(filename,"rb");
    if (filePtr == NULL)
        return NULL;

    //read the bitmap file header
    fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER),1,filePtr);

    //verify that this is a bmp file by check bitmap id
    if (bitmapFileHeader.bfType !=0x4D42)
    {
        fclose(filePtr);
        return NULL;
    }

    //read the bitmap info header
    fread(bitmapInfoHeader, sizeof(BITMAPINFOHEADER),1,filePtr); // small edit. forgot to add the closing bracket at sizeof

    //move file point to the begging of bitmap data
    fseek(filePtr, bitmapFileHeader.bOffBits, SEEK_SET);

    //allocate enough memory for the bitmap image data

    bitmapImage = (unsigned char*)malloc(bitmapInfoHeader->biSizeImage);



    //verify memory allocation
    if (!bitmapImage)
    {
        free(bitmapImage);
        fclose(filePtr);
        return NULL;
    }

    //read in the bitmap image data
    //will read the
    fread(bitmapImage,bitmapInfoHeader->biSizeImage,1,filePtr);

//    int count = 0;
//    count = fread(bitmapImage,sizeof(RGBTRIPLE),bitmapInfoHeader->biSizeImage/sizeof(RGBTRIPLE),filePtr);
//    cout<<"Count: "<< count << endl;

    //make sure bitmap image data was read
    if (bitmapImage == NULL)
    {
        fclose(filePtr);
        return NULL;
    }

    //swap the r and b values to get RGB (bitmap is BGR)
    for (imageIdx = 0;imageIdx < bitmapInfoHeader->biSizeImage;imageIdx+=3)
    {
        tempRGB = bitmapImage[imageIdx];
        bitmapImage[imageIdx] = bitmapImage[imageIdx + 2];
        bitmapImage[imageIdx + 2] = tempRGB;
    }

    RGBbitmap = (RGBTRIPLE*)malloc(bitmapInfoHeader->biSizeImage * sizeof(RGBTRIPLE));
    RGBTRIPLE temp;
    int px = 0;

    for (imageIdx = 0;imageIdx < bitmapInfoHeader->biSizeImage;imageIdx+=3)
    {
    	temp.rgbtRed = bitmapImage[imageIdx];
    	temp.rgbtBlue = bitmapImage[imageIdx + 1];
    	temp.rgbtGreen = bitmapImage[imageIdx + 2];
    	RGBbitmap[px++] = temp;

    }


    //close file and return bitmap iamge data
    fclose(filePtr);
    free(bitmapImage);
    return RGBbitmap;
}

void PrintHeaderInfo(BITMAPINFOHEADER *bitmapInfoHeader)
{

    cout<< bitmapInfoHeader->biSize  <<" //specifies the number of bytes required by the struct" << endl;
    cout<< bitmapInfoHeader->biWidth <<" //specifies width in pixels"<< endl;
    cout<< bitmapInfoHeader->biHeight <<" //species height in pixels" << endl;
    cout<< bitmapInfoHeader->biPlanes <<" //specifies the number of color planes, must be 1"<<endl;
    cout<< bitmapInfoHeader->biBitCount <<" //specifies the number of bit per pixel" <<endl;
    cout<< bitmapInfoHeader->biCompression <<" //spcifies the type of compression" <<endl;
    printf("%d //size of image in bytes\n", bitmapInfoHeader->biSizeImage);

    cout<< bitmapInfoHeader->biXPelsPerMeter <<" //number of pixels per meter in x axis" <<endl;
    cout<< bitmapInfoHeader->biYPelsPerMeter  <<" //number of pixels per meter in y axis" <<endl;
    printf("%d //number of colors used by the bitmap\n", bitmapInfoHeader->biClrUsed);
    printf("%d //number of colors that are important\n", bitmapInfoHeader->biClrImportant);

}



#endif /* BMP2RGB_HPP_ */

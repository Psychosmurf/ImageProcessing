#include <iostream>
#include <fstream>
#include <malloc.h>
#include "SobelTrying.hpp"
#include "bmp2rgb.hpp"

using namespace std;


int main(int argc, char *argv[])
{


	BITMAPINFOHEADER bitmapInfoHeader;
	RGBTRIPLE *pixelmap;
	RGBTRIPLE pixel;
	unsigned char *bitmapData;
	pixelmap = LoadBitmapFile(argv[1],&bitmapInfoHeader);
	PrintHeaderInfo(&bitmapInfoHeader);

	for(int i = 0; i < bitmapInfoHeader.biSizeImage/sizeof(RGBTRIPLE);i++)
	{
		pixel = pixelmap[i];
		printf("R:%d , B:%d , G:%d\n",pixel.rgbtRed,pixel.rgbtBlue,pixel.rgbtGreen);

	}

	return 0;
}

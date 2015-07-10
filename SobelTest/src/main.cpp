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
	RGBTRIPLE **pixelmap2;
	pixelmap = LoadBitmapFile(argv[1],&bitmapInfoHeader);
	PrintHeaderInfo(&bitmapInfoHeader);


	pixelmap2 = ConvertTo2D(pixelmap);
	pixelmap2 = ToGrayScale(pixelmap2);
	//pixelmap2 = DeltaFrameGeneration(pixelmap2,pixelmap2);

	sobel_printf(pixelmap2);

	free(pixelmap2);
	free(pixelmap);
	return 0;
}

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

	pixelmap = LoadBitmapFile(argv[1],&bitmapInfoHeader);
	//PrintHeaderInfo(&bitmapInfoHeader);
	//PrintRGB(pixelmap,bitmapInfoHeader.biSizeImage);




	return 0;
}

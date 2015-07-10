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
	//PrintHeaderInfo(&bitmapInfoHeader);


	pixelmap2 = ConvertTo2D(pixelmap);
	pixelmap2 = ToGrayScale(pixelmap2);

	for (int i = 0; i < ROWS; i++)
	{
		for (int j = 0; j < COLS; j++)
		{
			printf("[%d] R:%d , B:%d , G:%d\n",i,pixelmap2[i][j].rgbtRed,pixelmap2[i][j].rgbtBlue,pixelmap2[i][j].rgbtGreen);

		}
	}

	free(pixelmap2);
	free(pixelmap);
	return 0;
}

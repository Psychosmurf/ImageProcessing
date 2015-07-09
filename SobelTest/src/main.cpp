#include <iostream>
#include <fstream>
#include <malloc.h>
#include "SobelTrying.hpp"

using namespace std;


int main(int argc, char *argv[])
{


	BITMAPINFOHEADER bitmapInfoHeader;
	unsigned char *bitmapData;
	bitmapData = LoadBitmapFile(argv[1],&bitmapInfoHeader);
	PrintHeaderInfo(&bitmapInfoHeader);

	//for(int i = 0; i < bitmapInfoHeader.biSizeImage-1;i++)
	//	printf("%d,",bitmapData[i]);


	return 0;
}

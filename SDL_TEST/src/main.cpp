
//The headers
#include "SDL.h"
#include "SDL_image.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "ImageViewer.hpp"
#include "SobelTrying.hpp"
#include "bmp2rgb.hpp"

using namespace std;



int main( int argc, char* argv[] )
{

	bool quit = false;
	SDL_Surface *screen;
	SDL_Event event;

//	BITMAPINFOHEADER bitmapInfoHeader1;
	BITMAPINFOHEADER bitmapInfoHeader2;
//	RGBTRIPLE *background_buf;
	RGBTRIPLE *object_buf;

//	RGBTRIPLE **background_map;
	RGBTRIPLE **object_map;
//	RGBTRIPLE **delta_frame;


//	background_buf = LoadBitmapFile(argv[1],&bitmapInfoHeader1);

	object_buf 	= LoadBitmapFile(argv[1],&bitmapInfoHeader2);
	setDimensions(&bitmapInfoHeader2);
//  FindMedian(object_buf);
//
//	sobel_printf(object_map);

//	background_map = ConvertTo2D(background_buf);
//	background_map = ToGrayScale(background_map);

	object_map = ConvertTo2D(object_buf);
	object_map = ToGrayScale(object_map);
//
	screen = init();

//	delta_frame = DeltaFrameGeneration(background_map,object_map);
	display(screen,object_map);

	//display_bmp(argv[1],screen);
	//test_pixel_display(screen);












	while(quit == false)
	{
		while( SDL_PollEvent( &event ) )
		{
			if( event.type == SDL_QUIT )
				quit = true;
		}
	}


	clean_up(screen);
    return 0;
}

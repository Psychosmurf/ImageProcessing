/*
 * ImageViewer.hpp
 *
 *  Created on: Jul 10, 2015
 *      Author: Peter
 */

#ifndef IMAGEVIEWER_HPP_
#define IMAGEVIEWER_HPP_

#include "SDL.h"
#include "SDL_image.h"
#include "SobelTrying.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string>

using namespace std;

//Screen attributes
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int SCREEN_BPP = 8;

SDL_Surface* init()
{
	SDL_Surface* screen;

    //Initialize all SDL subsystems
    if( SDL_Init( SDL_INIT_VIDEO) < 0 )
    {
    	fprintf(stderr, "Could not Initialize SDL: %s\n",SDL_GetError());
    	exit(1);
	}

    //Set up the screen
    screen = SDL_SetVideoMode( SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_SWSURFACE );

    //If there was an error in setting up the screen
    if( screen == NULL )
    {
    	fprintf(stderr, "Couldn't set 640x480x8 video mode: %s\n",SDL_GetError());
    	exit(1);
	}

	printf("Set 640x480 at %d bits-per-pixel mode\n",screen->format->BitsPerPixel);

    //Set the window caption
    SDL_WM_SetCaption( "Pixel Me", NULL );

    return screen;
}

void display_bmp(char *file_name,SDL_Surface*& screen)
{
    SDL_Surface *image;

    /* Load the BMP file into a surface */
    image = SDL_LoadBMP(file_name);
    if (image == NULL) {
        fprintf(stderr, "Couldn't load %s: %s\n", file_name, SDL_GetError());
        return;
    }

    /*
     * Palettized screen modes will have a default palette (a standard
     * 8*8*4 colour cube), but if the image is palettized as well we can
     * use that palette for a nicer colour matching
     */
    if (image->format->palette && screen->format->palette) {
    SDL_SetColors(screen, image->format->palette->colors, 0,
                  image->format->palette->ncolors);
    }

    /* Blit onto the screen surface */
    if(SDL_BlitSurface(image, NULL, screen, NULL) < 0)
        fprintf(stderr, "BlitSurface error: %s\n", SDL_GetError());

    SDL_UpdateRect(screen, 0, 0, image->w, image->h);

    /* Free the allocated BMP surface */
    SDL_FreeSurface(image);
}

void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to set */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
    *p = pixel;

}

void clean_up(SDL_Surface*& image)
{
	SDL_FreeSurface( image );
	SDL_Quit();
}

void test_pixel_display(SDL_Surface*& screen)
{


    /* Code to set a yellow pixel at the center of the screen */

    int x, y;
    Uint32 yellow;


    yellow = SDL_MapRGB(screen->format, 0xff, 0xff, 0x00);

    if ( SDL_LockSurface(screen) < 0 ) {
    	fprintf(stderr, "Can't lock screen: %s\n", SDL_GetError());
    	exit(-1);
    }

    for(int i = 0; i < screen->w; i++)
    	for(int j = 0; j < screen->h;j++)
    		putpixel(screen, i, j, yellow);

    SDL_FreeSurface(screen);
    SDL_Flip(screen);


}

void display(SDL_Surface*& screen,RGBTRIPLE **rgb)
{


    /* Code to set a yellow pixel at the center of the screen */

    int x, y;
    Uint32 color;

    if ( SDL_LockSurface(screen) < 0 ) {
    	fprintf(stderr, "Can't lock screen: %s\n", SDL_GetError());
    	exit(-1);
    }

    for (int i = 0; i < ROWS; i++)
    {
    	for (int j = 0; j < COLS; j++)
    	{
    		color = SDL_MapRGB(screen->format, rgb[i][j].rgbtRed, rgb[i][j].rgbtGreen, rgb[i][j].rgbtBlue);
    		putpixel(screen, i, j, color);

    	}
    }

    SDL_FreeSurface(screen);
    SDL_Flip(screen);


}

#endif /* IMAGEVIEWER_HPP_ */

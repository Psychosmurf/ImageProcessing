/*
 * IEEE@UIC
 * VISION HEADER
 *
 */
#ifndef SOBLETRYING_HPP_
#define SOBLETRYING_HPP_

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "bmp2rgb.hpp"

using namespace std;

int ROWS = 0;
int COLS = 0;
const int MAX_BUFF = ROWS * COLS;


//Global pointers

char *b1;		// img1 raw data buff1
char *b2;		// img2 raw data buff2
char **input1;	// grayed img1
char **input2;	// grayed img2
char **dfram1;	// delta fram2 gen1
char **dframe2;	// delta frame gen2



void setDimensions(BITMAPINFOHEADER *bitmapInfoHeader)
{

	ROWS = bitmapInfoHeader->biWidth;
	COLS = bitmapInfoHeader->biHeight;
}

inline RGBTRIPLE** alloc2D(int row,int col)
{
	RGBTRIPLE **multi_dim;
	multi_dim = (RGBTRIPLE**)malloc(col * row *sizeof(RGBTRIPLE));
	for (int k = 0; k < row; k++)
		multi_dim[k] = (RGBTRIPLE*)malloc(row * sizeof(RGBTRIPLE));


	return multi_dim;
}


inline char* fillBuffer(string inFileName)
{

	char* tbuff;
	tbuff = (char*)malloc(sizeof(char) * MAX_BUFF);

	FILE *fp;
	fp = fopen(inFileName.c_str(), "r");
	if(fp != NULL)
	{
		while ((fgets(tbuff, sizeof(tbuff), fp)) != NULL)
		{
			//printf("%s", tbuff);
		}

		fclose(fp);
	}

	return tbuff;
}

inline char** readInput(char *buff)
{

	printf("%s",buff);
	char **RedB;
	char **output;


	int MatVal = 0;
	int cp = 0;
	int ch;

	//Inititialize Arrays

	RedB = (char**)malloc(sizeof(char *) * ROWS);
	for (int k = 0; k < ROWS; k++)
		RedB[k] = (char*)malloc(sizeof(char) * COLS);

	output = (char**)malloc(sizeof(char *) * ROWS);
	for (int k = 0; k < ROWS; k++)
		output[k] = (char*)malloc(sizeof(char) * COLS);


	//cout<<"allocated arrays"<<endl;

	for (int i = 0; i < ROWS; i++)
	{
		for (int j = 0; j < COLS; j++)
		{
			while (true)
			{
				ch = buff[cp];

				if ((48 <= ch) && (ch <= 57))
				{
					MatVal = (MatVal * 10) + ch;
					//cout<<"		MatVal -> "<< MatVal << endl;

				}
				else
				{
					RedB[i][j] = MatVal;
					//cout<< (char)MatVal << endl;
 					MatVal = 0;
					cp++;

					if (ch == 10 || ch == 20 || ch == 3 || ch == 32)
					{
						cp++;
					}

					break;
				}
				cp++;
			}
			output[i][j] = RedB[i][j];
		}

	}

	for(int i = 0; i < ROWS; i++)
		for(int j = 0; j < COLS; j++)
			if( output[i][j] != NULL)
				cout<<"i: "<< i <<" j: "<< j <<" output: "<< output[i][j] << endl;

	free(RedB);
	return output;
}

inline RGBTRIPLE** DeltaFrameGeneration(RGBTRIPLE** in1, RGBTRIPLE** in2)
{
/*
 * Delta Frame generation is two images squashed together after being grayed out.
 * Any pixel that stands out i.e moves will stand out after this transformation.
 * Two picures that are identical will be the difference of zero on all RGB
 *
 * We first need to figure out the threshold before continuing
 */
	RGBTRIPLE **seg;
	seg = alloc2D(ROWS,COLS);

	RGBTRIPLE **seg1;
	seg1 = alloc2D(ROWS,COLS);

	for (int i = 0; i < ROWS; i++)
	{
		for (int j = 0; j < COLS; j++)
		{
			//double check this shit
			seg[i][j].rgbtRed = in1[i][j].rgbtRed - in2[i][j].rgbtRed;
			seg[i][j].rgbtGreen = in1[i][j].rgbtGreen - in2[i][j].rgbtGreen;
			seg[i][j].rgbtBlue = in1[i][j].rgbtBlue - in2[i][j].rgbtBlue;

			if(seg[i][j].rgbtRed > 20 && seg[i][j].rgbtGreen > 20 && seg[i][j].rgbtBlue > 20)
			{

				seg1[i][j].rgbtRed = 255;
				seg1[i][j].rgbtBlue = 255;
				seg1[i][j].rgbtGreen = 255;

				/*
				 * After gray scale conversion on both images we take the difference of the images
				 * and purify the image of the constants.
				 *
				 */

//				if(seg[i][j].rgbtRed > 20)
//					seg1[i][j].rgbtRed = 255;
//				if (seg[i][j].rgbtGreen > 20)
//					seg1[i][j].rgbtGreen = 255;
//				if(seg[i][j].rgbtBlue > 20)
//					seg1[i][j].rgbtBlue = 255;
			}
			else
			{
				seg1[i][j].rgbtRed = 0;
				seg1[i][j].rgbtBlue = 0;
				seg1[i][j].rgbtGreen = 0;

			}

		}
	}

	/*
	 * swap ack later
	 */

	free(seg);
	return seg1;
}

inline char **MedianFilter(char **seg)
{
//
//	int bbr = 0;
//	char **firstfilter;
//	firstfilter = (char**)malloc(sizeof(char *) * ROWS);
//	for (int k = 0; k < ROWS; k++)
//		firstfilter[k] = (char*)malloc(sizeof(char) * COLS);
//
//	for (int i = 0; i < ROWS; i++)
//	{
//		for (int j = 0; j < COLS; j++)
//		{
//			if( i != 0 || j != 0 || i != ROWS || j != COLS )
//				bbr = FindMedian(seg);
//			firstfilter[i][j] = bbr;
//		}
//	}
//
//	free(seg);
//	return firstfilter;

}

inline RGBTRIPLE **Thresholding(RGBTRIPLE **filter)
{
	//Example of Gray Level Thresholding

//
//	RGBTRIPLE **ObjectPixels;
//	ObjectPixels = alloc2D(ROWS,COLS);
//
//	for (int i = 0; i < ROWS; i++)
//	{
//		for (int j = 0; j < COLS; j++)
//		{
//			if(filter[i][j] > 40)//mean or median value
//				filter[i][j] = 255;
//			else
//				filter[i][j] = 0;
//
//			EdgeImage[i][j] = 255;
//		}
//	}
//	return EdgeImage;
}

inline char **EdgeDetection(char **filter)
{
	char **EdgeImage;
	EdgeImage = (char**)malloc(sizeof(char *) * ROWS);
	for (int k = 0; k < ROWS; k++)
		EdgeImage[k] = (char*)malloc(sizeof(char) * COLS);

	for (int i = 0; i < ROWS; i++)
	{
		for (int j = 0; j < COLS; j++)
		{
			if( i == 0 && j == 0)
			{
				if(filter[j][i+1] && filter[j+1][i+1] &&
						filter[j+1][i] == 255)
					EdgeImage[j][i] = 0;

			}
			else if(i == 0 && j == ROWS)
			{
				if(filter[j+1][i] && filter[j+1][i-1] &&
						filter[j][i-1] == 255)
					EdgeImage[j][i] = 0;

			}
			else if(i == ROWS && j == 0)
			{
				if(filter[j-1][i] && filter[j-1][i+1] &&
						filter[j][i+1] == 255)
					EdgeImage[j][i] = 0;

			}
			else if(i == ROWS && j == ROWS)
			{
				if(filter[j-1][i-1] && filter[j][i-1] &&
						filter[j-1][i] == 225)
					EdgeImage[j][i] = 0;

			}
			else if(i == 1)
			{
				if(filter[j][j-1] && filter[j+1][i-1] &&
						filter[j+1][i] && filter[j+1][i+1] &&
						filter[j][i+1] == 255)
					EdgeImage[j][i] = 0;

			}
			else if(i == ROWS)
			{
				if(filter[j-1][i-1] && filter[j+1][i-1] &&
						filter[j+1][i] && filter[j-1][i] &&
						filter[j][i-1] == 255)
					EdgeImage[j][i] = 0;
			}
			else if(j == 1)
			{
				if(filter[j][i-1] && filter[j-1][i-1] &&
						filter[j-1][i] && filter[j-1][i+1] &&
						filter[j][i+1] == 255)
					EdgeImage[j][i] = 0;
			}
			else{
				if(filter[j-1][i-1] && filter[j+1][i-1] &&
						filter[j+1][i] && filter[j+1][i+1] &&
						filter[j][i-1] && filter[j-1][i] == 255)
					EdgeImage[j][i] = 0;

			}
		}
	}
	return EdgeImage;
}

inline char** EnchanceImage(char **in1,char **in2)
{
	char **seg;
	seg = (char**)malloc(sizeof(char *) * ROWS);
	for (int k = 0; k < ROWS; k++)
		seg[k] = (char*)malloc(sizeof(char) * COLS);

	char **store;
	store = (char**)malloc(sizeof(char *) * ROWS);
	for (int k = 0; k < ROWS; k++)
		store[k] = (char*)malloc(sizeof(char) * COLS);

	for (int i = 0; i < ROWS; i++)
	{
		for (int j = 0; j < COLS; j++)
		{
			seg[i][j] = in1[i][j] - in2[i][j];

			//Not sure if this is correct check document
			if(seg[i][j] == 1)
				store[i][j] = seg[i][j];

		}
	}

	free(seg);
	return store;


}

inline RGBTRIPLE** ToGrayScale(RGBTRIPLE **rgbmap)
{
	double L = 0;
	RGBTRIPLE **graymap;
	graymap = alloc2D(ROWS,COLS);

	for (int i = 0; i < ROWS; i++)
	{
		for (int j = 0; j < COLS; j++)
		{

			L = 0.2126 * rgbmap[i][j].rgbtRed + 0.7152 * rgbmap[i][j].rgbtGreen + 0.0722 * rgbmap[i][j].rgbtBlue;

			graymap[i][j].rgbtRed = (unsigned char)(L+0.5);
			graymap[i][j].rgbtGreen = (unsigned char)(L+0.5);
			graymap[i][j].rgbtBlue = (unsigned char)(L+0.5);
		}
	}
	return graymap;
}

inline RGBTRIPLE** ConvertTo2D(RGBTRIPLE *rgbmap)
{
//	int ofs = 0;
//	RGBTRIPLE temp;
	RGBTRIPLE **multi_dim;
	multi_dim = alloc2D(ROWS,COLS);

	int px = 0;
	for (int i = 0; i < ROWS; i++)
	{
		for (int j = 0; j < COLS; j++)
		{

			//ofs = (j * ROWS) + i;

			multi_dim[i][j].rgbtRed = rgbmap[px].rgbtRed;
			multi_dim[i][j].rgbtBlue = rgbmap[px].rgbtBlue;
			multi_dim[i][j].rgbtGreen = rgbmap[px].rgbtGreen;
			px++;
		}
	}
	return multi_dim;
}

inline void sobel_printf(RGBTRIPLE ** rgbmap){

	/*
	 *  w pamieci MJ
	 *  karabin pow pw
	 *
	 */

	for (int i = 0; i < ROWS; i++)
	{
		for (int j = 0; j < COLS; j++)
		{
			printf("i-j[%d-%d] R:%d , B:%d , G:%d\n",i,j,rgbmap[i][j].rgbtRed,rgbmap[i][j].rgbtBlue,rgbmap[i][j].rgbtGreen);

		}
	}

}

inline void FindMedian(RGBTRIPLE *buff)
{

	/*
	 * ToDo How to find median with pixels?
	 *
	 *
	 */

	char tmp = 0;
	int cp  = 0;
	int count = -1;
	char ch = 0;
	int ofs = 0;

	RGBTRIPLE** frame;
	RGBTRIPLE* middle_pixel;
	RGBTRIPLE* pntr;


	RGBTRIPLE* tbuff;
	tbuff = (RGBTRIPLE*)malloc(sizeof(RGBTRIPLE) * 8);

	pntr = buff;


	for(int index = 0; index < 9; index+=3)
	{

		tbuff[index] = pntr[count++];
		tbuff[index+1] = pntr[count++];
		tbuff[index+2] = pntr[count++];

		cout << (unsigned int)tbuff[index].rgbtRed << endl;
		cout << (unsigned int)tbuff[index+1].rgbtRed << endl;
		cout << (unsigned int)tbuff[index+2].rgbtRed << endl;
		cout<<"counter: "<< count <<endl;

		pntr = &buff[ROWS-2];
		count = -1;


	}


	cout<<"sizeof tbuff: "<< sizeof(tbuff) << endl;
	for(int i = 0; i < 10;i++)
		printf("[%d] R:%d , G:%d , B:%d\n",i,tbuff[i].rgbtRed,tbuff[i].rgbtGreen,tbuff[i].rgbtBlue);



	middle_pixel = &tbuff[5];

	//ascending order sorting
//	for(int i = 0; i < 3; i++)
//	{
//		for(int j = 0; j < 3 - i; j++)
//		{
//			if(tbuff[j] > tbuff[j+1])
//			{
//				tmp = tbuff[j];
//				tbuff[j] = tbuff[j+1];
//				tbuff[j+1] = tmp;
//			}
//		}
//	}

	middle_pixel->rgbtRed = (tbuff[4].rgbtRed + tbuff[5].rgbtRed)/2;
	middle_pixel->rgbtGreen = (tbuff[4].rgbtGreen + tbuff[5].rgbtGreen)/2;
	middle_pixel->rgbtBlue = (tbuff[4].rgbtBlue + tbuff[5].rgbtBlue)/2;



	free(tbuff);

	//return middle_pixel;
}

#endif /* SOBLETRYING_HPP_ */

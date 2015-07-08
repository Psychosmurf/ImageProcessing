/*
 * IEEE@UIC
 * VISION HEADER
 *
 */
#ifndef SOBLETRYING_HPP_
#define SOBLETRYING_HPP_

#include <iostream>

using namespace std;

const int ROWS = 180;
const int COLS = 280;
const int MAX_BUFF = ROWS * COLS;


//Global pointers

char *b1;		// img1 raw data buff1
char *b2;		// img2 raw data buff2
char **input1;	// grayed img1
char **input2;	// grayed img2
char **dfram1;	// delta fram2 gen1
char **dframe2;	// delta frame gen2


inline int FindMedian(char **buff)
{

	/*
	 * ToDo Need to get the buffer filled with RGB first
	 *
	 *
	 */

	char tmp = 0;
	int cp  = 0;
	int count = 0;
	char ch = 0;

	char* tbuff;
	tbuff = (char*)malloc(sizeof(char) * MAX_BUFF);

	//move from 2d array into single buffers
	for (int i = 0; i < ROWS; i++)
	{
		for (int j = 0; j < COLS; j++)
		{
			tbuff[cp] = buff[ROWS][COLS];
			cp++;

		}
	}

	//ascending order sorting
	for(int i = 0; i < MAX_BUFF; i++)
	{
		for(int j = 0; j < MAX_BUFF - i; j++)
		{
			if(tbuff[j] > tbuff[j+1])
			{
				tmp = tbuff[j];
				tbuff[j] = tbuff[j+1];
				tbuff[j+1] = tmp;
			}
		}
	}

	//do not use until rgb buf is created
	return 0;
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

inline char** DeltaFrameGeneration(char** in1, char** in2)
{
/*
 * After subtracting the two gray scale images we then refine the photo even deeper
 * with DeltaFrameGeneration
 *
 */
	char **seg;
	seg = (char**)malloc(sizeof(char *) * ROWS);
	for (int k = 0; k < ROWS; k++)
		seg[k] = (char*)malloc(sizeof(char) * COLS);

	char **seg1;
	seg1 = (char**)malloc(sizeof(char *) * ROWS);
	for (int k = 0; k < ROWS; k++)
		seg1[k] = (char*)malloc(sizeof(char) * COLS);


	for (int i = 0; i < ROWS; i++)
	{
		for (int j = 0; j < COLS; j++)
		{
			seg[i][j] = in1[i][j] - in2[i][j];

			if(seg[i][j] > 20)
				seg1[i][j] = 255;
			else
				seg1[i][j] = 0;

		}
	}

	free(seg);
	return seg1;
}

inline char **MedianFilter(char **seg)
{

	int bbr = 0;
	char **firstfilter;
	firstfilter = (char**)malloc(sizeof(char *) * ROWS);
	for (int k = 0; k < ROWS; k++)
		firstfilter[k] = (char*)malloc(sizeof(char) * COLS);

	for (int i = 0; i < ROWS; i++)
	{
		for (int j = 0; j < COLS; j++)
		{
			if( i != 0 || j != 0 || i != ROWS || j != COLS )
				bbr = FindMedian(seg);
			firstfilter[i][j] = bbr;
		}
	}

	free(seg);
	return firstfilter;

}

inline char **Thresholding(char **filter)
{

	char **EdgeImage;
	EdgeImage = (char**)malloc(sizeof(char *) * ROWS);
	for (int k = 0; k < ROWS; k++)
		EdgeImage[k] = (char*)malloc(sizeof(char) * COLS);

	for (int i = 0; i < ROWS; i++)
	{
		for (int j = 0; j < COLS; j++)
		{
			if(filter[i][j] > 40)
				filter[i][j] = 255;
			else
				filter[i][j] = 0;

			EdgeImage[i][j] = 255;
		}
	}
	return EdgeImage;
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

			if(seg[i][j] == 1)
				store[i][j] = seg[i][j];

		}
	}

	free(seg);
	return store;


}



#endif /* SOBLETRYING_HPP_ */

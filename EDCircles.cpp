#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>

#define MAGNITUDE "magnitude"
#define DIRECTION "direction"

using namespace cv;
using namespace std;

struct BGR
{
    float B;
    float G;
    float R;
};//struct BGR

struct Anchor
{
    int value;

    int i;
    int j;

    bool isInEdge;
};//struct Anchor

struct EdgePixel
{
    Anchor     anchor;

    EdgePixel* next;  //right or up
    EdgePixel* prev;  //left  or down

};//struct EdgePixel

const float NORMALIZE_GRAYSCALE = 1.0/3.0;

//Gaussian filter normalized terms
const float GAUSS_TERM1 = 0.006;
const float GAUSS_TERM2 = 0.061;
const float GAUSS_TERM3 = 0.242;
const float GAUSS_TERM4 = 0.383;

//quantization error elimination threshold
//for the Prewitt Operator
const float QUANT_ERROR_ELIM_THRESH = 8.48;

//angles
const float VERTICAL   = 90;
const float HORIZONTAL = 0;

const int   NUM_PIXEL_VALUES = 256;

const int   TIME_TO_WAIT_FOR_INPUT = 1; //set to 0 for infinite
const char  ESC_KEY_CODE           = char(27);

vector< vector<BGR> > grayScale(vector< vector<BGR> >& input)
{
    int rows = input.size();
    int cols = input[0].size();

    vector< vector<BGR> > output(rows, vector<BGR>(cols));

    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            BGR pixel;

            pixel.B = NORMALIZE_GRAYSCALE*(input[i][j].R +
                                           input[i][j].G +
                                           input[i][j].B);
            pixel.G = pixel.B;
            pixel.R = pixel.B;

            output[i][j] = pixel;
        }//for
    }//for

    return output;
}//grayScale

//smoothing filter using two masks
//this filter ensures that we get 'nice' derivatives later
vector< vector<BGR> > gaussianFilter(vector< vector<BGR> >& input)
{
    int rows = input.size();
    int cols = input[0].size();

    vector< vector<BGR> > output = input;

    //horizontal mask, the values are obtained from
    //a gaussian mask after normalization
    for (int i = 0; i < rows; i++)
    {
        for (int j = 3; j < cols - 3; j++)
        {
            output[i][j].B = GAUSS_TERM1*input[i][j - 3].B
                            +GAUSS_TERM2*input[i][j - 2].B
                            +GAUSS_TERM3*input[i][j - 1].B
                            +GAUSS_TERM4*input[i][  j  ].B
                            +GAUSS_TERM3*input[i][j + 1].B
                            +GAUSS_TERM2*input[i][j + 2].B
                            +GAUSS_TERM1*input[i][j + 3].B;
        }//for
    }//for

    //vertical gaussian mask, same as above
    for (int i = 3; i < rows - 3; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            output[i][j].B = GAUSS_TERM1*input[i - 3][j].B
                            +GAUSS_TERM2*input[i - 2][j].B
                            +GAUSS_TERM3*input[i - 1][j].B
                            +GAUSS_TERM4*input[  i  ][j].B
                            +GAUSS_TERM3*input[i + 1][j].B
                            +GAUSS_TERM2*input[i + 2][j].B
                            +GAUSS_TERM1*input[i + 3][j].B;
        }//for
    }//for

    return output;
}//gaussianFilter

//applies the prewitt derivative
vector< vector<BGR> > prewittOp(vector< vector<BGR> >& input, string control)
{
    int rows = input.size();
    int cols = input[0].size();

    vector< vector<BGR> > output(rows, vector<BGR>(cols));
    vector< vector<BGR> > temp_x(rows, vector<BGR>(cols));
    vector< vector<BGR> > temp_y(rows, vector<BGR>(cols));
    vector< vector<BGR> > Gx    (rows, vector<BGR>(cols));
    vector< vector<BGR> > Gy    (rows, vector<BGR>(cols));

    //we apply the horizontal masks, [-1, 0, 1] (derivative mask for x) and
    //[1, 1, 1] (averaging mask for y)
    for (int i = 0; i < rows; i++)
    {
        for (int j = 1; j < cols - 1; j++)
        {
            temp_x[i][j].B = -1*input[i][j - 1].B +
                                input[i][j + 1].B;

            temp_y[i][j].B = input[i][j - 1].B +
                             input[i][  j  ].B +
                             input[i][j + 1].B;
        }//for
    }//for

    //we now take the previous results and apply the vertical masks,
    //[1; 1; 1] (averaging mask for x) and
    //[-1; 0; 1] (derivative mask for y)
    //this results in the x and y gradients, Gx and Gy
    for (int i = 1; i < rows - 1; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            Gx[i][j].B = temp_x[i - 1][j].B +
                         temp_x[  i  ][j].B +
                         temp_x[i + 1][j].B;

            Gy[i][j].B = -1*temp_y[i - 1][j].B + temp_y[i + 1][j].B;
        }//for
    }//for

    //calculate the gradient magnitude
    if (control == MAGNITUDE)
    {
        for (int i = 0; i < rows; i++)
        {
            for (int j = 0; j < cols; j++)
            {
                output[i][j].B = abs(Gx[i][j].B) + abs(Gy[i][j].B);

                if (output[i][j].B < QUANT_ERROR_ELIM_THRESH)
                {
                    output[i][j].B = 0;
                }//if
            }//for
        }//for
    }//if

    //calculate the gradient direction in degrees
    //only vertical and horizontal angles are distinguished
    else if (control == DIRECTION)
    {
        for (int i = 0; i < rows; i++)
        {
            for (int j = 0; j < cols; j++)
            {
                //if the gradient is stronger along the x direction,
                //then we have a vertical edge
                //similarly for the y direction, we have a horizontal edge
                //(angles are measured from the horizontal)
                if (Gx[i][j].B >= Gy[i][j].B)
                {
                    output[i][j].B = VERTICAL;
                }//if
                else
                {
                    output[i][j].B = HORIZONTAL;
                }//else
            }//for
        }//for
    }//else if

    return output;
}//prewittOp

vector< vector<Anchor> > extractAnchors(vector< vector<BGR> >& input,
                                        vector< vector<BGR> >& magnitudeMap,
                                        vector< vector<BGR> >& directionMap)
{
    int rows = input.size();
    int cols = input[0].size();

    vector< vector<Anchor> > output(rows, vector<Anchor>(cols));

    for (int i = 1; i < rows - 1; i++)
    {
        for (int j = 1; j < cols - 1; j++)
        {
            //we use non-maximum suppression to extract the anchors.
            //this means that we only take those maxima which are 'peaks'
            //in the intensity map. Taking ordianry maxima is not enough due
            //to the existence of saddle-points.
            if (directionMap[i][j].B == VERTICAL)
            {
                // in this case, we have a vertical edge,
                // so we need to make sure that
                // the point in question is the highest in the vertical
                // direction. So if it
                // is less than either the neighbor above or below it,
                // it is suppressed, and if not it is kept.
                if (magnitudeMap[i][j].B < magnitudeMap[i + 1][j].B ||
                    magnitudeMap[i][j].B < magnitudeMap[i - 1][j].B)
                {
                    output[i][j].value    = 0;
                    output[i][j].i        = i;
                    output[i][j].j        = j;
                    output[i][j].isInEdge = false;
                }//if
                else
                {
                    output[i][j].value    = magnitudeMap[i][j].B;
                    output[i][j].i        = i;
                    output[i][j].j        = j;
                    output[i][j].isInEdge = false;
                }//else
            }//if

            //this is the same as above except for horizontal edges, which are
            //compared to their left and right-hand neighbors.
            if (directionMap[i][j].B == HORIZONTAL)
            {
                if (magnitudeMap[i][j].B < magnitudeMap[i][j + 1].B ||
                    magnitudeMap[i][j].B < magnitudeMap[i][j - 1].B)
                {
                    output[i][j].value    = 0;
                    output[i][j].i        = i;
                    output[i][j].j        = j;
                    output[i][j].isInEdge = false;
                }//if
                else
                {
                    output[i][j].value    = magnitudeMap[i][j].B;
                    output[i][j].i        = i;
                    output[i][j].j        = j;
                    output[i][j].isInEdge = false;
                }//else
            }//if
        }//for
    }//for

    return output;
}//extractAnchors

vector< vector<BGR> > convertAnchorToBGR(vector< vector<Anchor> >& anchorMap)
{
    int rows = anchorMap.size();
    int cols = anchorMap[0].size();

    vector< vector<BGR> > output(rows, vector<BGR>(cols));

    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            output[i][j].B = anchorMap[i][j].value;
        }//for
    }//for

    return output;
}//convertAnchorToBGR

//M is simply the number of non-zero valued pixels in the
//magnitude map
float getM(vector< vector<BGR> >& magnitudeMap)
{
    int rows = magnitudeMap.size();
    int cols = magnitudeMap[0].size();

    int M = 0;

    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            if (magnitudeMap[i][j].B != 0)
            {
                ++M;
            }//if
        }//for
    }//for

    return float(M);
}//getM

//The empircal cumulative distribution is a function H(mu), where mu
//is the minimum value of a pixel on an edge, and the value H is given by
//the number of pixels in the magnitude map with values greater than or equal
//to mu. The function is represented here as a vector. This reduces an O(n)
//operation to O(1).
vector<float> getEmpCumDist(vector< vector<BGR> >& magnitudeMap)
{
    vector<float> output(NUM_PIXEL_VALUES);

    int rows = magnitudeMap.size();
    int cols = magnitudeMap[0].size();

    float M = getM(magnitudeMap);

    output[0] = rows*cols;

    for (int k = 1; k < NUM_PIXEL_VALUES; k++)
    {
        output[k] = 0;

        for (int i = 0; i < rows; i++)
        {
            for (int j = 0; j < cols; j++)
            {
                if (magnitudeMap[i][j].B >= k)
                {
                    output[k] = output[k] + 1;
                }//if
            }//for
        }//for

        output[k] = output[k]/M;
    }//for

    return output;
}//getEmpCumDist

//Takes all non-zero pixels in the anchor map and puts them in a list
//to be sorted and accessed in order.
vector<Anchor> getAnchorList(vector< vector<Anchor> >& anchorMap)
{
    int rows     = anchorMap.size();
    int cols     = anchorMap[0].size();
    int listSize = 0;

    vector<Anchor> anchorList(listSize);

    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            if (anchorMap[i][j].isInEdge == false)
            {
                ++listSize;

                anchorList.resize(listSize);

                anchorList[listSize - 1] = anchorMap[i][j];
            }//if
        }//for
    }//for

    return anchorList;
}//getAnchorList

//Quicksort partition function. Needed for sortAnchorList(...)
int partitionList(vector<Anchor>& anchorList, int l, int r)
{
    Anchor pivot;
    Anchor temp;

    pivot = anchorList[l];

    int i = l; int j = r + 1;

    while (1)
    {
        do ++i; while (anchorList[i].value <= pivot.value && i <= r);
        do --j; while (anchorList[j].value >  pivot.value);

        if (i >= j)
        {
            break;
        }//if

        temp          = anchorList[i];
        anchorList[i] = anchorList[j];
        anchorList[j] = temp;
    }//while

    temp          = anchorList[i];
    anchorList[i] = anchorList[j];
    anchorList[j] = temp;

    return j;
}//partionList

//Sorts the anchors in the anchor list using quicksort
vector<Anchor> sortAnchorList(vector<Anchor>& anchorList, int l, int r)
{
    int j;

    if (l < r)
    {
        j = partitionList(anchorList, l, r);

        sortAnchorList(anchorList, l,     j - 1);
        sortAnchorList(anchorList, j + 1, r    );
    }//if

    return anchorList;
}//sortAnchorList

/*
void connectAnchors(Anchor& anchor, vector<EdgePixel>& edgeList,
                                    vector<BGR>&       magnitudeMap,
                                    vector<BGR>&       directionMap)
{

}//connectAnchors

*/

vector<EdgePixel> getEdgeList(vector<Anchor>&           anchorList,
                              vector< vector<BGR> >&    magnitudeMap,
                              vector< vector<BGR> >&    directionMap,
                              vector< vector<Anchor> >& achorMap)
{
    int length = anchorList.size();
    int edgeListSize = 0;

    vector<EdgePixel> edgeList(edgeListSize);

    for (int i = 0; i < length; i++)
    {
        if (anchorList[i].isInEdge == false)
        {
            ++edgeListSize;

            edgeList.resize(edgeListSize);

            //connectAnchors(anchorList[i], edgeList, magnitudeMap, directionMap);
        }
    }

    return edgeList;
}//getEdgeList

//once we have extracted the edges, we will apply a circular arc detection
//algorithm
//and then use that to get the x-y locations of balls in the image.

int main(int argc, char** argv)
{
    namedWindow("input" , CV_WINDOW_NORMAL);
    namedWindow("output", CV_WINDOW_NORMAL);

    string       inFileName = "/home/nikola/bouncyBalls.flv";
    VideoCapture inVideo    = VideoCapture(inFileName.c_str());

    Mat frame;

    while(1)
    {
        inVideo >> frame;

        imshow("input" , frame);

        vector< vector<BGR> > inVec;
        inVec.resize(frame.rows, vector<BGR>(frame.cols));

        for (int i = 0; i < frame.rows; i++)
        {
            for (int j = 0; j < frame.cols; j++)
            {
                inVec[i][j].B = frame.data[frame.step[0]*i + frame.step[1]*j + 0];
                inVec[i][j].G = frame.data[frame.step[0]*i + frame.step[1]*j + 1];
                inVec[i][j].R = frame.data[frame.step[0]*i + frame.step[1]*j + 2];
            }//for
        }//for

        inVec = grayScale     (inVec);
        inVec = gaussianFilter(inVec);

        vector< vector<BGR> > magnitudeMap = prewittOp(inVec, MAGNITUDE);
        vector< vector<BGR> > directionMap = prewittOp(inVec, DIRECTION);

        vector< vector<Anchor> > anchorMap;
        anchorMap = extractAnchors(inVec, magnitudeMap, directionMap);

        inVec = convertAnchorToBGR(anchorMap);
        //inVec = createEdgeMap (inVec, magnitudeMap, directionMap);

        for (int i = 0; i < frame.rows; i++)
        {
            for (int j = 0; j < frame.cols; j++)
            {
                BGR pixel2 = inVec[i][j];

                frame.data[frame.step[0]*i + frame.step[1]*j + 0] = pixel2.B;
                frame.data[frame.step[0]*i + frame.step[1]*j + 1] = pixel2.G;
                frame.data[frame.step[0]*i + frame.step[1]*j + 2] = pixel2.R;
            }//for
        }//for

        inVec.clear();
        magnitudeMap.clear();
        directionMap.clear();

        imshow("output", frame);

        char c = cvWaitKey(TIME_TO_WAIT_FOR_INPUT);
        if (c == ESC_KEY_CODE) break;
    }//while

    return 0;
}

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>

using namespace cv;
using namespace std;

struct BGR
{
    float B;
    float G;
    float R;
};//struct BGR

//this struct is needed to sort anchors
struct AnchorList
{
    int value;
    int i;
    int j;

    AnchorList *next;
    AnchorList *prev;
};//struct AnchorList

struct EdgePixel
{
    int value;
    int lineLength;
    float minVal;

    EdgePixel *left;  //or up
    EdgePixel *right; //or down
};//struct EdgePixel

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

            pixel.B = (float(1)/float(3))*(input[i][j].R + input[i][j].G + input[i][j].B);
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

    //horizontal mask, the values are obtained from a gaussian mask after normalization
    for (int i = 0; i < rows; i++)
    {
        for (int j = 3; j < cols - 3; j++)
        {
            output[i][j].B = 0.006*input[i][j - 3].B
                            +0.061*input[i][j - 2].B
                            +0.242*input[i][j - 1].B
                            +0.383*input[i][  j  ].B
                            +0.242*input[i][j + 1].B
                            +0.061*input[i][j + 2].B
                            +0.006*input[i][j + 3].B;
        }//for
    }//for

    //vertical gaussian mask, same as above
    for (int i = 3; i < rows - 3; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            output[i][j].B = 0.006*input[i - 3][j].B
                            +0.061*input[i - 2][j].B
                            +0.242*input[i - 1][j].B
                            +0.383*input[  i  ][j].B
                            +0.242*input[i + 1][j].B
                            +0.061*input[i + 2][j].B
                            +0.006*input[i + 3][j].B;
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
            temp_x[i][j].B = -1*input[i][j - 1].B + input[i][j + 1].B;

            temp_y[i][j].B = input[i][j - 1].B + input[i][j].B + input[i][j + 1].B;
        }//for
    }//for

    //we now take the previous results and apply the vertical masks, [1; 1; 1] (averaging mask for x) and
    //[-1; 0; 1] (derivative mask for y)
    //this results in the x and y gradients, Gx and Gy
    for (int i = 1; i < rows - 1; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            Gx[i][j].B = temp_x[i - 1][j].B + temp_x[i][j].B + temp_x[i + 1][j].B;

            Gy[i][j].B = -1*temp_y[i - 1][j].B + temp_y[i + 1][j].B;
        }//for
    }//for

    //calculate the gradient magnitude
    if (control == "magnitude")
    {
        for (int i = 0; i < rows; i++)
        {
            for (int j = 0; j < cols; j++)
            {
                output[i][j].B = abs(Gx[i][j].B) + abs(Gy[i][j].B);

                if (output[i][j].B < 8.48)  //quantization error elimination threshold for the prewitt operator
                {
                    output[i][j].B = 0;
                }//if
            }//for
        }//for
    }//if

    //calculate the gradient direction in degrees
    //only vertical and horizontal angles are distinguished
    else if (control == "direction")
    {
        for (int i = 0; i < rows; i++)
        {
            for (int j = 0; j < cols; j++)
            {
                //if the gradient is stronger along the x direction, then we have a vertical edge
                //similarly for the y direction, we have a horizontal edge
                //(angles are measured from the horizontal)
                if (Gx[i][j].B >= Gy[i][j].B)
                {
                    output[i][j].B = 90;  //90 degrees
                }//if
                else
                {
                    output[i][j].B = 0;
                }//else
            }//for
        }//for
    }//else if

    return output;
}//prewittOp

vector< vector<BGR> > extractAnchors(vector< vector<BGR> >& input,
                                     vector< vector<BGR> >& magnitudeMap,
                                     vector< vector<BGR> >& directionMap)
{
    int rows = input.size();
    int cols = input[0].size();

    vector< vector<BGR> > output(rows, vector<BGR>(cols));

    for (int i = 1; i < rows - 1; i++)
    {
        for (int j = 1; j < cols - 1; j++)
        {
            //we use non-maximum suppression to extract the anchors.
            //this means that we only take those maxima which are 'peaks'
            //in the intensity map. Taking ordianry maxima is not enough due
            //to the existence of saddle-points.
            if (directionMap[i][j].B == 90)
            {
                // in this case, we have a vertical edge, so we need to make sure that
                // the point in question is the highest in the vertical direction. So if it
                // is less than either the neighbor above or below it, it is suppressed, and if not
                // it is kept.
                if (magnitudeMap[i][j].B < magnitudeMap[i + 1][j].B ||
                    magnitudeMap[i][j].B < magnitudeMap[i - 1][j].B)
                {
                    output[i][j].B = 0;
                }//if
                else
                {
                    output[i][j].B = magnitudeMap[i][j].B;
                }//else
            }//if

            //this is the same as above except for horizontal edges, which are
            //compared to their left and right-hand neighbors.
            if (directionMap[i][j].B == 0)
            {
                if (magnitudeMap[i][j].B < magnitudeMap[i][j + 1].B ||
                    magnitudeMap[i][j].B < magnitudeMap[i][j - 1].B)
                {
                    output[i][j].B = 0;
                }//if
                else
                {
                    output[i][j].B = magnitudeMap[i][j].B;
                }//else
            }//if
        }//for
    }//for

    return output;
}//extractAnchors

float calc_M(vector< vector<BGR> >& magnitudeMap)
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
}//calc_M

vector<float> get_empCumDist(vector< vector<BGR> >& magnitudeMap)
{
    vector<float> output(256);

    int rows = magnitudeMap.size();
    int cols = magnitudeMap[0].size();

    float M = calc_M(magnitudeMap);

    output[0] = rows*cols;

    for (int k = 1; k < 256; k++)
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
}//get_empCumDist

vector< vector<BGR> > convertEdgePixelToBGR(vector< vector<EdgePixel> >& input)
{
    int rows = input.size();
    int cols = input[0].size();

    vector< vector<BGR> > output(rows, vector<BGR>(cols));

    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            output[i][j].B = input[i][j].value;
        }//for
    }//for

    return output;
}//convertEdgePixelToBGR

//this funciton connects the anchors into single-line edges.
//it's still not finished, as we have yet to incorporate edege-validation.
vector< vector<BGR> > createEdgeMap(vector< vector<BGR> >& input,
                                    vector< vector<BGR> >& magnitudeMap,
                                    vector< vector<BGR> >& directionMap)
{
    int rows = input.size();
    int cols = input[0].size();

    vector< vector<EdgePixel> > edgeMap(rows, vector<EdgePixel>(cols));

    AnchorList *anchorList = new AnchorList;
                anchorList -> next = NULL;
                anchorList -> prev = NULL;
                anchorList -> value = -1;
                anchorList -> i = -1;
                anchorList -> j = -1;

    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            edgeMap[i][j].value = 0;

            if (input[i][j].B != 0)
            {
                AnchorList* newAnchor = new AnchorList;
                            newAnchor -> value = input[i][j].B;
                            newAnchor -> i     = i;
                            newAnchor -> j     = j;
                            newAnchor -> next  = anchorList;
                            newAnchor -> prev  = NULL;

                while (newAnchor -> value < newAnchor -> next -> value)
                {
                    newAnchor -> next = newAnchor -> next -> next;
                }//while

                if (newAnchor -> next -> next == NULL && newAnchor -> next -> prev == NULL)
                {
                    newAnchor -> next -> prev = newAnchor;
                }//if
                else if (newAnchor -> next -> next == NULL && newAnchor -> next -> prev != NULL)
                {
                    newAnchor -> next -> prev -> next = newAnchor;
                    newAnchor -> prev = newAnchor -> next -> prev;
                    newAnchor -> next -> prev = newAnchor;
                }//else if
                else
                {
                    newAnchor -> next = newAnchor -> next -> next;
                    newAnchor -> prev = newAnchor -> next -> prev;
                    newAnchor -> prev -> next = newAnchor;
                    newAnchor -> next -> prev = newAnchor;
                }//else

                while (anchorList -> prev != NULL)
                {
                    anchorList = anchorList -> prev;
                }//while
            }//if
        }//for
    }//for

    AnchorList *topOfList = anchorList;

    vector< vector<int> > edgeHead(1, vector<int>(2));

    int numEdgeHeads = 0;

    while (anchorList -> next != NULL)
    {
        int x = anchorList -> i;
        int y = anchorList -> j;

        int prevX;
        int prevY;

        if (directionMap[x][y].B == 90)
        {
            ++numEdgeHeads;

            edgeHead.resize(numEdgeHeads, vector<int>(2));

            edgeHead[numEdgeHeads - 1][0] = x;
            edgeHead[numEdgeHeads - 1][1] = y;

            edgeMap[anchorList -> i][anchorList -> j].lineLength = 1;
            edgeMap[anchorList -> i][anchorList -> j].minVal     = 10000;

            // go left
            while (magnitudeMap[x][y].B      >   0 &&
                   directionMap[x][y].B     ==  90 &&
                   edgeMap     [x][y].value != 255)
            {
                edgeMap[x][y].value = 255;

                if (magnitudeMap[i][j].B < edgeMap[anchorList -> i][anchorList -> j].minVal)
                {
                    edgeMap[anchorList -> i][anchorList -> j].minVal = magnitudeMap[i][j].B;
                }//if

                prevX = x;
                prevY = y;

                if (magnitudeMap[x - 1][y - 1].B > magnitudeMap[  x  ][y - 1].B &&
                    magnitudeMap[x - 1][y - 1].B > magnitudeMap[x + 1][y - 1].B)
                {
                    x = x - 1;
                    y = y - 1;
                }//if
                else if (magnitudeMap[  x  ][y - 1].B > magnitudeMap[x - 1][y - 1].B &&
                         magnitudeMap[  x  ][y - 1].B > magnitudeMap[x + 1][y - 1].B)
                {
                    y = y - 1;
                }//else if
                else
                {
                    x = x + 1;
                    y = y - 1;
                }//else

                edgeMap[prevX][prevY].left = &edgeMap[x][y];

                ++edgeMap[anchorList -> i][anchorList -> j].lineLength;
            }//while

            x = anchorList -> i;
            y = anchorList -> j;

            // go right
            while (magnitudeMap[x][y].B      >   0 &&
                   directionMap[x][y].B     ==  90 &&
                   edgeMap     [x][y].value != 255)
            {
                edgeMap[x][y].value = 255;

                if (magnitudeMap[i][j].B < edgeMap[anchorList -> i][anchorList -> j].minVal)
                {
                    edgeMap[anchorList -> i][anchorList -> j].minVal = magnitudeMap[i][j].B;
                }//if

                prevX = x;
                prevY = y;

                if (magnitudeMap[x - 1][y + 1].B > magnitudeMap[  x  ][y + 1].B &&
                    magnitudeMap[x - 1][y + 1].B > magnitudeMap[x + 1][y + 1].B)
                {
                    x = x - 1;
                    y = y + 1;
                }//if
                else if (magnitudeMap[  x  ][y + 1].B > magnitudeMap[x - 1][y + 1].B &&
                         magnitudeMap[  x  ][y + 1].B > magnitudeMap[x + 1][y + 1].B)
                {
                    y = y + 1;
                }//else if
                else
                {
                    x = x + 1;
                    y = y + 1;
                }//else

                edgeMap[prevX][prevY].right = &edgeMap[x][y];

                ++edgeMap[anchorList -> i][anchorList -> j].lineLength;
            }//while
        }//if
        else
        {
            ++numEdgeHeads;

            edgeHead.resize(numEdgeHeads, vector<int>(2));

            edgeHead[numEdgeHeads - 1][0] = x;
            edgeHead[numEdgeHeads - 1][1] = y;

            edgeMap[anchorList -> i][anchorList -> j].lineLength = 1;
            edgeMap[anchorList -> i][anchorList -> j].minVal     = 10000;

            // go up
            while (magnitudeMap[x][y].B      >   0 &&
                   directionMap[x][y].B     ==   0 &&
                   edgeMap     [x][y].value != 255)
            {
                edgeMap[x][y].value = 255;

                if (magnitudeMap[i][j].B < edgeMap[anchorList -> i][anchorList -> j].minVal)
                {
                    edgeMap[anchorList -> i][anchorList -> j].minVal = magnitudeMap[i][j].B;
                }//if

                prevX = x;
                prevY = y;

                if (magnitudeMap[x - 1][y - 1].B > magnitudeMap[x - 1][  y  ].B &&
                    magnitudeMap[x - 1][y - 1].B > magnitudeMap[x - 1][y + 1].B)
                {
                    x = x - 1;
                    y = y - 1;
                }//if
                else if (magnitudeMap[x - 1][  y  ].B > magnitudeMap[x - 1][y - 1].B &&
                         magnitudeMap[x - 1][  y  ].B > magnitudeMap[x - 1][y + 1].B)
                {
                    x = x - 1;
                }//esle if
                else
                {
                    x = x - 1;
                    = y + 1;
                }//else

                edgeMap[prevX][prevY].left = &edgeMap[x][y];

                ++edgeMap[anchorList -> i][anchorList -> j].lineLength;
            }//while

            x = anchorList -> i;
            y = anchorList -> j;

            // go down
            while (magnitudeMap[x][y].B      >   0 &&
                   directionMap[x][y].B     ==   0 &&
                   edgeMap     [x][y].value != 255)
            {
                edgeMap[x][y].value = 255;

                if (magnitudeMap[i][j].B < edgeMap[anchorList -> i][anchorList -> j].minVal)
                {
                    edgeMap[anchorList -> i][anchorList -> j].minVal = magnitudeMap[i][j].B;
                }//if

                prevX = x;
                prevY = y;

                if (magnitudeMap[x + 1][y - 1].B > magnitudeMap[x + 1][  y  ].B &&
                    magnitudeMap[x + 1][y - 1].B > magnitudeMap[x + 1][y + 1].B)
                {
                    x = x + 1;
                    y = y - 1;
                }//if
                else if (magnitudeMap[x + 1][  y  ].B > magnitudeMap[x + 1][y - 1].B &&
                         magnitudeMap[x + 1][  y  ].B > magnitudeMap[x + 1][y + 1].B)
                {
                    x = x + 1;
                }//else if
                else
                {
                    x = x + 1;
                    y = y + 1;
                }//else

                edgeMap[prevX][prevY].right = &edgeMap[x][y];

                ++edgeMap[anchorList -> i][anchorList -> j].lineLegnth;
            }//while
        }//else
    }//while

    anchorList = anchorList -> next;

    while (topOfList != NULL)
    {
        AnchorList* current = topOfList;

        topOfList = topOfList -> next;

        delete current;
    }//while

    vector< vector<BGR> > edgeMapBGR;

    edgeMapBGR = convertEdgePixelToBGR(edgeMap);

    return edgeMapBGR;
}//createEdgeMap

//once we have extracted the edges, we will apply a circular arc detection algorithm
//and then use that to get the x-y locations of balls in the image.

int main()
{
    namedWindow("input" , CV_WINDOW_NORMAL);
    namedWindow("output", CV_WINDOW_NORMAL);

    string       inFileName = "/home/nikola/Desktop/ImageProcessing/Resources/bouncyBalls.flv";
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

        vector< vector<BGR> > magnitudeMap = prewittOp(inVec, "magnitude");
        vector< vector<BGR> > directionMap = prewittOp(inVec, "direction");

        inVec = extractAnchors(inVec, magnitudeMap, directionMap);
        inVec = createEdgeMap (inVec, magnitudeMap, directionMap);

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

        char c = cvWaitKey(33);
        if (c == 27) break;
    }//while

    return 0;
}

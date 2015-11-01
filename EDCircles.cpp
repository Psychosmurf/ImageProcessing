//SEL4055
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <stdlib.h>

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
    float value;
    int   i;
    int   j;
};//struct Anchor

typedef Anchor* AnchorArray;

typedef struct Edge_Struct
{
    int         length;
    AnchorArray anchors;
} Edge;

typedef Edge*       EdgeHandle;
typedef EdgeHandle* DynamicEdge;

typedef struct EdgeList_Struct
{
    int         length;
    DynamicEdge edges;
} EdgeList;

typedef EdgeList*       EdgeListHandle;
typedef EdgeListHandle* DynamicEdgeList;

void grayScale(vector< vector<BGR> >& input)
{
    const float NORMALIZE_GRAYSCALE = 1.0/3.0;

    int rows = input.size();
    int cols = input[0].size();

    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            input[i][j].B = NORMALIZE_GRAYSCALE*(input[i][j].R +
                                                 input[i][j].G +
                                                 input[i][j].B);
            input[i][j].G = input[i][j].B;
            input[i][j].R = input[i][j].B;
        }//for
    }//for
}//grayScale

//smoothing filter using two masks
//this filter ensures that we get 'nice' derivatives later
vector< vector<BGR> > gaussianFilter(vector< vector<BGR> >& input)
{
    //Gaussian filter normalized terms
    const float GAUSS_TERM1 = 0.006;
    const float GAUSS_TERM2 = 0.061;
    const float GAUSS_TERM3 = 0.242;
    const float GAUSS_TERM4 = 0.383;

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
    //quantization error elimination threshold
    //for the Prewitt Operator
    const float QUANT_ERROR_ELIM_THRESH = 8.48;

    //angles
    const float VERTICAL   = 90;
    const float HORIZONTAL = 0;

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

void suppressPixel(vector< vector<Anchor> >& output, int i, int j)
{
    output[i][j].value    = 0;
    output[i][j].i        = i;
    output[i][j].j        = j;
}//suppressPixel

void keepPixel(vector< vector<Anchor> >& output, int i, int j, float value)
{
    output[i][j].value    = value;
    output[i][j].i        = i;
    output[i][j].j        = j;
}//keepPixel

vector< vector<Anchor> > getAnchorMap(vector< vector<BGR> >& input,
                                      vector< vector<BGR> >& magnitudeMap,
                                      vector< vector<BGR> >& directionMap)
{
    //angles
    const float VERTICAL   = 90;
    const float HORIZONTAL = 0;

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
                    suppressPixel(output, i, j);
                }//if
                else
                {
                    keepPixel(output, i, j, magnitudeMap[i][j].B);
                }//else
            }//if

            //this is the same as above except for horizontal edges, which are
            //compared to their left and right-hand neighbors.
            if (directionMap[i][j].B == HORIZONTAL)
            {
                if (magnitudeMap[i][j].B < magnitudeMap[i][j + 1].B ||
                    magnitudeMap[i][j].B < magnitudeMap[i][j - 1].B)
                {
                    suppressPixel(output, i, j);
                }//if
                else
                {
                    keepPixel(output, i, j, magnitudeMap[i][j].B);
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
    const int NUM_PIXEL_VALUES = 256;

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
            if (anchorMap[i][j].value != 0)
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

//*****************************************************************************
//The following functions are for manipulating edge lists.
//*****************************************************************************

EdgeHandle initEdge(int length)
{
    EdgeHandle edge = (EdgeHandle) malloc(sizeof(Edge));

    edge -> length  = length;
    edge -> anchors = (AnchorArray) calloc(length, sizeof(Anchor));

    return edge;
}//initEdge

void copyEdge(EdgeHandle from, EdgeHandle to)
{
    int i;
    for (i = 0; i < from -> length; i++)
    {
        to -> anchors[i] = from -> anchors[i];
    }//for
}//copyEdge

void deleteEdge(DynamicEdge edge)
{
    free((*edge) -> anchors);
    free(*edge);

    *edge = NULL;
}//deleteEdge

void growEdge(DynamicEdge edge)
{
    EdgeHandle temp = initEdge(((*edge) -> length)*2 + 1);

    copyEdge(*edge, temp);

    deleteEdge(edge);

    *edge = temp;
}//growEdge

void insertAnchor(DynamicEdge edge, int pixelNum, Anchor newAnchor)
{
    while (pixelNum + 1 >= (*edge) -> length)
    {
        growEdge(edge);
    }//while

    (*edge) -> anchors[pixelNum] = newAnchor;
}//insertAnchor

EdgeListHandle initEdgeList(int length, int edgeLength)
{
    EdgeListHandle edgeList = (EdgeListHandle) malloc(sizeof(EdgeList));

    edgeList -> length = length;
    edgeList -> edges  = (DynamicEdge) malloc(sizeof(EdgeHandle)*length);

    int i;
    for (i = 0; i < length; i++)
    {
        edgeList -> edges[i] = initEdge(edgeLength);
    }//for

    return edgeList;
}//initEdgeList

void copyEdgeList(EdgeListHandle from, EdgeListHandle to)
{
    int i;
    int j;
    for (i = 0; i < from -> length; i++)
    {
        for (j = 0; j < from -> edges[i] -> length; j++)
        {
            insertAnchor(&(to -> edges[i]), j, from -> edges[i] -> anchors[j]);
        }//for
    }//for
}//copyEdgeList

void deleteEdgeList(DynamicEdgeList edgeList)
{
    int i;
    for (i = 0; i < (*edgeList) -> length; i++)
    {
        deleteEdge(&((*edgeList) -> edges[i]));
    }//for

    free(*edgeList);

    *edgeList = NULL;
}//deleteEdgeList

void growEdgeList(DynamicEdgeList edgeList)
{
    EdgeListHandle temp = initEdgeList(((*edgeList) -> length)*2 + 1, 0);

    copyEdgeList(*edgeList, temp);

    deleteEdgeList(edgeList);

    *edgeList = temp;
}//growEdgeList

void insertAnchorToList(DynamicEdgeList edgeList, int edgeNum, int pixelNum, Anchor newAnchor)
{
    while (edgeNum + 1 >= (*edgeList) -> length)
    {
        growEdgeList(edgeList);
    }//while

    insertAnchor(&((*edgeList) -> edges[edgeNum]), pixelNum, newAnchor);
}//insertAnchorToList

//*****************************************************************************
//end of edge list manipulating functions
//*****************************************************************************

//This function uses a smart pathing algorithm to store edge pixels into edges
void getEdgePixels(DynamicEdgeList edgeList,
                   Anchor          anchor,
                   int             edgeNum,
                   vector< vector<BGR> >& magnitudeMap,
                   vector< vector<BGR> >& directionMap,
                   vector< vector<Anchor> >& anchorMap)
{
    const int HORIZONTAL =  0;
    const int VERTICAL   = 90;
    const int ROWS       = magnitudeMap.size();
    const int COLS       = magnitudeMap[0].size();

    const int UP    = 0;
    const int DOWN  = 1;
    const int LEFT  = 2;
    const int RIGHT = 3;

    int direction;

    int currentPixel;

    currentPixel = 0;

    while(magnitudeMap[anchor.i][anchor.j].B != 0 && anchor.i < ROWS && anchor.j < COLS)
    {
        insertAnchorToList(edgeList, edgeNum, currentPixel, anchor);

        if (anchorMap[anchor.i][anchor.j].value != 0) //if pixel is an anchor
        {
            if (directionMap[anchor.i][anchor.j].B == HORIZONTAL)
            {
                if (magnitudeMap[anchor.i - 1][anchor.j - 1].B >= magnitudeMap[anchor.i    ][anchor.j - 1].B &&
                    magnitudeMap[anchor.i - 1][anchor.j - 1].B >= magnitudeMap[anchor.i + 1][anchor.j - 1].B &&
                    magnitudeMap[anchor.i - 1][anchor.j - 1].B >= magnitudeMap[anchor.i - 1][anchor.j + 1].B &&
                    magnitudeMap[anchor.i - 1][anchor.j - 1].B >= magnitudeMap[anchor.i    ][anchor.j + 1].B &&
                    magnitudeMap[anchor.i - 1][anchor.j - 1].B >= magnitudeMap[anchor.i + 1][anchor.j + 1].B)
                {
                    anchor.i     = anchor.i - 1;
                    anchor.j     = anchor.j - 1;
                    anchor.value = magnitudeMap[anchor.i][anchor.j].B;

                    ++currentPixel;

                    direction = LEFT;
                }//if
                else if (magnitudeMap[anchor.i    ][anchor.j - 1].B >= magnitudeMap[anchor.i - 1][anchor.j - 1].B &&
                         magnitudeMap[anchor.i    ][anchor.j - 1].B >= magnitudeMap[anchor.i + 1][anchor.j - 1].B &&
                         magnitudeMap[anchor.i    ][anchor.j - 1].B >= magnitudeMap[anchor.i - 1][anchor.j + 1].B &&
                         magnitudeMap[anchor.i    ][anchor.j - 1].B >= magnitudeMap[anchor.i    ][anchor.j + 1].B &&
                         magnitudeMap[anchor.i    ][anchor.j - 1].B >= magnitudeMap[anchor.i + 1][anchor.j + 1].B)
                {
                    anchor.j     = anchor.j - 1;
                    anchor.value = magnitudeMap[anchor.i][anchor.j].B;

                    ++currentPixel;

                    direction = LEFT;
                }//else if
                else if (magnitudeMap[anchor.i + 1][anchor.j - 1].B >= magnitudeMap[anchor.i    ][anchor.j - 1].B &&
                         magnitudeMap[anchor.i + 1][anchor.j - 1].B >= magnitudeMap[anchor.i - 1][anchor.j - 1].B &&
                         magnitudeMap[anchor.i + 1][anchor.j - 1].B >= magnitudeMap[anchor.i - 1][anchor.j + 1].B &&
                         magnitudeMap[anchor.i + 1][anchor.j - 1].B >= magnitudeMap[anchor.i    ][anchor.j + 1].B &&
                         magnitudeMap[anchor.i + 1][anchor.j - 1].B >= magnitudeMap[anchor.i + 1][anchor.j + 1].B)
                {
                    anchor.i     = anchor.i + 1;
                    anchor.j     = anchor.j - 1;
                    anchor.value = magnitudeMap[anchor.i][anchor.j].B;

                    ++currentPixel;

                    direction = LEFT;
                }//else if
                else if (magnitudeMap[anchor.i - 1][anchor.j + 1].B >= magnitudeMap[anchor.i    ][anchor.j - 1].B &&
                         magnitudeMap[anchor.i - 1][anchor.j + 1].B >= magnitudeMap[anchor.i - 1][anchor.j - 1].B &&
                         magnitudeMap[anchor.i - 1][anchor.j + 1].B >= magnitudeMap[anchor.i + 1][anchor.j - 1].B &&
                         magnitudeMap[anchor.i - 1][anchor.j + 1].B >= magnitudeMap[anchor.i    ][anchor.j + 1].B &&
                         magnitudeMap[anchor.i - 1][anchor.j + 1].B >= magnitudeMap[anchor.i + 1][anchor.j + 1].B)
                {
                    anchor.i = anchor.i - 1;
                    anchor.j = anchor.j + 1;
                    anchor.value = magnitudeMap[anchor.i][anchor.j].B;

                    ++currentPixel;

                    direction = RIGHT;
                }//else if
                else if (magnitudeMap[anchor.i    ][anchor.j + 1].B >= magnitudeMap[anchor.i    ][anchor.j - 1].B &&
                         magnitudeMap[anchor.i    ][anchor.j + 1].B >= magnitudeMap[anchor.i + 1][anchor.j - 1].B &&
                         magnitudeMap[anchor.i    ][anchor.j + 1].B >= magnitudeMap[anchor.i - 1][anchor.j + 1].B &&
                         magnitudeMap[anchor.i    ][anchor.j + 1].B >= magnitudeMap[anchor.i - 1][anchor.j - 1].B &&
                         magnitudeMap[anchor.i    ][anchor.j + 1].B >= magnitudeMap[anchor.i + 1][anchor.j + 1].B)
                {
                    anchor.j     = anchor.j + 1;
                    anchor.value = magnitudeMap[anchor.i][anchor.j].B;

                    ++currentPixel;

                    direction = RIGHT;
                }//else if
                else
                {
                    anchor.i = anchor.i + 1;
                    anchor.j = anchor.j + 1;
                    anchor.value = magnitudeMap[anchor.i][anchor.j].B;

                    ++currentPixel;

                    direction = RIGHT;
                }//else
            }//if
            else //directionMap[anchor.i][anchor.j].B == VERTICAL
            {
                if (magnitudeMap[anchor.i - 1][anchor.j - 1].B >= magnitudeMap[anchor.i - 1][anchor.j    ].B &&
                    magnitudeMap[anchor.i - 1][anchor.j - 1].B >= magnitudeMap[anchor.i - 1][anchor.j + 1].B &&
                    magnitudeMap[anchor.i - 1][anchor.j - 1].B >= magnitudeMap[anchor.i + 1][anchor.j - 1].B &&
                    magnitudeMap[anchor.i - 1][anchor.j - 1].B >= magnitudeMap[anchor.i + 1][anchor.j    ].B &&
                    magnitudeMap[anchor.i - 1][anchor.j - 1].B >= magnitudeMap[anchor.i + 1][anchor.j + 1].B)
                {
                    anchor.i     = anchor.i - 1;
                    anchor.j     = anchor.j - 1;
                    anchor.value = magnitudeMap[anchor.i][anchor.j].B;

                    ++currentPixel;

                    direction = UP;
                }//if
                else if (magnitudeMap[anchor.i - 1][anchor.j    ].B >= magnitudeMap[anchor.i - 1][anchor.j - 1].B &&
                         magnitudeMap[anchor.i - 1][anchor.j    ].B >= magnitudeMap[anchor.i - 1][anchor.j + 1].B &&
                         magnitudeMap[anchor.i - 1][anchor.j    ].B >= magnitudeMap[anchor.i + 1][anchor.j - 1].B &&
                         magnitudeMap[anchor.i - 1][anchor.j    ].B >= magnitudeMap[anchor.i + 1][anchor.j    ].B &&
                         magnitudeMap[anchor.i - 1][anchor.j    ].B >= magnitudeMap[anchor.i + 1][anchor.j + 1].B)
                {
                    anchor.i     = anchor.i - 1;
                    anchor.value = magnitudeMap[anchor.i][anchor.j].B;

                    ++currentPixel;

                    direction = UP;
                }//else if
                else if (magnitudeMap[anchor.i - 1][anchor.j + 1].B >= magnitudeMap[anchor.i - 1][anchor.j    ].B &&
                         magnitudeMap[anchor.i - 1][anchor.j + 1].B >= magnitudeMap[anchor.i - 1][anchor.j - 1].B &&
                         magnitudeMap[anchor.i - 1][anchor.j + 1].B >= magnitudeMap[anchor.i + 1][anchor.j - 1].B &&
                         magnitudeMap[anchor.i - 1][anchor.j + 1].B >= magnitudeMap[anchor.i + 1][anchor.j    ].B &&
                         magnitudeMap[anchor.i - 1][anchor.j + 1].B >= magnitudeMap[anchor.i + 1][anchor.j + 1].B)
                {
                    anchor.i     = anchor.i - 1;
                    anchor.j     = anchor.j + 1;
                    anchor.value = magnitudeMap[anchor.i][anchor.j].B;

                    ++currentPixel;

                    direction = UP;
                }//else if
                else if (magnitudeMap[anchor.i + 1][anchor.j - 1].B >= magnitudeMap[anchor.i - 1][anchor.j    ].B &&
                         magnitudeMap[anchor.i + 1][anchor.j - 1].B >= magnitudeMap[anchor.i - 1][anchor.j + 1].B &&
                         magnitudeMap[anchor.i + 1][anchor.j - 1].B >= magnitudeMap[anchor.i - 1][anchor.j - 1].B &&
                         magnitudeMap[anchor.i + 1][anchor.j - 1].B >= magnitudeMap[anchor.i + 1][anchor.j    ].B &&
                         magnitudeMap[anchor.i + 1][anchor.j - 1].B >= magnitudeMap[anchor.i + 1][anchor.j + 1].B)
                {
                    anchor.i     = anchor.i + 1;
                    anchor.j     = anchor.j - 1;
                    anchor.value = magnitudeMap[anchor.i][anchor.j].B;

                    ++currentPixel;

                    direction = DOWN;
                }//else if
                else if (magnitudeMap[anchor.i + 1][anchor.j    ].B >= magnitudeMap[anchor.i - 1][anchor.j    ].B &&
                         magnitudeMap[anchor.i + 1][anchor.j    ].B >= magnitudeMap[anchor.i - 1][anchor.j + 1].B &&
                         magnitudeMap[anchor.i + 1][anchor.j    ].B >= magnitudeMap[anchor.i + 1][anchor.j - 1].B &&
                         magnitudeMap[anchor.i + 1][anchor.j    ].B >= magnitudeMap[anchor.i - 1][anchor.j - 1].B &&
                         magnitudeMap[anchor.i + 1][anchor.j    ].B >= magnitudeMap[anchor.i + 1][anchor.j + 1].B)
                {
                    anchor.i     = anchor.i + 1;
                    anchor.value = magnitudeMap[anchor.i][anchor.j].B;

                    ++currentPixel;

                    direction = DOWN;
                }//else if
                else
                {
                    anchor.i     = anchor.i + 1;
                    anchor.j     = anchor.j + 1;
                    anchor.value = magnitudeMap[anchor.i][anchor.j].B;

                    ++currentPixel;

                    direction = DOWN;
                }//else
            }//else
        }//if
        else //else if pixel is not an anchor
        {
            if (direction == LEFT)
            {
                if (magnitudeMap[anchor.i - 1][anchor.j - 1].B >= magnitudeMap[anchor.i    ][anchor.j - 1].B &&
                    magnitudeMap[anchor.i - 1][anchor.j - 1].B >= magnitudeMap[anchor.i + 1][anchor.j - 1].B)
                {
                    anchor.i     = anchor.i - 1;
                    anchor.j     = anchor.j - 1;
                    anchor.value = magnitudeMap[anchor.i][anchor.j].B;

                    ++currentPixel;
                }//if
                else if (magnitudeMap[anchor.i    ][anchor.j - 1].B >= magnitudeMap[anchor.i - 1][anchor.j - 1].B &&
                         magnitudeMap[anchor.i    ][anchor.j - 1].B >= magnitudeMap[anchor.i + 1][anchor.j - 1].B)
                {
                    anchor.j     = anchor.j - 1;
                    anchor.value = magnitudeMap[anchor.i][anchor.j].B;

                    ++currentPixel;
                }//else if
                else
                {
                    anchor.i     = anchor.i + 1;
                    anchor.j     = anchor.j - 1;
                    anchor.value = magnitudeMap[anchor.i][anchor.j].B;

                    ++currentPixel;
                }//else
            }//if
            else if (direction == RIGHT)
            {
                if (magnitudeMap[anchor.i - 1][anchor.j + 1].B >= magnitudeMap[anchor.i    ][anchor.j + 1].B &&
                    magnitudeMap[anchor.i - 1][anchor.j + 1].B >= magnitudeMap[anchor.i + 1][anchor.j + 1].B)
                {
                    anchor.i     = anchor.i - 1;
                    anchor.j     = anchor.j + 1;
                    anchor.value = magnitudeMap[anchor.i][anchor.j].B;

                    ++currentPixel;
                }//if
                else if (magnitudeMap[anchor.i    ][anchor.j + 1].B >= magnitudeMap[anchor.i - 1][anchor.j + 1].B &&
                         magnitudeMap[anchor.i    ][anchor.j + 1].B >= magnitudeMap[anchor.i + 1][anchor.j + 1].B)
                {
                    anchor.j     = anchor.j + 1;
                    anchor.value = magnitudeMap[anchor.i][anchor.j].B;

                    ++currentPixel;
                }//else if
                else
                {
                    anchor.i     = anchor.i + 1;
                    anchor.j     = anchor.j + 1;
                    anchor.value = magnitudeMap[anchor.i][anchor.j].B;

                    ++currentPixel;
                }//else
            }//else if
            else if (direction == UP)
            {
                if (magnitudeMap[anchor.i - 1][anchor.j - 1].B >= magnitudeMap[anchor.i - 1][anchor.j    ].B &&
                    magnitudeMap[anchor.i - 1][anchor.j - 1].B >= magnitudeMap[anchor.i - 1][anchor.j + 1].B)
                {
                    anchor.i     = anchor.i - 1;
                    anchor.j     = anchor.j - 1;
                    anchor.value = magnitudeMap[anchor.i][anchor.j].B;

                    ++currentPixel;
                }//if
                else if (magnitudeMap[anchor.i - 1][anchor.j    ].B >= magnitudeMap[anchor.i - 1][anchor.j - 1].B &&
                         magnitudeMap[anchor.i - 1][anchor.j    ].B >= magnitudeMap[anchor.i - 1][anchor.j + 1].B)
                {
                    anchor.i     = anchor.i - 1;
                    anchor.value = magnitudeMap[anchor.i][anchor.j].B;

                    ++currentPixel;
                }//else if
                else
                {
                    anchor.i     = anchor.i - 1;
                    anchor.j     = anchor.j + 1;
                    anchor.value = magnitudeMap[anchor.i][anchor.j].B;

                    ++currentPixel;
                }//else
            }//else if
            else //direction == DOWN
            {
                if (magnitudeMap[anchor.i + 1][anchor.j - 1].B >= magnitudeMap[anchor.i + 1][anchor.j    ].B &&
                    magnitudeMap[anchor.i + 1][anchor.j - 1].B >= magnitudeMap[anchor.i + 1][anchor.j + 1].B)
                {
                    anchor.i     = anchor.i + 1;
                    anchor.j     = anchor.j - 1;
                    anchor.value = magnitudeMap[anchor.i][anchor.j].B;

                    ++currentPixel;
                }//if
                else if (magnitudeMap[anchor.i + 1][anchor.j    ].B >= magnitudeMap[anchor.i + 1][anchor.j - 1].B &&
                         magnitudeMap[anchor.i + 1][anchor.j    ].B >= magnitudeMap[anchor.i + 1][anchor.j + 1].B)
                {
                    anchor.i     = anchor.i + 1;
                    anchor.value = magnitudeMap[anchor.i][anchor.j].B;

                    ++currentPixel;
                }//else if
                else
                {
                    anchor.i     = anchor.i + 1;
                    anchor.j     = anchor.j + 1;
                    anchor.value = magnitudeMap[anchor.i][anchor.j].B;

                    ++currentPixel;
                }//else
            }
        }//else
    }//while
}//getEdgePixels

//This function extracts edges from our maps and stores them into a list
EdgeListHandle extractEdges(vector<Anchor>&         anchorList,
                            vector< vector<BGR> >&  magnitudeMap,
                            vector< vector<BGR> >&  directionMap,
                            vector< vector<Anchor> >& anchorMap)
{
    int length = anchorList.size();

    //edgeList is initialized to 0 edges of 0 length.
    //we will change these numbers after taking into account
    //the average number of edges and average pixels per edge
    //(once the data becomes available) in order to minimize
    //the number of times this array is resized.
    EdgeListHandle edgeList = initEdgeList(0, 0);

    for (int i = 0; i < length; i++)
    {
        getEdgePixels(&edgeList, anchorList[i], i, magnitudeMap, directionMap, anchorMap);
    }//for

    return edgeList;
}//getEdgeList

//once we have extracted the edges, we will apply a circular arc detection
//algorithm
//and then use that to get the x-y locations of balls in the image.

void frameToVector(Mat& frame, vector< vector<BGR> >& output)
{
    for (int i = 0; i < frame.rows; i++)
    {
        for (int j = 0; j < frame.cols; j++)
        {
            output[i][j].B = frame.data[frame.step[0]*i + frame.step[1]*j + 0];
            output[i][j].G = frame.data[frame.step[0]*i + frame.step[1]*j + 1];
            output[i][j].R = frame.data[frame.step[0]*i + frame.step[1]*j + 2];
        }//for
    }//for
}//frameToVector

void vectorToFrame(vector< vector<BGR> >& input, Mat& frame)
{
    for (int i = 0; i < frame.rows; i++)
    {
        for (int j = 0; j < frame.cols; j++)
        {
            frame.data[frame.step[0]*i + frame.step[1]*j + 0] = input[i][j].B;
            frame.data[frame.step[0]*i + frame.step[1]*j + 1] = input[i][j].G;
            frame.data[frame.step[0]*i + frame.step[1]*j + 2] = input[i][j].R;
        }//for
    }//for
}//vectorToFrame

int main(int argc, char** argv)
{
    const int   TIME_TO_WAIT_FOR_INPUT = 1; //set to 0 for infinite
    const char  ESC_KEY_CODE           = char(27);

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

        frameToVector(frame, inVec);

        grayScale(inVec);

        inVec = gaussianFilter(inVec);

        vector< vector<BGR> > magnitudeMap = prewittOp(inVec, MAGNITUDE);
        vector< vector<BGR> > directionMap = prewittOp(inVec, DIRECTION);

        vector< vector<Anchor> > anchorMap;
        anchorMap = getAnchorMap(inVec, magnitudeMap, directionMap);

        inVec = convertAnchorToBGR(anchorMap);
        //inVec = createEdgeMap (inVec, magnitudeMap, directionMap);

        vectorToFrame(inVec, frame);

        inVec.clear();
        magnitudeMap.clear();
        directionMap.clear();

        imshow("output", frame);

        char c = cvWaitKey(TIME_TO_WAIT_FOR_INPUT);
        if (c == ESC_KEY_CODE) break;
    }//while

    return 0;
}//main

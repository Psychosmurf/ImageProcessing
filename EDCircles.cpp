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
};

struct AnchorList
{
    int value;
    int i;
    int j;

    AnchorList *next;
    AnchorList *prev;
};

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
        }
    }

    return output;
}

vector< vector<BGR> > gaussianFilter(vector< vector<BGR> >& input)
{
    int rows = input.size();
    int cols = input[0].size();

    vector< vector<BGR> > output = input;

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
        }
    }

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
        }
    }

    return output;
}

vector< vector<BGR> > prewittOp(vector< vector<BGR> >& input, string control)
{
    int rows = input.size();
    int cols = input[0].size();

    vector< vector<BGR> > output(rows, vector<BGR>(cols));
    vector< vector<BGR> > temp_x(rows, vector<BGR>(cols));
    vector< vector<BGR> > temp_y(rows, vector<BGR>(cols));
    vector< vector<BGR> > Gx    (rows, vector<BGR>(cols));
    vector< vector<BGR> > Gy    (rows, vector<BGR>(cols));

    for (int i = 0; i < rows; i++)
    {
        for (int j = 1; j < cols - 1; j++)
        {
            temp_x[i][j].B = -1*input[i][j - 1].B + input[i][j + 1].B;

            temp_y[i][j].B = input[i][j - 1].B + input[i][j].B + input[i][j + 1].B;
        }
    }

    for (int i = 1; i < rows - 1; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            Gx[i][j].B = temp_x[i - 1][j].B + temp_x[i][j].B + temp_x[i + 1][j].B;

            Gy[i][j].B = -1*temp_y[i - 1][j].B + temp_y[i + 1][j].B;
        }
    }

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
                }
            }
        }
    }
    else if (control == "direction")
    {
        for (int i = 0; i < rows; i++)
        {
            for (int j = 0; j < cols; j++)
            {
                if (Gx[i][j].B >= Gy[i][j].B)
                {
                    output[i][j].B = 90;  //90 degrees
                }
                else
                {
                    output[i][j].B = 0;
                }
            }
        }
    }

    return output;
}

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
            if (directionMap[i][j].B == 90)
            {
                if (magnitudeMap[i][j].B < magnitudeMap[i + 1][j].B ||
                    magnitudeMap[i][j].B < magnitudeMap[i - 1][j].B)
                {
                    output[i][j].B = 0;
                }
                else
                {
                    output[i][j].B = magnitudeMap[i][j].B;
                }
            }

            if (directionMap[i][j].B == 0)
            {
                if (magnitudeMap[i][j].B < magnitudeMap[i][j + 1].B ||
                    magnitudeMap[i][j].B < magnitudeMap[i][j - 1].B)
                {
                    output[i][j].B = 0;
                }
                else
                {
                    output[i][j].B = magnitudeMap[i][j].B;
                }
            }
        }
    }

    return output;
}

vector< vector<BGR> > createEdgeMap(vector< vector<BGR> >& input,
                                    vector< vector<BGR> >& magnitudeMap,
                                    vector< vector<BGR> >& directionMap)
{
    int rows = input.size();
    int cols = input[0].size();

    vector< vector<BGR> > edgeMap(rows, vector<BGR>(cols));

    AnchorList *anchorList = new AnchorList;
                anchorList -> next = NULL;
                anchorList -> prev = NULL;
                anchorList -> value = -1;
                anchorList -> i = -1;
                anchorList -> j = -1;

    int anchorCount = 0;

    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            edgeMap[i][j].B = 0;

            if (input[i][j].B != 0)
            {
                ++anchorCount;

                AnchorList* newAnchor = new AnchorList;
                            newAnchor -> value = input[i][j].B;
                            newAnchor -> i     = i;
                            newAnchor -> j     = j;
                            newAnchor -> next  = anchorList;
                            newAnchor -> prev  = NULL;

                while (newAnchor -> value < newAnchor -> next -> value)
                {
                    newAnchor -> next = newAnchor -> next -> next;
                }

                if (newAnchor -> next -> next == NULL && newAnchor -> next -> prev == NULL)
                {
                    newAnchor -> next -> prev = newAnchor;
                }
                else if (newAnchor -> next -> next == NULL && newAnchor -> next -> prev != NULL)
                {
                    newAnchor -> next -> prev -> next = newAnchor;
                    newAnchor -> prev = newAnchor -> next -> prev;
                    newAnchor -> next -> prev = newAnchor;
                }
                else
                {
                    newAnchor -> next = newAnchor -> next -> next;
                    newAnchor -> prev = newAnchor -> next -> prev;
                    newAnchor -> prev -> next = newAnchor;
                    newAnchor -> next -> prev = newAnchor;
                }

                while (anchorList -> prev != NULL)
                {
                    anchorList = anchorList -> prev;
                }
            }
        }
    }

    cout << "anchors in anchorMap = " << anchorCount << "\n";

    anchorCount = 0;

    AnchorList *topOfList = anchorList;

    while (anchorList -> next != NULL)
    {
        ++anchorCount;

        int x = anchorList -> i;
        int y = anchorList -> j;

        while (magnitudeMap[x][y].B >    0 &&
               directionMap[x][y].B ==  90 &&
               edgeMap     [x][y].B != 255)
        {
            edgeMap[anchorList -> i][anchorList -> j].B = 255;

            if (magnitudeMap[x - 1][y - 1].B > magnitudeMap[  x  ][y - 1].B &&
                magnitudeMap[x - 1][y - 1].B > magnitudeMap[x + 1][y - 1].B)
            {
                x = x - 1;
                y = y - 1;
            }
            else if (magnitudeMap[  x  ][y - 1].B > magnitudeMap[x - 1][y - 1].B &&
                     magnitudeMap[  x  ][y - 1].B > magnitudeMap[x + 1][y - 1].B)
            {
                y = y - 1;
            }
            else
            {
                x = x + 1;
                y = y - 1;
            }
        }

        x = anchorList -> i;
        y = anchorList -> j;

        while (magnitudeMap[x][y].B >    0 &&
               directionMap[x][y].B ==  90 &&
               edgeMap     [x][y].B != 255)
        {
            edgeMap[anchorList -> i][anchorList -> j].B = 255;

            if (magnitudeMap[x - 1][y + 1].B > magnitudeMap[  x  ][y + 1].B &&
                magnitudeMap[x - 1][y + 1].B > magnitudeMap[x + 1][y + 1].B)
            {
                x = x - 1;
                y = y + 1;
            }
            else if (magnitudeMap[  x  ][y + 1].B > magnitudeMap[x - 1][y + 1].B &&
                     magnitudeMap[  x  ][y + 1].B > magnitudeMap[x + 1][y + 1].B)
            {
                y = y + 1;
            }
            else
            {
                x = x + 1;
                y = y + 1;
            }
        }

        x = anchorList -> i;
        y = anchorList -> j;

        while (magnitudeMap[x][y].B >    0 &&
               directionMap[x][y].B ==   0 &&
               edgeMap     [x][y].B != 255)
        {
            edgeMap[anchorList -> i][anchorList -> j].B = 255;

            if (magnitudeMap[x - 1][y - 1].B > magnitudeMap[x - 1][  y  ].B &&
                magnitudeMap[x - 1][y - 1].B > magnitudeMap[x - 1][y + 1].B)
            {
                x = x - 1;
                y = y - 1;
            }
            else if (magnitudeMap[x - 1][  y  ].B > magnitudeMap[x - 1][y - 1].B &&
                     magnitudeMap[x - 1][  y  ].B > magnitudeMap[x - 1][y + 1].B)
            {
                x = x - 1;
            }
            else
            {
                x = x - 1;
                y = y + 1;
            }
        }

        x = anchorList -> i;
        y = anchorList -> j;

        while (magnitudeMap[x][y].B >    0 &&
               directionMap[x][y].B ==   0 &&
               edgeMap     [x][y].B != 255)
        {
            edgeMap[anchorList -> i][anchorList -> j].B = 255;

            if (magnitudeMap[x + 1][y - 1].B > magnitudeMap[x + 1][  y  ].B &&
                magnitudeMap[x + 1][y - 1].B > magnitudeMap[x + 1][y + 1].B)
            {
                x = x + 1;
                y = y - 1;
            }
            else if (magnitudeMap[x + 1][  y  ].B > magnitudeMap[x + 1][y - 1].B &&
                     magnitudeMap[x + 1][  y  ].B > magnitudeMap[x + 1][y + 1].B)
            {
                x = x + 1;
            }
            else
            {
                x = x + 1;
                y = y + 1;
            }
        }

        anchorList = anchorList -> next;
    }

    while (topOfList != NULL)
    {
        AnchorList* current = topOfList;

        topOfList = topOfList -> next;

        delete current;
    }

    cout << "anchors in edgeMap = " << anchorCount << "\n";

    return edgeMap;
}

int main()
{
    namedWindow("input" , CV_WINDOW_NORMAL);
    namedWindow("output", CV_WINDOW_NORMAL);

    string       inFileName = "/home/nikola/Videos/bouncyBalls.mp4";
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
                BGR pixel;

                pixel.B = frame.data[frame.step[0]*i + frame.step[1]*j + 0];
                pixel.G = frame.data[frame.step[0]*i + frame.step[1]*j + 1];
                pixel.R = frame.data[frame.step[0]*i + frame.step[1]*j + 2];

                inVec[i][j] = pixel;
            }
        }

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
            }
        }

        inVec.clear();
        magnitudeMap.clear();
        directionMap.clear();

        imshow("output", frame);

        char c = cvWaitKey(33);
        if (c == 27) break;
    }

    return 0;
}

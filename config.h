#include "cv.h"
#include "highgui.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <time.h>
#include <ctype.h>
 
#ifdef _EiC
#define WIN32
#endif
/*
*
* debug showing
*
*/ 
void cvShowImg(IplImage* img);
void cvCircleFaces(IplImage* img, CvSeq* faces, float scale);
static CvScalar COLORS[] = 
    {
        {{0,0,255}},
        {{0,128,255}},
        {{0,255,255}},
        {{0,255,0}},
        {{255,128,0}},
        {{255,255,0}},
        {{255,0,0}},
        {{255,0,255}}
    };

/*
* toolkits
*/
CvRect cvDetectFace(IplImage* img, int min, int max);
CvRect cvMinWrapRect(CvSeq* faces, CvSize imgsize);

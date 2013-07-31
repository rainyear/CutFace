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
CvRect cvDetectFaces(IplImage* img, int min, int max);
CvRect cvMinWrapRect(CvSeq* faces, CvSize imgsize, float scale);

void cvExpandWrapRect(CvRect* wrapper, int exp);

void cvCenterSquare(IplImage* img, int square, CvRect* face_square);
void cvFaceSquare(IplImage* img, int square, CvRect faces_wrapper, CvRect* face_square);

void cvCutAndSave(IplImage* src, CvRect face_square, char* dest);

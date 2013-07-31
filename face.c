#include "config.h"

#define DEBUG 1

#define SCALETO 290
#define SCALE 1.3
#define MINFACE 10
#define MAXFACE 200

float scale = SCALE;

int main(int argc, char** argv){
	IplImage* source = 0;
	CvRect face_win;

	int square;

	if(argc<2){
    	printf("Usage: main <image-file-name>\n\7");
    	exit(0);
  	} 
 
  	// load an image  
  	source = cvLoadImage(argv[1], 1);// load as rgb
  	if(!source){
    	printf("Could not load image file: %s\n",argv[1]);
    	exit(0);
  	}

  	square = source->width < source->height ? source->width : source->height;
  	// skip cutting images size below SCALETO
  	if(square <= SCALETO){
  		printf("Too small to cut.\n");
  		exit(0);
  	}
  	
  	face_win = cvDetectFaces(source, MINFACE, MAXFACE);
  	if(face_win.width == 0){
  		// no face detected, just cut center square.
  		cvCutCenterSquare(source, square);
  	}else{
  		// fit faces' min rectangle wrapper with square region.
  		cvCutFaceSquare(source, square, face_win);
  	}

  	cvReleaseImage(&source);
	return 0;
}

void cvShowImg(IplImage* img){
  	cvNamedWindow("source", CV_WINDOW_AUTOSIZE);
  	cvShowImage("source", img);
  	cvWaitKey(0);
  	cvDestroyWindow("source");
}
void cvCircleFaces(IplImage* img, CvSeq* faces, float scale){
	int i;
	for( i = 0; i < (faces ? faces->total : 0); i++ ){
        	CvRect* r = (CvRect*)cvGetSeqElem( faces, i );
        	CvPoint center;
        	int radius;
        	center.x = cvRound((r->x + r->width*0.5)*scale);
       		center.y = cvRound((r->y + r->height*0.5)*scale);
       		radius = cvRound((r->width + r->height)*0.25*scale);
        	cvCircle( img, center, radius, COLORS[i%8], 3, 8, 0 );
    }
}

CvRect cvDetectFaces(IplImage* img, int min, int max){
	int square;
	CvRect face_win;
	static CvMemStorage* storage = 0;
	static CvHaarClassifierCascade* cascade = 0;

	//float scale  =  1.5;

	IplImage* gray = cvCreateImage( cvGetSize(img), 8, 1 );
	IplImage* small_img = cvCreateImage( cvSize( cvRound (img->width/scale),
                         cvRound (img->height/scale)),
                     8, 1 );
	cvCvtColor( img, gray, CV_BGR2GRAY );
	cvResize( gray, small_img, CV_INTER_LINEAR );
    cvEqualizeHist( small_img, small_img );

	char* cascade_name ="/Users/rainy/Projects/C/CutFace/haarcascades/haarcascade_frontalface_alt.xml";
	cascade = (CvHaarClassifierCascade*)cvLoad( cascade_name, 0, 0, 0 );

	storage = cvCreateMemStorage(0);
	CvSeq* faces = cvHaarDetectObjects(small_img, cascade, storage, 1.1, 2, 0, cvSize(min, min), cvSize(max, max));

	int i;
	if(0 == faces->total){
		face_win = cvRect(0, 0, 0, 0);
  		if(DEBUG)
  			printf("rect:[%d, %d, %d, %d]\n", face_win.x, face_win.y, face_win.width, face_win.height);
  	}else{
  		cvCircleFaces(img, faces, scale);
  		face_win = cvMinWrapRect(faces, cvGetSize(img), scale);
  		
  	}
  	//cvCircleFaces(img, faces, scale);
    //cvShowImg(img);

    cvReleaseImage( &gray );
    cvReleaseImage( &small_img );

    return face_win;

}
CvRect cvMinWrapRect(CvSeq* faces, CvSize imgsize, float scale){
	CvPoint left_top = cvPoint(imgsize.width, imgsize.height);
	CvPoint right_bot = cvPoint(0, 0);
	int i;

	for(i = 0; i < faces->total; i++){
		CvRect* r = (CvRect*)cvGetSeqElem( faces, i );

		left_top.x = r->x < left_top.x ? r->x : left_top.x;
		left_top.y = r->y < left_top.y ? r->y : left_top.y;

		right_bot.x = r->x+r->width > right_bot.x ? r->x+r->width : right_bot.x;
		right_bot.y = r->y+r->height > right_bot.y ? r->y+r->height : right_bot.y;

	}

	if(DEBUG)
		printf("cvMinWrapRect: [%d, %d, %d, %d]\n", left_top.x, left_top.y, right_bot.x - left_top.x, right_bot.y - left_top.y);
	return cvRect(cvRound(left_top.x * scale),
				cvRound(left_top.y * scale),
				cvRound((right_bot.x - left_top.x) * scale),
				cvRound((right_bot.y - left_top.y) * scale));
}


// cut!
void cvCutCenterSquare(IplImage* img, int square){
	CvRect face_win;

  	int x = img->width < img->height ? 0 : cvRound((img->width - img->height) * 0.5);
  	int y = img->width < img->height ? cvRound((img->height - img->width) * 0.5) : 0;

  	face_win = cvRect(x, y, square, square);

  	cvRectangle(img,
    		cvPoint(face_win.x,face_win.y),
    		cvPoint(face_win.x+face_win.width, face_win.y+face_win.height),
    		CV_RGB(255,250,250), 2, 8, 0);

  	cvShowImg( img );
}
void cvCutFaceSquare(IplImage* img, int square, CvRect face_win){
	CvPoint face_center = cvPoint(face_win.x + cvRound(face_win.width * 0.5), face_win.y + cvRound(face_win.height * 0.5));
	CvRect face_square;

	if(img->width > img->height){
		// locate the square with cord-x
		if(face_center.x < square * 0.5){
			face_square = cvRect(0, 0, square, square);// left side
		}else if(face_center.x > img->width - square * 0.5){
			face_square = cvRect(img->width - square, 0, square, square);// right side
		}else{
			face_square = cvRect(face_center.x - cvRound(square * 0.5), 0, square, square);// face center is the square center
		}
	}else{
		// locate the square withe cord-y
		if(face_center.y < square * 0.5){
			face_square = cvRect(0, 0, square, square);
		}else if(face_center.y > img->height - square * 0.5){
			face_square = cvRect(0, img->height - square, square, square);
		}else{
			face_square = cvRect(0, face_center.y - cvRound(square * 0.5), square, square);
		}
	}
	cvRectangle(img,
  			cvPoint(cvRound(face_square.x), cvRound(face_square.y)),
  			cvPoint(cvRound((face_square.x + face_square.width)), cvRound((face_square.y + face_square.height))),
  			CV_RGB(255,255,255), 2, 8, 0);

	cvShowImg( img );
}
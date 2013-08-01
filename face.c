#include "config.h"

#define DEBUG 0
#define CENTER 1

#define DESTW 110
#define DESTH 90
#define CLIPPER 30

#define SCALETO 290

#define SCALE 1.5
#define MINFACE 10
#define MAXFACE 300

float scale = SCALE;

int main(int argc, char** argv){
	IplImage* source = NULL;
	IplImage* operat = NULL;
	CvRect clip_roi;
	CvRect face_win;
	CvRect face_square;
	CvSize frame;

	float dest_rate, oper_rate;
	int square;
	char* dest;

	if(argc<3){
    	printf("Usage: main <image-file-name> <dest-file-name>\n\7");
    	exit(0);
  	} 
 
  	// load an image  
  	source = cvLoadImage(argv[1], 1);// load as rgb
  	if(!source){
    	printf("Could not load image file: %s\n",argv[1]);
    	exit(0);
  	}

  	if(DESTW >= source->width - CLIPPER * 2 || DESTH >= source->height - CLIPPER * 2){
  		printf("Too small to cut.\n");
  		exit(0);
  	}
  	operat = cvCreateImage(cvSize(source->width - CLIPPER * 2, source->height - CLIPPER * 2),
  							source->depth,
  							source->nChannels);
  	clip_roi = cvRect(CLIPPER, CLIPPER, source->width - CLIPPER * 2, source->height - CLIPPER * 2);
  	cvSetImageROI(source, clip_roi);
  	cvCopy(source, operat, NULL);

  	dest = argv[2];

  	dest_rate = DESTH * 1.0 / DESTW;
  	oper_rate = operat->height * 1.0 / operat->width;

  	if(dest_rate == oper_rate){
  		// just scale to dest-size
  		if(DEBUG)
  			printf("Just scale to dest-size\n");
  		face_square = cvRect(0, 0, operat->width, operat->height);
  	}else{
  		if(dest_rate < oper_rate){
  			frame = cvSize(operat->width, cvRound((DESTH * operat->width) / (1.0 * DESTW)));
  		}else{
  			frame = cvSize(cvRound((DESTW * operat->height) / (1.0 * DESTH)), operat->height);
  		}

  		face_win = cvDetectFaces(operat, MINFACE, MAXFACE);

  		if(face_win.width == 0 || CENTER){
  			// no face detected, just cut center square.
  			cvCenterSquare(operat, frame, &face_square);
  		}else{
  			// fit faces' min rectangle wrapper with square region.
  			cvFaceSquare(operat, frame, face_win, &face_square);
  		}
  	}


  	if(DEBUG)
  		printf("face_square: [%d, %d, %d, %d]\n", face_square.x, face_square.y, face_square.width, face_square.height);

  	cvCutAndSave(operat, face_square, dest);

  	cvResetImageROI(source);
  	cvReleaseImage(&source);
  	cvReleaseImage(&operat);
	return 0;
}
void cvCutAndSave(IplImage* src, CvRect face_square, char* dest){
	cvSetImageROI(src, face_square);

	IplImage* ROImage = cvCreateImage(cvSize(face_square.width, face_square.height),
										src->depth,
										src->nChannels);
	IplImage* dest_img = cvCreateImage(cvSize(DESTW, DESTH),
										src->depth,
										src->nChannels);
	cvCopy(src, ROImage, NULL);
	cvResize(ROImage, dest_img, CV_INTER_LINEAR);
	cvSaveImage(dest, dest_img, 0);

	cvResetImageROI(src);
	cvReleaseImage(&ROImage);
	cvReleaseImage(&dest_img);
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
  		if(DEBUG)
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
void cvCenterSquare(IplImage* img, CvSize frame, CvRect* face_win){
	int x = frame.width == img->width ? 0 : cvRound((img->width - frame.width) * 0.5);
	int y = frame.height == img->height ? 0 : cvRound((img->height - frame.height) * 0.5);

  	*face_win = cvRect(x, y, frame.width, frame.height);

  	if(DEBUG){
  		cvRectangle(img,
    		cvPoint(face_win->x,face_win->y),
    		cvPoint(face_win->x+face_win->width, face_win->y+face_win->height),
    		CV_RGB(255,250,250), 2, 8, 0);

  		cvShowImg( img );
  	}
}
void cvFaceSquare(IplImage* img, CvSize frame, CvRect face_win, CvRect* face_square){
	CvPoint face_center = cvPoint(face_win.x + cvRound(face_win.width * 0.5), face_win.y + cvRound(face_win.height * 0.5));

	if(img->height == frame.height){
		// locate the square with cord-x
		if(face_center.x < frame.width * 0.5){
			*face_square = cvRect(0, 0, frame.width, frame.height);// left side
		}else if(face_center.x > img->width - frame.width * 0.5){
			*face_square = cvRect(img->width - frame.width, 0, frame.width, frame.height);// right side
		}else{
			*face_square = cvRect(face_center.x - cvRound(frame.width * 0.5), 0, frame.width, frame.height);// face center is the square center
		}
	}else{
		// locate the square withe cord-y
		if(face_center.y < frame.height * 0.5){
			*face_square = cvRect(0, 0, frame.width, frame.height);
		}else if(face_center.y > img->height - frame.height * 0.5){
			*face_square = cvRect(0, img->height - frame.height, frame.width, frame.height);
		}else{
			*face_square = cvRect(0, face_center.y - cvRound(frame.height * 0.5), frame.width, frame.height);
		}
	}
	if(DEBUG){
		cvRectangle(img,
  			cvPoint(cvRound(face_square->x), cvRound(face_square->y)),
  			cvPoint(cvRound((face_square->x + face_square->width)), cvRound((face_square->y + face_square->height))),
  			CV_RGB(255,255,255), 2, 8, 0);

		cvShowImg( img );
	}
}
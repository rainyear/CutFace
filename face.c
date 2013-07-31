#include "config.h"

#define SCALETO 290
#define DEBUG 1

float scale = 0;

int main(int argc, char** argv){
	int square;
	IplImage* source = 0;

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
  	if(square <= SCALETO){
  		printf("Too small to cut.\n");
  		exit(0);
  	}

  	scale = 1.5;

  	cvDetectFace(source, 10, 200);

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

CvRect cvDetectFace(IplImage* img, int min, int max){
	int square;
	CvRect cut_win;
	CvRect wrap_faces;
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
	if(!faces){
  		square = img->width < img->height ? img->width : img->height;
  		int x = img->width < img->height ? 0 : cvRound((img->width - img->height) / 2);
  		int y = img->width < img->height ? cvRound((img->height - img->width) / 2) : 0;

  		cut_win = cvRect(x, y, square, square);

  		printf("rect:[%d, %d, %d, %d]\n", cut_win.x, cut_win.y, cut_win.width, cut_win.height);
    	cvRectangle(img,
    		cvPoint(cut_win.x,cut_win.y),
    		cvPoint(cut_win.x+cut_win.width, cut_win.y+cut_win.height),
    		CV_RGB(255,250,250), 2, 8, 0);

  		return cut_win;

  	}else{
  		cvCircleFaces(img, faces, scale);
  		wrap_faces = cvMinWrapRect(faces, cvGetSize(img));

  		cvRectangle(img,
  			cvPoint(cvRound(wrap_faces.x*scale), cvRound(wrap_faces.y*scale)),
  			cvPoint(cvRound((wrap_faces.x + wrap_faces.width)*scale), cvRound((wrap_faces.y + wrap_faces.height)*scale)),
  			CV_RGB(255,255,255), 2, 8, 0);


  	}

  	//cvCircleFaces(img, faces, scale);


    cvShowImg(img);

    cvReleaseImage( &gray );
    cvReleaseImage( &small_img );

    return cut_win;

}
CvRect cvMinWrapRect(CvSeq* faces, CvSize imgsize){
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

	printf("cvMinWrapRect: [%d, %d, %d, %d]\n", left_top.x, left_top.y, right_bot.x - left_top.x, right_bot.y - left_top.y);
	return cvRect(left_top.x, left_top.y, right_bot.x - left_top.x, right_bot.y - left_top.y);
}

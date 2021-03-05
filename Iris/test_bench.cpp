#include "top_level.hpp"
#include <hls_opencv.h>

using namespace cv;

void test_canny(){
	Mat dst = imread("C://Users//Dennis//VivadoHLS//Final//Database//IITD Database//001//01_L.bmp", IMREAD_GRAYSCALE);
	uint8_t *imageIn = dst.data;
	static uint8_t *imageOut = (uint8_t *) malloc(sizeof(uint8_t)*MAX_WIDTH*MAX_HEIGHT);
	segmentaion::Canny<MAX_WIDTH,MAX_HEIGHT>(imageIn,imageOut);
	Mat out = Mat(MAX_HEIGHT,MAX_WIDTH,CV_8UC1 ,imageOut);
	imwrite("CannyOut.jpg",out);
}

void test_Hough(){
	Mat image = imread("C://Users//Dennis//VivadoHLS//Final//Database//IITD Database//001//01_L.bmp", IMREAD_GRAYSCALE);
	uint8_t *imageIn = image.data;
	int x,y;
	static uint8_t *imageOut = (uint8_t *) malloc(sizeof(uint8_t)*MAX_WIDTH*MAX_HEIGHT);
	segmentaion::removeWhite<MAX_WIDTH,MAX_HEIGHT>(imageIn, imageOut);
	segmentaion::morphOpening<MAX_WIDTH,MAX_HEIGHT>(imageOut,imageIn);
	segmentaion::Canny<MAX_WIDTH,MAX_HEIGHT>(imageIn,imageOut);
	//segmentaion::HOUGH_Circle_Center<MAX_WIDTH,MAX_HEIGHT>(imageOut, x, y);
	segmentaion::getPupil_center<MAX_WIDTH,MAX_HEIGHT>(imageOut,imageIn, x,y);
	std::cout << x << " ; " << y << "\n";
	Mat out = Mat(MAX_HEIGHT,MAX_WIDTH,CV_8UC1 ,imageOut);
	imwrite("CannyOpening.jpg",out);
}

void test_morph(){
	Mat dst = imread("C://Users//Dennis//VivadoHLS//Final//Database//IITD Database//001//01_L.bmp", IMREAD_GRAYSCALE);
	uint8_t *imageIn = dst.data;
	static uint8_t *imageEro = (uint8_t *) malloc(sizeof(uint8_t)*MAX_WIDTH*MAX_HEIGHT);
	static uint8_t *imageDil = (uint8_t *) malloc(sizeof(uint8_t)*MAX_WIDTH*MAX_HEIGHT);
	static uint8_t *imageOpe = (uint8_t *) malloc(sizeof(uint8_t)*MAX_WIDTH*MAX_HEIGHT);

	erode<MAX_WIDTH,MAX_HEIGHT>(imageIn,imageEro);
	dilate<MAX_WIDTH,MAX_HEIGHT>(imageIn,imageDil);
	morphOpening<MAX_WIDTH,MAX_HEIGHT>(imageIn,imageOpe);

	Mat out = Mat(MAX_HEIGHT,MAX_WIDTH,CV_8UC1 ,imageEro);
	imwrite("ErodeOut.jpg",out);

	out = Mat(MAX_HEIGHT,MAX_WIDTH,CV_8UC1 ,imageDil);
	imwrite("dilateOut.jpg",out);

	out = Mat(MAX_HEIGHT,MAX_WIDTH,CV_8UC1 ,imageOpe);
	imwrite("openingOut.jpg",out);
}

void testCordic() {
	std::cout << "Testing angular accuracy: " << std::endl;

	for (int i = 0; i <= 360; i++) {
		float s1 = sin(i * types::DegToRad);
		float c1 = cos(i * types::DegToRad);
		floatSin c,s;
		floatArg arg =i * types::DegToRad;
		cordic_fix(arg,s,c);
		float s2,c2;
		s2 = s.to_float();
		c2 = c.to_float();

		std::cout << "(1) Math.h sinus(" << i << ") = " << s1 << "\n";
		std::cout << "(2) CORDIC. sinus(" << i << ") = " << s2 << "\n";
		std::cout << "\tDiff. sinus: " << abs(((s1 - s2) / s1) * 100) << "%\n";

		std::cout << "(1) Math.h cosine(" << i << ") = " << c1 << "\n";
		std::cout << "(2) Estim. cosine(" << i << ") = " << c2 << "\n";
		std::cout << "\tDiff. cosine: " << abs(((s1 - s2) / s1) * 100) << "%\n";

	}
}

void testNormalisation(){
	Mat pic = imread("C://Users//Dennis//VivadoHLS//Final//Database//IITD Database//001//01_L.bmp", IMREAD_GRAYSCALE);
	uint8_t *imageIn = pic.data;
	uint8_t imageOut1[NORM_WIDTH*NORM_HEIGHT];
	uint8_t imageOut2[NORM_WIDTH*NORM_HEIGHT];
	norm_fix(imageIn,imageOut1,129,141,46,102);
	norm_float(imageIn,imageOut2,129,141,46,102);

	Mat out1 = Mat(NORM_HEIGHT,NORM_WIDTH,CV_8UC1 ,imageOut1);
	Mat out2 = Mat(NORM_HEIGHT,NORM_WIDTH,CV_8UC1 ,imageOut2);
	imwrite("fix.jpg",out1);
	imwrite("float.jpg",out2);
}


void hls_test1(){
	IplImage* src_image = new IplImage;

	AXI_STREAM src_stream;

	src_image = cvLoadImage("C://Users//Dennis//VivadoHLS//Final//Database//IITD Database//001//01_L.bmp");

	uint8_t norm_image[NORM_WIDTH*NORM_HEIGHT];

	IplImage2AXIvideo(src_image, src_stream);
	top_level_normalisation(src_stream,norm_image);

	Mat out = Mat(NORM_HEIGHT,NORM_WIDTH,CV_8UC1 ,norm_image);
	imwrite("norm.jpg",out);

	cvReleaseImage(&src_image);
}

int main(){
	//test_canny();
	//test_Hough();
	testNormalisation();
	return 0;
}

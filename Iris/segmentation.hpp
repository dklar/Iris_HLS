#include <stdint.h>
#include "types.hpp"

#ifndef SEG_H
#define SEG_H

using namespace types;

namespace segmentaion {

/* Direction of the ascent of a pixel
 * in a picture only 4 angles are possible.
 */
enum Direction {
	GRAD_0, GRAD_45, GRAD_90, GRAD_135
};

/* STRUCT used to store the color and
 * the direction(ascent) of a picture
 */
struct Pixel {
	uint8_t color;
	Direction dir;
};

/* Helper STRUCT to save line information
 * for HOUGH Circle transform
 */
struct line {
	uint16_t begin;
	uint16_t end;
};

/* STRUCT to save the probability value
 * of a 2D point
 */
struct point {
	uint16_t X, Y;
	uint16_t Val;
};

/* STRUCT to save the probability value
 * of a 1D point
 */
struct point2D {
	uint16_t X, Val;
};

/* STRUCT to save the probability value
 * of a 2D point
 */
struct point3D {
	uint16_t X, Y, Val;
};

struct coord{
	uint16_t x,y;
	coord(uint16_t _x,uint16_t _y){
		x = _x;
		y = _y;
	}
	coord(){
		x=0;
		y=0;
	}

};

template<int w>
void initList(uint16_t *in, uint16_t val) {
	initLoop:
	for (int i = 0; i < w; i++) {
		in[i] = val;
	}
}

template<int w,int h>
void MatToArray(GRAY_IMAGE &in, uint8_t* out) {
	PIXELGRAY pixel_value;

	loopPixel: for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
#pragma HLS loop_flatten off
#pragma HLS pipeline II=1
			in >> pixel_value;
			out[x + y * w] = pixel_value.val[0];
		}
	}
}

template<int w, int h>
void MatToGrayArray(RGB_IMAGE &in, uint8_t* out) {
	RGBPIXEL pixel_value;
	loopPixel: for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
#pragma HLS loop_flatten off
#pragma HLS pipeline II=1
			in >> pixel_value;
			uint8_t r = (pixel_value.val[0] * 77)  >> 8;	//*0.299
			uint8_t g = (pixel_value.val[1] * 150) >> 8;	//*0.587
			uint8_t b = (pixel_value.val[2] * 28)  >> 8;	//0.114
			out[x + y * w] = r + g + b;
		}
	}
}

template<int w, int h>
void ArrayToMat(uint8_t* in, RGB_IMAGE &out) {
	RGBPIXEL px1;

	backConvertLoop: for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
#pragma HLS PIPELINE II=1
#pragma HLS LOOP_FLATTEN off
			px1.val[0] = in[x + y * w];
			px1.val[1] = in[x + y * w];
			px1.val[2] = in[x + y * w];
			out << px1;
		}
	}
}

template<int w, int h>
void ArrayToMat(uint8_t* in, RGB_NORM_IMAGE &out) {
	RGBPIXEL px1;

	backConvertLoop: for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
#pragma HLS PIPELINE II=1
#pragma HLS LOOP_FLATTEN off
			px1.val[0] = in[x + y * w];
			px1.val[1] = in[x + y * w];
			px1.val[2] = in[x + y * w];
			out << px1;
		}
	}
}

template<int w, int h>
void removeWhite(uint8_t *src, uint8_t *dst) {
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
#pragma HLS PIPELINE II=1
#pragma HLS LOOP_FLATTEN off
			uint8_t v = src[y * w + x];
			if (v > 235)
				dst[y * w + x] = 0;
			else
				dst[y * w + x] = v;
		}
	}

}

/**
* Morphological erode operation
* see https://docs.opencv.org/3.4/db/df6/tutorial_erosion_dilatation.html
* @tparam WIDTH width of the images
* @tparam HEIGHT height of the images
* @param src Source image
* @param dst destination image
**/
template<int WIDTH, int HEIGHT>
void erode(uint8_t *src, uint8_t *dst) {
	const int WINDOW_SIZE = 3;

	uint8_t line_buf[WINDOW_SIZE][WIDTH];
	uint8_t window_buf[WINDOW_SIZE][WINDOW_SIZE];

#pragma HLS ARRAY_RESHAPE variable=line_buf complete dim=1
#pragma HLS ARRAY_PARTITION variable=window_buf complete dim=0

	for (int i = 0; i < WINDOW_SIZE; i++) {
		for (int j = 0; j < WIDTH; j++) {
			line_buf[i][j] = 255;
		}
	}
	for (int i = 0; i < WINDOW_SIZE; i++) {
		for (int j = 0; j < WINDOW_SIZE; j++) {
			window_buf[i][j] = 255;
		}
	}

	erodeLoop: for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
#pragma HLS PIPELINE II=1
#pragma HLS LOOP_FLATTEN off

			for (int i = 0; i < WINDOW_SIZE - 1; i++)
				line_buf[i][x] = line_buf[i + 1][x];

			line_buf[WINDOW_SIZE - 1][x] = src[x + y * WIDTH];

			for (int i = 0; i < WINDOW_SIZE; i++) {
				for (int j = 0; j < WINDOW_SIZE - 1; j++) {
					window_buf[i][j] = window_buf[i][j + 1];
				}
			}

			for (int i = 0; i < WINDOW_SIZE; i++)
				window_buf[i][WINDOW_SIZE - 1] = line_buf[i][x];

			uint8_t min = 255;

			for (int i = 0; i < WINDOW_SIZE; i++) {
				for (int j = 0; j < WINDOW_SIZE; j++) {
					min = min > window_buf[i][j] ? window_buf[i][j] : min;
				}
			}
			dst[x + y * WIDTH] = min;
		}
	}
}

/**
* Morphological dilate operation
* see https://docs.opencv.org/3.4/db/df6/tutorial_erosion_dilatation.html
* @tparam WIDTH width of the images
* @tparam HEIGHT height of the images
* @param src Source image
* @param dst destination image
**/
template<int WIDTH, int HEIGHT>
void dilate(uint8_t *src, uint8_t *dst) {
	const int WINDOW_SIZE = 3;

	uint8_t line_buf[WINDOW_SIZE][WIDTH];
	uint8_t window_buf[WINDOW_SIZE][WINDOW_SIZE];

#pragma HLS ARRAY_RESHAPE variable=line_buf complete dim=1
#pragma HLS ARRAY_PARTITION variable=window_buf complete dim=0

	for (int i=0;i<WINDOW_SIZE;i++){
		for (int j=0;j<WIDTH;j++){
			line_buf[i][j] = 0;
		}
	}
	for (int i=0;i<WINDOW_SIZE;i++){
		for (int j=0;j<WINDOW_SIZE;j++){
			window_buf[i][j] = 0;
		}
	}

	dilateLoop:
	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
#pragma HLS PIPELINE II=1
#pragma HLS LOOP_FLATTEN off


			for (int i = 0; i < WINDOW_SIZE - 1; i++) 
				line_buf[i][x] = line_buf[i + 1][x];

			line_buf[WINDOW_SIZE - 1][x] = src[x + y * WIDTH];


			for (int i = 0; i < WINDOW_SIZE; i++) {
				for (int j = 0; j < WINDOW_SIZE - 1; j++) {
					window_buf[i][j] = window_buf[i][j + 1];
				}
			}

			for (int i = 0; i < WINDOW_SIZE; i++)
				window_buf[i][WINDOW_SIZE - 1] = line_buf[i][x];

			uint8_t max = 0;
			for (int i = 0; i < WINDOW_SIZE; i++) {
				for (int j = 0; j < WINDOW_SIZE; j++) {
					max = max < window_buf[i][j] ? window_buf[i][j] : max;
				}
			}
			dst[x + y * WIDTH] = max;
		}
	}
}

/**
* Close small holes in a picture by executing
* first erosion then dilation.
* Used here to minimize the probability of a missdetection
* of a circle. (the algorithm to detect circles is very
* sensitive to holes.
* @tparam WIDTH width of the images
* @tparam HEIGHT height of the images
* @param src Source image
* @param dst destination image
**/
template<int WIDTH, int HEIGHT>
void morphOpening(uint8_t *src, uint8_t *dst){
#pragma HLS DATAFLOW
	uint8_t fifo1[WIDTH * HEIGHT];
#pragma HLS STREAM variable=fifo1 depth=1 dim=1
	erode<WIDTH, HEIGHT>(src, fifo1);
	dilate<WIDTH, HEIGHT>(fifo1,dst);
}

/**
 * 5x5 Gauss filter
 * @param src Source image
 * @param dst Destination image
 * @tparam WIDTH Width of the source and destination image
 * @tparam HEIGHT Height of the source and destination image
 */
template<int WIDTH, int HEIGHT>
void Gauss(uint8_t* src, uint8_t *dst) {
	const int k_size = 5;

	uint8_t line_buf[k_size][WIDTH];
	uint8_t window_buf[k_size][k_size];

#pragma HLS ARRAY_RESHAPE variable=line_buf complete dim=1
#pragma HLS ARRAY_PARTITION variable=window_buf complete dim=0

	const int G_KERN[k_size][k_size] = {
			{ 1, 4, 7, 4, 1 },
			{ 4, 16, 26, 16, 4 },
			{ 7, 26, 41, 26, 7 },
			{ 4, 16, 26, 16, 4 },
			{ 1,  4,  7,  4, 1 } };
#pragma HLS ARRAY_PARTITION variable=G_KERN complete dim=0

	GaussianConvolveLoop: for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
#pragma HLS PIPELINE II=1
#pragma HLS LOOP_FLATTEN off

			prepareLineBuffer: for (int i = 0; i < k_size - 1; i++)
				line_buf[i][x] = line_buf[i + 1][x];

			line_buf[k_size - 1][x] = src[x + y * WIDTH];

			for (int i = 0; i < k_size; i++) {
				for (int j = 0; j < k_size - 1; j++) {
					window_buf[i][j] = window_buf[i][j + 1];
				}
			}

			for (int i = 0; i < k_size; i++)
				window_buf[i][k_size - 1] = line_buf[i][x];

			uint8_t pixel = 0;

			conv: for (int i = 0; i < k_size; i++) {
				for (int j = 0; j < k_size; j++) {
					pixel += window_buf[i][j] * G_KERN[i][j];
				}
			}
			pixel >>= 8;
			dst[x + y * WIDTH] = pixel;
		}
	}
}

/**
 * 3x3 Gauss filter
 * @param src Source image
 * @param dst Destination image
 * @tparam WIDTH Width of the source and destination image
 * @tparam HEIGHT Height of the source and destination image
 */
template <int WIDTH,int HEIGHT>
void Gauss3(uint8_t* src, uint8_t *dst){
	const int k_size = 3;

	uint8_t line_buf[k_size][WIDTH];
	uint8_t window_buf[k_size][k_size];

	#pragma HLS ARRAY_RESHAPE variable=line_buf complete dim=1
	#pragma HLS ARRAY_PARTITION variable=window_buf complete dim=0

	const uint8_t G_KERN[k_size][k_size] = { { 1, 2, 1 }, { 2, 4, 2 }, { 1, 2, 1 } };
	#pragma HLS ARRAY_PARTITION variable=G_KERN complete dim=0

	GaussianConvolveLoop:
	for (int y = 0;y< HEIGHT;y++){
		for(int x = 0;x< WIDTH;x++){
			#pragma HLS PIPELINE II=1
			#pragma HLS LOOP_FLATTEN off

			prepareLineBuffer:
			for(int i = 0; i< k_size-1;i++)
				line_buf[i][x] = line_buf[i+1][x];

			line_buf[k_size - 1][x] = src[x + y*WIDTH];

			for(int i = 0; i < k_size; i++) {
				for(int j = 0; j < k_size - 1; j++) {
					window_buf[i][j] = window_buf[i][j + 1];
				}
			}


			for(int i = 0; i < k_size; i++)
				window_buf[i][k_size - 1] = line_buf[i][x];

			uint8_t pixel = 0;

			conv:
			for(int i = 0; i < k_size; i++) {
				for(int j = 0; j < k_size; j++) {
					pixel += window_buf[i][j] * G_KERN[i][j];
				}
			}
			pixel >>=4;
			dst[x + y*WIDTH] = pixel;
		}
	}
}

template<int width, int height>
void duplicate(uint8_t *in, uint8_t *out1, uint8_t* out2) {
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
#pragma HLS PIPELINE II=1
#pragma HLS LOOP_FLATTEN off
			out1[x + y * width] = in[x + y * width];
			out2[x + y * width] = in[x + y * width];
		}
	}
}

template<int WIDTH, int HEIGHT>
void triple(uint8_t *in, uint8_t *out1, uint8_t* out2,uint8_t* out3) {
	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
#pragma HLS PIPELINE II=1
#pragma HLS LOOP_FLATTEN off
			out1[x + y * WIDTH] = in[x + y * WIDTH];
			out2[x + y * WIDTH] = in[x + y * WIDTH];
			out3[x + y * WIDTH] = in[x + y * WIDTH];
		}
	}
}

template<int WIDTH, int HEIGHT>
void SobelXY(uint8_t* src, Pixel* dst) {
	const int K_SIZE = 3;

	uint8_t line_buf[K_SIZE][WIDTH];
	uint8_t window_buf[K_SIZE][K_SIZE];

#pragma HLS ARRAY_RESHAPE variable=line_buf complete dim=1
#pragma HLS ARRAY_PARTITION variable=window_buf complete dim=0

	const int H_KERN[K_SIZE][K_SIZE] = { { 1, 0, -1 }, { 2, 0, -2 },
			{ 1, 0, -1 } };

	const int V_KERN[K_SIZE][K_SIZE] = { { 1, 2, 1 }, { 0, 0, 0 },
			{ -1, -2, -1 } };

#pragma HLS ARRAY_PARTITION variable=H_KERN complete dim=0
#pragma HLS ARRAY_PARTITION variable=V_KERN complete dim=0

	sobelLoop:
	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
#pragma HLS PIPELINE II=1
#pragma HLS LOOP_FLATTEN off

			uint8_t pix_sobel;
			Direction grad_sobel;

			for (int i = 0; i < K_SIZE - 1; i++)
				line_buf[i][x] = line_buf[i + 1][x];

			line_buf[K_SIZE - 1][x] = src[x + y * WIDTH];

			for (int i = 0; i < K_SIZE; i++) {
				for (int j = 0; j < K_SIZE - 1; j++) {
					window_buf[i][j] = window_buf[i][j + 1];
				}
			}

			for (int i = 0; i < K_SIZE; i++)
				window_buf[i][K_SIZE - 1] = line_buf[i][x];

			int pix_h_sobel = 0;
			int pix_v_sobel = 0;

			for (int i = 0; i < K_SIZE; i++) {
				for (int j = 0; j < K_SIZE; j++) {
					pix_h_sobel += window_buf[i][j] * H_KERN[i][j];
					pix_v_sobel += window_buf[i][j] * V_KERN[i][j];

				}
			}
			//pix_sobel = hls::sqrt(float(pix_h_sobel * pix_h_sobel + pix_v_sobel * pix_v_sobel));
			pix_sobel = hls::abs(pix_h_sobel) + hls::abs(pix_v_sobel);

			int t_int;
			if (pix_h_sobel != 0) {
				t_int = pix_v_sobel * 256 / pix_h_sobel;
			} else {
				t_int = 0x7FFFFFFF;
			}

			if (-618 < t_int && t_int <= -106) {
				grad_sobel = GRAD_135;
			}
			else if (-106 < t_int && t_int <= 106) {
				grad_sobel = GRAD_0;
			}
			else if (106 < t_int && t_int < 618) {
				grad_sobel = GRAD_45;
			}
			else {
				grad_sobel = GRAD_90;
			}

			dst[x + y * WIDTH].color = pix_sobel;
			dst[x + y * WIDTH].dir = grad_sobel;

		}
	}
}

template<int WIDTH, int HEIGHT>
void NonMaxSuppression(Pixel* src, uint8_t* dst) {
	const int WINDOW_SIZE = 3;

	Pixel line_buf[WINDOW_SIZE][WIDTH];
	Pixel window_buf[WINDOW_SIZE][WINDOW_SIZE];

	#pragma HLS ARRAY_RESHAPE variable=line_buf complete dim=1
	#pragma HLS ARRAY_PARTITION variable=window_buf complete dim=0

	NonMaxLoop:
	for(int y = 0; y < HEIGHT; y++) {
		for(int x = 0; x < WIDTH; x++) {
			#pragma HLS PIPELINE II=1
			#pragma HLS LOOP_FLATTEN off

			uint8_t value_nms;
			Direction grad_nms;

			for(int yl = 0; yl < WINDOW_SIZE - 1; yl++)
				line_buf[yl][x] = line_buf[yl + 1][x];

			line_buf[WINDOW_SIZE - 1][x] = src[x + y*WIDTH];


			for(int i = 0; i < WINDOW_SIZE; i++) {
				for(int j = 0; j < WINDOW_SIZE - 1; j++) {
					window_buf[i][j] = window_buf[i][j + 1];
				}
			}

			for(int i = 0; i < WINDOW_SIZE; i++)
				window_buf[i][WINDOW_SIZE - 1] = line_buf[i][x];


			value_nms = window_buf[WINDOW_SIZE / 2][WINDOW_SIZE / 2].color;
			grad_nms = window_buf[WINDOW_SIZE / 2][WINDOW_SIZE / 2].dir;


			if(grad_nms == GRAD_0) {
				if(value_nms < window_buf[WINDOW_SIZE / 2][0].color ||
				   value_nms < window_buf[WINDOW_SIZE / 2][WINDOW_SIZE - 1].color) {
					value_nms = 0;
				}
			}

			else if(grad_nms == GRAD_45) {
				if(value_nms < window_buf[0][0].color ||
				   value_nms < window_buf[WINDOW_SIZE - 1][WINDOW_SIZE - 1].color) {
					value_nms = 0;
				}
			}
			else if(grad_nms == GRAD_90) {
				if(value_nms < window_buf[0][WINDOW_SIZE - 1].color ||
				   value_nms < window_buf[WINDOW_SIZE - 1][WINDOW_SIZE / 2].color) {
					value_nms = 0;
				}
			}

			else if(grad_nms == GRAD_135) {
				if(value_nms < window_buf[WINDOW_SIZE - 1][0].color ||
				   value_nms < window_buf[0][WINDOW_SIZE - 1].color) {
					value_nms = 0;
				}
			}

			dst[x + y * WIDTH] = value_nms;
		}
	}
}

template<int WIDTH, int HEIGHT>
void HystThreshold(uint8_t* src, uint8_t* dst,uint8_t threshold) {

	ThresholdLoop:
	for(int y = 0; y < HEIGHT; y++) {
		for(int x = 0; x < WIDTH; x++) {
			#pragma HLS PIPELINE II=1
			#pragma HLS LOOP_FLATTEN off
			uint8_t pixel = src[x + y * WIDTH] < threshold ? 0 : 255;
			dst[x + y * WIDTH] = pixel;
		}
	}
}

template<int WIDTH, int HEIGHT>
void Canny(uint8_t* src, uint8_t* dst){

	#pragma HLS DATAFLOW
	Pixel fifo1[WIDTH * HEIGHT];
	uint8_t fifo2[WIDTH * HEIGHT];

#pragma HLS STREAM variable=fifo1 depth=1 dim=1
#pragma HLS STREAM variable=fifo2 depth=1 dim=1

	SobelXY<WIDTH, HEIGHT>(src, fifo1);
	NonMaxSuppression<WIDTH, HEIGHT>(fifo1, fifo2);
	HystThreshold<WIDTH, HEIGHT>(fifo2, dst,200);

}

/**
 * to avoid sine/cosine we're checking the average color
 * of the squares. (based daugman circle detection)
 *
 */
template<int WIDTH, int HEIGHT>
uint8_t getSquareColor(uint8_t* src, int x, int y) {
//#pragma HLS inline
	int col = 0;
	sumColor:
	for (int j = -32; j <= 31; j++) {
		for (int i = -32; i <= 31; i++) {
			col += src[(y + j) * WIDTH + x + i];
		}	//8point
	}	//32x32 loops 1024 points
	return col >> 12;
}

/**
 * @see hough_approx
 * the minimal rMin restrictions are to minimize noise.
 */
template<int w, int h>
void searchY(uint8_t* src, int rMin,int rMax, uint16_t *vote) {
	initList<h>(vote, 0);
	line lin;
	lin.begin = 0;

	searchForY:
	for (int x = 0; x < w; x++) {
		for (int y = 0; y < h; y++) {
		#pragma HLS PIPELINE II=1
		#pragma HLS LOOP_FLATTEN off
			if (src[x + y * w] == 255) {
				lin.end = y;
				int radius = (lin.end - lin.begin)>>1;
				if (radius > rMin && radius<rMax)//radius > rMin &&
					vote[lin.begin + radius] += 1;
				lin.begin = y;
			}
		}
	}
}

/**
 * @see "An Efficient Implementation of the One-Dimensional Hough Transform Algorithm for Circle Detection on the FPGA"
 * the minimal rMin restrictions are to minimize noise on
 */
template<int w, int h>
void searchX(uint8_t* src, int rMin,int rMax, uint16_t *vote) {
	initList<w>(vote, 0);
	line lin;
	lin.begin = 0;
	searchForX:
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++){
		#pragma HLS PIPELINE II=1
		#pragma HLS LOOP_FLATTEN off
			if (src[x + y * w] == 255) {
				lin.end = x;
				int radius = (lin.end - lin.begin)>>1;
				if (radius > rMin && radius<rMax)
					vote[lin.begin + radius] += 1;
				lin.begin = x;
			}
		}
		lin.begin = 0;
	}
}

/*
 * Returns the 3 points with the biggest propabilty to be
 * to be center candidate.
 */
template<int length>
void getProbPoints(uint16_t *in, point2D *p) {
	for (int i = 0; i < 3; i++) {
		#pragma HLS unroll
		p[i].Val = 0;
		p[i].X = 0;
	}

	for (int i = 0; i < length; i++) {
		int totalPro = in[i];

		if (totalPro > p[0].Val) {
			p[2].Val = p[1].Val;
			p[1].Val = p[0].Val;
			p[0].Val = totalPro;

			p[2].X = p[1].X;
			p[1].X = p[0].X;
			p[0].X = i;

		} else if (totalPro > p[1].Val) {
			p[2].Val = p[1].Val;
			p[1].Val = totalPro;

			p[2].X = p[1].X;
			p[1].X = i;

		} else if (totalPro > p[2].Val) {
			p[2].Val = totalPro;
			p[2].X = i;
		}
	}
}

/**
 * Streaming possible, but input is randomly accessed BRAM so no need
 * for makeing a FIFO
 */
template<int length>
void getLocalMaxima(uint16_t *in) {
	auto prev_value = 0;
	bool direction = false;	//false -> Ascending, true -> Descending
	for (int i = 0; i < length; i++) {
		auto current_value = in[i];
		if (prev_value < current_value) {
			direction = false;
			in[i - 1] = 0;
		} else if (prev_value > current_value) {
			if (!direction) {
				direction = true;
				in[i - 1] = prev_value;
			} else {
				in[i - 1] = 0;
			}
		}else{
			in[i - 1] = 0;
		}
		prev_value = current_value;
	}
}

/**
 * Parts of the detection is done with an approximated Hough circle transform loosely based on this idea:
 * @see An Efficient Implementation of the One-Dimensional Hough Transform Algorithm for Circle Detection on the FPGA
 * @tparam WIDTH width of the images
 * @tparam HEIGHT height of the images
 * @param Canny pre-filtered canny image
 * @param orginal unfilterd image to verify the detected circel
 */
template<int WIDTH, int HEIGHT>
void getPupil_center(uint8_t *canny, uint8_t *orginal, int &x_c, int &y_c) {

	uint16_t scoreX[WIDTH];
	uint16_t scoreY[HEIGHT];

	point3D z_p[9];
	point2D x_p[3];
	point2D y_p[3];

#pragma HLS data_pack variable=z_p
#pragma HLS data_pack variable=x_p
#pragma HLS data_pack variable=y_p

	searchX<WIDTH, HEIGHT>(canny, 10, 60, scoreX);
	searchY<WIDTH, HEIGHT>(canny, 10, 60, scoreY);

	//search for local maximas
	getLocalMaxima<WIDTH>(scoreX);
	getLocalMaxima<HEIGHT>(scoreY);

	// get the 3 biggest points
	getProbPoints<WIDTH>(scoreX, x_p);
	getProbPoints<HEIGHT>(scoreY, y_p);


	SumUpProb: for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			z_p[i * 3 + j].Val = x_p[j].Val + y_p[i].Val;
			z_p[i * 3 + j].X = x_p[j].X;
			z_p[i * 3 + j].Y = y_p[i].X;
		}
	}

	uint8_t color[9];

	for (int i = 0; i < 9; i++)
		color[i] = getSquareColor<WIDTH, HEIGHT>(orginal, z_p[i].X, z_p[i].Y);

	uint8_t color_max1 = 255;
	uint8_t color_max2 = 255;
	uint8_t color_max3 = 255;
	uint8_t pos1, pos2, pos3;
	for (int i = 0; i < 9; i++) {
		if (color[i] < color_max1) {
			color_max3 = color_max2;
			color_max2 = color_max1;
			color_max1 = color[i];
			pos1 = i;
		} else if (color[i] < color_max2) {
			color_max3 = color_max2;
			color_max2 = color[i];
			pos2 = i;
		} else if (color[i] < color_max3) {
			color_max3 = color[i];
			pos3 = i;
		}

	}

	x_c = z_p[pos1].X - 5;
	y_c = z_p[pos1].Y - 5;
}

inline bool validSum(uint8_t sum){
	return sum<50? false:true;
}

template<int bufferwidth>
int linear_CircleSum_Iris(uint8_t *buffer,int x, int r){
	int addSum  = 0;
	int rigthSum= 0;
	int leftSum = 0;

	for (int i = 0; i<5;i++){
		rigthSum +=  buffer[0 * bufferwidth + x + r + i];
		rigthSum +=  buffer[1 * bufferwidth + x + r + i];
		rigthSum +=  buffer[2 * bufferwidth + x + r + i];
		rigthSum +=  buffer[3 * bufferwidth + x + r + i];
		rigthSum +=  buffer[4 * bufferwidth + x + r + i];
	}
	for (int i = 0; i<5;i++){
		leftSum +=  buffer[0 * bufferwidth + x - r - i];
		leftSum +=  buffer[1 * bufferwidth + x - r - i];
		leftSum +=  buffer[2 * bufferwidth + x - r - i];
		leftSum +=  buffer[3 * bufferwidth + x - r - i];
		leftSum +=  buffer[4 * bufferwidth + x - r - i];
	}
    rigthSum /= 25;
    leftSum  /= 25;
    if (validSum(rigthSum)){
    	if (validSum(leftSum)){
    		addSum = rigthSum + leftSum;
    	}else{
    		addSum += rigthSum * 2;
    	}
    }else{
		if (validSum(leftSum)) {
			addSum = leftSum * 2;
		} else {
			addSum = 255;;
		}
    }
    return addSum >> 1;

}

template<uint8_t start_search, uint8_t end_search, int width, int height>
int getBorder_Iris(uint8_t* image_in, int x, int y) {
	uint8_t line_buffer[5 * width];

	assignOutputLoop:
	for (int yi = 0; yi < height; yi++) {
		for (int xi = 0; xi < width; xi++) {
			#pragma HLS PIPELINE II=1
			#pragma HLS LOOP_FLATTEN off
			uint8_t val = image_in[width * yi + xi];
			if (yi >= y - 2 && yi <= y + 2) {
				line_buffer[xi] = val;
			}
		}
	}
	int hist_inner = linear_CircleSum_Iris<width>(line_buffer,x,start_search)/64;
	int hist_outer = 0;
	int count = 0;
	IrisSegmentaionLoop:
	for (int r = start_search; r < end_search; r+=5) {
		hist_outer = linear_CircleSum_Iris<width>(line_buffer,x,r)/64;
		if (hist_outer > hist_inner){
			count ++;
			if (count == 2 )
				return r - 5*2;
		}else if(hist_outer<hist_inner){
			hist_inner  = hist_outer;
			count = 0;
		}else{
			count = 0;
		}
	}
	return end_search;
}

inline bool validSum_pupil(uint8_t sum){
	return sum>50? false:true;
}

template<int bufferwidth>
int linear_CircleSum_pupil(uint8_t *buffer,int x, int r){
	int addSum  = 0;
	int rigthSum= 0;
	int leftSum = 0;
	int tmp = 0;
	int cnt = 0;

	for (int i = 0; i<5;i++){
		tmp = x + r + i;
		cnt++;
		rigthSum +=  buffer[0 * bufferwidth + tmp];
		rigthSum +=  buffer[1 * bufferwidth + tmp];
		rigthSum +=  buffer[2 * bufferwidth + tmp];
		rigthSum +=  buffer[3 * bufferwidth + tmp];
		rigthSum +=  buffer[4 * bufferwidth + tmp];
	}
	for (int i = 0; i<5;i++){
		tmp = x - r - i;
		leftSum +=  buffer[0 * bufferwidth + tmp];
		leftSum +=  buffer[1 * bufferwidth + tmp];
		leftSum +=  buffer[2 * bufferwidth + tmp];
		leftSum +=  buffer[3 * bufferwidth + tmp];
		leftSum +=  buffer[4 * bufferwidth + tmp];
	}
    rigthSum /= 25;
    leftSum  /= 25;
    if (validSum_pupil(rigthSum)){
    	if (validSum_pupil(leftSum)){
    		addSum = rigthSum + leftSum;
    	}else{
    		addSum = rigthSum * 2;
    	}
    }else{
		if (validSum_pupil(leftSum)) {
			addSum = leftSum * 2;
		} else {
			addSum = 255;;
		}
    }
    return addSum >> 1;

}
/**
 * Streaming possible
 **/
template<uint8_t start_search, uint8_t end_search, int width, int height>
int getBorder_Pupil(uint8_t* image_in, uint8_t* image_out, int x, int y) {
	uint8_t line_buffer[5 * width];
	int tmp=0;
	assignOutputLoop:
	for (int yi = 0; yi < height; yi++) {
		for (int xi = 0; xi < width; xi++) {
			#pragma HLS PIPELINE II=1
			#pragma HLS LOOP_FLATTEN off
			uint8_t val = image_in[width * yi + xi];
			image_out[width * yi + xi] = val;
			if (yi >= y - 2 && yi <= y + 2) {
				line_buffer[tmp * width +xi] = val;

			}
		}
		if (yi >= y - 2 && yi <= y + 2) {
			tmp++;
		}
	}
	int hist_inner = linear_CircleSum_pupil<width>(line_buffer,x,start_search)/64;
	int hist_outer = 0;
	int count = 0;
	IrisSegmentaionLoop:
	for (int r = start_search; r < end_search; r+=5) {
		int tmp = linear_CircleSum_pupil<width>(line_buffer,x,r);
		hist_outer = tmp/64;
		if (hist_outer > hist_inner){
			count ++;
			if (count == 2 )
				return r - 5*2;
		}else if(hist_outer<hist_inner){
			hist_inner  = hist_outer;
			count = 0;
		}else{
			count = 0;
		}
	}
	//if we don't find a valid border return the maximal possible border
	return end_search;
}

template<uint8_t start_search, uint8_t end_search, int width, int height>
void findIrisBorder(uint8_t* image_in, int x, int y,int &r){
	r = getBorder_Iris<start_search,end_search,width,height>(image_in,x,y);
}

template<uint8_t start_search, uint8_t end_search, int width, int height>
void findPupilBorder(uint8_t* image_in, uint8_t* image_out, int x, int y,int &r){
	r = getBorder_Pupil<start_search,end_search,width,height>(image_in,image_out,x,y);
}

/**
 * Search for circles in image and return the coordinates and radius
 * loosley based on this paper: "An Efficient Implementation of the One-Dimensional Hough
 * Transform Algorithm for Circle Detection on the FPGA"
 * @tparam WIDTH width of the image
 * @tparam HEIGHT height of the image
 * @tparam rmin minimal radius to search
 * @tparam rmax maximal radius to search
 * @param canny input image prefiltered with e.g. Canny (255 have to be edges)
 * @param x_c return coordinate X
 * @param y_c return coordinate Y
 * @param r_c return radius
 **/
template<int WIDTH, int HEIGHT, int rmin,int rmax>
void hough_approx(uint8_t* canny, int &x_c, int &y_c, int &r_c) {

	uint16_t scoreX[WIDTH];
	uint16_t scoreY[HEIGHT];

	point2D x_p[3];
	point2D y_p[3];

	uint16_t scoreRadii[9][rmax-rmin];
	coord coords[9];

#pragma HLS data_pack variable=x_p
#pragma HLS data_pack variable=y_p

	for(int i = 0; i<9;i++){
		for(int j = 0;j<rmax-rmin;j++){
#pragma HLS PIPELINE II=1
			scoreRadii[i][j]=0;
		}
	}

	searchX<WIDTH, HEIGHT>(canny, 10, 60, scoreX);
	searchY<WIDTH, HEIGHT>(canny, 10, 60, scoreY);

	//search for local maximas
	getLocalMaxima<WIDTH>(scoreX);
	getLocalMaxima<HEIGHT>(scoreY);

	// get the 3 biggest points
	getProbPoints<WIDTH>(scoreX, x_p);
	getProbPoints<HEIGHT>(scoreY, y_p);


	/*
	 * the 3 X values and 3 Y values result in 9 coordinates
	 * where coords[0] got the highest score. coords[8] the lowest
	 *
	 */
	coords[0] = coord(x_p[0].X,y_p[0].X);
	coords[1] = coord(x_p[0].X,y_p[1].X);
	coords[2] = coord(x_p[0].X,y_p[2].X);
	coords[3] = coord(x_p[1].X,y_p[0].X);
	coords[4] = coord(x_p[1].X,y_p[1].X);
	coords[5] = coord(x_p[1].X,y_p[2].X);
	coords[6] = coord(x_p[2].X,y_p[0].X);
	coords[7] = coord(x_p[2].X,y_p[1].X);
	coords[8] = coord(x_p[2].X,y_p[2].X);


	/*
	 * We go through the image (streaming possible)
	 * The input image is a binary image 0 or 255.
	 * if I(x,y) (the current pixel) is an edge the
	 * euclidean distance(for accurate results),
	 * or the Manhattan distance is used to calculate
	 * the radius of I(x,y) and all 9 points of the center
	 * candidates. If the radius is in the template predefined
	 * range of minimal and maximal radius, we increment the 2nd
	 * index element of the corresponding center candidate.
	 * The index element 0 correspond to the rmin radius.
	 * the last index to rmax.
	 */
	for (uint16_t y = 0; y < HEIGHT; y++) {
		for (uint16_t x = 0; x < WIDTH; x++) {
			#pragma HLS PIPELINE II=2
			#pragma HLS LOOP_FLATTEN off
			uint8_t pixel = canny[x+y*WIDTH];
			if (pixel==255){
				for (int i = 0; i < 9; i++) {
					int tx = hls::abs(x - coords[i].x);
					int ty = hls::abs(y - coords[i].y);
					//int r = hls::sqrt(tx*tx+ty*ty);
					int r = tx+ty;
					if (r>=rmin && r<=rmax){
						scoreRadii[i][r-rmin]+=1;
					}
				}
			}
		}
	}


	/*
	 * get the maxima of the first candidate
	 * if we want to detect multiple circles
	 * we have to enlarge this loop.
	 */
	uint16_t max = 0;
	int r = 0;
	for(int i=0;i<rmax-rmin;i++){
		if (max < scoreRadii[0][i]){
			max = scoreRadii[0][i];
			r = i;
		}
	}

	/*
	 * At the moment we only return the first point
	 * with the highest probability. Because r represent
	 * the index of the array shifted of the value of rmin
	 * we have to add the offset to get the result.
	 * ( variable r = 0 => radius = 0 + rmin)
	 */
	r_c = r + rmin;
	x_c = coords[0].x;
	y_c = coords[0].y;
}

template<int WIDTH,int HEIGHT>
void preprocessing(AXI_STREAM& inputStream, uint8_t *canny_out,uint8_t *orginal_out ){

#pragma HLS DATAFLOW
	RGB_IMAGE img0(HEIGHT, WIDTH);
	RGB_NORM_IMAGE img1(HEIGHT,WIDTH);

	uint8_t FIFO_Gray1[WIDTH * HEIGHT];
	uint8_t FIFO_Gray2[WIDTH * HEIGHT];
	uint8_t FIFO_WhiteRemoved[WIDTH*HEIGHT];
	uint8_t FIFO_Morph[WIDTH*HEIGHT];
	uint8_t FIFO_Canny[WIDTH*HEIGHT];

#pragma HLS STREAM variable=FIFO_Gray1 depth=1 dim=1
#pragma HLS STREAM variable=FIFO_Gray2 depth=1 dim=1
#pragma HLS STREAM variable=FIFO_WhiteRemoved depth=1 dim=1
#pragma HLS STREAM variable=FIFO_Morph depth=1 dim=1


	hls::AXIvideo2Mat(inputStream, img0);
	MatToGrayArray<WIDTH, HEIGHT>(img0, FIFO_Gray1);
	duplicate<WIDTH, HEIGHT>(FIFO_Gray1, FIFO_Gray2, orginal_out);
	removeWhite<WIDTH, HEIGHT>(FIFO_Gray2, FIFO_WhiteRemoved);
	morphOpening<WIDTH, HEIGHT>(FIFO_WhiteRemoved, FIFO_Morph);
	Canny<WIDTH, HEIGHT>(FIFO_Morph, canny_out);
}

} // namespace segmentaion

#endif

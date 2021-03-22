#pragma once
#ifndef _ENCODE_
#define _ENCOE_
#include <stdint.h>
#include "hls_math.h"
#include "ap_fixed.h"
#include "hls_video.h"

#define MAX_WIDTH_CASIA  320
#define MAX_HEIGHT_CASIA  280

#define MAX_WIDTH_IITD 320
#define MAX_HEIGHT_IITD 240

#define MAX_WIDTH MAX_WIDTH_IITD
#define MAX_HEIGHT MAX_HEIGHT_IITD

#define NORM_WIDTH_IITD 432
#define NORM_HEIGHT_IITD 48

#define NORM_WIDTH_CUST 180
#define NORM_HEIGHT_CUST 32//16

#define NORM_HEIGHT NORM_HEIGHT_IITD
#define NORM_WIDTH  NORM_WIDTH_IITD

#define ENCODED_PIXELS  1024
#define BITCODE_LENGTH  2*ENCODED_PIXELS

#define MAX_BITWIDTH 16

#define MAX_KERN NORM_HEIGHT/3

#define k_size 9
#define maxRho  (NORM_HEIGHT  - (int)(k_size/2) * 2)
#define startOffset  k_size/2

namespace types {
	typedef hls::stream<ap_axiu<32, 1, 1, 1> > AXI_STREAM; //32 bit data stream
	typedef hls::Mat<MAX_HEIGHT, MAX_WIDTH, HLS_8UC3> RGB_IMAGE; //RGB image from type HLS::Mat
	typedef hls::Mat<MAX_HEIGHT, MAX_WIDTH, HLS_8UC1> GRAY_IMAGE; //Gray image from type HLS::Mat
	typedef hls::Mat<NORM_HEIGHT, NORM_WIDTH, HLS_8UC3> RGB_NORM_IMAGE; //RGB image from type HLS::Mat
	typedef hls::Mat<NORM_HEIGHT, NORM_WIDTH, HLS_8UC1> GRAY_NORM_IMAGE; //Gray image from type HLS::Mat
	typedef hls::Scalar<1, uint8_t> PIXELGRAY;
	typedef hls::Scalar<3, uint8_t> RGBPIXEL;
	static const float DegToRad = 0.01745329;
}

using namespace types;

namespace cordic{


typedef ap_fixed<MAX_BITWIDTH,5> floatGabor;
typedef ap_ufixed<MAX_BITWIDTH,0> floatTan;
typedef ap_fixed<MAX_BITWIDTH,2> floatSin;
//max. 3 bit for integer part plus sign Range: -2PI
typedef ap_fixed<MAX_BITWIDTH,4> floatArg;
static const float PI_float = 3.1415926535;

static const ap_ufixed<MAX_BITWIDTH,2> PI 		  = 3.1415926535;
static const ap_ufixed<MAX_BITWIDTH,0> TWO_DIV_PI = 0.6366197723;
static const ap_ufixed<MAX_BITWIDTH,1> PI_HALF    = 1.5707963267;

static const floatTan arctan[] = {
		0.7853981633974483,//0.1100100100..
		0.4636476090008061,//0.0111011010..
		0.24497866312686414,//0.0011111010..
		0.12435499454676144,//0.000111111101010
		0.06241880999595735,// bits
		0.031239833430268277,
		0.015623728620476831,
		0.007812341060101111,
		0.0039062301319669718,
		0.0019531225164788188,//10 it
		0.0009765621895593195,
		0.0004882812111948983,
		0.00024414062014936177,
		0.00012207031189367021,
		6.103515617420877e-05,
		3.0517578115526096e-05,
		1.5258789061315762e-05,
		7.62939453110197e-06,
		3.814697265606496e-06,
		1.907348632810187e-06,//20 it
		9.536743164059608e-07
		};

void cordic_intern(floatSin &x, floatSin &y, floatArg phi){

	//accuracy is aprox. 1 iteration per bit
	int nMax = MAX_BITWIDTH-2;//20;

	floatSin z_r,z_i;
	floatSin z_r_tmp,z_i_tmp;

	cordicLoop:
	for (int n =0;n<nMax;n++){
		#pragma HLS pipeline
		floatTan a = arctan[n];
		z_r_tmp = x >> n;
		z_i_tmp = y >> n;
		if (phi.is_neg()){
			phi += a;
			z_r = x + z_i_tmp;
			z_i = y - z_r_tmp;

		}else{
			phi -= a;
			z_r = x - z_i_tmp;
			z_i = y + z_r_tmp;
		}
		x = z_r;
		y = z_i;
	}
}

void cordic_fix(floatArg phi,floatSin &s,floatSin &c){
	floatArg absPhi;
	if (phi.is_neg())
		absPhi = -phi;
	else
		absPhi = phi;



	ap_ufixed<MAX_BITWIDTH,2> multible = absPhi*TWO_DIV_PI;//function defined in range +/- pi/2 so how many multible is in the input K*pi/2 = x <=> k = 2/pi * x
	ap_uint<2> intK = multible;// take the integer part for k so we know which quadrant is used
	ap_ufixed<MAX_BITWIDTH,1> dif = absPhi - intK * PI_HALF;//upper limit quadrant - diff sin(x) = sin(pi/2 -dif)

	floatArg alpha;
	if (intK==1 || intK==3){
		alpha = PI_HALF - dif;
	}else{
		alpha = dif;
	}

	floatSin z_r = 0.607252935;
	floatSin z_i = 0;
	cordic_intern(z_r,z_i,alpha);


	if (intK==0){
		c = z_r;
		s = z_i;
	}else if(intK==1){
		c = -z_r;
		s = z_i;
	}else if(intK==2){
		c = -z_r;
		s = -z_i;
	}else{
		c =  z_r;
		s = -z_i;
	}

	if (phi.is_neg())
		s = -s;
}

}

using namespace cordic;

namespace encodeing{

	struct database{
		ap_uint<1> data[BITCODE_LENGTH];
		int id;
	};

	/*
	 * Cyclic modulo function so that convolution kernel
	 * select the right pixel/angle after 360 degree
	 * again 0 degree
	 * @param a dividend
	 * @param b divisor
	 */
	int MODULO(int a, int b) {
	#pragma HLS inline
		int res = a % b;
		return res < 0 ? res + b : res;
	}

	/*
	 * Based on Ridge Energy Detection
	 * <a href="https://ieeexplore.ieee.org/document/7576523">Paper</a>
	 * Convolution with big Filter horizontal and big vertical Filter.
	 * Then comparison which direction has the bigger amplitude.
	 */
	void encode_RED(uint8_t norm_img[NORM_HEIGHT * NORM_WIDTH],ap_uint<1> bit_code[maxRho * NORM_WIDTH]) {

		int8_t kernelH[k_size][k_size]= {
				{-1,-1,-1,-1,-1,-1,-1,-1,-1},
				{-1,-1,-1,-1,-1,-1,-1,-1,-1},
				{-1,-1,-1,-1,-1,-1,-1,-1,-1},
				{+2,+2,+2,+2,+2,+2,+2,+2,+2},
				{+2,+2,+2,+2,+2,+2,+2,+2,+2},
				{+2,+2,+2,+2,+2,+2,+2,+2,+2},
				{-1,-1,-1,-1,-1,-1,-1,-1,-1},
				{-1,-1,-1,-1,-1,-1,-1,-1,-1},
				{-1,-1,-1,-1,-1,-1,-1,-1,-1}
		};
		int8_t kernelV[k_size][k_size]= {
				{-1,-1,-1,+2,+2,+2,-1,-1,-1},
				{-1,-1,-1,+2,+2,+2,-1,-1,-1},
				{-1,-1,-1,+2,+2,+2,-1,-1,-1},
				{-1,-1,-1,+2,+2,+2,-1,-1,-1},
				{-1,-1,-1,+2,+2,+2,-1,-1,-1},
				{-1,-1,-1,+2,+2,+2,-1,-1,-1},
				{-1,-1,-1,+2,+2,+2,-1,-1,-1},
				{-1,-1,-1,+2,+2,+2,-1,-1,-1},
				{-1,-1,-1,+2,+2,+2,-1,-1,-1}
		};

	#pragma HLS RESOURCE variable=kernelH core=ROM_1P
	#pragma HLS RESOURCE variable=kernelV core=ROM_1P

		int informationCount = 0;

		for (int slice = 0 ;slice <= maxRho;slice++){
			for (int theta = 0 ; theta < NORM_WIDTH;theta++){
				int current_row = startOffset + slice;
				int current_col = startOffset + theta;
				int p_v = 0;
				int p_h = 0;
				for(int i=0;i<k_size;i++){
					for(int j=0;j<k_size;j++){
						p_v += norm_img[(current_row + i) * NORM_WIDTH + current_col + j] * kernelV[i][j];
						p_h += norm_img[(current_row + i) * NORM_WIDTH + current_col + j] * kernelH[i][j];
					}
				}
				if (p_v >= p_h)
					bit_code[informationCount] = 0;
				else
					bit_code[informationCount] = 1;
				informationCount +=1;
			}
		}
	}

	void GaussKernel(int size, int peak, float gauss[MAX_KERN][MAX_KERN]) {
		float alpha = (size - 1) * 0.477;
		float alphaPower = alpha * alpha;
		for (int i = 0; i < size; i++) {
			float y = i - ((float) size / 2.0);
			float yy = y * y;
			for (int j = 0; j < size; j++) {
				float phi = j - ((float) size / 2.0);
				float temp1 = -(yy + phi * phi) / alphaPower;
				float temp = peak * hls::expf(temp1);
				gauss[i][j] = temp;
			}
		}
	}

	void SinKernel(int size, float sinK[MAX_KERN], float cosK[MAX_KERN]) {
		int temp = size >>1;
		CalcRow:
		for (int i = 0; i < size; i++) {
			int phi = i - temp;
			float val_sin = sin(PI_float * phi / temp);
			float val_cos = cos(PI_float * phi / temp);
			sinK[i] = val_sin;
			cosK[i] = val_cos;
		}
	}

	void GaborKernel(int size, float sin_m[MAX_KERN][MAX_KERN],
			float cos_m[MAX_KERN][MAX_KERN]) {

		float gauss[MAX_KERN][MAX_KERN];
		float sinK[MAX_KERN];
		float cosK[MAX_KERN];
		SinKernel(size, sinK, cosK);
		GaussKernel(size, 15, gauss);

		GaborMulti:
		for (int i = 0; i < size; i++) {
			float rho = i - (size / 2);
			float row_sum_sin = 0;
			float row_sum_cos = 0;
			for (int j = 0; j < size; j++) {
				sin_m[i][j] = sinK[j] * gauss[i][j];
				cos_m[i][j] = cosK[j] * gauss[i][j];
				row_sum_sin += sin_m[i][j];
				row_sum_cos += cos_m[i][j];
			}
			for (int j = 0; j < size; j++) {
				float old_sin = sin_m[i][j];
				float old_cos = cos_m[i][j];
				sin_m[i][j] = old_sin - (row_sum_sin / (float) size);
				cos_m[i][j] = old_cos - (row_sum_cos / (float) size);
			}
		}
	}

	void gaborPixel(int rho, int phi, uint8_t norm_img[NORM_HEIGHT * NORM_WIDTH],
		int fsize, float sinM[MAX_KERN][MAX_KERN],
		float cosM[MAX_KERN][MAX_KERN], ap_uint<1> code[BITCODE_LENGTH], int pos) {
		int angles = NORM_WIDTH;
		float total_i = 0.0;
		float total_r = 0.0;

		GaborPixeLoop: for (int i = 0; i < fsize; i++) {
			for (int j = 0; j < fsize; j++) {
				int image_y = j + phi - (fsize / 2);
				image_y = MODULO(image_y, angles);
				int image_x = i + rho - (fsize / 2);
				int tmp = norm_img[image_x * NORM_WIDTH + image_y];
				total_i += sinM[i][j] * tmp;
				total_r += cosM[i][j] * tmp;
			}
		}
		code[pos] = total_r >= 0 ? 1 : 0;
		code[pos + 1] = total_i >= 0 ? 1 : 0;
	}

	/*
	 * Gabor encodeing float. Convolve with a Gabor kernel. Check if the imaginary or real part
	 * is bigger or smaller then zero. Based on this decision the bitcode at the position
	 * will assigned a zero or a one (for booth value RE and IM)
	 * @param norm_img Normalised image
	 * @param code output code
	 */
	void encode(uint8_t norm_img[NORM_HEIGHT * NORM_WIDTH],ap_uint<1> code[BITCODE_LENGTH]){
		float sin_filter_matrix[MAX_KERN][MAX_KERN];
		float cos_filter_matrix[MAX_KERN][MAX_KERN];
		int radial_slice = ENCODED_PIXELS / NORM_WIDTH;
		int index = 0;

		for (int r_slice = 0; r_slice < radial_slice; r_slice++) {
			int radius = ((r_slice * (NORM_HEIGHT - 6)) / (2 * radial_slice)) + 3;
			int filter_height;

			if (radius < (NORM_HEIGHT - radius))
				filter_height = 2 * radius - 1;
			else
				filter_height = 2 * (NORM_HEIGHT - radius) - 1;

			if (filter_height > MAX_KERN)
				filter_height = MAX_KERN;

			GaborKernel(filter_height, sin_filter_matrix,
					cos_filter_matrix);

			for (int theta = 0; theta < NORM_WIDTH; theta++) {
				gaborPixel(radius, theta, norm_img, filter_height,
						sin_filter_matrix, cos_filter_matrix, code, index);
				index += 2;
			}
		}
	}

	/**
	 * max(size) = 16 -> 4 bit unsigned / 5 bit sigend
	 *
	 *
	 */
	void GaussKernel_fix(int size,int peak,floatGabor gauss[MAX_KERN][MAX_KERN]) {
		floatGabor alpha = (size - 1) * 0.477;
		ap_ufixed<16,7> alphaPower = alpha * alpha;
		ap_ufixed<8,4> temp = ((ap_ufixed<8,4>) size) >> 1;

		CreateGauss:
		for (int i = 0; i < size; i++) {
			ap_fixed<8, 4> y = i - temp;
			ap_ufixed<16, 10> YPower = y * y;
			for (int j = 0; j < size; j++) {
				ap_fixed<8, 4> x = j - temp;
				ap_ufixed<16, 10> XPower = x * x;
				ap_fixed<16, 6> exponent = -(YPower + XPower) / alphaPower;
				floatGabor value  = peak * hls::exp(exponent);
				gauss[i][j] = value;
			}
		}
	}

	void SinKernel_fix(int size,floatSin sink[MAX_KERN],floatSin cosk[MAX_KERN]) {
		int temp = size / 2;
		CalcFirstRow:
		for (int i = 0; i < size; i++) {
			int phi = i - temp;
			floatArg angle = PI * phi / temp;
			floatSin val_sin;
			floatSin val_cos;
			cordic_fix(angle, val_sin, val_cos);

			float tmpSIN = val_sin.to_float();
			float tmpCOS = val_cos.to_float();

			sink[i] = val_sin;
			cosk[i] = val_cos;
		}

	}

	void GaborKernel_fix(int size,floatGabor sinM[MAX_KERN][MAX_KERN],floatGabor cosM[MAX_KERN][MAX_KERN]) {

		floatSin sin_k[MAX_KERN];
		floatSin cos_k[MAX_KERN];
		floatGabor gauss[MAX_KERN][MAX_KERN];

		SinKernel_fix(size, sin_k, cos_k);
		GaussKernel_fix(size,15,gauss);

		GaussMultiSine:
		for (int i = 0; i < size; i++) {
			ap_fixed<16,5> row_sum_sin = 0;
			ap_fixed<16,5> row_sum_cos = 0;

			for (int j = 0; j < size; j++) {
				sinM[i][j] = sin_k[j] * gauss[i][j];
				cosM[i][j] = cos_k[j] * gauss[i][j];

				row_sum_sin += sin_k[i][j];
				row_sum_cos += cos_k[i][j];
			}

			floatGabor stmp = (row_sum_sin / (floatGabor) size);
			floatGabor ctmp = (row_sum_cos / (floatGabor) size);
			for (int j = 0; j < size; j++) {
				sinM[i][j] = sinM[i][j] - stmp;
				cosM[i][j] = cosM[i][j] - ctmp;
			}

		}
	}

	void gaborPixel_fix(uint16_t rho, uint16_t phi,
			uint8_t norm_img[NORM_HEIGHT * NORM_WIDTH],
			uint8_t size,
			floatGabor sinM[MAX_KERN][MAX_KERN],
			floatGabor cosM[MAX_KERN][MAX_KERN],
			ap_uint<1> code[BITCODE_LENGTH],
			int pos)
	{

		ap_fixed<16, 14> total_i = 0;
		ap_fixed<16, 14> total_r = 0;

		int tmp = size / 2;

		GaborPixeLoop:
		for (int i = 0; i < size; i++) {
			uint16_t y = i + rho - tmp;
			for (int j = 0; j < size; j++) {
				int x = j + phi - tmp;//negative intermediate values possible
				x = MODULO(x, NORM_WIDTH);// left or rigth out of range start on the other site again
				int index = y * NORM_WIDTH + x;

				ap_uint<8> tmp = (ap_uint<8>)norm_img[index];

				ap_fixed<16, 14> valueS = (sinM[i][j] * tmp);//if compiler say file to big
				ap_fixed<16, 14> valueC = (cosM[i][j] * tmp);//-> using Windows? ->change to linux

				total_i = total_i + valueS;
				total_r = total_r + valueC;
			}
		}
		code[pos] = total_r >= 0 ? 1 : 0;
		code[pos + 1] = total_i >= 0 ? 1 : 0;
	}

	/*
	 * Gabor encodeing Fixpoint. Convolve with a Gabor kernel. Check if the imaginary or real part
	 * is bigger or smaller then zero. Based on this decision the bitcode at the position
	 * will assigned a zero or a one (for booth value RE and IM)
	 * @param norm_img Normalised image
	 * @param code output code
	 */
	void encode_fix(uint8_t norm_img[NORM_HEIGHT * NORM_WIDTH],
			ap_uint<1> bit_code[BITCODE_LENGTH]) {

		floatGabor sin_filter_matrix[MAX_KERN][MAX_KERN];
		floatGabor cos_filter_matrix[MAX_KERN][MAX_KERN];

		int angular_slice = NORM_WIDTH;
		int radial_slice = ENCODED_PIXELS / angular_slice;
		int index = 0;
		for (int r_slice = 0; r_slice < radial_slice; r_slice++) {
			int radius = ((r_slice * (NORM_HEIGHT - 6)) / (2 * radial_slice)) + 3;
			int filter_height;

			filter_height = radius < (NORM_HEIGHT - radius)? 2 * radius - 1:filter_height = 2 * (NORM_HEIGHT - radius) - 1;

			if (filter_height > MAX_KERN) filter_height = MAX_KERN;

			GaborKernel_fix(filter_height, sin_filter_matrix,
					cos_filter_matrix);

			CountConvLoop:
			for (int theta = 0; theta < angular_slice; theta++) {
				gaborPixel_fix(radius, theta, norm_img, filter_height,
						sin_filter_matrix, cos_filter_matrix, bit_code, index);
				index += 2;
			}
		}
	}

	/*
	 * next 2 functions are only for tb to compare the accuracy of fix point operation vs. floating point
	 */

	void gaborPixel_compare(uint16_t rho, uint16_t phi,
			uint8_t norm_img[NORM_HEIGHT * NORM_WIDTH], uint8_t size,
			floatGabor sinM[MAX_KERN][MAX_KERN],
			floatGabor cosM[MAX_KERN][MAX_KERN],
			float sinM_float[MAX_KERN][MAX_KERN],
			float cosM_float[MAX_KERN][MAX_KERN]) {

		ap_fixed<16, 14> total_i = 0; //ap_fixed<16, 14>
		ap_fixed<16, 14> total_r = 0;
		float totalI = 0;
		float totalR = 0;

		int tmp = size / 2;

		GaborPixeLoop: for (int i = 0; i < size; i++) {
			uint16_t y = i + rho - tmp;
			for (int j = 0; j < size; j++) {
				int x = j + phi - tmp; //negative intermediate values possible
				x = MODULO(x, NORM_WIDTH); // left or rigth out of range start on the other site again
				int index = y * NORM_WIDTH + x;

				ap_uint<8> tmp = (ap_uint<8> ) norm_img[index];
				int factor_image = norm_img[index];
				float factor_sin_fix = sinM[i][j].to_float();
				float factor_sin_flo = sinM_float[i][j];

				ap_fixed<16, 14> valueS = (sinM[i][j] * tmp); //ap_fixed<16, 14>
				ap_fixed<16, 14> valueC = (cosM[i][j] * tmp); //
				float valueS_float = sinM_float[i][j] * (float) norm_img[index];
				float valueC_float = cosM_float[i][j] * (float) norm_img[index];
				valueS_float = floorf(valueS_float * 100) / 100;
				valueC_float = floorf(valueC_float * 100) / 100;

				total_i = total_i + valueS;
				total_r = total_r + valueC;

				totalI += valueS_float;
				totalR += valueC_float;
			}
		}
		/*
		if ((total_i <= 0 && totalI <= 0) || (total_i > 0 && totalI > 0)) {
			std::cout << "same sign\n";
		} else {
			std::cout << std::setprecision(4) << "different sign " << totalI
					<< " (float) vs " << total_i.to_float() << "(fix)\n";
		}
		*/
		}


	void encode_compare(uint8_t norm_img[NORM_HEIGHT * NORM_WIDTH]) {

		floatGabor sinF[MAX_KERN][MAX_KERN];
		floatGabor cosF[MAX_KERN][MAX_KERN];

		float sin_filter[MAX_KERN][MAX_KERN];
		float cos_filter[MAX_KERN][MAX_KERN];

		int angular_slice = NORM_WIDTH;
		int radial_slice = ENCODED_PIXELS / angular_slice;
		int index = 0;
		for (int r_slice = 0; r_slice < radial_slice; r_slice++) {
			int radius = ((r_slice * (NORM_HEIGHT - 6)) / (2 * radial_slice)) + 3;
			int filter_height;

			filter_height = radius < (NORM_HEIGHT - radius)? 2 * radius - 1:filter_height = 2 * (NORM_HEIGHT - radius) - 1;

			if (filter_height > MAX_KERN) filter_height = MAX_KERN;

			GaborKernel_fix(filter_height, sinF,cosF);
			GaborKernel(filter_height, sin_filter,cos_filter);

			for (int theta = 0; theta < angular_slice; theta++) {
				gaborPixel_compare(radius, theta, norm_img, filter_height,
						sinF, cosF,sin_filter,cos_filter);
				index += 2;
			}
		}
	}

}

#endif

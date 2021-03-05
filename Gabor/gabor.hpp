#ifndef GABOR_H
#define GABOR_H
#include "types.hpp"
#include "cordic.hpp"
#include <math.h>

using namespace types;
using namespace cordic;


ap_uint<2> data[50][BITCODE_LENGTH];
struct database{
	ap_uint<1> data[BITCODE_LENGTH];
	int id;
};

const float PI = 3.14159;

int MODULO(int a, int b) {
#pragma HLS inline
	//negative mod is not defined very well
	int res = a % b;
	return res < 0 ? res + b : res;
}

//------------------------------------------------------------------------------------
//					Gabor Kernel Float
//-----------------------------------------------------------------------------------

void generateGaussKernel(int k_size, int peak,
		float gauss[MAX_KERN_SIZE][MAX_KERN_SIZE]) {
	float alpha = (k_size - 1) * 0.4770322291;
	float alphaPower = alpha * alpha;
	CreateGauss: for (int i = 0; i < k_size; i++) {
		float rho = i - ((float) k_size / 2.0);
		float rhoPower2 = rho * rho;
		for (int j = 0; j < k_size; j++) {
			float phi = j - ((float) k_size / 2.0);
			float temp1 = -(rhoPower2 + phi * phi) / alphaPower;
			float temp = peak * hls::expf(temp1);
			gauss[i][j] = temp;
		}
	}
}

void generateSinKernel(int kern_size,
		float sinK[MAX_KERN_SIZE],
		float cosK[MAX_KERN_SIZE]) {
	float sum_row_sin = 0;
	float sum_row_cos = 0;
	CalcFirstRow:
	for (int i = 0; i < kern_size; i++) {
		int phi = i - (kern_size / 2);
		int temp = kern_size / 2;
		float val_sin = sin(PI * phi / temp);
		float val_cos = cos(PI * phi / temp);

		sinK[i] = val_sin;
		cosK[i] = val_cos;
		sum_row_sin += val_sin;
		sum_row_cos += val_cos;
	}

	NormalizeRow:
	for (int i = 0; i < kern_size; i++) {
		float old_v_s = sinK[i];
		float old_v_c = cosK[i];
		sinK[i] = old_v_s - (sum_row_sin / (float) kern_size);
		cosK[i] = old_v_c - (sum_row_cos / (float) kern_size);
	}
}

/**
 * A Gabor kernel is the result of a gauss kerenl multiply with a sin kernel
 * Because we need different sizes of such a kernel in the calculation we have to
 * calculate the kernel.
 **/
void generateGaborKernel(int kern_size,
		float sin_filter_matrix[MAX_KERN_SIZE][MAX_KERN_SIZE],
		float cos_filter_matrix[MAX_KERN_SIZE][MAX_KERN_SIZE]) {


	float gauss[MAX_KERN_SIZE][MAX_KERN_SIZE];
	float sinK[MAX_KERN_SIZE];
	float cosK[MAX_KERN_SIZE];
	generateSinKernel(kern_size,sinK,cosK);
	generateGaussKernel(kern_size,15,gauss);

	GaussMulti:
	for (int i = 0; i < kern_size; i++) {
		float rho = i - (kern_size / 2);
		for (int j = 0; j < kern_size; j++) {
			sin_filter_matrix[i][j] = sinK[j] * gauss[i][j];
			cos_filter_matrix[i][j] = cosK[j] * gauss[i][j];
		}
	}
/*
	NormalizeGausGabor:
	for (int i = 0; i < kern_size; i++) {
		float row_sum_sin = 0;
		float row_sum_cos = 0;
		for (int j = 0; j < kern_size; j++) {
			row_sum_sin += sin_filter_matrix[i][j];
			row_sum_cos += cos_filter_matrix[i][j];
		}
		for (int j = 0; j < kern_size; j++) {
			float old_sin = sin_filter_matrix[i][j];
			float old_cos = cos_filter_matrix[i][j];
			sin_filter_matrix[i][j] = old_sin
					- (row_sum_sin / (float) kern_size);
			cos_filter_matrix[i][j] = old_cos
					- (row_sum_cos / (float) kern_size);
		}
	}*/
}

void gaborPixel(int rho, int phi, uint8_t norm_img[NORM_HEIGHT * NORM_WIDTH],
		int filter_size, float sin_filter_matrix[MAX_KERN_SIZE][MAX_KERN_SIZE],
		float cos_filter_matrix[MAX_KERN_SIZE][MAX_KERN_SIZE],uint8_t code[BITCODE_LENGTH], int pos) {
	int angles = NORM_WIDTH;
	float total_i = 0.0;
	float total_r = 0.0;

	GaborPixeLoop:
	for (int i = 0; i < filter_size; i++) {
		for (int j = 0; j < filter_size; j++) {
			int image_y = j + phi - (filter_size / 2);
			image_y = MODULO(image_y, angles);

			int image_x = i + rho - (filter_size / 2);
			int tmp = norm_img[image_x * NORM_WIDTH + image_y];
			total_i += sin_filter_matrix[i][j] * tmp;
			total_r += cos_filter_matrix[i][j] * tmp;
		}
	}

	//std::cout << total_i <<"\n";
	//std::cout << total_r <<"\n";

	code[pos] = total_r >= 0 ? 1:0;
	code[pos+1] = total_i >= 0 ? 1:0;
}

void encode(uint8_t norm_img[NORM_HEIGHT * NORM_WIDTH],
		uint8_t bit_code[BITCODE_LENGTH]) {

	float sin_filter_matrix[MAX_KERN_SIZE][MAX_KERN_SIZE];
	float cos_filter_matrix[MAX_KERN_SIZE][MAX_KERN_SIZE];

	int angular_slice = NORM_WIDTH;
	int radial_slice = ENCODED_PIXELS / angular_slice;
	int index = 0;

	for (int r_slice = 0; r_slice < radial_slice; r_slice++) {
		int radius = ((r_slice * (NORM_HEIGHT - 6)) / (2 * radial_slice)) + 3;
		int filter_height;

		if (radius < (NORM_HEIGHT - radius))
			filter_height = 2 * radius - 1;
		else
			filter_height = 2 * (NORM_HEIGHT - radius) - 1;

		if (filter_height > MAX_KERN_SIZE)
			filter_height = MAX_KERN_SIZE;

		generateGaborKernel(filter_height, sin_filter_matrix,
				cos_filter_matrix);

		for (int theta = 0; theta < angular_slice; theta++) {
			gaborPixel(radius, theta, norm_img, filter_height,
					sin_filter_matrix, cos_filter_matrix, bit_code, index);
			index += 2;
		}
	}
}

float hamming(ap_uint<1> code_1[BITCODE_LENGTH],
		ap_uint<1> code_2[BITCODE_LENGTH],
		ap_uint<1> mask_1[BITCODE_LENGTH],
		ap_uint<1> mask_2[BITCODE_LENGTH]) {
	int nonZero =0;
	for (int i = 0; i < BITCODE_LENGTH; i++) {
		ap_uint<1> xorC  = code_1[i] ^ code_2[i];
		ap_uint<1> maskC = mask_1[i] & mask_2[i];
		nonZero = xorC & maskC == 1?nonZero+1:nonZero;
	}
	//return ((nonZero << 7) / BITCODE_LENGTH);
	return (float)nonZero/(float)BITCODE_LENGTH;
}


void checkdataBase(ap_uint<1> bit_code_1[BITCODE_LENGTH],database data[50],bool match){

	ap_uint<1> tmpMask[BITCODE_LENGTH];
	for (int i = 0; i<BITCODE_LENGTH; i++)
		tmpMask[i] = 1;
	const float thres = 0.2;//20% difference
	match = false;
	for (int i=0;i<50;i++){
		if (hamming(bit_code_1,data[i].data,tmpMask,tmpMask)<thres){
			match = true;
			break;
		}
	}
}

//------------------------------------------------------------------------------------
//					Gabor Kernel Fix Point
//-----------------------------------------------------------------------------------


void generateGaussKernel_fix(int kern_size, int peak,
		floatGauss gauss[MAX_KERN_SIZE][MAX_KERN_SIZE]) {
	//Min/Max numbers are calc. with MAX_KERN_SIZE = 10 ->height = 32
	ap_ufixed<16, 5> alpha = (kern_size - 1) * 0.47703222; //2.38..4.77
	ap_ufixed<16, 5> alphaPower = alpha * alpha; //9..22.75
	ap_ufixed<16, 5> temp = ((ap_ufixed<16, 5> ) kern_size/ (ap_ufixed<16, 5> ) 2); //5
	CreateGauss:
	for (int i = 0; i < kern_size; i++) {
		ap_fixed<16, 6> rho = i - temp; //-5..-1
		ap_fixed<16, 6> rhoPower2 = rho * rho; //25..1
		for (int j = 0; j < kern_size; j++) {
			ap_fixed<16, 6> phi = j - temp; //-5..-1
			ap_fixed<16, 6> temp1 = -(rhoPower2 + phi * phi) / alphaPower; //2.19...0.79
			gauss[i][j] = peak * hls::exp(temp1);
		}
	}
}

void generateSinKernel_fix(int kern_size,
		floatSin sink[MAX_KERN_SIZE],
		floatSin cosk[MAX_KERN_SIZE]) {
	//Min/Max numbers are calc. with MAX_KERN_SIZE = 10 ->height = 32

	ap_fixed<16,3> sum_row_sin = 0;
	ap_fixed<16,3> sum_row_cos = 0;
	CalcFirstRow:
	for (int i = 0; i < kern_size; i++) {
		int phi = i - ( kern_size / 2);
		int temp = kern_size / 2;
		floatArg angle = PI * phi / temp;
		floatSin val_sin;
		floatSin val_cos;
		cordic_fix(angle, val_sin, val_cos);

		sink[i] = val_sin;
		cosk[i] = val_cos;
		sum_row_sin += val_sin;
		sum_row_cos += val_cos;
	}
	NormalizeFirstRow:
	for (int i = 0; i < kern_size; i++) {
		/*sin[0][i] = sin[0][i]
				- (ap_fixed<16,3>) (sum_row_sin / (ap_fixed<16,3>) kern_size);
		cos[0][i] = cos[0][i]
				- (ap_fixed<16,3>) (sum_row_cos / (ap_fixed<16,3>) kern_size);
				*/
		sink[i] = sink[i]-(floatSin)((float)sum_row_sin / (float) kern_size);
		cosk[i] = cosk[i]-(floatSin)((float)sum_row_cos / (float) kern_size);
	}
}

void generateGaborKernel_fix(int kern_size,
		floatGabor sin_filter_matrix[MAX_KERN_SIZE][MAX_KERN_SIZE],
		floatGabor cos_filter_matrix[MAX_KERN_SIZE][MAX_KERN_SIZE]) {

	floatSin sin_k[MAX_KERN_SIZE];
	floatSin cos_k[MAX_KERN_SIZE];

	generateSinKernel_fix(kern_size, sin_k, cos_k);

	floatGauss gauss[MAX_KERN_SIZE][MAX_KERN_SIZE];
	generateGaussKernel_fix(kern_size,15,gauss);

	GaussMultiSine:
	for (int i = 0; i < kern_size; i++) {
		for (int j = 0; j < kern_size; j++) {
			sin_filter_matrix[i][j] = sin_k[j] * gauss[i][j];
			cos_filter_matrix[i][j] = cos_k[j] * gauss[i][j];
		}
	}
	/*
	NormalizeGausGabor: for (int i = 0; i < kern_size; i++) {
		ap_fixed<16,5> row_sum_sin = 0;
		ap_fixed<16,5> row_sum_cos = 0;
		for (int j = 0; j < kern_size; j++) {
			row_sum_sin += sin_filter_matrix[i][j];
			row_sum_cos += cos_filter_matrix[i][j];
		}
		for (int j = 0; j < kern_size; j++) {
			sin_filter_matrix[i][j] = sin_filter_matrix[i][j]
					- (row_sum_sin / (ap_fixed<16,5>) kern_size);
			cos_filter_matrix[i][j] = cos_filter_matrix[i][j]
					- (row_sum_cos / (ap_fixed<16,5>) kern_size);
		}
	}
*/
}

void gaborPixel_fix(int rho, int phi,
		uint8_t norm_img[NORM_HEIGHT * NORM_WIDTH], int filter_size,
		floatGabor sin_filter_matrix[MAX_KERN_SIZE][MAX_KERN_SIZE],
		floatGabor cos_filter_matrix[MAX_KERN_SIZE][MAX_KERN_SIZE],
		ap_uint<1> code[BITCODE_LENGTH], int pos) {
	ap_uint<9> angles = NORM_WIDTH;
	ap_fixed<15,14> total_i = 0;//15,14
	ap_fixed<15,14> total_r = 0;

	GaborPixeLoop: for (int i = 0; i < filter_size; i++) {
		ap_uint<9> image_x = i + rho - (filter_size / 2);
		for (int j = 0; j < filter_size; j++) {
			ap_uint<9> image_y = j + phi - (filter_size / 2);
			image_y = MODULO(image_y, angles);
			int index = image_x * angles + image_y;
			ap_uint<8> tmp = (ap_uint<8>)norm_img[index];
			ap_fixed<15,14> valueS = (sin_filter_matrix[i][j]*tmp);
			ap_fixed<15,14> valueC = (cos_filter_matrix[i][j]*tmp);

			total_i = total_i + valueS; //255*6.4
			total_r = total_r + valueC;
			//std::cout << total_i.to_float() <<"\n";
		}
	}
	code[pos] = total_r >= 0 ? 1:0;
	code[pos+1] = total_i >= 0 ? 1:0;
}

void encode_fix(uint8_t norm_img[NORM_HEIGHT * NORM_WIDTH],
		ap_uint<1> bit_code[BITCODE_LENGTH]) {

	floatGabor sin_filter_matrix[MAX_KERN_SIZE][MAX_KERN_SIZE];
	floatGabor cos_filter_matrix[MAX_KERN_SIZE][MAX_KERN_SIZE];

	int angular_slice = NORM_WIDTH;
	int radial_slice = ENCODED_PIXELS / angular_slice;
	int index = 0;
	for (int r_slice = 0; r_slice < radial_slice; r_slice++) {
		int radius = ((r_slice * (NORM_HEIGHT - 6)) / (2 * radial_slice)) + 3;
		int filter_height;

		filter_height = radius < (NORM_HEIGHT - radius)? 2 * radius - 1:filter_height = 2 * (NORM_HEIGHT - radius) - 1;

		if (filter_height > MAX_KERN_SIZE) filter_height = MAX_KERN_SIZE;

		generateGaborKernel_fix(filter_height, sin_filter_matrix,
				cos_filter_matrix);

		CountConvLoop: for (int theta = 0; theta < angular_slice; theta++) {
			gaborPixel_fix(radius, theta, norm_img, filter_height,
					sin_filter_matrix, cos_filter_matrix, bit_code, index);
			index += 2;
		}
	}
}

#endif

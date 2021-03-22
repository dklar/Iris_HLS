#pragma once
#ifndef _TOP_
#define _TOP_
#include "toplevel.hpp"

void top_level_RED(uint8_t norm_img[NORM_HEIGHT * NORM_WIDTH],ap_uint<1> bit_code[maxRho * NORM_WIDTH]){
#pragma HLS INTERFACE bram port=bit_code
#pragma HLS INTERFACE bram port=norm_img
	encodeing::encode_RED(norm_img, bit_code);
}

void top_level_Gabor(uint8_t norm_img[NORM_HEIGHT * NORM_WIDTH],ap_uint<1> bit_code[BITCODE_LENGTH]){
#pragma HLS INTERFACE bram port=bit_code
#pragma HLS INTERFACE bram port=norm_img
	encodeing::encode(norm_img, bit_code);
}

void top_level_Gabor_FIX(uint8_t norm_img[NORM_HEIGHT * NORM_WIDTH],ap_uint<1> bit_code[BITCODE_LENGTH]){
#pragma HLS INTERFACE bram port=bit_code
#pragma HLS INTERFACE bram port=norm_img
	encodeing::encode_fix(norm_img, bit_code);
}

#endif


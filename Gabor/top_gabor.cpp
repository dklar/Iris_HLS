#include "stdint.h"
#include "types.hpp"
#include "gabor.hpp"

void top_gabor(uint8_t NORM_IMAGE[180*16],database data[50],bool match){
#pragma HLS INTERFACE ap_ctrl_none port=return
#pragma HLS INTERFACE bram port=NORM_IMAGE
#pragma HLS INTERFACE bram port=data

	ap_uint<1> bit_code[BITCODE_LENGTH];
	encode_fix(NORM_IMAGE,bit_code);
	checkdataBase(bit_code,data,match);
}

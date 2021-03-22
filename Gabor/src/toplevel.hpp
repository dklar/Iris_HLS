#include "gabor.hpp"

void top_level_RED(uint8_t norm_img[NORM_HEIGHT * NORM_WIDTH],ap_uint<1> bit_code[maxRho * NORM_WIDTH]);
void top_level_Gabor(uint8_t norm_img[NORM_HEIGHT * NORM_WIDTH],ap_uint<1> bit_code[BITCODE_LENGTH]);
void top_level_Gabor_FIX(uint8_t norm_img[NORM_HEIGHT * NORM_WIDTH],ap_uint<1> bit_code[BITCODE_LENGTH]);

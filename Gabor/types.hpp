#include <stdint.h>
#include "hls_math.h"
#include "ap_fixed.h"
#include "hls_video.h"

#ifndef TYPES_H
#define TYPES_H

#define MAX_WIDTH_CASIA  320
#define MAX_HEIGHT_CASIA  280

#define MAX_WIDTH_IITD 320
#define MAX_HEIGHT_IITD 240

#define NORM_WIDTH 180
#define NORM_HEIGHT 16

#define MAX_WIDTH MAX_WIDTH_IITD
#define MAX_HEIGHT MAX_HEIGHT_IITD

#define NORM_HEIGHT 16
#define NORM_WIDTH  180

#define MAX_KERN_SIZE NORM_HEIGHT/3

#define ENCODED_PIXELS  1024
#define BITCODE_LENGTH  2048     //each encoded pixel brings 2 bits to the bitcode re and im

#define MAX_BITWIDTH 16

namespace types {
	typedef hls::stream<ap_axiu<32, 1, 1, 1> > AXI_STREAM; //32 bit data stream
	typedef hls::Mat<MAX_HEIGHT, MAX_WIDTH, HLS_8UC3> RGB_IMAGE; //RGB image from type HLS::Mat
	typedef hls::Mat<MAX_HEIGHT, MAX_WIDTH, HLS_8UC1> GRAY_IMAGE; //Gray image from type HLS::Mat
	typedef hls::Mat<NORM_HEIGHT, NORM_WIDTH, HLS_8UC3> RGB_NORM_IMAGE; //RGB image from type HLS::Mat
	typedef hls::Mat<NORM_HEIGHT, NORM_WIDTH, HLS_8UC1> GRAY_NORM_IMAGE; //Gray image from type HLS::Mat
	typedef hls::Scalar<1, uint8_t> PIXELGRAY;
	typedef hls::Scalar<3, uint8_t> RGBPIXEL;
	static const float DegToRad = 0.01745329;

	typedef ap_fixed<16,5> floatGauss;
	typedef ap_fixed<16,5> floatGabor;//orginal 18,5
	typedef ap_ufixed<MAX_BITWIDTH,0> floatTan;
	typedef ap_fixed<MAX_BITWIDTH,2> floatSin;
	typedef ap_fixed<MAX_BITWIDTH,4> floatArg;

}

#endif

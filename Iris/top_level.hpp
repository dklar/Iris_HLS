#ifndef TOP_H
#define TOP_H

#include "types.hpp"
#include "segmentation.hpp"
#include "normalisation.hpp"

using namespace types;
using namespace segmentaion;
using namespace normalisation;

/*
 * Top Level module to perform all calculation steps until normalization.
 */
void top_level_normalisation(AXI_STREAM& inputStream,uint8_t NORM_IMAGE[NORM_WIDTH * NORM_HEIGHT]){
#pragma HLS INTERFACE axis port=inputStream
#pragma HLS INTERFACE bram port=NORM_IMAGE
#pragma HLS INTERFACE ap_ctrl_none port=return

#pragma HLS DATAFLOW

	RGB_IMAGE img0(MAX_HEIGHT, MAX_WIDTH);
	RGB_NORM_IMAGE img1(NORM_HEIGHT,NORM_WIDTH);

	uint8_t fifoGrey1[MAX_WIDTH * MAX_HEIGHT];
	uint8_t fifoGrey2[MAX_WIDTH * MAX_HEIGHT];

	uint8_t fifoTrib1[MAX_WIDTH * MAX_HEIGHT];
	uint8_t fifoTrib2[MAX_WIDTH * MAX_HEIGHT];
	uint8_t fifoTrib3[MAX_WIDTH * MAX_HEIGHT];

	uint8_t fifoRef[MAX_WIDTH * MAX_HEIGHT];
	uint8_t fifoMorph[MAX_WIDTH * MAX_HEIGHT];
	uint8_t fifocanny[MAX_WIDTH * MAX_HEIGHT];
	uint8_t fifoPupil[MAX_WIDTH * MAX_HEIGHT];

	uint8_t BRAM_IMAGE_1[MAX_WIDTH * MAX_HEIGHT];		// Input image on Block RAM

#pragma HLS STREAM variable=fifoGrey1 depth=1 dim=1
#pragma HLS STREAM variable=fifoGrey2 depth=1 dim=1
#pragma HLS STREAM variable=fifoMorph depth=1 dim=1
#pragma HLS STREAM variable=fifoRef depth=1 dim=1
#pragma HLS STREAM variable=fifocanny depth=1 dim=1
#pragma HLS STREAM variable=fifoPupil depth=1 dim=1


#pragma HLS STREAM variable=fifoTrib1 depth=1 dim=1
#pragma HLS STREAM variable=fifoTrib2 depth=1 dim=1
#pragma HLS STREAM variable=fifoTrib3 depth=1 dim=1

	hls::AXIvideo2Mat(inputStream, img0);
	MatToGrayArray<MAX_WIDTH, MAX_HEIGHT>(img0, fifoGrey1);
	duplicate<MAX_WIDTH, MAX_HEIGHT>(fifoGrey1, fifoGrey2, BRAM_IMAGE_1);
	removeWhite<MAX_WIDTH, MAX_HEIGHT>(fifoGrey2, fifoRef);
	morphOpening<MAX_WIDTH, MAX_HEIGHT>(fifoRef, fifoMorph);
	triple<MAX_WIDTH, MAX_HEIGHT>(fifoMorph, fifoTrib1, fifoTrib2, fifoTrib3);
	Canny<MAX_WIDTH, MAX_HEIGHT>(fifoTrib1, fifocanny);
	int x, y, rp, ri;
	getPupil_center<MAX_WIDTH, MAX_HEIGHT>(fifocanny, fifoTrib2,x, y);
	findPupilBorder<30, 80, MAX_WIDTH, MAX_HEIGHT>(fifoTrib3, fifoPupil, x, y, rp);
	findIrisBorder<80, 130, MAX_WIDTH, MAX_HEIGHT>(fifoPupil, x, y, ri);
	norm_float(BRAM_IMAGE_1,NORM_IMAGE,x,y,rp,ri);
}



#endif

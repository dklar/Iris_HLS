#pragma once

#ifndef NORM_H
#define NORM_H

#include "types.hpp"
#include "cordic.hpp"

namespace normalisation{

using namespace types;
using namespace cordic;

void norm_fix(uint8_t *image_in,uint8_t *image_out, int x, int y, int r1,int r2) {
	int i=0;
	for (int theta = 0; theta < 360; theta+=2) {
		floatArg thetaDot = theta * DegToRad;

		floatSin tempSin, tempCos;
		cordic_fix(thetaDot,tempSin,tempCos);
		floatSin r1_c =  r1 * tempCos;
		floatSin r1_s =  r1 * tempSin;
		floatSin r2_c =  r2 * tempCos;
		floatSin r2_s =  r2 * tempSin;

		ap_ufixed<16,9> temp1, temp2, temp3, temp4;
		temp1 = (ap_ufixed<16,9>) (x + r1_c);//max: 320 + 120*1 = 440 -> 9 bit int
		temp2 = (ap_ufixed<16,9>) (x + r2_c);
		temp3 = (ap_ufixed<16,9>) (y + r1_s);
		temp4 = (ap_ufixed<16,9>) (y + r2_s);

		for (int r = 0; r < NORM_HEIGHT; r++) {
			ap_ufixed<6,1> radius = r / ((float) NORM_HEIGHT);//radius check
			ap_uint<9> xi = (ap_uint<9> ) ((1 - radius) * temp1 + radius * temp2);
			ap_uint<9> yi = (ap_uint<9> ) ((1 - radius) * temp3 + radius * temp4);

			int pos = yi.to_int() * MAX_WIDTH + xi.to_int();//array index require normal int!

			image_out[r * NORM_WIDTH + i] = image_in[pos];
		}
		i+=1;
	}
}

void norm_float(uint8_t *image_in, uint8_t *image_out, int x, int y, int r1,int r2) {
	int i = 0;
	for (int theta = 0; theta < 360; theta += 2) {

		float thetaDot = theta * DegToRad;
		float temp1, temp2, temp3, temp4;
		float tempSin, tempCos;
		hls::sincos(thetaDot, &tempSin, &tempCos);
		temp1 = (float) (x + r1 * tempCos);
		temp2 = (float) (x + r2 * tempCos);
		temp3 = (float) (y + r1 * tempSin);
		temp4 = (float) (y + r2 * tempSin);

		for (int r = 0; r < NORM_HEIGHT; r++) {
			float radius = r / ((float) NORM_HEIGHT);
			int xi = int((1 - radius) * temp1 + radius * temp2);
			int yi = int((1 - radius) * temp3 + radius * temp4);

			int pos = yi * MAX_WIDTH + xi;

			image_out[r * NORM_WIDTH + i] = image_in[pos];
		}
		i+=1;
	}
}

}


#endif

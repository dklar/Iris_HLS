#ifndef CORD_H
#define CORD_H

#include "types.hpp"
#define MAX_BITWIDTH 16

using namespace types;

namespace cordic{

typedef ap_fixed<16,5> floatGauss;
typedef ap_fixed<16,5> floatGabor;//orginal 18,5
typedef ap_ufixed<MAX_BITWIDTH,0> floatTan;
typedef ap_fixed<MAX_BITWIDTH,2> floatSin;
typedef ap_fixed<MAX_BITWIDTH,4> floatArg;


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
		9.536743164059608e-07,
		4.7683715820308884e-07,
		2.3841857910155797e-07,
		1.1920928955078068e-07,
		5.960464477539055e-08,
		2.9802322387695303e-08,
		1.4901161193847655e-08,
		7.450580596923828e-09,
		3.725290298461914e-09,
		1.862645149230957e-09,
		9.313225746154785e-10,
		4.656612873077393e-10
		};
/*
class cordic{
public:
	void cordic_fix(floatArg phi,floatSin &s,floatSin &c);
};
*/
void cordic_intern(floatSin &x, floatSin &y, floatArg phi){
	int nMax = 20;

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
	//ap_uint<1> sgn = (phi>0)? 1:0;
	floatArg absPhi;
	if (phi.is_neg()){
		absPhi = -phi;
	}else{
		absPhi = phi;
	}

	const ap_ufixed<MAX_BITWIDTH,0> TWO_DIV_PI("0x0.A2F9836E4E43");
	const ap_ufixed<MAX_BITWIDTH,1> PI_HALF("0x1.921FB54442D18");
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

#endif

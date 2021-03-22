#include "toplevel.hpp"
#include <hls_opencv.h>
#include <fstream>
#include <dirent.h>
#include <stdio.h>

#ifdef _WIN32

#include <windows.h>
std::string databasePath ="C://Users//Dennis//VivadoHLS//Final//Database//IITD Database//Normalized_Images//";

static ap_uint<1> bit_code[maxRho * NORM_WIDTH];
void test(std::string path,std::string outname){
	cv::Mat picture =cv::imread(path);
	uint8_t *data = picture.data;

	for (int i = 0; i < maxRho * NORM_WIDTH; i++)
		bit_code[i] = 0;

	encodeing::encode_RED(data,bit_code);

	uint8_t tempSave[maxRho * NORM_WIDTH];


	for (int i=0;i<maxRho * NORM_WIDTH;i++)
		tempSave[i] = bit_code[i].to_int();

	FILE *file = fopen(outname.c_str(), "wb");
	fwrite(tempSave, sizeof(uint8_t), maxRho * NORM_WIDTH, file);
	fclose(file);
}

void testDetectionTop() {

	std::string path = "C://Users//Dennis//VivadoHLS//Final//Database//IITD Database//Normalized_Images//*";

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile(path.c_str(), &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE) {
		printf("FindFirstFile failed (%d)\n", GetLastError());
		return;
	}
	path = path.substr(0, path.size() - 1);
	int i=100;
	do {
		if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			std::string pathTemp = path + FindFileData.cFileName;
			std::string fileName = FindFileData.cFileName;
			fileName = fileName.substr(0, fileName.size()-3)+"dat";
			test(pathTemp,fileName);
			//i--;
			std::cout <<"done with "<<fileName <<"\n";
		}

	} while (FindNextFile(hFind, &FindFileData) != 0 && i>0);

	FindClose(hFind);

}
#endif

void testGaborFloat() {
	float sin_m[MAX_KERN][MAX_KERN];
	float cos_m[MAX_KERN][MAX_KERN];
	encodeing::GaborKernel(7, sin_m, cos_m);
	for (int i = 0; i < 7; i++) {
		for (int j = 0; j < 7; j++) {
			std::cout << sin_m[i][j] << " ";
		}
		std::cout << "\n";
	}
}

void testGaborFix() {
	floatGabor sin_m[MAX_KERN][MAX_KERN];
	floatGabor cos_m[MAX_KERN][MAX_KERN];
	encodeing::GaborKernel_fix(7, sin_m, cos_m);
	for (int i = 0; i < 7; i++) {
		for (int j = 0; j < 7; j++) {
			std::cout << sin_m[i][j] << " ";
		}
		std::cout << "\n";
	}
}

void testSineFloat() {
	float sin_m[MAX_KERN];
	float cos_m[MAX_KERN];

	encodeing::SinKernel(7,sin_m,cos_m);
	for (int i = 0; i < 7; i++) {
		for (int j = 0; j < 7; j++) {
			std::cout << sin_m[j] << " ";
		}
		std::cout << "\n";
	}
}

void testSineFix() {
	floatSin sin_m[MAX_KERN];
	floatSin cos_m[MAX_KERN];

	encodeing::SinKernel_fix(7,sin_m,cos_m);
	for (int i = 0; i < 7; i++) {
		for (int j = 0; j < 7; j++) {
			std::cout << sin_m[j] << " ";
		}
		std::cout << "\n";
	}
}

void testCosineFloat() {
	float sin_m[MAX_KERN];
	float cos_m[MAX_KERN];

	encodeing::SinKernel(7,sin_m,cos_m);
	for (int i = 0; i < 7; i++) {
		for (int j = 0; j < 7; j++) {
			std::cout << cos_m[j] << " ";
		}
		std::cout << "\n";
	}
}

void testCosineFix() {
	floatSin sin_m[MAX_KERN];
	floatSin cos_m[MAX_KERN];

	encodeing::SinKernel_fix(7,sin_m,cos_m);
	for (int i = 0; i < 7; i++) {
		for (int j = 0; j < 7; j++) {
			std::cout << cos_m[j] << " ";
		}
		std::cout << "\n";
	}
}

void testGaussFloat() {
	float gauss[MAX_KERN][MAX_KERN];
	encodeing::GaussKernel(7,15, gauss);
	for (int i = 0; i < 7; i++) {
		for (int j = 0; j < 7; j++) {
			std::cout << gauss[i][j] << " ";
		}
		std::cout << "\n";
	}
}

void testGaussFix() {
	floatGabor gauss[MAX_KERN][MAX_KERN];
	encodeing::GaussKernel_fix(7,15,gauss);
	for (int i = 0; i < 7; i++) {
		for (int j = 0; j < 7; j++) {
			std::cout << gauss[i][j] << " ";
		}
		std::cout << "\n";
	}
}

void compareKernel(){
	floatGabor sin_m[MAX_KERN][MAX_KERN];
	floatGabor cos_m[MAX_KERN][MAX_KERN];
	float sin_f[MAX_KERN][MAX_KERN];
	float cos_f[MAX_KERN][MAX_KERN];
	std::cout <<"\n-----Start Test----------\n";
	std::cout << std::fixed;
	for (int ksize = 8;ksize <MAX_KERN;ksize++){
		encodeing::GaborKernel_fix(ksize, sin_m, cos_m);
		encodeing::GaborKernel(ksize, sin_f, cos_f);

		for (int i = 0; i < ksize; i++) {
			for (int j = 0; j < ksize; j++) {
				float errABS = abs(sin_m[i][j].to_float()) - abs(sin_f[i][j]);
				float errREL = errABS / abs(sin_f[i][j]);
				if (sin_f[i][j]>0.000001 && sin_f[i][j]<-0.000001){
					if (errREL > 0.1 )
						std::cout<< std::noshowpos<< std::setprecision(6)<< "big error at kernel ="<<ksize <<" ; "<<i << ";"<<j <<" :"<< std::showpos << sin_m[i][j].to_float() << " ; " << sin_f[i][j]<< " ";
					std::cout << std::setprecision(6) << errREL << "\n";
				}
			}
		}
	}
	std::cout <<"\n\n-----End Test---------\n";
}


void gaborCodeFloat() {
	struct dirent *entry = nullptr;
	DIR *dp = nullptr;

	dp = opendir("/home/dennis/IITD Database/Normalized_Images/");
	if (dp != nullptr) {
		while ((entry = readdir(dp))){
			std::string file(entry->d_name);
			if(file.find("bmp") != std::string::npos){
				cv::Mat picture = cv::imread("/home/dennis/IITD Database/Normalized_Images/"+file);
				uint8_t *picturedata = picture.data;
				ap_uint<1> code[BITCODE_LENGTH];
				uint8_t tempSave[BITCODE_LENGTH];
				encodeing::encode(picturedata,code);

				for (int i=0;i<BITCODE_LENGTH;i++)
					tempSave[i] = code[i].to_int();
				std::string fileName = file.substr(0, file.size()-3)+"dat";
				FILE *file = fopen(fileName.c_str(), "wb");
				fwrite(tempSave, sizeof(uint8_t), BITCODE_LENGTH, file);
				fclose(file);

			}
		}
	}

	closedir(dp);
}

void gaborCodeFix() {
	struct dirent *entry = nullptr;
	DIR *dp = nullptr;

	dp = opendir("/home/dennis/IITD Database/Normalized_Images/");
	if (dp != nullptr) {
		while ((entry = readdir(dp))){
			std::string file(entry->d_name);
			if(file.find("bmp") != std::string::npos){
				cv::Mat picture = cv::imread("/home/dennis/IITD Database/Normalized_Images/"+file);
				uint8_t *picturedata = picture.data;
				ap_uint<1> code[BITCODE_LENGTH];
				uint8_t tempSave[BITCODE_LENGTH];
				encodeing::encode_fix(picturedata,code);

				for (int i=0;i<BITCODE_LENGTH;i++)
					tempSave[i] = code[i].to_int();
				std::string fileName = file.substr(0, file.size()-3)+"dat";
				FILE *file = fopen(fileName.c_str(), "wb");
				fwrite(tempSave, sizeof(uint8_t), BITCODE_LENGTH, file);
				fclose(file);

			}
		}
	}

	closedir(dp);
}

void gaborCompare() {
	cv::Mat picture = cv::imread("/home/dennis/IITD Database/Normalized_Images/001_1.bmp");
	uint8_t *picturedata = picture.data;
	ap_uint<1> code[BITCODE_LENGTH];
	uint8_t tempSave[BITCODE_LENGTH];
	encodeing::encode_compare(picturedata);

}

void testSine(){
	for (int alpha =0;alpha<=360;alpha++){
		float arg = PI * alpha/180;
		floatArg argFix = arg;
		floatSin sinusApr;
		floatSin cosinApr;

		float sinusAcc = sin(arg);
		cordic::cordic_fix(argFix,sinusApr,cosinApr);
		std::cout << alpha <<" : " << sinusApr.to_float() << " ; " << sinusAcc << "\n";
	}
}

void showKernels(){
	std::cout<<"-------------GaborFix:---------\n";
	testGaborFix();
	std::cout<<"-------------GaborFloat:-------\n";
	testGaborFloat();
	std::cout<<"-------------SinFix:-----------\n";
	testSineFix();
	std::cout<<"-------------SineFloat:--------\n";
	testSineFloat();
	std::cout<<"-------------CosinFix:-----------\n";
	testCosineFix();
	std::cout<<"-------------CosineFloat:--------\n";
	testCosineFloat();
	std::cout<<"-------------GaussFix:--------\n";
	testGaussFix();
	std::cout<<"-------------GaussFloat:--------\n";
	testGaussFloat();
}

int main() {

	//testDetectionTop();

	gaborCompare();
	return 0;
}

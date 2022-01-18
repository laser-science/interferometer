/*
 * mainCCD.cpp
 *
 *  Created on: Jan 4, 2022
 *      Author: Evan
 */

/*********************** Header ***********************/
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>
#include <cmath>
#include <cstdlib>
#include <time.h>       // time stamps
// Headers from sampleCCD code
#include "visa.h"
#include "TLCCS.h"      // the device driver header

using namespace std;

//==============================================================================
// Constants
//===========================================================================

//#define MY_INTEGRATION_TIME   0.1            // Set the optical integration time in seconds (i.e. 0.1 s = 100 ms)
#define MY_SAMPLE_FILE        "sample.txt"   // the file to store the values to
#define MY_SCAN_COUNT         3              // we take 10 scans (E: 3?)

//===========================================================================
// Globals
//===========================================================================

ViSession   instr    = VI_NULL;                 // instrument handle
FILE*       my_file  = NULL;                    // file handlin

int main(){
	ViStatus    err      = VI_SUCCESS;           // error variable
	ViUInt32    cnt      = 0;                    // counts found devices
	ViFindList findList;                         // this is the container for the handle identifying the search session
	ViChar      rscStr[VI_FIND_BUFLEN];          // resource string
	ViReal64	MY_INTEGRATION_TIME = 0.1;   	 // Set the integration time in seconds
		ViSession sesn; // This will contain the resource manager session
	viOpenDefaultRM(&sesn); /* This gets the resource manager session handle. The & symbol directs gcc/g++ to the memory location of sesn.
							  Google "C++ pointers" for more info." */
	err = viFindRsrc(sesn, TLCCS_FIND_PATTERN, &findList, &cnt, rscStr);
	if(err){
	   cout << "error with viFindRsrc" << endl;
	   system("pause");
	   exit(1);
	}
	err = tlccs_init(rscStr, VI_OFF, VI_OFF, &instr);
	if(err){
	   cout << "error with tlccs_init" << endl;
	   system("pause");
	   exit(1);
	}
	// set integration time
	err = tlccs_setIntegrationTime(instr, MY_INTEGRATION_TIME);
	if(err){
	   cout << "error with setIntegrationTime" << endl;
	   system("pause");
	   exit(1);
	}
	ViReal64 getTimeplz;
	tlccs_getIntegrationTime(instr, &getTimeplz); // This gets and outputs the the integration time we just input
	cout << getTimeplz << endl;
	system("pause");
	//return 0;

	//triggers CCS to take a single scan
	tlccs_startScan(instr);


	// outputs the scan data

	ViReal64 _VI_FAR intensitydata[3648];

	tlccs_getScanData(instr, intensitydata);

	cout << intensitydata[300] <<endl;
	system("pause");

	ViInt16 dataSet = 0;
	ViReal64 _VI_FAR wavedata[3648];
	ViPReal64 minwav = NULL;
	ViPReal64 maxwav = NULL;

	tlccs_getWavelengthData(instr, dataSet, wavedata, minwav, maxwav);

	cout << wavedata[122] << endl;
	system("pause");

	/*ViReal64 _VI_FAR testarray[1][1];
	testarray[0] = {data[0], wavdatArr[0]}; //do this tmw
	cout << testarray[0] << endl;
	system("pause"); */

	ofstream MyFile("spec_file.txt");
	int index = 0;
	while (index <= 3648){
		MyFile << wavedata[index]  << " " << intensitydata[index] << endl;
		index  = index + 1;
	}
	MyFile.close();
	system("pause");


	//return 0;




}




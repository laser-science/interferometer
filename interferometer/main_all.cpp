/*
 * main_all.cpp
 *
 * Created on: Jan 18, 2022
 * Author: Evan J. Zach A. Mitchell G. Matthew P. 
 */

 /*********************** Header ***********************/
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>
#include <cmath>
#include <cstdlib>

// Actuator Headers 
#include "Thorlabs.MotionControl.KCube.DCServo.h"

// spectrometer headers
#include "visa.h"
#include "TLCCS.h"      

// Headers from sampleCCD code
#include "tl_camera_sdk.h"
#include "tl_camera_sdk_load.h"

//==============================================================================
// Spectrometer Constants
//===========================================================================

//#define MY_INTEGRATION_TIME   0.1          // Set the optical integration time in seconds (i.e. 0.1 s = 100 ms)
#define MY_SAMPLE_FILE        "sample.txt"   // the file to store the values to
#define MY_SCAN_COUNT         3              // we take 10 scans (E: 3?)

//===========================================================================
// Spectrometer Globals
//===========================================================================

ViSession instr = VI_NULL;     // instrument handle
FILE* my_file = NULL;          // file handlin


using namespace std;

int main() {
	/**************************Actuator Code******************************************************/
	int deviceID = 0;
	vector<string> serialNumba;
	TLI_BuildDeviceList();
	// get device list size 
	short n = TLI_GetDeviceListSize();
	cout << n << endl;
	system("pause");

	/**************************Spectrometer Code**************************************************/
	ViStatus    err = VI_SUCCESS;           // error variable
	ViUInt32    cnt = 0;                    // counts found devices
	ViFindList findList;                         // this is the container for the handle identifying the search session
	ViChar      rscStr[VI_FIND_BUFLEN];          // resource string
	ViReal64	MY_INTEGRATION_TIME = 0.1;   	 // Set the integration time in seconds
	ViSession sesn; // This will contain the resource manager session
	viOpenDefaultRM(&sesn); /* This gets the resource manager session handle. The & symbol directs gcc/g++ to the memory location of sesn.
							  Google "C++ pointers" for more info." */
	err = viFindRsrc(sesn, TLCCS_FIND_PATTERN, &findList, &cnt, rscStr);
	if (err) {
		cout << "error with viFindRsrc" << endl;
		system("pause");
		//exit(1); // I commented out the exit for this and the following conditionals. Normally you would keep them in but I wanted to ignore them for testing purposes
	}
	err = tlccs_init(rscStr, VI_OFF, VI_OFF, &instr);
	if (err) {
		cout << "error with tlccs_init" << endl;
		system("pause");
		//exit(1);
	}
	// set integration time
	err = tlccs_setIntegrationTime(instr, MY_INTEGRATION_TIME);
	if (err) {
		cout << "error with setIntegrationTime" << endl;
		system("pause");
		//exit(1);
	}
	ViReal64 getTimeplz;
	tlccs_getIntegrationTime(instr, &getTimeplz); // This gets and outputs the the integration time we just input
	cout << getTimeplz << endl;
	system("pause");

	/*********************CCD Camera code*********************************************************/
	err = tl_camera_sdk_dll_initialize();
	cout << "Hello" << endl;
	cout << err << endl;
	system("pause");
	tl_camera_open_sdk();
	cout << "Pass" << endl;
	system("pause");

	const int bufferLen = 256; // 256 bytes
	char serialNum[bufferLen];
	char* serialP = serialNum;


	tl_camera_discover_available_cameras(serialP, bufferLen);

	void* chP;
	tl_camera_open_camera(serialP, &chP);
	tl_camera_arm(chP, 2);
	tl_camera_issue_software_trigger(chP);


	//initialize frame variables
	unsigned short* image_buffer = 0;
	int frame_count = 0;
	unsigned char* metadata = 0;
	int metadata_size_in_bytes = 0;

	int width = 0;
	int height = 0;
	tl_camera_get_image_height(chP, &height);
	tl_camera_get_image_width(chP, &width);
	cout << "Width in pixels: " << width << endl;
	cout << "Height in pixels: " << height << endl;
	/***************************************************************/
	system("pause");
	/***************************************************************/
	// THIS PRECEDING PAUSE ALLOWS THE CAMERA TIME TO WRITE TO THE IMAGE BUFFER. A TIMER WOULD ALSO WORK.
	/***************************************************************/

	tl_camera_get_pending_frame_or_null(chP, &image_buffer, &frame_count, &metadata, &metadata_size_in_bytes);

	ofstream frame;
	frame.open("testFrame.pgm");
	frame << "P2" << endl; // This is the type for netpbm called the "magic number". In this case, P2 corresponds to ASCII greyscale
	frame << width << " " << height << endl;
	frame << 1022 << endl; // This is the maximum pixel value
	int pixelCounter = 0;
	for (int i = 1; i <= height; i++) {
		for (int j = 1; j <= width; j++) {
			frame << image_buffer[pixelCounter] << " ";
			pixelCounter++;
		}
		frame << endl;
	}
	frame.close();

	printf("Images received! Closing camera...\n");
	tl_camera_disarm(chP);
	tl_camera_close_camera(chP);
	tl_camera_close_sdk();
	tl_camera_sdk_dll_terminate();
	return 0;
};
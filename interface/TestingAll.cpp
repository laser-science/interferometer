using namespace std;
/*
 Interfacing Code
 Authors: Zach A. Mitchell G. Matthew P. Evan J
 1/18/2022
*/
/*********************** Header ***********************/
//General Headers
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>
#include <cmath>
#include <cstdlib>
#include <time.h>
#include <stdio.h>
//Header from actuator code
#include "Thorlabs.MotionControl.KCube.DCServo.h"
#include "stdafx.h"
#include <stdlib.h>
#include <conio.h>
// Headers from spectrometer code
#include "visa.h"
#include "TLCCS.h"      // the device driver header

// Headers from Camera code
#include "tl_camera_sdk.h"
#include "tl_camera_sdk_load.h"

//Forward Declaration
int checkForError(ViReal64 MY_INTEGRATION_TIME, ViUInt32 cnt, ViFindList &findList, ViSession &sesn);
int getSpectrometerData();
int writeToFile(ViReal64 _VI_FAR wavedata[], ViReal64 _VI_FAR intensitydata[]);

//==============================================================================
// Constants
//===========================================================================

//#define MY_INTEGRATION_TIME   0.1            // Set the optical integration time in seconds (i.e. 0.1 s = 100 ms)
#define MY_SAMPLE_FILE        "sample.txt"   // the file to store the values to
#define MY_SCAN_COUNT         3              // we take 10 scans (E: 3?)

//===========================================================================
// Globals
//===========================================================================

ViSession   instr = VI_NULL;                 // instrument handle
FILE* my_file = NULL;                    // file handlin
int main() {
	/****************************************************Global Variables***************************************/
	ViReal64	MY_INTEGRATION_TIME = 0.1;	//This sets integration time, can be changed as needed.
	ViUInt32    cnt = 0;                    // counts found devices
	ViFindList  findList;                    // this is the container for the handle identifying the search session
	ViStatus    err = VI_SUCCESS;           // error variable
	ViChar      rscStr[VI_FIND_BUFLEN];     // resource string// Set the integration time in seconds
	ViSession   sesn;							// This will contain the resource manager session
	ViReal64    getTimeplz;
	ViReal64 _VI_FAR intensitydata[3648];   //intensity data array for spectrometer
	ViReal64 _VI_FAR wavedata[3648];		//wave data array for spectrometer
	ViInt16 dataSet = 0;
	ViPReal64 minwav = NULL;
	ViPReal64 maxwav = NULL;
	int serialNo = 27260232;
	double stepSize = 0;
	int position = 0;
	double real_unit = 0;
	int device_unit = 1;
	int unitType = 0;
	// identify and access device
	char testSerialNo[16];
	sprintf_s(testSerialNo, "%d", serialNo);
	int key = 0;
	#define KEY_LEFT 75
	#define KEY_RIGHT 77
	/*********************************Error Checking************************************************************/
	//This section checks for the errors in the program before continuing with running the program
	viOpenDefaultRM(&sesn);					/* This gets the resource manager session handle. The & symbol directs the compiler to the memory location of sesn.
											Google "C++ pointers" for more info." */
	//This checks the spectrometer to see if it is connected
	err = viFindRsrc(sesn, TLCCS_FIND_PATTERN, &findList, &cnt, rscStr);
	if (err) {
		cout << "error with viFindRsrc" << endl;
		system("pause");
		exit(1);
	}
	//checks for error with the tlccs dlls
	err = tlccs_init(rscStr, VI_OFF, VI_OFF, &instr);
	if (err) {
		cout << "error with tlccs_init" << endl;
		system("pause");
		exit(1);
	}
	// checks error with the integration time
	err = tlccs_setIntegrationTime(instr, MY_INTEGRATION_TIME);
	if (err) {
		cout << "error with setIntegrationTime" << endl;
		system("pause");
		exit(1);
	}
	/***************************************Spectrometer****************************************/
	//This is where the program will run. In this program, one section will move the actuator while the other section will
	//take data with the spectrometer. Finally, one method has been abstracted to write the data gained to a file. This all runs 
	//in a loop. the integer x in the loop determines how many times it runs and can be changed.
	
	cout << "Enter step size in millimeters: ";
	cin >> stepSize;
	cout << stepSize << endl;

	for (int x = 0; x < 2; x++) {
		//put movement commands here
		//This will tell the actuator which way to move
		switch ((key = _getch())) {
		case KEY_LEFT:
			cout << endl << "Left" << endl;  // key left
			break;
		case KEY_RIGHT:
			cout << endl << "Right" << endl;  // key right
			break;
		default:
			cout << endl << "null" << endl;  // not arrow
			break;
		}
		// open device
		if (CC_Open(testSerialNo) == 0)
		{
			CC_GetDeviceUnitFromRealValue(testSerialNo, stepSize, &device_unit, unitType);
			
			cout << "device units: " << device_unit << endl;
			system("pause");
			// start the device polling at 200ms intervals
			CC_StartPolling(testSerialNo, 200);

			Sleep(3000);
			// Home device
			CC_ClearMessageQueue(testSerialNo);
			CC_Home(testSerialNo);
			printf("Device %s homing\r\n", testSerialNo);

			// wait for completion
			WORD messageType;
			WORD messageId;
			DWORD messageData;
			CC_WaitForMessage(testSerialNo, &messageType, &messageId, &messageData);
			while (messageType != 2 || messageId != 0)
			{
				CC_WaitForMessage(testSerialNo, &messageType, &messageId, &messageData);
			}
			// move to position (channel 1)
			CC_ClearMessageQueue(testSerialNo);
			CC_MoveToPosition(testSerialNo, position);
			printf("Device %s moving\r\n", testSerialNo);

			// wait for completion
			CC_WaitForMessage(testSerialNo, &messageType, &messageId, &messageData);
			while (messageType != 2 || messageId != 1)
			{
				CC_WaitForMessage(testSerialNo, &messageType, &messageId, &messageData);
			}

			// get actual position
			int pos = CC_GetPosition(testSerialNo);
			printf("Device %s moved to %d\r\n", testSerialNo, pos);

			// stop polling
			CC_StopPolling(testSerialNo);
			// close device
			CC_Close(testSerialNo);
		}

		//above here
		tlccs_getIntegrationTime(instr, &getTimeplz); // This gets and outputs the the integration time we just input
		//triggers CCS to take a single scan
		tlccs_startScan(instr);
		//gets intensity data
		tlccs_getScanData(instr, intensitydata);
		//gets wave data
		tlccs_getWavelengthData(instr, dataSet, wavedata, minwav, maxwav);
		writeToFile(wavedata, intensitydata);
	}
	
	return 0;
};



/********************************Methods***********************************************************************/
//This write the wave and intensity data to a file. It appends it when run multiple times
int writeToFile(ViReal64 _VI_FAR wavedata[], ViReal64 _VI_FAR intensitydata[]) {
	ofstream MyFile;
	//opens the file as appending
	MyFile.open("spec_file.txt", ios::app);
	//runs through the passed arrays which should be 3648 data points each. 
	for (int index = 0; index < 3648; index++) {
		MyFile << wavedata[index] << " " << intensitydata[index] << endl;
	}
	MyFile.close();
	cout << "written successfully" << endl;
	return 0;
}



/***************************************Camera Code*****************************************/
/**

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
//system("pause");
/***************************************************************/
// THIS PRECEDING PAUSE ALLOWS THE CAMERA TIME TO WRITE TO THE IMAGE BUFFER. A TIMER WOULD ALSO WORK.
/***************************************************************/
/**
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
	/*******************************************************************************************/
	
    
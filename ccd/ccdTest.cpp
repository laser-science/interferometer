/*
 * ccdTest.cpp
 *
 *  Created on: Jan 4, 2022
 *      Author: Evan and the Gang
 */

/*********************** Header ***********************/
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

// Headers from Camera code
#include "tl_camera_sdk.h"
#include "tl_camera_sdk_load.h"

using namespace std;

int main(){
	/****************************************************Global Variables***************************************/
	//these variables are for the actuator. real units refer to millimeters. Device units are the 
	//smallest unit the device can move
	//unit type is set to position. 1 is velocity. 2 is acceleration
	int serialNo = 27260232;
	double stepSize = 0;
	int position = 0;
	double real_unit = 0;
	int device_unit = 0;
	int unitType = 0;
	int scanNo = 0;
	// identify and access device
	char testSerialNo[16];
	sprintf_s(testSerialNo, "%d", serialNo);
	int key = 0;
	//these give the messages for the movement of the actuator
	WORD messageType;
	WORD messageId;
	DWORD messageData;
	bool running = true;
	//define the keys for user input
#define KEY_LEFT 75
#define KEY_RIGHT 77
#define KEY_UP 72
#define KEY_DOWN 80
	cout << "Enter step size in nanometers: ";
	cin >> stepSize;
	cout << stepSize << endl;
	stepSize = stepSize / 1000000;
	device_unit = int(stepSize * 34555); //calculations take from the specifications website

	cout << "Number of Scans: ";
	cin >> scanNo;
	// Build list of connected device
	if (TLI_BuildDeviceList() == 0)
	{
		// get device list size 
		short n = TLI_GetDeviceListSize();
		// get KDC serial numbers
		char serialNos[100];
		TLI_GetDeviceListByTypeExt(serialNos, 100, 27);
	}
	// start the device polling at 200ms intervals
	CC_StartPolling(testSerialNo, 200);
	// open device
	if (CC_Open(testSerialNo) == 0)
	{
		Sleep(3000);
		// Home device
		CC_ClearMessageQueue(testSerialNo);
		CC_Home(testSerialNo);
		printf("Device %s homing\r\n", testSerialNo);
		// wait for completion
		CC_WaitForMessage(testSerialNo, &messageType, &messageId, &messageData);
		while (messageType != 2 || messageId != 0)
		{
			CC_WaitForMessage(testSerialNo, &messageType, &messageId, &messageData);
		}

		int width = 3648;
		int height = 100 * scanNo;
		ofstream frame;
		frame.open("specImage.pgm", ios::app);
		frame << "P2" << endl; // This is the type for netpbm called the "magic number". In this case, P2 corresponds to ASCII greyscale
		frame << width << " " << height << endl;
		frame << 65535 << endl; // This is the maximum pixel value
		frame.close();
		while (running) {
			//This will tell the actuator which way to move
			cout << "Hit the left or right arrow key to move the motor" << endl;
			cout << "Hit the up arrow key to end the program" << endl;
			cout << "Hit the down arrow key to home the device" << endl;
			//need to use getch twice. The second value is the key code
			_getch();
			switch ((key = _getch())) {
			case KEY_LEFT:
				cout << endl << "Left" << endl;  // key left
				CC_MoveRelative(testSerialNo, -1 * device_unit);
				// wait for completion
				CC_WaitForMessage(testSerialNo, &messageType, &messageId, &messageData);
				while (messageType != 2 || messageId != 1)
				{
					CC_WaitForMessage(testSerialNo, &messageType, &messageId, &messageData);
				}
				break;
			case KEY_RIGHT:
				cout << endl << "Right" << endl;  // key right
				CC_MoveRelative(testSerialNo, device_unit);
				// wait for completion
				CC_WaitForMessage(testSerialNo, &messageType, &messageId, &messageData);
				while (messageType != 2 || messageId != 1)
				{
					CC_WaitForMessage(testSerialNo, &messageType, &messageId, &messageData);
				}
				break;
			case KEY_UP:
				cout << "Ending Program" << endl;
				running = false;
				break;
			case KEY_DOWN:
				CC_Home(testSerialNo);
				printf("Device %s homing\r\n", testSerialNo);
				break;
			default:
				cout << endl << "null" << endl;  // not arrow
				break;
			}
			if (running) {




			}


		}
		// stop polling
		CC_StopPolling(testSerialNo);
		// close device
		CC_Close(testSerialNo);

	}
//Camera Code

    int err;
	err = tl_camera_sdk_dll_initialize();
    cout << err << endl;
    system("pause");
	tl_camera_open_sdk();
    cout << "Pass" << endl;
    system("pause");

	const int bufferLen = 256; // 256 bytes
	char serialNum[bufferLen];
	char* serialP = serialNum;


	tl_camera_discover_available_cameras(serialP,bufferLen);

	void *chP;
	tl_camera_open_camera(serialP, &chP);
	tl_camera_arm(chP, 2);
	tl_camera_issue_software_trigger(chP);


    //initialize frame variables
    unsigned short *image_buffer = 0;
    int frame_count = 0;
    unsigned char *metadata = 0;
    int metadata_size_in_bytes = 0;

    int width = 0;
    int height = 0;
    tl_camera_get_image_height(chP,&height);
    tl_camera_get_image_width(chP,&width);
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
    for(int i = 1; i <= height; i++){
    	for(int j = 1; j <= width; j++){
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

    

}




using namespace std;
/*
 Interfacing Code
 Authors: Zach A. Mitchell G. Matthew P. Evan J
 2/2/2022
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
#include <stdlib.h>
#include <conio.h>
#include <ctime> 

//Header from actuator code
#include "Thorlabs.MotionControl.KCube.DCServo.h"
#include "stdafx.h"

// Headers from spectrometer code
#include "visa.h"
#include "TLCCS.h"      // the device driver header

// Headers from Camera code
#include "tl_camera_sdk.h"
#include "tl_camera_sdk_load.h"

// Forward Declaration
int writeToFile(ViReal64 _VI_FAR wavedata[], ViReal64 _VI_FAR intensitydata[]);

//===========================================================================
// Globals
//===========================================================================

ViSession   instr = VI_NULL;                 // instrument handle
FILE* my_file = NULL;                    // file handlin

int main() {
	/****************************************************Global Variables***************************************/
	//these variables are for the actuator. real units refer to millimeters. Device units are the 
	//smallest unit the device can move
	//CCD Camera variables

	tl_camera_sdk_dll_initialize();
	tl_camera_open_sdk();

	const int bufferLen = 256; // 256 bytes
	char serialNum[bufferLen];
	char* serialP = serialNum;
	ofstream frame;

	void* chP = nullptr;
	

	//initialize frame variables
	unsigned short* image_buffer = 0;
	int frame_count = 0;
	unsigned char* metadata = 0;
	int metadata_size_in_bytes = 0;

	int width = 0;
	int height = 0;

	//variable for timestamp 
	time_t currTime;
	char* date;
	
	//actuator variables
	//unit type is set to position. 1 is velocity. 2 is acceleration
	int serialNo = 27260232;
	double stepSize = 0;
	int position = 0;
	double real_unit = 0;
	int Z812B_unit = 0;
	int unitType = 0;
	double initial_pos = 0;
	double final_pos = 0;
	int scan_count = 0;
	int counter = 0;
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
#define A_KEY 97
/*********************************Error Checking************************************************************/
/* This block scans for potential errors in the program and it's connection with external equipment. First it checks if the
spectrometer is connected properly. Second, the system checks for errors eith the TLLCC DLLS. Lastly, the code checks for any
iteration time errors.
If the scematic finds errors, the process will quit and return an error message. If no erros are found, the program will continue.
Parameters: sesn(), TLCCS_FIND_PATTERN(), &findList, &count, rscStr, VI_OFF, &instr
Returns: nothing || string
*/
	

	/***************************************CCD Camera****************************************/
	/* This block moves the actuator and records data from the spectrometer. The output is an abstract
	graphic and the number of times looped is determined by scan_count. First the user inputs starting and final positions
	in millimeters, and then the preferred stepsize in nanometers. The program homes the device and stores the
	user's specifications.
	Parameters: initial_pos, final_pos, stepsize, scan_count, device_unit, serialNo, testSerialNo, messageType, messageId,
				messageData, KEY_LEFT, KEY_RIGHT_ KEY_UP, KEY_DOWN, getTimeplz, instr
	Returns: int
	*/
	//sets up the cameras 
	tl_camera_discover_available_cameras(serialP, bufferLen);
	cout << "Enter starting position in millimeters: ";
	cin >> initial_pos;
	cout << "Enter ending position in millimeters: ";
	cin >> final_pos;
	cout << "Enter step size in nanometers: ";
	cin >> stepSize;
	//cout << stepSize << endl;
	stepSize = stepSize / 1000000;
	scan_count = (final_pos - initial_pos) / stepSize;
	cout << "Number of scans: " << scan_count << endl;
	Z812B_unit = int(stepSize * 34555); //calculations take from the specifications website
	initial_pos = initial_pos * 34555;  // converting the initial position in device units

	// Build list of connected device
	/*if (TLI_BuildDeviceList() == 0)
	{
		// get device list size 
		short n = TLI_GetDeviceListSize();
		// get KDC serial numbers
		char serialNos[100];
		TLI_GetDeviceListByTypeExt(serialNos, 100, 27);
	}
	// start the device polling at 200ms intervals
	CC_StartPolling(testSerialNo, 100);
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
		
		
		*/
		while (running) {
			//This will tell the actuator which way to move
			cout << "Hit the left or right arrow key to move the motor" << endl;
			cout << "Hit the up arrow key to end the program" << endl;
			cout << "Hit the down arrow key to home the device" << endl;
			//need to use getch twice. The second value is the key code
			_getch();
			switch ((key = _getch())) {
				/*
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
				*/
			case KEY_UP:
				cout << "Ending Program" << endl;
				running = false;
				break;
				/*case KEY_DOWN:
					CC_Home(testSerialNo);
					printf("Device %s homing\r\n", testSerialNo);
					break;*/
			case KEY_DOWN:

				/*cout << "Moving to starting position" << endl;
				CC_MoveRelative(testSerialNo, initial_pos);
				//wait for completion
				CC_WaitForMessage(testSerialNo, &messageType, &messageId, &messageData);
				while (messageType != 2 || messageId != 1)
				{
					CC_WaitForMessage(testSerialNo, &messageType, &messageId, &messageData);
				}*/
				while (counter < scan_count) {
					tl_camera_open_camera(serialP, &chP);
					tl_camera_arm(chP, 2);
					tl_camera_issue_software_trigger(chP);
					tl_camera_get_image_height(chP, &height);
					tl_camera_get_image_width(chP, &width);
					system("pause");
					/*CC_MoveRelative(testSerialNo, Z812B_unit);
					//wait for completion
					CC_WaitForMessage(testSerialNo, &messageType, &messageId, &messageData);
					while (messageType != 2 || messageId != 1)
					{
						CC_WaitForMessage(testSerialNo, &messageType, &messageId, &messageData);
					}
					
					// get actual position
					int pos = CC_GetPosition(testSerialNo);
					printf("Device %s moved to %d\r\n ", testSerialNo, pos);
					*/
					  // current date/time based on current system
					currTime = time(0);
   
					// convert now to string form
					date = ctime(&currTime);

					cout << "The local date and time is: " << date << endl;
					system("pause");
					tl_camera_get_pending_frame_or_null(chP, &image_buffer, &frame_count, &metadata, &metadata_size_in_bytes);
					cout << "image buffer " << image_buffer[1] << endl;
					system("pause");
					char filename[32];
					sprintf(filename, "zachtest%d.pgm", counter);
	
					system("pause");
					frame.open(filename);
					frame << "P2" << endl; // This is the type for netpbm called the "magic number". In this case, P2 corresponds to ASCII greyscale
					frame << width << " " << height << endl;
					frame << 1022 << endl; // This is the maximum pixel value
					for (int i = 0; i < height; i++) {
						for (int j = 0; j < width; j++) {
							frame << image_buffer[j+1+(i)*width] << " ";
						}
						frame << endl;
					}
					frame.close();
					//ends using the camera
					cout << "Images received! Closing camera...\n" << endl;
					tl_camera_disarm(chP);
					tl_camera_close_camera(chP);
					counter++;
					cout << "scan No : " << counter << endl;

				}
				break;

				//default:
					//cout << endl << "null" << endl;  // not arrow
					//break;
			}


		}
		
		tl_camera_close_sdk();
		tl_camera_sdk_dll_terminate();

		// stop polling
		//CC_StopPolling(testSerialNo);
		// close device
		//CC_Close(testSerialNo);

	}


	//return 0;
//}



/********************************Methods***********************************************************************/
/* This block records the wave and intensity data to the file in consecutive order.
Parameters :  ViReal64 _VI_FAR wavedata (array), ViReal64 _VI_FAR intensitydata (array)
Returns : nothing
*/




/***************************************Camera Code*****************************************/


/***************************************************************/

/***************************************************************/
// THIS PRECEDING PAUSE ALLOWS THE CAMERA TIME TO WRITE TO THE IMAGE BUFFER. A TIMER WOULD ALSO WORK.
/***************************************************************/





	

	/*******************************************************************************************/
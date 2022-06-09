using namespace std;
/*
 Interfacing Code
 Authors: Zach A. Mitchell G. Matthew P. Evan J
 1/18/2022
*/
/*********************** Header ***********************/
// General pre-existing headers 
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>
#include <cmath>
#include <cstdlib>
#include <time.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <ctime>
 
// The rest of these includes are user-defined headers from various devices

//Header for actuator 
#include "Thorlabs.MotionControl.KCube.DCServo.h"
#include "stdafx.h"

// Headers for spectrometer 
#include "visa.h"
#include "TLCCS.h"      // the device driver header

// Headers from CCD camera 
#include "tl_camera_sdk.h"
#include "tl_camera_sdk_load.h"

// Forward declaration of a method used to write spectrometer data to a file
int writeToFile(ViReal64 _VI_FAR wavedata[], ViReal64 _VI_FAR intensitydata[]);
 // Forward declaration of a method used to check actuator errors
int actuator_check(int actuator_err);
//Forward Declaration of function used to write to and frg file
int writeToFRG(int numScans, double lambdaCenter, double temporalCalibration, double spatialCalibration, int deltalambda, double stepSize);
//===========================================================================
// Globals      
//===========================================================================

ViSession   instr = VI_NULL;             // instrument's handle
FILE* my_file = NULL;                    // file handlin

int main() {
	/****************************************************Global Variables***************************************/
	// error variables for the various error checks throughout the code

	ViReal64	MY_INTEGRATION_TIME = 0.05;	// sets integration time for how long the spectrometer takes in data per scan. Can be changed as needed.
	ViUInt32    count = 0;                  // counts found devices
	ViFindList  findList;                   // this is the container for the handle identifying the search session
	ViStatus    err = VI_SUCCESS;			// error variable
	// ViStatus    err = true;					// error
	ViChar      rscStr[VI_FIND_BUFLEN];     // resource string
	ViSession   sesn;						// this will contain the resource manager session
	ViReal64    getTimeplz;					// confirms the integration time previously set by user
	ViReal64 _VI_FAR intensitydata[3648];   // intensity data array for spectrometer
	ViReal64 _VI_FAR wavedata[3648];		// wave data array for spectrometer
	ViInt16 dataSet = 0;
	ViPReal64 minwav = NULL;
	ViPReal64 maxwav = NULL;
	// these variables are for the actuator. real units refer to millimeters. device units are the smallest unit the actuator can move
	// unit type is set to position. 1 is velocity. 2 is acceleration
	int serialNo = 27260232; // serial number of the actuator 
	double stepSize = 0; // the rest of these variables are simply being initialzed for later use
	int position = 0;
	double real_unit = 0;
	int Z812B_unit = 0;
	int unitType = 0;
	double initial_pos = 0;
	double final_pos = 0;
	double deltalambda = 0;
	double centrallambda = 0;
	double numScans = 0;
	//spatial calibration is device specific. It is the range of wavelengths divided by the size of the array outputted by the spectrometer
	double spatialCalibration = 800.0 / 3648.0;
	double temporalCalibration = 0;
	/*************************************/
	int counter = 0;
	int actuator_err = 0; // for a later switch statement
	// identify and access actuator
	char testSerialNo[16];
	sprintf_s(testSerialNo, "%d", serialNo);
	int key = 0; // variable for a later switch statement 
	//these give the messages for the movement of the actuator
	WORD messageType;
	WORD messageId;
	DWORD messageData;
	bool running = true;
	bool satisfied = false;
	// define the keys for user input. numbers based on their ASCII decimal number
#define KEY_LEFT	75
#define KEY_RIGHT	77
#define KEY_UP		72
#define KEY_DOWN	80
#define A_KEY		97
#define M_KEY		109
#define C_Key       99
#define X_Key       120
/*********************************Error Checking************************************************************/
/* This block scans for potential errors in the program and it's connection with external equipment. First it checks if the
spectrometer is connected properly. Second, the system checks for errors eith the TLLCC DLLS. Lastly, the code checks for any
iteration time errors.
If the scematic finds errors, the process will quit and return an error message. If no erros are found, the program will continue.
Parameters: sesn(), TLCCS_FIND_PATTERN(), &findList, &count, rscStr, VI_OFF, &instr
Returns: nothing || string
*/
	viOpenDefaultRM(&sesn);					// This gets the resource manager session handle. The & symbol directs the compiler to the memory location of sesn.

	err = viFindRsrc(sesn, TLCCS_FIND_PATTERN, &findList, &count, rscStr);
	if (err) {
		cout << "error with viFindRsrc \n"; system("pause"); //exit(1);
	}
	err = tlccs_init(rscStr, VI_OFF, VI_OFF, &instr);
	if (err) {
		cout << "error with tlccs_init \n"; system("pause"); //exit(1);
	}
	err = tlccs_setIntegrationTime(instr, MY_INTEGRATION_TIME);
	if (err) {
		cout << "error with setIntegrationTime \n"; system("pause"); //exit(1);
	}
	// more error codes for the actuator 
	if (CC_CheckConnection(testSerialNo) != 0) {
		cout << "error with actuator connection  \n"; system("pause"); //exit(1);
	}
	/***************************************Spectrometer****************************************/
	/* This block moves the actuator and records data from the spectrometer. The output is an abstract
	graphic and the number of times looped is determined by scan_count. First the user inputs starting and final positions
	in millimeters, and then the preferred stepsize in nanometers. The program homes the actuator and stores the
	user's specifications.
	Parameters: initial_pos, final_pos, stepsize, scan_count, device_unit, serialNo, testSerialNo, messageType, messageId,
				messageData, KEY_LEFT, KEY_RIGHT_ KEY_UP, KEY_DOWN, getTimeplz, instr
	Returns: int
	*/
	
	// this switch statement is for the user to either press a for automatic interferometry or m for manuel interferometry
	
	cout << "Press the 'a' key for automatic interferometry. Press the 'm' key for manuel interferometry." << endl;
	switch ((key = _getch())) 
	{
	case A_KEY: // this series of cin and couts are for the user to input their needed range of spectra
		cout << "You are now in Automatic Mode" << endl;
		cout << endl;
		while (!satisfied) {
			cout << "Please enter the number of scans you would like to take in powers of 2. \n Note that the wavelength range taken will ~.11*numScans" << endl;
			cin >> numScans;
			cout << endl;
			cout << "Enter starting position in millimeters: " << endl;;
			cin >> initial_pos;
			cout << endl;
			cout << "Enter ending position in millimeters: " << endl;
			cin >> final_pos;
			cout << endl;
			cout << "Enter the central wavelength in nanometers: " << endl;
			cin >> centrallambda;
			cout << endl;
			stepSize = (final_pos - initial_pos) / numScans;
			deltalambda = (numScans * spatialCalibration) / 2;
			temporalCalibration = (stepSize / 1000) / 299792458;
			stepSize = stepSize / 1000;
			cout << "Step Size: " << stepSize << endl;
			cout << "Wavelength Range: " << deltalambda << endl;
			cout << "Number of scans: " << numScans << endl;
			cout << endl;
			cout << "Press C to confirm these are the values you want. Press X to enter them again " << endl;
			cout << endl;

			switch ((key = _getch())) {

			case C_Key:
				satisfied = TRUE;
				break;
			case X_Key:
				satisfied = FALSE;
				break;
			default:
				break;
			}
			cout << endl;
		}
		cout << "Confirmed" << endl;
		Z812B_unit = int(stepSize * 34555); //calculations take from the specifications website
		initial_pos = initial_pos * 34555;  // converting the initial position in device units

		actuator_err = TLI_BuildDeviceList(); //     NEW STUFF
		if (actuator_err == 0) // This function builds an internal collection of all devices found on the USB that are not currently open
		{
			// get serial number of actuator attached to KDC 
			short n = TLI_GetDeviceListSize();
			// get KDC serial numbers
			char serialNos[100];
			TLI_GetDeviceListByTypeExt(serialNos, 100, 27);
		}
		else {
			actuator_check(actuator_err);
		}
		// start the KDC polling at 200ms intervals
		CC_StartPolling(testSerialNo, 100);
		// opens the actuator for communication
		actuator_err = CC_Open(testSerialNo);
		if (actuator_err == 0)
		{
			Sleep(3000); // this pauses the running code for 3000ms
			// Homes the actuator
			CC_ClearMessageQueue(testSerialNo);
			CC_Home(testSerialNo);
			printf("Actuator %s homing\r\n", testSerialNo);
			// must wait for the homing to be complete before moving onward
			CC_WaitForMessage(testSerialNo, &messageType, &messageId, &messageData);
			while (messageType != 2 || messageId != 0)
			{
				CC_WaitForMessage(testSerialNo, &messageType, &messageId, &messageData);
			}
			/* initializing the image f+or that the width is fixed at 3648 pixels, and each scan adds 25 pixels of
			length to the image. The image size is a function of the scan count*/
			int width = 3648;
			int height = numScans;
			ofstream frame;
			frame.open("specImage.pgm", ios::app);
			frame << "P2" << endl; // This is the type for netpbm called the "magic number". In this case, P2 corresponds to ASCII greyscale
			frame << width << " " << height << endl;
			frame << 65535 << endl; // This is the maximum pixel value
			frame.close();

			cout << "Moving to starting position" << endl;
			CC_MoveRelative(testSerialNo, initial_pos);
			//wait for completion
			CC_WaitForMessage(testSerialNo, &messageType, &messageId, &messageData);
			while (messageType != 2 || messageId != 1)
			{
				CC_WaitForMessage(testSerialNo, &messageType, &messageId, &messageData);
			}
			while (counter < numScans)
			{
				CC_MoveRelative(testSerialNo, Z812B_unit);
				//wait for completion
				CC_WaitForMessage(testSerialNo, &messageType, &messageId, &messageData);
				while (messageType != 2 || messageId != 1)
				{
					CC_WaitForMessage(testSerialNo, &messageType, &messageId, &messageData);
				}
				//spectrometer commands
				tlccs_getIntegrationTime(instr, &getTimeplz); // This gets and outputs the the integration time we just input
				//triggers CCS to take a single scan
				tlccs_startScan(instr);
				//gets intensity data
				tlccs_getScanData(instr, intensitydata);
				//gets wave data
				tlccs_getWavelengthData(instr, dataSet, wavedata, minwav, maxwav);

				writeToFile(wavedata, intensitydata);

				frame.open("specImage.pgm", ios::app);
				for (int i = 0; i < 1; i++)
				{
					for (int j = 0; j < width; j++)
					{
						frame << intensitydata[j] * 1000000 << " ";
					}
					frame << endl;
				}
				frame.close();
				// get actual position
				int pos = CC_GetPosition(testSerialNo);
				printf("Device %s moved to %d\r\n", testSerialNo, pos);

				counter++;
				cout << counter << endl;

			}
			//This rewrites the pgm file as an frg file
			writeToFRG(numScans,centrallambda,temporalCalibration, spatialCalibration, deltalambda, stepSize);
		}
		else{
			actuator_check(actuator_err);
		}
		break;

	case M_KEY:; // this is now the code for manuel mode
		cout << "You are now in Manual Mode" << endl; 
		cout << "Enter step size in nanometers: "; 
		cin >> stepSize; 
		cout << "Enter how many scans are needed? ";
		cin >> numScans;
		stepSize = stepSize / 1000000; // converting the entered stepSize into nm
		Z812B_unit = int(stepSize * 34555); //calculations take from the specifications website

		// Build list of connected devices
		actuator_err = TLI_BuildDeviceList();
		if (actuator_err == 0) // so if all the devices connected via USB are open...
		{
			// get device list size 
			short n = TLI_GetDeviceListSize();
			// get KDC serial numbers
			char serialNos[100];
			TLI_GetDeviceListByTypeExt(serialNos, 100, 27);
		}
		else {
			actuator_check(actuator_err);
		}
		// start the actuator polling at 100ms intervals
		CC_StartPolling(testSerialNo, 100);
		// open actuator
		actuator_err = CC_Open(testSerialNo);
		if (actuator_err == 0)
		{
			Sleep(3000);
			// Home actuator to positon 0
			CC_ClearMessageQueue(testSerialNo);
			CC_Home(testSerialNo);
			printf("Actuator %s homing\r\n", testSerialNo);
			// waiting for home completion, then waiting for next command
			CC_WaitForMessage(testSerialNo, &messageType, &messageId, &messageData);
			while (messageType != 2 || messageId != 0)
			{
				CC_WaitForMessage(testSerialNo, &messageType, &messageId, &messageData);
			}

			int width = 3648;
			int height = 25 * numScans;
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
				cout << "Hit the down arrow key to home the actuator" << endl;
				//need to use getch twice. The second value is the key code
				_getch();
				switch ((key = _getch())) {

				case KEY_LEFT:
					cout << endl << "Left" << endl;  // key left
					CC_MoveRelative(testSerialNo, -1 * Z812B_unit);
					// wait for completion
					CC_WaitForMessage(testSerialNo, &messageType, &messageId, &messageData);
					while (messageType != 2 || messageId != 1)
					{
						CC_WaitForMessage(testSerialNo, &messageType, &messageId, &messageData);
					}
					break;
				case KEY_RIGHT:
					cout << endl << "Right" << endl;  // key right
					CC_MoveRelative(testSerialNo, Z812B_unit);
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
					printf("Actuator %s homing\r\n", testSerialNo);
					break;
				default:
					cout << endl << "null" << endl;  // not arrow
					break;
				}

				if (running) {

					tlccs_getIntegrationTime(instr, &getTimeplz); // This gets and outputs the the integration time we just input
					//triggers CCS to take a single scan
					tlccs_startScan(instr);
					//gets intensity data
					tlccs_getScanData(instr, intensitydata);
					//gets wave data
					tlccs_getWavelengthData(instr, dataSet, wavedata, minwav, maxwav);
					writeToFile(wavedata, intensitydata);


					frame.open("specImage.pgm", ios::app);
					for (int i = 0; i < 25; i++) {
						for (int j = 0; j < width; j++) {
							frame << intensitydata[j] * 1000000 << " ";
						}
						frame << endl;
					}

					frame.close();
					// get actual position
					int pos = CC_GetPosition(testSerialNo);
					printf("Actuator %s moved to %d\r\n", testSerialNo, pos);
					counter++ ;
					Sleep(1000);
					cout << counter << endl;
				}


			}
			// stop polling
			CC_StopPolling(testSerialNo);
			// close actuator
			CC_Close(testSerialNo);
		}
		else{
		actuator_check(actuator_err);
		}
		break;
	}
	return 0;
}



/********************************Methods***********************************************************************/
/* This block records the wave and intensity data to the file in consecutive order. 
Parameters :  ViReal64 _VI_FAR wavedata (array), ViReal64 _VI_FAR intensitydata (array)
Returns : nothing
*/
int writeToFile(ViReal64 _VI_FAR wavedata[], ViReal64 _VI_FAR intensitydata[]) {
	ofstream MyFile;
	//opens the file as appending
	MyFile.open("spec_file.txt", ios::app);
	//runs through the passed arrays which should be 3648 data points each. 
	for (int index = 0; index < 3648; index++) {
		MyFile << wavedata[index] << " " << intensitydata[index] << endl;
	}
	MyFile.close();
	return 0;
}
//This will read from a pgm file and rewrite it to an FRG file to be used with the Frog code. It will also transpose the matrix elements
//lambdaCenter is the central wavelength of spectrometer. Temporal calibration is the stepsize/c. Spatialcalibration is lambda_max-lambda_min/3648
int writeToFRG(int numScans, double lambdaCenter, double temporalCalibration, double spatialCalibration, int deltalambda, double stepSize) {

	int lambdaCeneterPos = ((lambdaCenter - 200) / spatialCalibration) + 12;
	int height = numScans;
	int width = height;
	
	vector<vector<double>> interArray(height);
	//resizing the array
	/*for (int k = 0; k < height; k++) {
		interArray[k].resize(height);
	} */
	//creates a vector array
	vector<vector<double>> data;
	//opens the pgm file to read it
	ifstream file("specImage.pgm");
	string line;
	//skips first 3 lines of pgm
	for (int x = 0; x < 3; x++)
		getline(file, line);
	//reads from the file and inputs it into a 2D vector array
	while (getline(file, line))
	{
		vector<double>   lineData;
		stringstream  lineStream(line);

		double value;
		// Read an integer at a time from the line
		while (lineStream >> value)
		{
			// Add the integers from a line to a 1D array (vector)
			lineData.push_back(value);
		}
		// When all the integers have been read, add the 1D array
		// into a 2D array (as one line in the 2D array)
		data.push_back(lineData);

		//we create 25 lines in png file as copies. This only takes one of those.
		/*for(int x = 0; x<25; x++) {
			getline(file,line);
		}*/
	}

	//writes the data to an intermidiary array that is square.
	
	for (int i = 0; i < data[0].size(); i++) {
		for (int j = lambdaCenter - numScans / 2.0; j < lambdaCenter + numScans / 2.0 && j < data.size(); j++) {
			interArray[i].push_back(data[i][j]);
		}
	}
	
	//This section will write to a frg file while transposing the matrix
	ofstream MyFile;
	MyFile.open("frogfile.frg", ios::app);
	MyFile << width << " " << height << " " << temporalCalibration << " " << spatialCalibration << " " << lambdaCenter << endl;
	for (int i = 0; i < numScans; i++) {
		for (int j = 0; j < numScans; j++) {
			MyFile << interArray[j][i] << " ";
			MyFile << endl;
		}
	}
	MyFile.close();

	return 0;
}
/* This block is the error check code for the actuator. Prints the specific error to the screen based on the error code.
* Parameters : The error variable actuator_err
* Returns : Nothing
*/

int actuator_check(int actuator_err) {
		
		switch (actuator_err)
		{
		case 1:
			cout << "The FTDI functions have not been initialized." << endl; system("pause"); exit(1);
			break;
		case 2:
			cout << "The actuator driver ( could not be found. Check USB connection." << endl; system("pause"); exit(1);
			break;
		case 4:
			cout << "There is an error with the physical FTDI actuator chip." << endl; system("pause"); exit(1);
			break;
		case 5:
			cout << "There are insufficient resources to run this application. Try a different PC." << endl; system("pause"); exit(1);
			break;
		case 6:
			cout << "An invalid parameter was given." << endl; system("pause"); exit(1);
			break;
		case 7:
			cout << "The actuator has been disconnected." << endl; system("pause"); exit(1);
			break;
		case 8:
			cout << "The actuator that is detected by our system is not the correct device that can function with this application." << endl; system("pause"); exit(1);
			break;
		case 33:
			cout << "The actuator has stopped responding." << endl; system("pause"); exit(1);
			break;
		case 36:
				cout << "The function could not be completed because the actuator is disconnected." << endl; system("pause"); exit(1);
			break;
		case 41:
			cout << "There is a problem with the driver firmware. Big problem." << endl; system("pause"); exit(1);
			break;
		case 38:
			cout << "The function cannot be performed as it would result in an illegal position." << endl; system("pause"); exit(1);
			break;
		default:
			cout << "There is not a genertic error. Please visit the API for Thorlabs KDC101 within your files (downloaded with KDC101 software). Under the function TLI_BuildDeviceList, check for the following error:" << actuator_err << endl; system("pause"); exit(1);
		}
		return 0;
}



/***************************************Camera Code*****************************************/
/*

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
/* system("pause");
/***************************************************************/
// THIS PRECEDING PAUSE ALLOWS THE CAMERA TIME TO WRITE TO THE IMAGE BUFFER. A TIMER WOULD ALSO WORK.
/***************************************************************/
/*
tl_camera_get_pending_frame_or_null(chP, &image_buffer, &frame_count, &metadata, &metadata_size_in_bytes);

// Making a PGM image that can be opened with image processors like GIMP
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

	/******************************************************************************************/
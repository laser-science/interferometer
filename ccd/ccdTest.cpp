/*
 * ccdTest.cpp
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
#include "tl_camera_sdk.h"
#include "tl_camera_sdk_load.h"

using namespace std;

int main(){
    int err;
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




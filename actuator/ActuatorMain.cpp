// Example_KDC101.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <stdlib.h>
#include <conio.h>

#if defined TestCode
#include "..\..\..\Instruments\ThorLabs.KCube.DCServo\ThorLabs.KCube.DCServo\Thorlabs.MotionControl.KCube.DCServo.h"
#else
#include "Thorlabs.MotionControl.KCube.DCServo.h"
#endif

int __cdecl wmain(int argc, wchar_t* argv[])
{
    if (argc < 1)
    {
        printf("Usage = Example_KDC101 [serial_no] [position: optional (0 - 1715200)] [velocity: optional (0 - 3838091)]\r\n");
        char c = _getch();
        return 1;
    }

    int serialNo = 27260232;
    if (argc > 1)
    {
        serialNo = _wtoi(argv[1]);
    }

    // get parameters from command line
    int position = 0;
    if (argc > 2)
    {
        position = _wtoi(argv[2]);
    }

    int velocity = 0;
    if (argc > 3)
    {
        velocity = _wtoi(argv[3]);
    }

    // identify and access device
    char testSerialNo[16];
    sprintf_s(testSerialNo, "%d", serialNo);

    // Build list of connected device
    if (TLI_BuildDeviceList() == 0)
    {
        // get device list size 
        short n = TLI_GetDeviceListSize();
        // get KDC serial numbers
        char serialNos[100];
        TLI_GetDeviceListByTypeExt(serialNos, 100, 27);

        // output list of matching devices
        {
            char* searchContext = nullptr;
            char* p = strtok_s(serialNos, ",", &searchContext);

            while (p != nullptr)
            {
                TLI_DeviceInfo deviceInfo;
                // get device info from device
                TLI_GetDeviceInfo(p, &deviceInfo);
                // get strings from device info structure
                char desc[65];
                strncpy_s(desc, deviceInfo.description, 64);
                desc[64] = '\0';
                char serialNo[9];
                strncpy_s(serialNo, deviceInfo.serialNo, 8);
                serialNo[8] = '\0';
                // output
                printf("Found Device %s=%s : %s\r\n", p, serialNo, desc);
                p = strtok_s(nullptr, ",", &searchContext);
            }
        }

        // open device
        if (CC_Open(testSerialNo) == 0)
        {
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

            // set velocity if desired
            if (velocity > 0)
            {
                int currentVelocity, currentAcceleration;
                CC_GetVelParams(testSerialNo, &currentAcceleration, &currentVelocity);
                CC_SetVelParams(testSerialNo, currentAcceleration, velocity);
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

            // get actual poaition
            int pos = CC_GetPosition(testSerialNo);
            printf("Device %s moved to %d\r\n", testSerialNo, pos);

            // stop polling
            CC_StopPolling(testSerialNo);
            // close device
            CC_Close(testSerialNo);
        }
    }

    char c = _getch();
    return 0;
}






















/*
 * kdc101main.cpp
 *
 *  Created on: Jan 4, 2022
 *      Author: Evan
 */

 /*********************** Header ***********************/
/**
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

#include "Thorlabs.MotionControl.KCube.DCServo.h"
    
using namespace std;

int main()
{
    int deviceID = 0;
    //vector<string> serialNumba = 27260232;
    int err = TLI_BuildDeviceList();
    cout << err << endl;
    system("pause");
    // get device list size 
    short n = TLI_GetDeviceListSize();
    cout << n << endl;
    system("pause");

    // trying to open communication with the device
    char serialNumba = '1';
    if (CC_Open(&serialNumba) == 0)
    {
        cout << "Device is open" << endl;
    }
    // Checking the connection to the device 
    if (CC_CheckConnection(&serialNumba) == true)
    {
        cout << "Device is connected" << endl;
    }
    // Check if we can perform a home
    if (CC_CanHome(&serialNumba) == true)
    {
        cout << "The device can perform a home" << endl;
    }

    // setting the relative distance movement
    int distance = 10000;
    err = CC_SetMoveRelativeDistance(&serialNumba, distance);
    cout << err << endl;
    CC_MoveRelativeDistance(&serialNumba);
    //cout << CC_GetHomingVelocity(&serialNumba) << endl;
   // CC_GetHomingParamsBlock(&serialNumba, MOT_HomingParameters *homingParams);

    cout << CC_Home(&serialNumba) << endl;
    system("pause");

    int device_unit = 0;
    int unitType = 0;
    double real_unit = 6;

    CC_GetDeviceUnitFromRealValue(&serialNumba, real_unit, &device_unit, unitType);
    cout << device_unit << endl;
    system("pause");
    // CC_MoveToPosition(&serialNumba, device_unit);

    return 0;
    
} */
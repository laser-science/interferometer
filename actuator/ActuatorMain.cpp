/*
 * kdc101main.cpp
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
    
}
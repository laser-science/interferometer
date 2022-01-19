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

#include "Thorlabs.MotionControl.KCube.DCServo.h"

using namespace std;

int main()
{
    int deviceID = 0;
    //vector<string> serialNumba;
    TLI_BuildDeviceList();
    // get device list size 
    short n = TLI_GetDeviceListSize();
    cout << n << endl;
    system("pause");


    const char serialNumba = 27260232;
    CC_Open(&serialNumba);
    CC_Home(&serialNumba);
    //if (CC_CanMoveWithoutHomingFirst(&serialNumba)) {
    //    cout << "yay";
    //system("pause");
    cout << CC_GetHomingVelocity(&serialNumba) << endl;
    // CC_RequestHomingParams(&serialNumba);

     //CC_GetHomingParamsBlock(&serialNumba)

    cout << CC_CanHome(&serialNumba) << endl;
    system("pause");

    int device_unit = 0;
    int unitType = 0;
    double real_unit = 1;

    CC_GetDeviceUnitFromRealValue(&serialNumba, real_unit, &device_unit, unitType);
    cout << device_unit << endl;
    system("pause");
    CC_MoveToPosition(&serialNumba, device_unit);
    return 0;

}


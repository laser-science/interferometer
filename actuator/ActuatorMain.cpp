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
    vector<string> serialNumba;
    TLI_BuildDeviceList();
    // get device list size 
    short n = TLI_GetDeviceListSize();
	cout << n << endl;
	system("pause");
	return 0;
}


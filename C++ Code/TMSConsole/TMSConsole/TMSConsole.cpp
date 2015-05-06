// TMSConsole.cpp : Defines the entry point for the console application.
//
#define ARMA_DONT_USE_BLAS
#define M_PI 3.14159265358979323846  /* pi */
#include "stdafx.h"
#include "OPCFunctions.h"
#include <iostream>
#include <string>
#include <stdint.h>
#include <thread>
#include <iostream>
#include <armadillo>
#include <windows.h>
#include <time.h> 
#include <mutex>

using namespace std;
using namespace arma;

//BrainSight tracker signals in 4x4 position matrix with coordinates in the last column
double brainSightST[16];
double brainSightP[16];
double brainSightCT[16];
double brainSightLCT[16];
double brainSightTT[16];
double brainSightCB[16];
double brainSightSRMatrix[16];
int const dataLength = 6;
int32_t dataToRead[dataLength] = { 0 };
double curPos[dataLength] = { 0.0 };
char *charCurPosURL;
char *charPR2URL;
mutex m;
//Contains the timestamp of the Brainsight output file.
struct tm lastUpdate;
time_t lastUpdateTime;

//Status of data parsed from BrainSight. 1 signifies the value is up-to-date with lastUpdate timestamp, 0 signifies out of date value
//Variable may fall out of date if tracker is not visible by Polaris camera or last update is too old
int STStatus = 0;
int PStatus = 0;
int CTStatus = 0;
int LCTStatus = 0;
int TTStatus = 0;
int CBStatus = 0;
int SRStatus = 0;

//Wrapper function for LabView OPCRead
void ReadOPC(char *OPCURL, int32_t *OPCData, int32_t len)
{
	OPCRead(OPCURL, OPCData, len);
}

// absolute orientation method adapted from a matlab program (absoluteOrientationQuarternion.m) copyright ETH Zurich, Computer Vision Laboratory, Switzerland
// http://www.mathworks.com/matlabcentral/fileexchange/22422-absolute-orientation
mat transformationComputation(vector<double*> aPoints, vector<double*> bPoints)
{
	int transformationMatrixSize = 4;
	int i;
	int j;
	
	// take the mean across x's, y's and z's for robot coords and cam coords
	double cA [3];
	double cB [3];
	for (i = 0; i < 3; i++) {
		cA[i] = ((aPoints.at(0)[i] + aPoints.at(1)[i] + aPoints.at(2)[i] + aPoints.at(3)[i]) / 4);
		cB[i] = ((bPoints.at(0)[i] + bPoints.at(1)[i] + bPoints.at(2)[i] + bPoints.at(3)[i]) / 4);
	}

	// remove the centroid from the collection of robot and camera points
	double aVectorsNoCentroids[3][4];
	double bVectorsNoCentroids[3][4];
	for (i = 0; i < 3; i++) {
		for (j = 0; j < 4; j++) {
			aVectorsNoCentroids[i][j] = aPoints.at(j)[i] - cA[i];
			bVectorsNoCentroids[i][j] = bPoints.at(j)[i] - cB[i];
		}
	}
	
	// compute the quarternions for robot and camera matrices
	mat preliminaryTransformation;
	preliminaryTransformation	<< 0 << 0 << 0 << 0 << endr
								<< 0 << 0 << 0 << 0 << endr
								<< 0 << 0 << 0 << 0 << endr
								<< 0 << 0 << 0 << 0 << endr;
	for (i = 0; i < 4; i++) {
		mat quarternionAMatrix;
		mat quarternionBMatrix;

		quarternionAMatrix	<< 0 << -1 * aVectorsNoCentroids[0][i] << -1 * aVectorsNoCentroids[1][i] << -1 * aVectorsNoCentroids[2][i] << endr
								<< aVectorsNoCentroids[0][i] << 0 << aVectorsNoCentroids[2][i] << -1 * aVectorsNoCentroids[1][i] << endr
								<< aVectorsNoCentroids[1][i] << -1 * aVectorsNoCentroids[2][i] << 0 << aVectorsNoCentroids[0][i] << endr
								<< aVectorsNoCentroids[2][i] << aVectorsNoCentroids[1][i] << -1 * aVectorsNoCentroids[0][i] << 0 << endr;

		quarternionBMatrix	<< 0 << -1 * bVectorsNoCentroids[0][i] << -1 * bVectorsNoCentroids[1][i] << -1 * bVectorsNoCentroids[2][i] << endr
								<< bVectorsNoCentroids[0][i] << 0 << -1 * bVectorsNoCentroids[2][i] << bVectorsNoCentroids[1][i] << endr
								<< bVectorsNoCentroids[1][i] << bVectorsNoCentroids[2][i] << 0 << -1 * bVectorsNoCentroids[0][i] << endr
								<< bVectorsNoCentroids[2][i] << -1 * bVectorsNoCentroids[1][i] << bVectorsNoCentroids[0][i] << 0 << endr;
		
		quarternionAMatrix = trans(quarternionAMatrix);
		preliminaryTransformation = preliminaryTransformation + (quarternionAMatrix*quarternionBMatrix);
	}

	// get the eigenvalues for further calculation
	cx_vec eigval;
	cx_mat eigvec;
	eig_gen(eigval, eigvec, preliminaryTransformation);

	// calculate the rotational matrix
	cx_mat rotationalMatrix;
	cx_mat rotationalAMatrix;
	cx_mat rotationalBMatrix;

	rotationalAMatrix << eigvec.at(0, 0) << -1.0 * eigvec.at(1, 0) << -1.0 * eigvec.at(2, 0) << -1.0 * eigvec.at(3, 0) << endr
						<< eigvec.at(1, 0) << eigvec.at(0, 0) << eigvec.at(3, 0) << -1.0 * eigvec.at(2, 0) << endr
						<< eigvec.at(2, 0) << -1.0 * eigvec.at(3, 0) << eigvec.at(0, 0) << eigvec.at(1, 0) << endr
						<< eigvec.at(3, 0) << eigvec.at(2, 0) << -1.0 * eigvec.at(1, 0) << eigvec.at(0, 0) << endr;

	rotationalBMatrix << eigvec.at(0, 0) << -1.0 * eigvec.at(1, 0) << -1.0 * eigvec.at(2, 0) << -1.0 * eigvec.at(3, 0) << endr
						<< eigvec.at(1, 0) << eigvec.at(0, 0) << -1.0 * eigvec.at(3, 0) << eigvec.at(2, 0) << endr
						<< eigvec.at(2, 0) << eigvec.at(3, 0) << eigvec.at(0, 0) << -1.0 * eigvec.at(1, 0) << endr
						<< eigvec.at(3, 0) << -1.0 * eigvec.at(2, 0) << eigvec.at(1, 0) << eigvec.at(0, 0) << endr;

	rotationalAMatrix = trans(rotationalAMatrix);
	rotationalMatrix = rotationalAMatrix * rotationalBMatrix;

	// retrieve the important portion of the rotational matrix
	cx_mat preliminaryRotational;
	mat reducedRotational;
	preliminaryRotational	<< rotationalMatrix.at(1, 1) << rotationalMatrix.at(1, 2) << rotationalMatrix.at(1, 3) << endr
							<< rotationalMatrix.at(2, 1) << rotationalMatrix.at(2, 2) << rotationalMatrix.at(2, 3) << endr
							<< rotationalMatrix.at(3, 1) << rotationalMatrix.at(3, 2) << rotationalMatrix.at(3, 3) << endr;
	reducedRotational = conv_to<mat>::from(preliminaryRotational);

	// convert means into matrices
	mat cAMatrix;
	mat cBMatrix;
	cAMatrix << cA[0] << endr << cA[1] << endr << cA[2] << endr;
	cBMatrix << cB[0] << endr << cB[1] << endr << cB[2] << endr;
 
	// retrieve the translational portion
	mat translational = cBMatrix - (reducedRotational * cAMatrix);

	// setup the returned transformation matrix
	mat transformationMatrix(4, 4);

	transformationMatrix(0, 0) = reducedRotational(0, 0);
	transformationMatrix(0, 1) = reducedRotational(0, 1);
	transformationMatrix(0, 2) = reducedRotational(0, 2);
	transformationMatrix(1, 0) = reducedRotational(1, 0);
	transformationMatrix(1, 1) = reducedRotational(1, 1);
	transformationMatrix(1, 2) = reducedRotational(1, 2);
	transformationMatrix(2, 0) = reducedRotational(2, 0);
	transformationMatrix(2, 1) = reducedRotational(2, 1);
	transformationMatrix(2, 2) = reducedRotational(2, 2);
	transformationMatrix(0, 3) = translational(0, 0);
	transformationMatrix(1, 3) = translational(1, 0);
	transformationMatrix(2, 3) = translational(2, 0);
	transformationMatrix(3, 0) = 0;
	transformationMatrix(3, 1) = 0;
	transformationMatrix(3, 2) = 0;
	transformationMatrix(3, 3) = 1;

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			printf("%f ", transformationMatrix(i, j));
		}
		printf("\n");
	}

	return transformationMatrix;
}

//Given a string, returns equivalent pointer to char
char* stringToChar(string str)
{
	const char *constCharStr = str.c_str();
	char *charStr = new char[str.length() + 1];
	strcpy(charStr, constCharStr);
	charStr[str.length()] = '\0';
	return charStr;
}

//This function parses Brainsight output file for tracker coordinates and updates global variables accordingly given Brainsight log output file path
void parseBrainsight(string logPath)
{
	ifstream filestream;
	time_t now;
	time_t old;
	time(&now);
	time(&old);
	double timeDifference;
	double period = 0.4; //Brainsight parsing period length. Lower this variable to increase output.txt parsing speed
	int hh, mm, ss;
	int year, date, month;
	string line;
	stringstream linestream;
	string IDTag; //holds IDtag of line parsed from brainsight (e.g. timestamp, subject tracker)
	string token;
	while (true){
		time(&now);
		timeDifference = difftime(now, old);
		if (timeDifference < period) continue;
		else{
			time(&old);
			filestream.open(logPath);
			while (filestream.good())
			{
				m.lock();
				while (getline(filestream, line))
				{
					linestream << line;
					//sample line : <timestamp> 2015-02-27 14:01:03.353
					//get identifying tag value
					//note that getline function sets the value of token to the string until the delimiter character is reached
					getline(linestream, token, '<');
					getline(linestream, token, '>');
					IDTag = token;
					getline(linestream, token, ' '); //removes leading blank space;
					int i = 0;

					if (IDTag == "timestamp"){
						getline(linestream, token, ' '); //get date
						sscanf(token.c_str(), "%d-%d-%d", &year, &month, &date);
						lastUpdate.tm_year = year - 1900;
						lastUpdate.tm_mon = month - 1;
						lastUpdate.tm_mday = date;
						getline(linestream, token, ' '); //get time
						sscanf(token.c_str(), "%d:%d:%d", &hh, &mm, &ss);
						lastUpdate.tm_hour = hh;
						lastUpdate.tm_min = mm;
						lastUpdate.tm_sec = ss;
						lastUpdateTime = mktime(&lastUpdate);
						//stringstream strings;
						//strings << lastUpdateTime;
						//cout << strings.str() << "\n";
						//printf("Current local time and date: %s", asctime(&lastUpdate));
					}
					else if (IDTag == "tracker ST"){
						while (getline(linestream, token, ' ')){
							if (token != "none"){
								brainSightST[i] = stod(token);
								STStatus = 1;
								i++;
							}
							else{
								STStatus = 0;
							}
						}
					}
					else if (IDTag == "tracker P"){
						while (getline(linestream, token, ' ')){
							if (token != "none"){
								brainSightP[i] = stod(token);
								PStatus = 1;
								i++;
							}
							else{
								PStatus = 0;
							}
						}
					}
					else if (IDTag == "tracker CT"){
						while (getline(linestream, token, ' ')){
							if (token != "none"){
								brainSightCT[i] = stod(token);
								CTStatus = 1;
								i++;
							}
							else{
								CTStatus = 0;
							}
						}
					}
					else if (IDTag == "tracker LCT"){
						while (getline(linestream, token, ' ')){
							if (token != "none"){
								brainSightLCT[i] = stod(token);
								LCTStatus = 1;
								i++;
							}
							else{
								LCTStatus = 0;
							}
						}
					}
					else if (IDTag == "tracker TT"){
						while (getline(linestream, token, ' ')){
							if (token != "none"){
								brainSightTT[i] = stod(token);
								TTStatus = 1;
								i++;
							}
							else{
								TTStatus = 0;
							}
						}
					}
					else if (IDTag == "tracker CB"){
						while (getline(linestream, token, ' ')){
							if (token != "none"){
								brainSightCB[i] = stod(token);
								CBStatus = 1;
								i++;
							}
							else{
								CBStatus = 0;
							}
						}
					}
					else if (IDTag == "subject registration"){
						while (getline(linestream, token, ' ')){
							if (token != "none"){
								brainSightSRMatrix[i] = stod(token);
								SRStatus = 1;
								i++;
							}
							else{
								SRStatus = 0;
							}
						}
					}
					linestream.clear();
				}
			}
			filestream.close();
			filestream.clear();
			m.unlock();
		}
	}
}

// used to pull the camera points and robot points, calls a calibration routine and returns the resulting matrix
mat calibrationRoutine()
{
	int i = 0;
	int j = 0;
	char c;
	
	// distinction for robotic control reasons
	// points multiplied by a factor of 1000 to keep precision
	vector<int*> calibrationPoints = vector<int*>();
	vector<int*> preciseCalibrationPoints = vector<int*>();
	vector<double*> calibrationCamPoints = vector<double*>();

	//CalibrationPoints contains 6D coordinates in robot space separated by commas on each line

	//Begin Calibration routine
	ifstream filestream("CalibrationPoints.txt");
	string line;
	const char delimiter[2] = ",";
	while (filestream.good())
	{
		while (getline(filestream, line))
		{
			//for each line, make a new array of points and add it to the calibration vector
			int *pointArray;
			int *preciseArray;
			double *camPointArray;
			pointArray = (int *)malloc(sizeof(int) * dataLength);
			preciseArray = (int *)malloc(sizeof(int) * dataLength);
			camPointArray = (double *)malloc(sizeof(double) * 16);
			char* charLine = stringToChar(line);
			int i = 0;
			char *coordinate = strtok(charLine, delimiter);
			while (coordinate != NULL) {
				pointArray[i] = (int)atof(coordinate) / 1000;
				preciseArray[i] = (int)atof(coordinate);
				coordinate = strtok(NULL, delimiter);
				i++;
			}
			calibrationPoints.push_back(pointArray);
			preciseCalibrationPoints.push_back(preciseArray);
			calibrationCamPoints.push_back(camPointArray);
		}
	}

	//print calibration coordinates
	cout << "Calibrating to the following coordinates:\n";
	for (i = 0; i < calibrationPoints.size(); i++){
		printf("Coordinate %d : ", i);
		for (j = 0; j < dataLength; j++){
			if (j == dataLength - 1) {
				cout << calibrationPoints.at(i)[j];
			}
			else {
				cout << calibrationPoints.at(i)[j] << ",";
			}
		}
		cout << "\n";
	}

	time_t now;
	time_t old;
	time_t notificationTime;
	time_t arrivalTime;

	time(&now);
	double seconds;
	double timeout = 3600;
	double elapsed = 0;
	for (i = 0; i < calibrationPoints.size(); i++){
		printf("Calibration Step %d out of 4: Moving to point %d... \n", i + 1, i + 1);
		// robot moves to points written to charPR2URL
		// must move to the precise position
		OPCWrite(charPR2URL, preciseCalibrationPoints.at(i), dataLength);
		time(&now);
		time(&old);
		while (true){
			//Check if current position matches calibration point every second
			time(&now);
			seconds = difftime(now, old);
			elapsed = elapsed + seconds;
			if (elapsed > timeout){
				printf("System timed out after %f seconds. Exiting... \n", timeout);
				exit(1);
			}
			if (seconds < 0.5) continue;
			else{
				time(&old);
				ReadGlobalVars(curPos, dataLength);
				int fail = 0;
				for (j = 0; j < dataLength; j++){
					if (curPos[j] != calibrationPoints.at(i)[j]){
						fail = 1;
						//debug
						//fail = 0;
						break;
					}
				}
				if (fail == 0) {
					Sleep(3000); // keeps calibration stopped for calculation purposes

					//Record timestamp of when we arrived to CalibrationPoint
					time(&arrivalTime);
					//Period at which notifications appear
					int notificationPeriod = 3;
					int notificationTimer = notificationPeriod;
					printf("Arrived to Point at: %s", asctime(localtime(&arrivalTime)));
					printf("Last Brainsight Update at: %s", asctime(&lastUpdate));
					while (difftime(arrivalTime, lastUpdateTime) > 0 || CTStatus != 1){ //busywait until brainsight data is up to date
						time(&notificationTime);
						
						//notify user of status of point every 2 seconds
						if (difftime(notificationTime, arrivalTime) > notificationTimer){
							if (difftime(arrivalTime, lastUpdateTime) > 0){
								cout << "Notification: Brainsight still out-of-date.\n";
								printf("First arrived to Point at: %s", asctime(localtime(&arrivalTime)));
								printf("Last Brainsight Update at: %s", asctime(&lastUpdate));
							}
							if (CTStatus != 1){
								cout << "Notification: Coil Tracker is in failed status. Please assure coil is visible to Polaris.\n";
							}
						}
					}
					m.lock();
					for (j = 0; j < 16; j++){
						calibrationCamPoints.at(i)[j] = brainSightCT[j];
					}
					m.unlock();
					cout << "Brainsight data up-to-date and Camera Point retrieved. Continuing calibration... \n";
					break;
				}
			}
		}
	}
	
	vector<double*> robPoints = vector<double*>();
	vector<double*> camPoints = vector<double*>();
	
	int arraySize = 6;
	for (i = 0; i < calibrationPoints.size(); i++){
		double *pointArray;
		pointArray = (double *)malloc(sizeof(double) * arraySize);
		for (j = 0; j < arraySize; j++){
			pointArray[j] = calibrationPoints.at(i)[j];
		}
		robPoints.push_back(pointArray);
	}

	for (i = 0; i < calibrationCamPoints.size(); i++){
		double *pointArray;
		pointArray = (double *)malloc(sizeof(double) * arraySize);
		for (j = 0; j < arraySize; j++){
			if (j > 2) pointArray[j] = 0; // set euler positions to 0
			else pointArray[j] = calibrationCamPoints.at(i)[3 + 4 * j]; //set cam points
		}
		camPoints.push_back(pointArray);
	}
	cout << "calibrationCamPoints:\n";
	for (i = 0; i < calibrationCamPoints.size(); i++){
		for (j = 0; j < 16; j++){
			printf("%f ", calibrationCamPoints.at(i)[j]);
		}
		cout << "\n";
	}

	mat transMatrix = transformationComputation(camPoints, robPoints);
	return transMatrix;
}

int _tmain(int argc, _TCHAR* argv[])
{
	// urls that allow connection to the main server
	string PR2URL("opc://localhost/National Instruments.NIOPCServers/Robotchan.GELPC.$PR2");
	string CurPosURL("opc://localhost/National Instruments.NIOPCServers/Robotchan.GELPC.$CurPos");
	charCurPosURL = stringToChar(CurPosURL);
	charPR2URL = stringToChar(PR2URL);
	int precisionFactor = 1000; // important variable for writing to robot position registers

	thread ReadOPCThread(ReadOPC, charCurPosURL, dataToRead, dataLength);
	//thread parseBrainsight(parseBrainsight, "C:\\Users\\Ashley\\Desktop\\cameradata.txt");
	thread parseBrainsight(parseBrainsight, "Z:\\cameradata.txt");
	cout << "Initializing OPCRead threads and Brainsight parsing threads...\n";
	Sleep(3000); //Sleep to allow for thread initialization

	int i = 0;
	int j = 0;
	char c;
	
	// transformation matrix calculation
	mat transMatrix = calibrationRoutine();

	time_t now;
	time_t old;
	time_t notificationTime;
	time_t arrivalTime;

	//getchar();
	transMatrix.print("transMatrix:");
	double pointArray[16]; //contains Brainsight Subject Tracker point (4x4 matrix)
	while (true){
		printf("Following ST Tracker: \n");
		time(&arrivalTime);
		printf("Retrieving data: Do not move Subject Tracker. \n");
		//Period at which notifications appear
		int notificationPeriod = 1;
		int notificationTimer = notificationPeriod;
		printf("Arrived to Point at: %s", asctime(localtime(&arrivalTime)));
		printf("Last Brainsight Update at: %s", asctime(&lastUpdate));
		while (difftime(arrivalTime, lastUpdateTime) > 0 || STStatus != 1){ //busywait until brainsight data is up to date
		//while (difftime(arrivalTime, lastUpdateTime) > 0 || CTStatus != 1){ //busywait until brainsight data is up to date
			time(&notificationTime);
			//notify user of status of point every 10 seconds
			if (difftime(notificationTime, arrivalTime) > notificationTimer){
				if (difftime(arrivalTime, lastUpdateTime) > 0){
					cout << "Notification: Brainsight still out-of-date.\n";
					printf("First arrived to Point at: %s", asctime(localtime(&arrivalTime)));
					printf("Last Brainsight Update at: %s", asctime(&lastUpdate));
				}
				if (STStatus != 1){
				//if (CTStatus != 1){
					cout << "Notification: Subject Tracker is in failed status. Please assure coil is visible to Polaris.\n";
				}
				notificationTimer = notificationTimer + notificationPeriod;
			}
		}
		
		printf("brainSightST:");
		m.lock();
		for (j = 0; j < 16; j++){
			pointArray[j] = brainSightST[j];
			printf("%f ", brainSightST[j]);
		}
		
		/*
		printf("brainSightCT:");
		m.lock();
		for (j = 0; j < 16; j++){
			pointArray[j] = brainSightCT[j];
			printf("%f ", pointArray[j]);
		}
		*/
		m.unlock();
		printf("Brainsight data up-to-date and Subject Tracker Camera Point retrieved. \n");

		//create vectors for each path point

		mat pointMatrix(4, 4);
		for (i = 0; i < 4; i++){
			for (j = 0; j < 4; j++){
				pointMatrix(i, j) = pointArray[(i)*4 + (j)];
			}
		}

		transMatrix.print("transMatrix");
		pointMatrix.print("pointMatrix");


		mat result = transMatrix*pointMatrix;
		vec resultVector(3);
		vec resultOrientation(3);
		resultVector << result(0,3) << result(1,3) << result(2,3) << endr;
		if (result(2,1) > 0.998){
			resultOrientation << 0 << atan2(result(2, 0), result(2, 2)) * 180 / M_PI << 90 << endr;
		}
		else if (result(2, 1) < -0.998){
			resultOrientation << 0 << atan2(result(2, 0), result(2, 2)) * 180 / M_PI << -90 << endr;
		}
		else{
			resultOrientation << atan2(-result(1, 2), result(1, 1)) * 180 / M_PI << atan2(-result(2, 0), result(0, 0)) * 180 / M_PI << asin(result(1, 0)) * 180 / M_PI << endr;
		}
		result.print("result:");
		resultOrientation.print("resultOrientation:");
		//int robotDestination[6] = { 0, 0, 0, 45, -86,131 };
		int robotDestination[6] = { 0, 0, 0, 0, 0, 0 };
		for (i = 0; i < 3; i++){
			robotDestination[i] = resultVector(i) * precisionFactor;
		}
		
		for (i = 3; i < 6; i++){
			robotDestination[i] = resultOrientation(i-3) * precisionFactor;
		}

		OPCWrite(charPR2URL, robotDestination, 6);
		Sleep(1000);
	}

	getchar();
	return 0;
}


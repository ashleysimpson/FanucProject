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

//Given 4, 6-length coordinate vectors in XYZWPR coordinates for the robot's points and the 4 corresponding camera points, computes the 4x4 transformation matrix
//Function is unused in final implementation as we use Horn's Method instead
mat computeTransformationMatrix(vector<double*> robPoints, vector<double*> camPoints)
{
	int transformationMatrixSize = 4;
	//Matrix M contains matrix of the form
	/*
	camx0 camy0 camz0 1
	camx1 camy1 camz1 1
	camx2 camy2 camz2 1
	camx3 camy3 camz3 1

	Used to calculate final transformation matrix by solving formulas:
	M*[Row 0 of Transformation Matrix] = [robx0 roby0 robz0 1]
	M*[Row 1 of Transformation Matrix] = [robx1 roby1 robz1 1]
	...etc.

	Note that the Transformation Matrix Rows are transposed
	Research change of basis matrix for more details
	*/
	mat M(transformationMatrixSize, transformationMatrixSize);

	int i;
	int j;
	//initialize matrix M
	for (i = 0; i < transformationMatrixSize; i++){
		for (j = 0; j < transformationMatrixSize; j++){
			if (j == transformationMatrixSize - 1) M(i, j) = 1;
			else M(i, j) = camPoints.at(i)[j];
		}
	}

	//create robot vectors [x y z 1] from xyz robot points
	vector<vec> robVectors = vector<vec>();
	for (i = 0; i < transformationMatrixSize; i++){
		vec robVector(4);
		robVector << robPoints.at(0)[i] << robPoints.at(1)[i] << robPoints.at(2)[i] << 1 << endr;
		robVectors.push_back(robVector);
	}

	//create cam vectors [x y z 1] from xyz cam points
	vector<vec> camVectors = vector<vec>();
	for (i = 0; i < transformationMatrixSize; i++){
		vec camVector(4);
		camVector << camPoints.at(i)[0] << camPoints.at(i)[1] << camPoints.at(i)[2] << 1 << endr;
		camVectors.push_back(camVector);
	}

	//debug printing
	M.print("M:");
	robVectors.at(0).print("RobV:");
	camVectors.at(0).print("CamV");

	//solve change of basis matrix
	//note that solve function gives X where A*X=B
	//in this case, M*M1=RobV0, etc.
	//Where M1 is the first row of the resulting Transformation matrix
	vec M1 = solve(M, robVectors.at(0));
	vec M2 = solve(M, robVectors.at(1));
	vec M3 = solve(M, robVectors.at(2));
	vec M4 = solve(M, robVectors.at(3));

	M1.print("M1:");
	M2.print("M2:");
	M3.print("M3:");
	M4.print("M4:");
	mat transMatrix(4, 4);
	transMatrix << M1(0) << M1(1) << M1(2) << M1(3) << endr
		<< M2(0) << M2(1) << M2(2) << M2(3) << endr
		<< M3(0) << M3(1) << M3(2) << M3(3) << endr
		<< 0 << 0 << 0 << 1 << endr;

	transMatrix.print("Transmatrix");
	return transMatrix;
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

mat calibrationRoutine()
{
	int i = 0;
	int j = 0;
	char c;

	vector<int*> calibrationPoints = vector<int*>();
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
			double *camPointArray;
			pointArray = (int *)malloc(sizeof(int) * dataLength);
			camPointArray = (double *)malloc(sizeof(double) * 16);
			char* charLine = stringToChar(line);
			int i = 0;
			char *coordinate = strtok(charLine, delimiter);
			while (coordinate != NULL) {
				pointArray[i] = (int)atof(coordinate);
				coordinate = strtok(NULL, delimiter);
				i++;
			}
			calibrationPoints.push_back(pointArray);
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
		//Robot moves to points written to charPR2URL
		OPCWrite(charPR2URL, calibrationPoints.at(i), dataLength);
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
					//Record timestamp of when we arrived to CalibrationPoint
					time(&arrivalTime);
					//Period at which notifications appear
					int notificationPeriod = 1;
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
							notificationTimer = notificationTimer + notificationPeriod;
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
	cout << "robPoints:\n";
	for (i = 0; i < robPoints.size(); i++){
		for (j = 0; j < 6; j++){
			printf("%f ", robPoints.at(i)[j]);
		}
		cout << "\n";
	}
	cout << "camPoints:\n";
	for (i = 0; i < camPoints.size(); i++){
		for (j = 0; j < 6; j++){
			printf("%f ", camPoints.at(i)[j]);
		}
		cout << "\n";
	}

	mat transMatrix = computeTransformationMatrix(robPoints, camPoints);
	return transMatrix;
}

int _tmain(int argc, _TCHAR* argv[])
{
	// urls that allow connection to the main server
	string PR2URL("opc://localhost/National Instruments.NIOPCServers/Robotchan.GELPC.$PR2");
	string CurPosURL("opc://localhost/National Instruments.NIOPCServers/Robotchan.GELPC.$CurPos");
	charCurPosURL = stringToChar(CurPosURL);
	charPR2URL = stringToChar(PR2URL);

	thread ReadOPCThread(ReadOPC, charCurPosURL, dataToRead, dataLength);
	//thread parseBrainsight(parseBrainsight, "C:\\Users\\Ashley\\Desktop\\cameradata.txt");
	thread parseBrainsight(parseBrainsight, "C:\\Users\\Ashley\\Desktop\\cameradata.txt");
	cout << "Initializing OPCRead threads and Brainsight parsing threads...\n";
	Sleep(3000); //Sleep to allow for thread initialization

	int i = 0;
	int j = 0;
	char c;

	//mat transMatrix = calibrationRoutine();

	/*mat transMatrix(4, 4);
	transMatrix << 0.0248456819988944 << 0.67846226913591 << 0.734214983124808 << 1129.83241064818 << endr
		<< -0.0340226467179108 << 0.734590116827388 << -0.677657597736374 << -574.051305674604 << endr
		<< -0.999112181687481 << -0.00814307180835949 << 0.0413344745364672 << 198.634541169709 << endr
		<< 0 << 0 << 0 << 1 << endr;
		*/
	double matrixArray[16] = { 0 };
	ifstream filestream("TransformationMatrix.txt");
	string line;
	const char delimiter[2] = ",";
	while (filestream.good())
	{
		while (getline(filestream, line))
		{
			//for each line, make a new array of points and add it to the calibration vector
			size_t sz;
			char* charLine = stringToChar(line);
			int i = 0;
			char *matrixValue = strtok(charLine, delimiter);
			while (matrixValue != NULL) {
				matrixArray[i] = stod(string(matrixValue), &sz);
				matrixValue = strtok(NULL, delimiter);
				i++;
			}
		}
	}

	mat transMatrix(4, 4);
	transMatrix << matrixArray[0] << matrixArray[1] << matrixArray[2] << matrixArray[3] << endr
		<< matrixArray[4] << matrixArray[5] << matrixArray[6] << matrixArray[7] << endr
		<< matrixArray[8] << matrixArray[9] << matrixArray[10] << matrixArray[11] << endr
		<< matrixArray[12] << matrixArray[13] << matrixArray[14] << matrixArray[15] << endr;


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
			robotDestination[i] = round(resultVector(i));
		}
		
		for (i = 3; i < 6; i++){
			robotDestination[i] = round(resultOrientation(i-3));
		}

		OPCWrite(charPR2URL, robotDestination, 6);
		Sleep(1000);
	}

	getchar();
	return 0;
}


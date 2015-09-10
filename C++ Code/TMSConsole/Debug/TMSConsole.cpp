// TMSConsole.cpp : Defines the entry point for the console application.
#define ARMA_DONT_USE_BLAS
#define M_PI 3.14159265358979323846  /* pi */
#include "stdafx.h"
#include "OPCFunctions.h"
#include <iostream>
#include <fstream>
#include <string>
#include <stdint.h>
#include <thread>
#include <iostream>
#include <armadillo>
#include <windows.h>
#include <time.h> 
#include <math.h>

using namespace std;
using namespace arma;

//BrainSight tracker signals in 4x4 position matrix with coordinates in the last column
int const dataLength = 6;
int32_t dataToRead[dataLength] = { 0 };
double curPos[dataLength] = { 0.0 };
char *charCurPosURL;
char *charPR2URL;
int precisionFactor = 1000; //used to keep the decimal precision when sending points to the robot

//Wrapper function for LabView OPCRead
void ReadOPC(char *OPCURL, int32_t *OPCData, int32_t len)
{
	OPCRead(OPCURL, OPCData, len);
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

void formatConsoleOutput() {
	cout << "---------------------------------------------\n\n\n";
	cout << "---------------------------------------------\n";
}

void waitForUserEnter()
{
	// extra logic to wait for user ready
	printf("Please hit enter to continue...");
	char input = '0';
	while (input != '\n') {
		cin.get(input);
	}
}

int retrieveUserNumberOfPoints() 
{
	bool userInputAccepted = false;
	int numberOfPoints = 0;

	// continue to query user until correct input accepted
	while (!userInputAccepted) {
		cout << "Please enter the number of points you wish to follow: \n";

		// retrieve user input and check action
		string userInput;
		getline(cin, userInput);
		int value = atoi(userInput.c_str());

		if (value > 0) {
			cout << "You will need to move to the " << value << " points with the robot!\n";
			numberOfPoints = value;
			userInputAccepted = true;
		}
		else {
			cout << "Undefined input, please input an actual point number...\n";
			userInputAccepted = false;
		}
	}

	return numberOfPoints;
}

vector<double*> gatherUserPoints(int numberOfPoints) {

	vector<double*> points = vector<double*>();

	// wait for the user to move to all points and store these points
	for (int i = 0; i < numberOfPoints; i++) {

		cout << "Please move to your point " << (i + 1) << " and then hit enter!\n";
		waitForUserEnter();

		ReadGlobalVars(curPos, dataLength);

		double *pointArray = (double *)malloc(sizeof(double) * dataLength);

		for (int j = 0; j < 6; j++) {
			pointArray[j] = curPos[j] * precisionFactor;
		}

		points.push_back(pointArray);
	}

	return points;
}

vector<double*> gatherTextfilePoints() {

	vector<double*> points = vector<double*>();

	cout << "Please move your text file to the desktop and rename to 'input.txt'.\n";
	cout << "Make sure the points in the file following format...\n";
	cout << "X Y Z W P R\n";

	// open the file and read
	boolean fileOpened = false;
	while (!fileOpened) {
		waitForUserEnter();
		string line;
		ifstream file("C:\\Users\\amir\\Desktop\\input.txt", ios::in);

		if (file.is_open()) {
			while (getline(file, line)) {
				double *pointArray = (double *)malloc(sizeof(double) * dataLength);
				string number = "";
				int value = 0;

				for (int i = 0; i < line.length(); i++) {
					if (line.at(i) == ' ') {
						pointArray[value] = stoi(number);
						value++;
						number = "";
						continue;
					} else {
						number += line.at(i);
					}
				}

				points.push_back(pointArray);
			}

			file.close();
			fileOpened = true;
		}
		else {
			cout << "Unable to open file!\n";
		}
	}

	return points;
}

vector<int> gatherTextfileSequence() {

	vector<int> sequence = vector<int>();

	cout << "Please move your text file to the desktop and rename to 'sequence.txt'.\n";
	cout << "Make sure the points are located on each line individually\n";
	cout << "If a point selected does not exist in the list of points, it will be skipped and the user prompted!\n";

	// open the file and read
	boolean fileOpened = false;
	while (!fileOpened) {
		waitForUserEnter();
		string line;
		ifstream file("C:\\Users\\amir\\Desktop\\sequence.txt", ios::in);

		if (file.is_open()) {
			while (getline(file, line)) {
				int sequencePoint = stoi(line);			

				sequence.push_back(sequencePoint);
			}

			file.close();
			fileOpened = true;
		}
		else {
			cout << "Unable to open file!\n";
		}
	}

	return sequence;
}

int retrieveUserSelection(int numberOfChoices, string message)
{
	bool userInputAccepted = false;
	int choice = 0;
	cout << message << "\n"; // print the message to the user

	// continue to query user until correct input accepted
	while (!userInputAccepted) {
		cout << "Please enter your choice from 1-" << numberOfChoices << ":\n";

		// retrieve user input and make sure its a correct choice
		string userInput;
		getline(cin, userInput);
		choice = atoi(userInput.c_str());

		if (choice > 0 && choice <= numberOfChoices) {
			cout << "You have chosen selection " << numberOfChoices << "!\n";
			userInputAccepted = true;
		}
		else {
			cout << "Undefined input, please choose ...\n";
			userInputAccepted = false;
		}
	}

	return choice;
}

void waitForRobotToFinishMovement(int* endingPoint)
{
	// will loop until the robot hits the ending point
	while (true) {
		int fail = 0;
		ReadGlobalVars(curPos, dataLength);

		for (int i = 0; i < dataLength; i++){
			if (curPos[i] * precisionFactor != endingPoint[i]){
				fail = 1;
				break;
			}
		}

		if (fail == 0) {
			break;
		}
	}
}

double degreesToRad(double degrees) 
{
	return (degrees*(M_PI/180));
}

void moveToSafetyPoint(int* point)
{
	int robotDestination[6] = { 0, 0, 0, 0, 0, 0 };

	double x = point[3]/precisionFactor; // x
	double y = point[4]/precisionFactor; // y
	double z = point[5]/precisionFactor; // z

	// zyx order of rotations

	double c1 = cos(degreesToRad(z));
	double s1 = sin(degreesToRad(z));
	double c2 = cos(degreesToRad(y));
	double s2 = sin(degreesToRad(y));
	double c3 = cos(degreesToRad(x));
	double s3 = sin(degreesToRad(x));

	double m02 = s1*s3 + c1*c3*s2;
	double m12 = c3*s1*s2 - c1*s3;
	double m22 = c2*c3;

	robotDestination[0] = point[0] + int(-50 * m02)*precisionFactor;
	robotDestination[1] = point[1] + int(-50 * m12)*precisionFactor;
	robotDestination[2] = point[2] + int(-50 * m22)*precisionFactor;
	robotDestination[3] = point[3];
	robotDestination[4] = point[4];
	robotDestination[5] = point[5];

	OPCWrite(charPR2URL, robotDestination, 6);
	waitForRobotToFinishMovement(robotDestination);
}

string padLeft(string input, int unpaddedLength, char padding) 
{
	while (input.length() < unpaddedLength) {
		input = padding + input;
	}

	return input;
}

string getFileLocation() {

	// get the current date
	time_t currentDate;
	tm* timeinfo;
	time(&currentDate);
	timeinfo = localtime(&currentDate);

	// pull out the useful information
	string weekday = padLeft(to_string(timeinfo->tm_mday), 2, '0');
	string month = padLeft(to_string(timeinfo->tm_mon + 1), 2, '0');
	string year = to_string(timeinfo->tm_year + 1900);
	string hour = padLeft(to_string(timeinfo->tm_hour), 2, '0');
	string minute = padLeft(to_string(timeinfo->tm_min), 2, '0');

	// convert to a char string
	string fileLocation = "C:\\Users\\amir\\Desktop\\TMSConsoleOutput\\" + weekday + "_" + month + "_" + year + "_" + hour + "_" + minute;

	return fileLocation;
}

void dumpPoints(vector<double*> points, string fileLocation)
{

	string pathStr = fileLocation + ".txt";
	char* path = new char[pathStr.length() + 1];
	strcpy(path, pathStr.c_str());

	// write the points to the file
	std::ofstream file(path);
	for (int i = 0; i < points.size(); i++) {
		for (int j = 0; j < 6; j++) {
			file << points.at(i)[j] << " ";
		}
		file << std::endl;
	}

	delete[] path;
	file.close();
}

void dumpSequencePoint(int pointNumber, string fileLocation) {

	// convert to a char string
	string pathStr = fileLocation + "_sequence.txt";
	char* path = new char[pathStr.length() + 1];
	strcpy(path, pathStr.c_str());

	// write the points to the file
	std::ofstream file;
	file.open(path, ios::out | ios::app);
	file << pointNumber << std::endl;

	delete[] path;
	file.close();
}

void moveDelay(int movementChoice, int seconds)
{
	// movement options, will condense into a single method
	if (movementChoice == 1) {
		waitForUserEnter();
	}
	if (movementChoice == 2) {
		Sleep(seconds * 1000);
	}
	if (movementChoice == 4) {
		system("pause");
	}
}

int getNextPoint(int currentPoint, int sequenceChoice, vector<int> sequenceOfPoints, int sequenceLineTracker, int numberOfPoints) {

	if (sequenceChoice == 1) {
		// if reached the end of list then reset from the beginning
		if (currentPoint == numberOfPoints - 1) {
			currentPoint = 0;
		}
		else {
			currentPoint++;
		}
	}
	else if (sequenceChoice == 2) {
		currentPoint = rand() % numberOfPoints;
	}
	else {
		currentPoint = sequenceOfPoints.at(sequenceLineTracker) - 1;

		// point number is greater than number of actual points
		if (currentPoint >= numberOfPoints || currentPoint < 0) {

			formatConsoleOutput();

			cout << "The point " << (currentPoint + 1) << " has been selected that is not part of the list!\n";

			if (currentPoint < 0) {
				currentPoint = 0;
			}
			else {
				currentPoint = currentPoint % numberOfPoints;
			}

			cout << "Therefore the point " << (currentPoint + 1) << " will be used instead!\n";
			cout << "If this is not desired then please exit the program and recheck you sequence of points.\n";
			cout << "Otherwise you can allow the program to continue after pressing 'Enter'.\n";
			waitForUserEnter();

			formatConsoleOutput();
		}
	}

	return currentPoint;
}

int _tmain(int argc, _TCHAR* argv[])
{
	// urls that allow connection to the main server
	string PR2URL("opc://localhost/National Instruments.NIOPCServers/Robotchan.GELPC.$PR2");
	string CurPosURL("opc://localhost/National Instruments.NIOPCServers/Robotchan.GELPC.$CurPos");
	string fileLocation = getFileLocation();
	vector<int> sequenceOfPoints;
	charCurPosURL = stringToChar(CurPosURL);
	charPR2URL = stringToChar(PR2URL);

	// start threads for reading brainsight information
	thread ReadOPCThread(ReadOPC, charCurPosURL, dataToRead, dataLength);

	// wait for user to begin the program
	formatConsoleOutput();
	waitForUserEnter();

	// ask the use if they would like to input points or follow existing
	string pointMessage = "Please choose between the following 2 choices\n1: Move robot to points\n2: Choose file for input\n";
	int pointsChoice = retrieveUserSelection(2, pointMessage);

	// based on the user choice gather points or retrieve points from the file
	vector<double*> points;
	int numberOfPoints;
	if (pointsChoice == 1) {
		// request number of points to navigate to
		formatConsoleOutput();
		numberOfPoints = retrieveUserNumberOfPoints();

		// retrieve the point information, and store for later traversal
		formatConsoleOutput();
		points = gatherUserPoints(numberOfPoints);

		// dump the points to an external text file for use with matlab
		dumpPoints(points, fileLocation);

		// ask user if they want to use points or exit program
		formatConsoleOutput();
		string exitMessage = "Please choose between the following 2 choices\n1: Use these points\n2: Exit program and use Matlab to interpolate points\n";
		int exitChoice = retrieveUserSelection(2,exitMessage);

		// loop and instruct user to exit thbe program, else continue normally
		while (exitChoice == 2) {
			cout << "Please exit the console window!\n";
			Sleep(5000);
		}
	}
	else {
		formatConsoleOutput();
		points = gatherTextfilePoints();
		numberOfPoints = points.size();
	}

	// retrieve the user choice for sequence of events
	formatConsoleOutput();
	string sequenceMessage = "Please choose between the following 3 choices\n1: Cycle through the points\n2: Randomly cycle through the points\n3: Use a sequence of user defined points\n";
	int sequenceChoice = retrieveUserSelection(3, sequenceMessage);

	// special case where the user must provide location of sequence file
	if (sequenceChoice == 3) {
		formatConsoleOutput();
		sequenceOfPoints = gatherTextfileSequence();
	}

	// retrieve the user choice for movement trigger
	formatConsoleOutput();
	int seconds = 0;
	string movementMessage = "Please choose between the following 4 choices\n1: User hits enter\n2: Wait so many seconds\n3: Continuously move\n4: Wait for keyboard trigger\n";
	int movementChoice = retrieveUserSelection(4, movementMessage);

	// special case where the user is asked for the number of seconds
	if (movementChoice == 2) {
		formatConsoleOutput();
		seconds = retrieveUserSelection(100, "Enter the number of seconds between each movement (1-100 seconds): ");
	}

	// wait for the user to run the karel program on the robot
	formatConsoleOutput();
	cout << "Start the robot movement program and then hit enter!\n";
	formatConsoleOutput();
	waitForUserEnter();

	// main loop, run through the points provided and stimulate
	int currentPoint = 0;
	int sequenceLineTracker = 0;
	
	// to prevent skipping the first point when cycling
	if (sequenceChoice != 1) {
		currentPoint = getNextPoint(currentPoint, sequenceChoice, sequenceOfPoints, sequenceLineTracker, numberOfPoints);
	}

	while (true) {
		int robotDestination[6] = { 0, 0, 0, 0, 0, 0 };

		// set the point indicated
		for (int i = 0; i < 6; i++) {
			robotDestination[i] = points.at(currentPoint)[i];
		}

		moveToSafetyPoint(robotDestination);
		moveDelay(movementChoice, seconds);

		OPCWrite(charPR2URL, robotDestination, 6);
		waitForRobotToFinishMovement(robotDestination);
		moveDelay(movementChoice, seconds);
		
		moveToSafetyPoint(robotDestination);
		moveDelay(movementChoice, seconds);

		dumpSequencePoint(currentPoint + 1, fileLocation);

		cout << "Finished movement to Point " << (currentPoint + 1) << "...\nPlease exit the program at anytime to finish.\n";

		// only increment line tracker when required
		if (sequenceChoice == 3) {
			sequenceLineTracker = (sequenceLineTracker + 1) % sequenceOfPoints.size();
		}

		// get the next point based on the user's inputs
		currentPoint = getNextPoint(currentPoint, sequenceChoice, sequenceOfPoints, sequenceLineTracker, numberOfPoints);
	}

	return EXIT_SUCCESS;
}


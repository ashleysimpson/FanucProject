// MatrixSolver.cpp : Defines the entry point for the console application.
//
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#define M_PI 3.14159265358979323846  /* pi */
#include <math.h>
#include "stdafx.h"
#include <iostream>
#include <armadillo>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <stdlib.h>
#include <stdio.h>

using namespace std;
using namespace arma;

struct quaternion {
	double w;
	double x;
	double y;
	double z;
};
typedef struct quaternion Quaternion;

struct eulerAngle {
	double rx;
	double ry;
	double rz;
};
typedef struct eulerAngle EulerAngle;
Quaternion eulerToQuaternion(EulerAngle angle);
EulerAngle quaternionToEuler(Quaternion q);
Quaternion normalizeQuaternion(Quaternion q);


void test()
{
	int arraySize = 6;
	const char delimiter[2] = ",";
	ifstream infile("coords4.txt");
	vector<double*> robPoints = vector<double*>();
	vector<double*> camPoints = vector<double*>();
	cout << "Please enter coordinates file:";

	int pointCounter = 0;
	int i = 0;
	string line;
	string fileLine;
	stringstream linestream;
	while (infile.good())
	{
		while (getline(infile, line))
		{
			linestream << line;
			while (getline(linestream, fileLine, ':'))
			{
				double *pointArray;
				pointArray = (double *)malloc(sizeof(double) * arraySize);

				const char *constCharLine = fileLine.c_str();
				char *charLine = new char[fileLine.length() + 1];
				strcpy(charLine, constCharLine);

				int i = 0;
				char *coordinate = strtok(charLine, delimiter);
				while (coordinate != NULL) {
					pointArray[i] = (double)atof(coordinate);
					coordinate = strtok(NULL, delimiter);
					i++;
				}

				if (pointCounter % 2 == 0){
					robPoints.push_back(pointArray);
				}
				else {
					camPoints.push_back(pointArray);
				}
				pointCounter++;
			}
			linestream.clear();
		}

	}

	cout << "Rob Points \n";
	for (i = 0; i < robPoints.size(); i++){
		int j;
		double *array = robPoints.at(i);
		for (j = 0; j < arraySize; j++){
			cout << array[j] << ",";
		}
		cout << "\n";
	}

	cout << "Cam Points \n";
	for (i = 0; i < camPoints.size(); i++){
		int j;
		double *array = camPoints.at(i);
		for (j = 0; j < arraySize; j++){
			cout << array[j] << ",";
		}
		cout << "\n";
	}

	int transformationMatrixSize = 4;
	mat M(transformationMatrixSize, transformationMatrixSize);

	int j;
	for (i = 0; i < transformationMatrixSize; i++){
		for (j = 0; j < transformationMatrixSize; j++){
			if (j == transformationMatrixSize - 1) M(i, j) = 1;
			else M(i, j) = camPoints.at(i)[j];
		}
	}

	//create vectors
	vector<vec> robVectors = vector<vec>();
	for (i = 0; i < transformationMatrixSize; i++){
		vec robVector(4);
		robVector << robPoints.at(0)[i] << robPoints.at(1)[i] << robPoints.at(2)[i] << robPoints.at(3)[i] << endr;
		robVectors.push_back(robVector);
	}

	vector<vec> camVectors = vector<vec>();
	for (i = 0; i < transformationMatrixSize; i++){
		vec camVector(4);
		camVector << camPoints.at(i)[0] << camPoints.at(i)[1] << camPoints.at(i)[2] << 1 << endr;
		camVectors.push_back(camVector);
	}

	M.print("M:");
	robVectors.at(0).print("RobV:");
	camVectors.at(0).print("CamV");

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
	vec testvec("57.18,49.58,-1184.91,1");
	mat result = transMatrix*testvec;
	result.print("result:");
	getchar();
}

mat computeTransformationMatrix(vector<double*> robPoints, vector<double*> camPoints)
{
	int arraySize = 6;
	int transformationMatrixSize = 4;
	mat M(transformationMatrixSize, transformationMatrixSize);

	int i;
	int j;
	for (i = 0; i < transformationMatrixSize; i++){
		for (j = 0; j < transformationMatrixSize; j++){
			if (j == transformationMatrixSize - 1) M(i, j) = 1;
			else M(i, j) = camPoints.at(i)[j];
		}
	}

	//create vectors
	vector<vec> robVectors = vector<vec>();
	for (i = 0; i < transformationMatrixSize; i++){
		vec robVector(4);
		robVector << robPoints.at(0)[i] << robPoints.at(1)[i] << robPoints.at(2)[i] << robPoints.at(3)[i] << endr;
		robVectors.push_back(robVector);
	}

	vector<vec> camVectors = vector<vec>();
	for (i = 0; i < transformationMatrixSize; i++){
		vec camVector(4);
		camVector << camPoints.at(i)[0] << camPoints.at(i)[1] << camPoints.at(i)[2] << 1 << endr;
		camVectors.push_back(camVector);
	}

	M.print("M:");
	robVectors.at(0).print("RobV:");
	camVectors.at(0).print("CamV");

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
	//vec testvec("57.18,49.58,-1184.91,1");
	//mat result = transMatrix*testvec;
	//result.print("result:");
	//getchar();
	return transMatrix;
}

Quaternion normalizeQuaternion(Quaternion q){
	double length = sqrt(q.w*q.w + q.x*q.x + q.y*q.y + q.z*q.z);
	Quaternion normalQ = {
		q.w / length,
		q.x / length,
		q.y / length,
		q.z / length
	};
	return normalQ;
}

Quaternion eulerToQuaternion(EulerAngle angle){
	double degToRad = M_PI / 180;
	double c1 = cos(angle.ry*degToRad / 2);
	double c2 = cos(angle.rx*degToRad / 2);
	double c3 = cos(angle.rz*degToRad / 2);
	double s1 = sin(angle.ry*degToRad / 2);
	double s2 = sin(angle.rx*degToRad / 2);
	double s3 = sin(angle.rz*degToRad / 2);
	Quaternion quaternion = {
		c1*c2*c3 - s1*s2*s3,
		s1*s2*c3 + c1*c2*s3,
		s1*c2*c3 + c1*s2*s3,
		c1*s2*c3 - s1*c2*s3
	};
	return quaternion;
}

EulerAngle quaternionToEuler(Quaternion q){
	double radToDeg = 180 / M_PI;
	double sqw = q.w*q.w;
	double sqx = q.x*q.x;
	double sqy = q.y*q.y;
	double sqz = q.z*q.z;
	double length = sqw + sqx + sqy + sqz;
	double test = q.x*q.y + q.z*q.w;
	if (test > 0.499*length){
		EulerAngle eulerAngle = {
			M_PI / 2 * radToDeg,
			2 * atan2(q.x, q.w) * radToDeg,
			0
		};
		return eulerAngle;
	}
	if (test < -0.499*length){
		EulerAngle eulerAngle = {
			M_PI / -2 * radToDeg,
			-2 * atan2(q.x, q.w) * radToDeg,
			0
		};
		return eulerAngle;
	}
	double rx = asin(2 * q.x*q.y + 2 * q.z*q.w);
	double ry = atan2(2 * q.y*q.w - 2 * q.x*q.z, 1 - 2 * sqy - 2 * sqz);
	double rz = atan2(2 * q.x*q.w - 2 * q.y*q.z, 1 - 2 * sqx - 2 * sqz);
	EulerAngle eulerAngle = {
		rx*radToDeg,
		ry*radToDeg,
		rz*radToDeg
	};
	return eulerAngle;
}



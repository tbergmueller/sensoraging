/*
 * SensorDefects.cpp
 *
 *  Created on: 10 Nov 2013
 *      Author: thomas
 */

#include "SensorDefects.h"
#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <random>


using namespace std;
using namespace cv;


#define MIN_HOT		1
#define MAX_HOT		100



SensorDefectSet::SensorDefectSet(int width, int height, double lamdaD, double lamdaC)
{
	_d = Mat::zeros(height,width,CV_8UC1);
	_c = Mat::zeros(height,width,CV_8UC1);
	_cMap = Mat::zeros(height,width,CV_8UC1);
	bitwise_not(_cMap,_cMap);


	_nrOfdefects_d = 0;
	_nrOfdefects_c = 0;



	_lamdaD = lamdaD / 1000 * ((double)(width*height)) / 365.0;	// per jahr per 1000 pixel... umrechnen in Tage!!
	_lamdaC= lamdaC/ 1000 * ((double)(width*height)) / 365.0;;
	_ageInDays = 0;
}

SensorDefectSet::SensorDefectSet()
{

}

int SensorDefectSet::getAge()
{
	return _ageInDays;
}

int SensorDefectSet::getExpectedDefectsD(int ageInDays)
{
	return (int)(_lamdaD * (double)ageInDays);
}

int SensorDefectSet::getExpectedDefectsC(int ageInDays)
{
	return (int)(_lamdaC * (double)ageInDays);
}


void SensorDefectSet::ageBy(int nrOfDays, SensorDefectSet& newSet)
{
	int nrOfDToAdd = getExpectedDefectsD(nrOfDays+this->_ageInDays) - this->_nrOfdefects_d;
	int nrOfCToAdd = getExpectedDefectsC(nrOfDays+this->_ageInDays) - this->_nrOfdefects_c;

	//srand(time(NULL));
	cout << this << " adding " << nrOfDToAdd << " defects..." << endl;

	newSet._d = this->_d.clone();
	newSet._c = this->_c.clone();

	newSet._nrOfdefects_d = _nrOfdefects_d;
	newSet._nrOfdefects_c = _nrOfdefects_c;
	newSet._ageInDays = nrOfDays + this->_ageInDays;
	newSet._lamdaD = _lamdaD;
	newSet._lamdaC = _lamdaC;


	for(int i=0; i<nrOfDToAdd; i++)
	{
		int x = rand() % _d.cols;
		int y = rand() % _d.rows;

		int amount = MIN_HOT + rand() % (MAX_HOT-1);		// some value

		newSet._d.at<uchar>(y,x) = amount;	// minimum offset is 128
		newSet._nrOfdefects_d++;
	}

	newSet._cMap = _cMap.clone();

	for(int i=0; i<nrOfCToAdd; i++)
	{
		int x = rand() % _d.cols;
		int y = rand() % _d.rows;

		int amount = rand() % 256;		// some value

		newSet._c.at<uchar>(y,x) = amount;	// Fixed offset since stuck...
		newSet._cMap.at<uchar>(y,x) = 0;
		newSet._nrOfdefects_c++;
	}

	imshow("Cmap",_cMap);
	Mat debug;
	normalize(_d,debug,0,255, CV_MINMAX, CV_8UC1);
	imshow("D normalized",debug);

	imshow("C", _c);
	waitKey(0);


	return;
}

void SensorDefectSet::apply(cv::Mat& on)
{

	// C are the stuck pixels


	// D are the partially stuck pixels..
	add(on,_d,on);


	// However, if it is stuck, only apply this defect
	bitwise_and(on,_cMap,on);	// wipe out wherever stuck
	add(on,_c,on);				// set stuck pixel value (0+value) = value




}

SensorDefects::SensorDefects(const cv::Size& imgSize)
{
	_width = imgSize.width;
	_height = imgSize.height;
}

SensorDefects::~SensorDefects() {
}



void SensorDefects::calcDefects(int timespan, int stepwidth)
{
	SensorDefectSet s(_width,_height,8*0.0017, 0.003717117); // zero day

	_defectsOverTime.push_back(s);

	for(int i = stepwidth; i<timespan; i+=stepwidth)
	{
		SensorDefectSet nextOlder;

		_defectsOverTime.back().ageBy(stepwidth, nextOlder);
		_defectsOverTime.push_back(nextOlder);

	}
}

void SensorDefects::applyDefects(cv::Mat& applyOn, int age)
{
	vector<SensorDefectSet>::iterator it = lower_bound(_defectsOverTime.begin(), _defectsOverTime.end(), age, SensorDefectSet::cmp);

	if(it == _defectsOverTime.end())
	{
		cerr << "No aging matrix found" << endl;
	}
	else
	{
		it->apply(applyOn);
	}


}


/*
 * SensorDefects.h
 *
 *  Created on: 10 Nov 2013
 *      Author: thomas
 */

#ifndef SENSORDEFECTS_H_
#define SENSORDEFECTS_H_

#include <map>
#include <opencv2/core/core.hpp>


class SensorDefectSet
{
public:
	SensorDefectSet(int width, int height, double lamdaD=0.01,  double lamdaC=0.01);
	SensorDefectSet();
	int getAge();
	void ageBy(int nrOfDays, SensorDefectSet& newSet);
	bool operator<(const SensorDefectSet& rhs) const { return _ageInDays < rhs._ageInDays; }
	void apply(cv::Mat& on);

	static bool cmp(const SensorDefectSet &a, const int b) { return a._ageInDays < b;}

private:
	cv::Mat _d;
	cv::Mat _c;
	cv::Mat _cMap;

	int _nrOfdefects_d;
	int _nrOfdefects_c;

	double _lamdaD;
	double _lamdaC;



	int _ageInDays;

	int getExpectedDefectsD(int ageInDays);
	int getExpectedDefectsC(int ageInDays);

};


class SensorDefects {
public:
	SensorDefects(const cv::Size& imageSize);
	virtual ~SensorDefects();

	void calcDefects(int timespan, int stepwidth);
	void applyDefects(cv::Mat& applyOn, int age);


private:
	int _width;
	int _height;

	std::vector<SensorDefectSet> _defectsOverTime;


};

#endif /* SENSORDEFECTS_H_ */

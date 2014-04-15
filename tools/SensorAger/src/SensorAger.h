/*
 * SensorAger.h
 *
 *  Created on: 10 Nov 2013
 *      Author: thomas
 */

#ifndef SENSORAGER_H_
#define SENSORAGER_H_

#include <string>
#include "SensorDefects.h"
#include <vector>

class SensorAger {
public:
	SensorAger(int maxmimumAgeInDays, const cv::Size& inputSize);
	void generateDataSet(std::string sourceFolderOriginalImages, std::string destinationFolder);
	virtual ~SensorAger();

private:
	SensorDefects _defects;
	std::vector<int> _agingSteps;


};

#endif /* SENSORAGER_H_ */

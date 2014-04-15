//============================================================================
// Name        : SensorAger.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>


#include "SensorDefects.h"
#include <opencv2/highgui/highgui.hpp>
#include "SensorAger.h"




using namespace cv;
using namespace std;

int main(int argc, char** argv)
{

	SensorAger sensor1(100*365, Size(320,240));

	sensor1.generateDataSet("/home/tbergmueller/workspaces/sensoraging/testdata/iitd/data/","/home/tbergmueller/sensor10");

	/*
	SensorDefects s(m1.size());
	s.calcDefects(5000, 100);


	Mat m = imread(filename, CV_LOAD_IMAGE_GRAYSCALE);
	for(int i=0; i<5000; i+= 200)
	{
		Mat cur = m.clone();

		s.applyDefects(cur,i);

		stringstream ss;
		ss << out << i << ".jpg";

		imwrite(ss.str(), cur);
	}


	s.applyDefects(m,1000);
	imshow("aged", m);
	waitKey();

	*/

	cout << "Test dataset generated. Done." << endl;

	return 0;
}

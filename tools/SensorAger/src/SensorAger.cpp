/*
 * SensorAger.cpp
 *
 *  Created on: 10 Nov 2013
 *      Author: thomas
 */

#include "SensorAger.h"

// #define LAMDA_C		0.01 // TODO		// Note this should change, due to paper "Characterization of Pixel Defect Development During Digital Imager Lifetime" => Split up in partly stuck pixels

using namespace std;

#include <boost/filesystem.hpp>
#include <opencv2/highgui/highgui.hpp>

inline char separator()
{
#ifdef _WIN32
    return '\\';
#else
    return '/';
#endif
}

using namespace boost::filesystem;
using namespace cv;

SensorAger::SensorAger(int maxAgeInDays, const cv::Size& inputSize ): _defects(inputSize)
{
	_defects.calcDefects(maxAgeInDays,maxAgeInDays / 25);

	for(int i=0; i<maxAgeInDays; i+= maxAgeInDays / 25)
	{
		_agingSteps.push_back(i);
	}
}


std::vector<string> getFileList(string folder)
{
	vector<string> fileList;

	for ( boost::filesystem::recursive_directory_iterator end, dir(folder);
	       dir != end; ++dir )
	{
		if(!is_directory(*dir))
		{

			 stringstream ss;
			 ss << *dir;

			 string absPath = ss.str().substr(1, ss.str().length()-2);
			 //cout << absPath << endl;
			 fileList.push_back(absPath);
		}

	}

	  return fileList;
}



void SensorAger::generateDataSet(std::string sourceFolderOriginalImages,
		std::string destinationFolder)
{
	// Find all inputs

	// find images in source folder

	vector<string> filesToLoad = getFileList(sourceFolderOriginalImages);

	cout << "Input dataset consists of " << filesToLoad.size() << " images" << endl;

	stringstream cmd;

	// security: limit to home function

	int loc = destinationFolder.rfind("/home/");

	if( loc != 0)
	{
		cerr << "Only allowed to write data to a user's home folder, thus aborting" << endl;
		return;
	}

	cmd << "rm -rf " << destinationFolder << separator() << "*";
	system(cmd.str().c_str());



	for(vector<int>::iterator age = _agingSteps.begin(); age != _agingSteps.end(); age++)
	{

		stringstream ofolder;
		ofolder <<destinationFolder << separator() << *age << separator();



		cmd.str("");
		cmd << "mkdir " << ofolder.str();
		system(cmd.str().c_str());


		cout << "### Generating dataset for age " << *age << " days in ..." << ofolder.str() << endl;
		// Make folder with age
		// store all images there
		for(vector<string>::iterator inFile = filesToLoad.begin(); inFile != filesToLoad.end(); inFile++)
		{
			string ifile = *inFile;
			int lstIndex = ifile.find_last_of(separator()); // last index of path separator

			stringstream ofile;
			ofile << ofolder.str();


			ofile << ifile.substr(lstIndex+1, ifile.length()-lstIndex-5); // base filename
			ofile << "_" << *age << ".tiff";	// _<age>.jpg is appended

		//	cout << ofile.str() << endl;


			Mat m = imread(ifile, CV_LOAD_IMAGE_GRAYSCALE); // load


			_defects.applyDefects(m,*age);					// modify (age)

			imwrite(ofile.str(), m);						// write


		}


	}
	// for all ages
	// For all:
	// Load, age accordingly
}

SensorAger::~SensorAger() {
	// TODO Auto-generated destructor stub
}


/*
 * main.cpp
 *
 *  Created on: Jan 3, 2014
 *      Author: tbergmueller
 */

#include <iostream>
#include <vector>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <assert.h>
#include <algorithm>

using namespace std;
using namespace cv;



#define NR_OF_FILES		4640
#define MEDIAN_KERNEL_SIZE	7


cv::Mat calcPixelMean(const vector<cv::Mat>& inputImages)
{
	assert(inputImages.size() > 2);
	Mat mean = Mat::zeros(inputImages[0].size(), CV_32FC1);

	float fact = (float)(1.0/inputImages.size());

	for(vector<cv::Mat>::const_iterator it = inputImages.begin(); it != inputImages.end(); it++)
	{
		assert(it->size() == mean.size());
		for(int x=0; x<it->cols; x++)
		{
			for(int y=0; y<it->rows; y++)
			{
				mean.at<float>(y,x) += (float)(it->at<uchar>(y,x));
			}
		}
	}

	mean = mean * fact;

	return mean;
}


void loadBitmaps(string prefix, int amount, vector<Mat>& output_images)
{
	output_images.clear();

	cout << "Load " << amount << " images from " << prefix << "... " << endl;

	for(int i=1; i<= amount; i++)
	{
		stringstream ss;
		ss << prefix << i << ".bmp";

		Mat buff = imread(ss.str(), CV_LOAD_IMAGE_GRAYSCALE);

		if(buff.empty())
		{
			cerr << "Could not load image " << ss.str() << "... Aborting... " << endl;
			output_images.clear();
			return;
		}

		output_images.push_back(buff);
	}

}



float calcmedian(vector<float>& scores)
{
  float median;
  size_t size = scores.size();

  sort(scores.begin(), scores.end());

  if (size  % 2 == 0)
  {
      median = (scores[size / 2 - 1] + scores[size / 2]) / 2;
  }
  else
  {
      median = scores[size / 2];
  }

  return median;
}



void myMedian(const Mat& input, Mat& output, int kernel)
{
	output = Mat::zeros(Size(input.cols, input.rows), CV_32FC1);

	assert(kernel % 2 == 1);
	int kHalf = (int)(kernel/2); // floor

	vector<float> list;

	for(int y = kHalf; y < (input.rows) - kHalf; y++)
	{
		for(int x=kHalf; x<(input.cols) - kHalf; x++)
		{
			// Inner loop... find median
			list.clear();

			for(int yy = -kHalf; yy <= kHalf; yy++)
			{
				for(int xx = -kHalf; xx <= kHalf; xx++)
				{
					list.push_back(input.at<float>(y+yy,x+xx));
				}
			}

			output.at<float>(y,x) = calcmedian(list);
		}
	}
}


Mat getDefectMat(const vector<Mat>& inputImages, Mat& oMean, int medianKernelSize = 3)
{
	Mat mean = calcPixelMean(inputImages);
	int kHalf = (int)(medianKernelSize / 2);


	Rect roi(kHalf,kHalf,mean.cols-2*kHalf, mean.rows-2*kHalf);

	// Reduce size because outside is not considered due to mean

	Mat smoothed;
	myMedian(mean, smoothed, medianKernelSize);

	oMean = mean(roi);

	imwrite("smoothed.png", smoothed(roi));

	Mat defects;
	subtract(mean,smoothed,defects);

	return defects(roi);


}



void writeImg(string prefix, string name, Mat& toWrite)
{
	stringstream ss;
	ss << prefix << "_" << name << ".png";

	//cout << ss.str() << endl;
	imwrite(ss.str(),toWrite);
}


int countDefects(const Mat& defectLocationMat)
{
	int cnt = 0;

	for(int x=0; x<defectLocationMat.cols; x++)
	{
		for(int y=0; y<defectLocationMat.rows; y++)
		{
			if(defectLocationMat.at<uchar>(y,x) != 0)
			{
				cnt++;
			}
		}
	}
	return cnt;
}


void findStuckPixels(const vector<Mat>& inputs, Mat& o_stuckPixels)
{
	Mat lastImage = inputs.begin()->clone();

	Mat diffs = Mat::zeros(Size(lastImage.cols, lastImage.rows), CV_8UC1);

	for(vector<cv::Mat>::const_iterator it = inputs.begin()+1; it != inputs.end(); it++)
	{

		diffs += abs((lastImage-*it));

		/*imshow("CurImg", *it);

		Mat debug;
		normalize(diffs,debug,0,255,CV_MINMAX,CV_8UC1);
		imshow("Curstep", debug);
		waitKey();
*/
		lastImage = it->clone();
	}

	threshold(diffs,o_stuckPixels,1,255,CV_THRESH_BINARY_INV);
	//imshow("Stucks", o_stuckPixels);
	//waitKey();
}


int global_areaCorrelated = 0;


void optimizeCorrectness(const Mat& defects09, const Mat& defects13, Mat& refinedLocations)
{
	// Thres
	int thres2009 = 217;
	int thres2013 = 206;

	Mat w09, w13;

	normalize(defects09,w09,0,255, NORM_MINMAX, CV_8UC1);
	normalize(defects13,w13,0,255, NORM_MINMAX, CV_8UC1);


	// Do classification by thresholding, FIXME: Hardcoded parameters
	threshold(w09,w09,thres2009,255,CV_THRESH_BINARY);
	threshold(w13,w13,thres2013,255,CV_THRESH_BINARY);

	Mat matches;

	bitwise_and(w09,w13,matches);
	writeImg("img/impr_2009", "defectLocation", w09);
	writeImg("img/impr_2013", "defectLocation", w13);
	writeImg("img/impr_", "matchingDefects",matches);

	imshow("Defect Locations 2009", w09);
	imshow("Defect Locations 2013", w13);
	imshow("Defect Locations 2009 that match 2013", matches);

	double defectsIn2013 = countDefects(w13);
	double defectsIn2009 = countDefects(w09);
	double defectsThatMatch = countDefects(matches);

	cout << "Defects in 2009 are " << defectsIn2009 << endl;
	cout << "Defects in 2013 are " << defectsIn2013 << endl;
	cout << "Defects that match are " << defectsThatMatch << endl;

	if(defectsIn2013 < defectsIn2009)
	{
		cout << "There are more defects in 2009 than in  2013 ... means there is something wrong..." << endl;
	}


	if(defectsIn2009)
	{
		double correctnessRatio = defectsThatMatch / defectsIn2009;
		cout << "Correctness ratio is " << correctnessRatio << endl;

		cout << "Sensor: " << defects09.cols * defects09.rows << " pixels" << endl;
		cout << "- correlated region (" << global_areaCorrelated << " pixels)" << endl;

		double nrOfPixels = defects09.cols * defects09.rows - global_areaCorrelated;

		cout << "CONSIDERED " << nrOfPixels << " pixels" << endl;

		double nrOfIncreases = (defectsIn2013-defectsIn2009);

		nrOfIncreases *= correctnessRatio;

		double lambda_ps = nrOfIncreases / (4 * nrOfPixels/1000);

		cout << "(corrected) lambda_ps = " << lambda_ps << " defects per year per 1000 pixel" << endl;


	}
	else
	{
		cout << "No defects detected in 2009.. impossible to run algo properly.." << endl;
	}


	refinedLocations = matches;



}


void processInputs(vector<Mat>& inputs, string file_prefix, Mat& o_defects, Mat& o_defect_locations, Mat& oMean)
{
	cout << "Calculate defects for " << file_prefix << endl;

	o_defects = getDefectMat(inputs, oMean, MEDIAN_KERNEL_SIZE);

	writeImg(file_prefix, "meanImage", oMean);
	writeImg(file_prefix, "defectMat", o_defects);

	Mat d; // defect working mat for classification
	d = o_defects.clone();

	Mat d_out;
	normalize(d, d_out,0,255,CV_MINMAX, CV_8UC1);
	writeImg(file_prefix, "defectMatNormalized", d_out);

	// TODO: Hardcoded
	// Rule out correlated region

	rectangle(d,Point(0,0), Point(d.cols-1, 2),Scalar::all(0),CV_FILLED);


	Rect middle(100,130,430,208);
	rectangle(d,middle,Scalar::all(0),CV_FILLED);

	global_areaCorrelated = middle.width*middle.height+2*d.cols;
//<<	global_areaCorrelated = 0;
	d.at<float>(0,0) = 0.0;

	o_defects = d.clone();

	normalize(d,d,0,255, NORM_MINMAX, CV_8UC1);
	writeImg(file_prefix, "defects_uncorrelated",d);

	// Do classification by thresholding, FIXME: Hardcoded parameters
	threshold(d,d,190,255,CV_THRESH_BINARY);

	writeImg(file_prefix, "defects_classified",d);


	o_defect_locations = d;
}



void stat_hotPixels(Mat& locations, Mat& defectMat, vector<float>& hotVals)
{

	for(int x=0; x<locations.cols; x++)
		{
			for(int y=0; y<locations.rows; y++)
			{
				if(locations.at<uchar>(y,x) != 0)
				{
					cout << defectMat.at<float>(y,x) << endl;
					hotVals.push_back(defectMat.at<float>(y,x));
				}
			}
		}
}


int main()
{
	vector<Mat> inputs;

	Mat defects_2009, defect_loc_2009;
	Mat mean_2009, mean_2013;
	Mat defects_2013, defect_loc_2013;
	Mat matches;

	loadBitmaps("/var/run/media/tbergmueller/WD 1TB/iris/aged/2009/h100/", NR_OF_FILES, inputs);
	//loadBitmaps("testimg/2009/", 10, inputs);
	processInputs(inputs,"img/2009",defects_2009, defect_loc_2009, mean_2009);

	loadBitmaps("/var/run/media/tbergmueller/WD 1TB/iris/aged/2013/h100/", NR_OF_FILES, inputs);
	//loadBitmaps("testimg/2013/", 10, inputs);
	processInputs(inputs,"img/2013",defects_2013, defect_loc_2013, mean_2013);

/*
	// Stucks
	Mat stucks09, stucks13;
	findStuckPixels(inputs,stucks09);
	findStuckPixels(inputs,stucks13);

	int addedStucks = countDefects(stucks13) - countDefects(stucks09);
	cout << "There was an increase of " << addedStucks << " stucks detected" << endl;

	bitwise_and(defect_loc_2009,defect_loc_2013,matches);
	writeImg("img/", "matchingDefects",matches);

	imshow("Defect Locations 2009", defect_loc_2009);
	imshow("Defect Locations 2013", defect_loc_2013);
	imshow("Defect Locations 2009 that match 2013", matches);

	double defectsIn2013 = countDefects(defect_loc_2013);
	double defectsIn2009 = countDefects(defect_loc_2009);
	double defectsThatMatch = countDefects(matches);

	cout << "Defects in 2009 are " << defectsIn2009 << endl;
	cout << "Defects in 2013 are " << defectsIn2013 << endl;
	cout << "Defects that match are " << defectsThatMatch << endl;

	if(defectsIn2013 < defectsIn2009)
	{
		cout << "There are more defects in 2009 than in  2013 ... means there is something wrong..." << endl;
		return 0;
	}


	if(defectsIn2009)
	{
		double correctnessRatio = defectsThatMatch / defectsIn2009;
		cout << "Correctness ratio is " << correctnessRatio << endl;

		cout << "Sensor: " << defect_loc_2009.cols * defect_loc_2009.rows << " pixels" << endl;
		cout << "- correlated region (" << global_areaCorrelated << " pixels)" << endl;

		double nrOfPixels = defect_loc_2009.cols * defect_loc_2009.rows - global_areaCorrelated;

		cout << "CONSIDERED " << nrOfPixels << " pixels" << endl;

		double nrOfIncreases = (defectsIn2013-defectsIn2009);

		nrOfIncreases *= correctnessRatio;

		double lambda_ps = nrOfIncreases / (4 * nrOfPixels/1000);

		cout << "(corrected) lambda_ps = " << lambda_ps << " defects per year per 1000 pixel" << endl;

	}
	else
	{
		cout << "No defects detected in 2009.. impossible to run algo properly.." << endl;
	}

	//stat_hotPixels(matches,mean_2013, MEDIAN_KERNEL_SIZE);
*/

	cout << "REFINE!!!!!!" << endl;

	Mat optimiziedLocations;
	optimizeCorrectness(defects_2009,defects_2013, optimiziedLocations);

	vector<float> a09,a13;

	stat_hotPixels(optimiziedLocations,defects_2009,a09);
	stat_hotPixels(optimiziedLocations,defects_2013,a13);

	cout << "DefectNr,amplitude09,amplitude13" << endl;
	for(int i=0; i<a09.size(); i++)
	{
		cout << i << "," << a09[i] << "," << a13[i] << endl;
	}


	waitKey();

	return 0;
}





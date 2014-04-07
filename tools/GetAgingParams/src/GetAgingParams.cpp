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

using namespace std;
using namespace cv;


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
				mean.at<float>(y,x) += (float)(it->at<uchar>(y,x)) *fact ;
			}
		}
	}

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

Mat getDefectMat(const vector<Mat>& inputImages, Mat& oMean, int medianKernelSize = 3)
{
	Mat mean = calcPixelMean(inputImages);
	oMean = mean;
	Mat mean_illus(mean.size(), CV_8UC1);

	for(int x=0; x<mean.cols; x++)
	{
		for(int y=0; y<mean.rows; y++)
		{
			mean_illus.at<uchar>(y,x) = (uchar) mean.at<float>(y,x);
		}
	}

	//imshow("mean", mean_illus);
	//waitKey();
	Mat smoothed;
	medianBlur(mean_illus, smoothed, medianKernelSize);

	Mat defects;
	subtract(mean_illus,smoothed,defects);

	return defects;


}



#define NR_OF_FILES		2840
#define MEDIAN_KERNEL_SIZE	3

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


void processInputs(vector<Mat>& inputs, string file_prefix, Mat& o_defects, Mat& o_defect_locations, Mat& oMean)
{
	cout << "Calculate defects for " << file_prefix << endl;

	o_defects = getDefectMat(inputs, oMean, MEDIAN_KERNEL_SIZE);

	Mat d; // defect working mat for classification
	normalize(o_defects,d,0,255, NORM_MINMAX, CV_8UC1);
	writeImg(file_prefix, "defects_with_correlated_data",d);

	// Do classification by thresholding, FIXME: Hardcoded parameters
	threshold(d,d,65,255,CV_THRESH_BINARY);
	writeImg(file_prefix, "defects_classified_with_correlated_data",d);

	// Remove correlated region (replace with black)
	rectangle(d,Point(0,0), Point(d.cols-1, 50),Scalar::all(0),CV_FILLED);
	rectangle(d,Point(100,50), Point(d.cols-100, d.rows-50),Scalar::all(0),CV_FILLED);
	writeImg(file_prefix, "defects_classified",d);


	o_defect_locations = d;

}


void stat_hotPixels(Mat& locations, Mat& values, int medianKernelSize=3)
{
	vector<float> hotVals;

	int k = (int)medianKernelSize / 2;
	for(int x=0; x<locations.cols; x++)
		{
			for(int y=0; y<locations.rows; y++)
			{
				if(locations.at<uchar>(y,x) != 0)
				{

					if( x-k < 0 || y-k < 0 || x+k > locations.cols-1 || y+k > locations.rows-1)
					{
						continue;
					}
					Mat a = values(Rect(x-k,y-k,medianKernelSize,medianKernelSize));

					float mean = 0;

					for(int ix=0; ix<a.cols; ix++)
					{
						for(int iy=0; iy<a.rows; iy++)
						{
							mean += a.at<float>(iy,ix);
						}
					}

					mean /= medianKernelSize*medianKernelSize;

					float diff = values.at<float>(y,x) - mean;
					cout << diff << endl;
					hotVals.push_back(diff);

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


	loadBitmaps("/home/tbergmueller/bs/2009/img", NR_OF_FILES, inputs);
	processInputs(inputs,"img/2009",defects_2009, defect_loc_2009, mean_2009);

	loadBitmaps("/home/tbergmueller/bs/2013/img", NR_OF_FILES, inputs);
	processInputs(inputs,"img/2013",defects_2013, defect_loc_2013, mean_2013);

	bitwise_and(defect_loc_2009, defect_loc_2013, matches);

	imshow("Defect Locations 2009", defect_loc_2009);
	imshow("Defect Locations 2013", defect_loc_2013);
	imshow("Defect Locations 2009 that match 2013", matches);

	cout << "Defects in 2009 are " << countDefects(defect_loc_2009) << endl;
	cout << "Defects in 2013 are " << countDefects(defect_loc_2013) << endl;
	cout << "Defects that match are " << countDefects(matches) << endl;

	if(countDefects(defect_loc_2009))
	{
		double correctnessRatio = (double)countDefects(matches) / (double)countDefects(defect_loc_2009);
		cout << "Correctness ratio is " << correctnessRatio << endl;
	}

	stat_hotPixels(matches,mean_2013, MEDIAN_KERNEL_SIZE);






	waitKey();

	return 0;
}





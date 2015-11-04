// DIP_HW.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "opencv2/opencv.hpp"
#include "pxcmetadata.h"
#include "pxcsensemanager.h"
#include "pxcprojection.h"
#include <iostream>
#include <cstdlib>

#define IMAGE_WIDTH   640	
#define IMAGE_HEIGHT  480
#define FPS            30


PXCSenseManager *mPXCSenseManager;
PXCProjection   *mPXCProjection;

using namespace std;
using namespace cv;

bool startVideoStream();
void pxcDepthImg2cvDepthImg(PXCImage *srcImg, Mat &depthImg);
void histogram(Mat image);

int _tmain(int argc, _TCHAR* argv[])
{
#if 1

	int index=0;
	if(!startVideoStream())
	{
		system("pause");
		exit(-1);
	}
	Mat depthImg;
	Mat nthuImg[300];
	Mat subImage;

	while (true)
	{
		string imgname = "paitaxing";
		string subname = ".bmp";
		string name;
		char tmp[5];

		_itoa_s(index,tmp,10);
		name = imgname + tmp + subname;
		
		if(index>=300)	break;

		PXCCapture::Sample *sample;
		if (mPXCSenseManager->AcquireFrame(true) < PXC_STATUS_NO_ERROR) break;

		sample = mPXCSenseManager->QuerySample();
		
		pxcDepthImg2cvDepthImg(sample->depth, depthImg);

		subImage = Mat(depthImg, Rect(280,  303 ,383-280 ,350-303 ));

		nthuImg[index] = subImage.clone();

		//imwrite ("depthImage.bmp", depthImg);
		imwrite(name, subImage);
		//goto End;
//		imwrite("subimage.bmp", subImage);

		mPXCSenseManager->ReleaseFrame();
		
		index++;

		if(waitKey(1) == 'q')
			break;
	}

	imwrite ("depthImage.bmp", depthImg);
	
	double miu, X, mean, var_x;
	int row, col;
	row = nthuImg[0].rows;
	col = nthuImg[0].cols;

	double **snsd = new double*[row];
	for(int i=0; i<row; i++){
		snsd[i] = new double[col];
	}

	mean=0;
	var_x=0;

	for(int i=0; i<row; i++){
		for(int j=0; j<col; j++){
			X=0;
			miu=0;
			for(int k=0; k<300; k++){
				X=X+(pow(nthuImg[k].at<ushort>(i, j), 2)/300);
				miu=miu+nthuImg[k].at<ushort>(i, j);
//		printf("nthuImg: %d, pow:%f, pow/300:%f\n",nthuImg[k].at<ushort>(i, j), pow(nthuImg[k].at<ushort>(i, j), 2), pow(nthuImg[k].at<ushort>(i, j), 2)/300);
			}
			miu=pow(miu/300, 2);
			if((X-miu)<0)
				snsd[i][j]=0;
			else
				snsd[i][j]=sqrt(X-miu);
			mean=mean+snsd[i][j];
//			printf("snsd:%f, X:%f, miu:%f \n", snsd[i][j], X, miu);

			var_x=var_x+(pow(snsd[i][j], 2));
			
		}
		//printf("\n");
	}
		
	mean=mean/(row*col);
	var_x=var_x/(row*col)-pow(mean, 2);
	printf("mean:%f  variance:%f\n", mean, var_x);

	Mat result = subImage.clone();

	for(int i=0; i<row; i++){
		for(int j=0; j<col; j++){
			result.at<ushort>(i, j)=snsd[i][j]*100;
			if(result.at<ushort>(i, j)>255){
				result.at<ushort>(i, j)=255;
			}
		}
	}

	Mat eq_result;

	/*printf("type:%d\n", eq_result.type());
	
	cvCvtColor(&eq_result, &eq_result, CV_RGB2GRAY);
	*/
	Mat gray;
	result.convertTo(gray,CV_8U);
	//cvtColor(result,gray,COLOR_BGR2GRAY);

	equalizeHist(gray, eq_result);

	imwrite("paitaxing_sub.bmp", subImage);
	
	imwrite("eq_result.bmp", eq_result);
	imwrite("result.bmp", result);

	mPXCSenseManager->Release();
End:
    return 0;

#endif
	/*
	Mat hist;
	hist = imread("C:/Users/Patrick/Desktop/report/80q.png", CV_LOAD_IMAGE_ANYCOLOR);
	histogram(hist);
	*/

    return 0;
}

void histogram(Mat image)
{
	int ROW = image.rows;
	int COL = image.cols;
	int		Hitg[256] = {0};


	for(int i=0; i < ROW; i++){
		for(int j=0; j < COL; j++)		{
		    uint tmpRGB = (uint)image.at<uchar>(i, j);
			Hitg[tmpRGB]++;
		}
	}

	FILE *fp;
	
	if(!fopen_s(&fp, "his.txt", "w")){
		for(int i=0; i<256; i++){
			fprintf(fp, "%d ", Hitg[i]);
		}

		fclose(fp);
	}
}

bool startVideoStream()
{
	mPXCSenseManager = PXCSenseManager::CreateInstance();
    if (!mPXCSenseManager) {
        wprintf_s(L"Unable to create the SenseManager\n");
        return false;
    }
	mPXCSenseManager->EnableStream(PXCCapture::STREAM_TYPE_DEPTH, IMAGE_WIDTH, IMAGE_HEIGHT, FPS);
	
	while (mPXCSenseManager->Init() != PXC_STATUS_NO_ERROR)
	{
		std::cout <<"init fail !~" << std::endl;
	}

	mPXCProjection = mPXCSenseManager->QueryCaptureManager()->QueryDevice()->CreateProjection();

	return true;
}



void pxcDepthImg2cvDepthImg(PXCImage *srcImg, Mat &depthImg)
{
	int width  = srcImg->QueryInfo().width;
	int height = srcImg->QueryInfo().height;

	//int width    = IMAGE_WIDTH;
	//int height   = IMAGE_HEIGHT;

	depthImg = Mat::zeros(height, width, CV_16UC1);

	//
	PXCImage::ImageData *data = new PXCImage::ImageData();

	//
	data->format = PXCImage::PIXEL_FORMAT_DEPTH;

	//
	srcImg->AcquireAccess(PXCImage::ACCESS_READ, data->format, data);

	ushort *depthValues = new ushort[width * height];
	memcpy(depthValues, data->planes[0], sizeof(ushort) * width * height);

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			int idx = width*i + j;
			depthImg.at<ushort>(i, j) = static_cast<ushort>(depthValues[idx]);
		}
	}

	srcImg->ReleaseAccess(data);
	delete data;
}


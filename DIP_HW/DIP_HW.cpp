// DIP_HW.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "opencv2/opencv.hpp"
#include "pxcmetadata.h"
#include "pxcsensemanager.h"
#include "pxcprojection.h"


#define IMAGE_WIDTH   640	
#define IMAGE_HEIGHT  480
#define FPS            30


PXCSenseManager *mPXCSenseManager;
PXCProjection   *mPXCProjection;

using namespace cv;

bool startVideoStream();
void pxcDepthImg2cvDepthImg(PXCImage *srcImg, Mat &depthImg);

int _tmain(int argc, _TCHAR* argv[])
{
	if(!startVideoStream())
	{
		system("pause");
		exit(-1);
	}

	while (true)
	{
		PXCCapture::Sample *sample;
		Mat depthImg;
		if (mPXCSenseManager->AcquireFrame(true) < PXC_STATUS_NO_ERROR) break;

		sample = mPXCSenseManager->QuerySample();

		
		pxcDepthImg2cvDepthImg(sample->depth, depthImg);


		imshow ("depth image", depthImg);

		imwrite("paitaxing.bmp", depthImg);
		break;

		mPXCSenseManager->ReleaseFrame();
		if(waitKey(1) == 'q')
			break;
	}

	mPXCSenseManager->Release();
    return 0;
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
			depthImg.at<ushort>(i, j) = static_cast<ushort>(depthValues[idx]*25);
		}
	}

	srcImg->ReleaseAccess(data);
	delete data;
}


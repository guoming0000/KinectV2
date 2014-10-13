// ColorBasic-OpenCV.cpp : 定义控制台应用程序的入口点。
/****************************************************
程序功能:Kinect V2彩色数据用OpenCV显示
开发环境:win32控制台应用程序 x86程序 (程序类型)
		VisualStudio 2012 (开发工具)
		OpenCV2.4.10 (显示界面库 vc11库, http://guoming.me/opencv)
		KinectSDK-v2.0-PublicPreview1409-Setup (Kinect SDK驱动版本, http://guoming.me/kinect2)
		Windows 8.1 (操作系统)
博客文章:http://guoming.me/kinectv2-color-opencv
代码地址:https://github.com/guoming0000/KinectV2/tree/master/ColorBasic-OpenCV

开发人员:小明
开发时间:2014-10-13~ 2014-10-14
联系方式:	i@guoming.me (邮箱，推荐联系方式)
		http://guoming.me (网站，体感资讯和知识汇总)
		http://weibo.com/guoming0000 (新浪微博，请勿私信)
******************************************************/
#include "stdafx.h"
#include "opencv2/opencv.hpp"
#include <windows.h>
#include <Kinect.h>// Kinect Header files
using namespace cv;
// Safe release for interfaces
template<class Interface>
inline void SafeRelease(Interface *& pInterfaceToRelease)
{
	if (pInterfaceToRelease != NULL)
	{
		pInterfaceToRelease->Release();
		pInterfaceToRelease = NULL;
	}
}

//定义Kinect方法类
class Kinect
{
public:
	static const int        cColorWidth  = 1920;
	static const int        cColorHeight = 1080;
	Kinect();
	~Kinect();
	HRESULT					InitKinect();//初始化Kinect
	void					Update();//更新数据
	void					ProcessColor(RGBQUAD* pBuffer, int nWidth, int nHeight);//处理得到的数据
private:
	
	IKinectSensor*          m_pKinectSensor;// Current Kinect
	IColorFrameReader*      m_pColorFrameReader;// Color reader
	RGBQUAD*                m_pColorRGBX;

};

//主函数
int main()
{
	Kinect kinect;
	kinect.InitKinect();
	while(1)
	{
		kinect.Update();
		if(waitKey(1) >= 0)//按下任意键退出
		{
			break;
		}
	}

	return 0;
}

Kinect::Kinect()
{
	m_pKinectSensor = NULL;
	m_pColorFrameReader = NULL;
	
	m_pColorRGBX = new RGBQUAD[cColorWidth * cColorHeight];// create heap storage for color pixel data in RGBX format
}

Kinect::~Kinect()
{
	if (m_pColorRGBX)
	{
		delete [] m_pColorRGBX;
		m_pColorRGBX = NULL;
	}

	SafeRelease(m_pColorFrameReader);// done with color frame reader
	
	if (m_pKinectSensor)
	{
		m_pKinectSensor->Close();// close the Kinect Sensor
	}
	SafeRelease(m_pKinectSensor);
}

HRESULT	Kinect::InitKinect()
{
	HRESULT hr;

	hr = GetDefaultKinectSensor(&m_pKinectSensor);
	if (FAILED(hr))
	{
		return hr;
	}

	if (m_pKinectSensor)
	{
		// Initialize the Kinect and get the color reader
		IColorFrameSource* pColorFrameSource = NULL;

		hr = m_pKinectSensor->Open();

		if (SUCCEEDED(hr))
		{
			hr = m_pKinectSensor->get_ColorFrameSource(&pColorFrameSource);
		}

		if (SUCCEEDED(hr))
		{
			hr = pColorFrameSource->OpenReader(&m_pColorFrameReader);
		}

		SafeRelease(pColorFrameSource);
	}

	if (!m_pKinectSensor || FAILED(hr))
	{
		printf("No ready Kinect found! \n");
		return E_FAIL;
	}

	return hr;
}

void Kinect::Update()
{
	if (!m_pColorFrameReader)
	{
		return;
	}

	IColorFrame* pColorFrame = NULL;

	HRESULT hr = m_pColorFrameReader->AcquireLatestFrame(&pColorFrame);

	if (SUCCEEDED(hr))
	{
		IFrameDescription* pFrameDescription = NULL;
		int nWidth = 0;
		int nHeight = 0;
		ColorImageFormat imageFormat = ColorImageFormat_None;
		UINT nBufferSize = 0;
		RGBQUAD *pBuffer = NULL;

		if (SUCCEEDED(hr))
		{
			hr = pColorFrame->get_FrameDescription(&pFrameDescription);
		}

		if (SUCCEEDED(hr))
		{
			hr = pFrameDescription->get_Width(&nWidth);
		}

		if (SUCCEEDED(hr))
		{
			hr = pFrameDescription->get_Height(&nHeight);
		}

		if (SUCCEEDED(hr))
		{
			hr = pColorFrame->get_RawColorImageFormat(&imageFormat);
		}

		if (SUCCEEDED(hr))
		{
			if (imageFormat == ColorImageFormat_Bgra)
			{
				hr = pColorFrame->AccessRawUnderlyingBuffer(&nBufferSize, reinterpret_cast<BYTE**>(&pBuffer));
			}
			else if (m_pColorRGBX)
			{
				pBuffer = m_pColorRGBX;
				nBufferSize = cColorWidth * cColorHeight * sizeof(RGBQUAD);
				hr = pColorFrame->CopyConvertedFrameDataToArray(nBufferSize, reinterpret_cast<BYTE*>(pBuffer), ColorImageFormat_Bgra);            
			}
			else
			{
				hr = E_FAIL;
			}
		}

		if (SUCCEEDED(hr))
		{
			ProcessColor(pBuffer, nWidth, nHeight);
		}

		SafeRelease(pFrameDescription);
	}

	SafeRelease(pColorFrame);
}

void Kinect::ProcessColor(RGBQUAD* pBuffer, int nWidth, int nHeight)
{
	// Make sure we've received valid data
	if (pBuffer && (nWidth == cColorWidth) && (nHeight == cColorHeight))
	{
		// Draw the data with OpenCV
		Mat ColorImage(nHeight, nWidth, CV_8UC4, pBuffer);
		Mat showImage;
		resize(ColorImage, showImage, Size(nWidth / 2, nHeight / 2));
		imshow("ColorImage", showImage);////imshow("ColorImage", ColorImage);
	}
}

#pragma once
#include <Kinect.h>// Kinect Header files
#include "include_opencv/opencv2/opencv.hpp"
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
class CKinect
{
public:
	static const int        cDepthWidth = 512;
	static const int        cDepthHeight = 424;
	static const int        cColorWidth = 1920;
	static const int        cColorHeight = 1080;
	CKinect();
	~CKinect();
	HRESULT					InitKinect();//初始化Kinect
	void					Update();//更新数据
	void                    ProcessFrame(INT64 nTime,
										 const UINT16* pDepthBuffer, int nDepthHeight, int nDepthWidth, USHORT nMinDepth, USHORT nMaxDepth,
										 const RGBQUAD* pColorBuffer, int nColorWidth, int nColorHeight,
										 int nBodyCount, IBody** ppBodies,
										 const BYTE* pBodyIndexBuffer, int nBodyIndexWidth, int nBodyIndexHeight);
private:
	IKinectSensor*          m_pKinectSensor;// Current Kinect
	// Frame reader
	IMultiSourceFrameReader*m_pMultiSourceFrameReader;

	ICoordinateMapper*      m_pCoordinateMapper;
	DepthSpacePoint*        m_pDepthCoordinates;
	RGBQUAD*                m_pOutputRGBX;
	RGBQUAD*                m_pColorRGBX;
	RGBQUAD*                m_pBackgroundRGBX;
	RGBQUAD*                m_pDepthRGBX;

	Mat						m_Depth;
	Mat						m_Color;
	Mat						m_BodyIndex;
};


#include "stdafx.h"
#include "Kinect.h"
#include "Head.h"
#include <math.h>
#include <limits>
#include <strsafe.h>
static const DWORD c_AppRunTime = 5 * 60;//程序运行时间(s)，设置5*60表示运行5分钟后程序自动关闭
static const float c_JointThickness = 3.0f;
static const float c_TrackedBoneThickness = 6.0f;
static const float c_InferredBoneThickness = 1.0f;
static const float c_HandSize = 30.0f;
CKinect::CKinect()
{
	m_pKinectSensor = NULL;
	m_pCoordinateMapper = NULL;
	m_pMultiSourceFrameReader = NULL;
	m_pDepthCoordinates = NULL;
	m_pOutputRGBX = NULL;
	m_pBackgroundRGBX = NULL;
	m_pColorRGBX = NULL;

	// create heap storage for composite image pixel data in RGBX format
	m_pOutputRGBX = new RGBQUAD[cColorWidth * cColorHeight];

	// create heap storage for background image pixel data in RGBX format
	m_pBackgroundRGBX = new RGBQUAD[cColorWidth * cColorHeight];

	// create heap storage for color pixel data in RGBX format
	m_pColorRGBX = new RGBQUAD[cColorWidth * cColorHeight];

	// create heap storage for the coorinate mapping from color to depth
	m_pDepthCoordinates = new DepthSpacePoint[cColorWidth * cColorHeight];

	m_pDepthRGBX = new RGBQUAD[cDepthWidth * cDepthHeight];// create heap storage for color pixel data in RGBX format

	//初始化OpenCV数组
	m_Depth.create(cDepthHeight, cDepthWidth, CV_16UC1);
	m_Color.create(cColorHeight, cColorWidth, CV_8UC4);
	m_BodyIndex.create(cDepthHeight, cDepthWidth, CV_8UC1);
}


CKinect::~CKinect()
{
	SAFE_RELEASE_VEC(m_pOutputRGBX);
	SAFE_RELEASE_VEC(m_pBackgroundRGBX);
	SAFE_RELEASE_VEC(m_pColorRGBX);
	SAFE_RELEASE_VEC(m_pDepthCoordinates);
	SAFE_RELEASE_VEC(m_pDepthRGBX);

	// done with frame reader
	SafeRelease(m_pMultiSourceFrameReader);

	// done with coordinate mapper
	SafeRelease(m_pCoordinateMapper);

	// close the Kinect Sensor
	if (m_pKinectSensor)
	{
		m_pKinectSensor->Close();
	}

	SafeRelease(m_pKinectSensor);
}


HRESULT	CKinect::InitKinect()
{
	HRESULT hr;

	hr = GetDefaultKinectSensor(&m_pKinectSensor);
	if (FAILED(hr))
	{
		return hr;
	}

	if (m_pKinectSensor)
	{
		// Initialize the Kinect and get coordinate mapper and the frame reader
		if (SUCCEEDED(hr))
		{
			hr = m_pKinectSensor->get_CoordinateMapper(&m_pCoordinateMapper);
		}

		hr = m_pKinectSensor->Open();

		if (SUCCEEDED(hr))
		{
			hr = m_pKinectSensor->OpenMultiSourceFrameReader(
				FrameSourceTypes::FrameSourceTypes_Depth | FrameSourceTypes::FrameSourceTypes_Color | FrameSourceTypes::FrameSourceTypes_Body | FrameSourceTypes::FrameSourceTypes_BodyIndex,
				&m_pMultiSourceFrameReader);

//			hr = m_pKinectSensor->OpenMultiSourceFrameReader(FrameSourceTypes::FrameSourceTypes_Color ,	&m_pMultiSourceFrameReader);
		}
	}

	if (!m_pKinectSensor || FAILED(hr))
	{
		return E_FAIL;
	}

	return hr;
}

void CKinect::Update()
{
	if (!m_pMultiSourceFrameReader)
	{
		return;
	}

	IMultiSourceFrame* pMultiSourceFrame = NULL;
	IDepthFrame* pDepthFrame = NULL;
	IColorFrame* pColorFrame = NULL;
	IBodyIndexFrame* pBodyIndexFrame = NULL;
	IBodyFrame *pBodyFrame = NULL;

	HRESULT hr = m_pMultiSourceFrameReader->AcquireLatestFrame(&pMultiSourceFrame);

	if (SUCCEEDED(hr))//深度信息
	{
		IDepthFrameReference* pDepthFrameReference = NULL;
		hr = pMultiSourceFrame->get_DepthFrameReference(&pDepthFrameReference);
		if (SUCCEEDED(hr))
		{
			hr = pDepthFrameReference->AcquireFrame(&pDepthFrame);
		}
		SafeRelease(pDepthFrameReference);
	}

	if (SUCCEEDED(hr))//彩色信息
	{
		IColorFrameReference* pColorFrameReference = NULL;
		hr = pMultiSourceFrame->get_ColorFrameReference(&pColorFrameReference);
		if (SUCCEEDED(hr))
		{
			hr = pColorFrameReference->AcquireFrame(&pColorFrame);
		}
		SafeRelease(pColorFrameReference);
	}

	if (SUCCEEDED(hr))//骨骼信息
	{
		IBodyFrameReference* pBodyFrameReference = NULL;
		hr = pMultiSourceFrame->get_BodyFrameReference(&pBodyFrameReference);
		if (SUCCEEDED(hr))
		{
			hr = pBodyFrameReference->AcquireFrame(&pBodyFrame);
		}
		SafeRelease(pBodyFrameReference);
	}

	if (SUCCEEDED(hr))//人体掩膜部分
	{
		IBodyIndexFrameReference* pBodyIndexFrameReference = NULL;
		hr = pMultiSourceFrame->get_BodyIndexFrameReference(&pBodyIndexFrameReference);
		if (SUCCEEDED(hr))
		{
			hr = pBodyIndexFrameReference->AcquireFrame(&pBodyIndexFrame);
		}
		SafeRelease(pBodyIndexFrameReference);
	}

	if (SUCCEEDED(hr))
	{
		INT64 nDepthTime = 0;
		IFrameDescription* pDepthFrameDescription = NULL;
		int nDepthWidth = 0;
		int nDepthHeight = 0;
		UINT nDepthBufferSize = 0;
		UINT16 *pDepthBuffer = NULL;
		USHORT nDepthMinReliableDistance = 0;
		USHORT nDepthMaxDistance = 0;

		IFrameDescription* pColorFrameDescription = NULL;
		int nColorWidth = 0;
		int nColorHeight = 0;
		ColorImageFormat imageFormat = ColorImageFormat_None;
		UINT nColorBufferSize = 0;
		RGBQUAD *pColorBuffer = NULL;

		IBody* ppBodies[BODY_COUNT] = { 0 };

		IFrameDescription* pBodyIndexFrameDescription = NULL;
		int nBodyIndexWidth = 0;
		int nBodyIndexHeight = 0;
		UINT nBodyIndexBufferSize = 0;
		BYTE *pBodyIndexBuffer = NULL;

		// get depth frame data
		hr = pDepthFrame->get_RelativeTime(&nDepthTime);

		if (SUCCEEDED(hr))
		{
			hr = pDepthFrame->get_FrameDescription(&pDepthFrameDescription);
		}

		if (SUCCEEDED(hr))
		{
			hr = pDepthFrameDescription->get_Width(&nDepthWidth);
		}

		if (SUCCEEDED(hr))
		{
			hr = pDepthFrameDescription->get_Height(&nDepthHeight);
		}

		if (SUCCEEDED(hr))
		{
			hr = pDepthFrame->get_DepthMinReliableDistance(&nDepthMinReliableDistance);
		}

		if (SUCCEEDED(hr))
		{
			nDepthMaxDistance = USHRT_MAX;
		}

		if (SUCCEEDED(hr))
		{
			hr = pDepthFrame->AccessUnderlyingBuffer(&nDepthBufferSize, &pDepthBuffer);
		}

		m_Depth = Mat(nDepthHeight, nDepthWidth, CV_16UC1, pDepthBuffer).clone();///////////////

		// get color frame data
		if (SUCCEEDED(hr))
		{
			hr = pColorFrame->get_FrameDescription(&pColorFrameDescription);
		}

		if (SUCCEEDED(hr))
		{
			hr = pColorFrameDescription->get_Width(&nColorWidth);
		}

		if (SUCCEEDED(hr))
		{
			hr = pColorFrameDescription->get_Height(&nColorHeight);
		}

		if (SUCCEEDED(hr))
		{
			hr = pColorFrame->get_RawColorImageFormat(&imageFormat);
		}

		if (SUCCEEDED(hr))
		{
			if (imageFormat == ColorImageFormat_Bgra)
			{
				hr = pColorFrame->AccessRawUnderlyingBuffer(&nColorBufferSize, reinterpret_cast<BYTE**>(&pColorBuffer));
			}
			else if (m_pColorRGBX)
			{
				pColorBuffer = m_pColorRGBX;
				nColorBufferSize = cColorWidth * cColorHeight * sizeof(RGBQUAD);
				hr = pColorFrame->CopyConvertedFrameDataToArray(nColorBufferSize, reinterpret_cast<BYTE*>(pColorBuffer), ColorImageFormat_Bgra);
			}
			else
			{
				hr = E_FAIL;
			}
		}

		m_Color = Mat(nColorHeight, nColorWidth, CV_8UC4, pColorBuffer);///////////////

		if (SUCCEEDED(hr))
		{
			hr = pBodyFrame->GetAndRefreshBodyData(_countof(ppBodies), ppBodies);
		}

		// get body index frame data
		if (SUCCEEDED(hr))
		{
			hr = pBodyIndexFrame->get_FrameDescription(&pBodyIndexFrameDescription);
		}

		if (SUCCEEDED(hr))
		{
			hr = pBodyIndexFrameDescription->get_Width(&nBodyIndexWidth);
		}

		if (SUCCEEDED(hr))
		{
			hr = pBodyIndexFrameDescription->get_Height(&nBodyIndexHeight);
		}

		if (SUCCEEDED(hr))
		{
			hr = pBodyIndexFrame->AccessUnderlyingBuffer(&nBodyIndexBufferSize, &pBodyIndexBuffer);
		}
		m_BodyIndex = Mat(nBodyIndexHeight, nBodyIndexWidth, CV_8UC1, pBodyIndexBuffer);

		if (SUCCEEDED(hr))
		{
			ProcessFrame(nDepthTime, pDepthBuffer, nDepthWidth, nDepthHeight, nDepthMinReliableDistance, nDepthMaxDistance,
						 pColorBuffer, nColorWidth, nColorHeight,
						 BODY_COUNT, ppBodies,
						 pBodyIndexBuffer, nBodyIndexWidth, nBodyIndexHeight);
		}

		SafeRelease(pDepthFrameDescription);
		SafeRelease(pColorFrameDescription);
		SafeRelease(pBodyIndexFrameDescription);

		for (int i = 0; i < _countof(ppBodies); ++i)
		{
			SafeRelease(ppBodies[i]);
		}
	}

	SafeRelease(pDepthFrame);
	SafeRelease(pColorFrame);
	SafeRelease(pBodyFrame);
	SafeRelease(pBodyIndexFrame);
	SafeRelease(pMultiSourceFrame);
}

void CKinect::ProcessFrame(INT64 nTime,
						   const UINT16* pDepthBuffer, int nDepthWidth, int nDepthHeight, USHORT nMinDepth, USHORT nMaxDepth,
						   const RGBQUAD* pColorBuffer, int nColorWidth, int nColorHeight,
						   int nBodyCount, IBody** ppBodies,
						   const BYTE* pBodyIndexBuffer, int nBodyIndexWidth, int nBodyIndexHeight)
{
	LARGE_INTEGER qpcNow = { 0 };
	WCHAR szStatusMessage[64];

	// Make sure we've received valid data
	if (m_pCoordinateMapper && m_pDepthCoordinates && m_pOutputRGBX &&
		pDepthBuffer && (nDepthWidth == cDepthWidth) && (nDepthHeight == cDepthHeight) &&
		pColorBuffer && (nColorWidth == cColorWidth) && (nColorHeight == cColorHeight) &&
		pBodyIndexBuffer && (nBodyIndexWidth == cDepthWidth) && (nBodyIndexHeight == cDepthHeight) &&
		m_pDepthRGBX)
	{
		HRESULT hr = m_pCoordinateMapper->MapColorFrameToDepthSpace(nDepthWidth * nDepthHeight, (UINT16*)pDepthBuffer, nColorWidth * nColorHeight, m_pDepthCoordinates);
		if (FAILED(hr))
		{
			return;
		}
		// loop over output pixels
		for (int colorIndex = 0; colorIndex < (nColorWidth*nColorHeight); ++colorIndex)
		{
			// default setting source to copy from the background pixel
			const RGBQUAD* pSrc = m_pBackgroundRGBX + colorIndex;

			DepthSpacePoint p = m_pDepthCoordinates[colorIndex];

			// Values that are negative infinity means it is an invalid color to depth mapping so we
			// skip processing for this pixel
			if (p.X != -std::numeric_limits<float>::infinity() && p.Y != -std::numeric_limits<float>::infinity())
			{
				int depthX = static_cast<int>(p.X + 0.5f);
				int depthY = static_cast<int>(p.Y + 0.5f);

				if ((depthX >= 0 && depthX < nDepthWidth) && (depthY >= 0 && depthY < nDepthHeight))
				{
					BYTE player = pBodyIndexBuffer[depthX + (depthY * cDepthWidth)];
					// if we're tracking a player for the current pixel, draw from the color camera
					if (player != 0xff)
					{
						// set source for copy to the color pixel
						pSrc = m_pColorRGBX + colorIndex;
					}
				}
			}
			// write output
			m_pOutputRGBX[colorIndex] = *pSrc;
		}
	}//确保参数都准确

	//imshow("color", m_Color);
	Mat showImage;
	resize(m_Color, showImage, Size(cColorWidth / 2, cColorHeight / 2));
	imshow("Color", showImage);////imshow("ColorImage", ColorImage);
	imshow("Depth", m_Depth);
	imshow("BodyIndex", m_BodyIndex);


	RGBQUAD* pRGBX = m_pDepthRGBX;
	// end pixel is start + width*height - 1
	const UINT16* pBufferEnd = pDepthBuffer + (nDepthWidth * nDepthHeight);
	while (pDepthBuffer < pBufferEnd)
	{
		USHORT depth = *pDepthBuffer;
		BYTE intensity = static_cast<BYTE>((depth >= nMinDepth) && (depth <= nMaxDepth) ? (depth % 256) : 0);
		pRGBX->rgbRed = intensity;
		pRGBX->rgbGreen = intensity;
		pRGBX->rgbBlue = intensity;
		++pRGBX;
		++pDepthBuffer;
	}

	// Draw the data nDepthHeight OpenCV
	Mat DepthImage(nDepthHeight, nDepthWidth, CV_8UC4, m_pDepthRGBX);
	Mat show = DepthImage.clone();
	imshow("DepthImage", show);


	waitKey(1);
}
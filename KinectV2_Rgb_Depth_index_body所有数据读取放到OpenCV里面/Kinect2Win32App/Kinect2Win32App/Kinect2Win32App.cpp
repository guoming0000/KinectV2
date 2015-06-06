// Kinect2Win32App.cpp : 定义控制台应用程序的入口点。
/****************************************************
程序功能:使用OpenCV显示KinectV2数据
开发环境:win32控制台应用程序 x86程序 (程序类型)
		VisualStudio 2013 (开发工具)
		KinectSDK-v2.0-PublicPreview1409-Setup (Kinect SDK驱动版本, http://guoming.me/kinect2)
		Windows 8.1 (操作系统)
开发人员:小明
开发时间:2015-4-11~ 2015-5-30
联系方式:	i@guoming.me (邮箱，推荐联系方式)
******************************************************/
#include "stdafx.h"
#include "Tools.h"
#include <windows.h>
#include <Mmsystem.h>//需要 Winmm.lib库的支持 ----timeGetTime()
#include "Kinect.h"
//主函数
int main()
{
	printf_s("start analysis Kinect for Windows V2 body data....");
	CKinect kinect;
	kinect.InitKinect();
	while(1)
	{
		kinect.Update();
	}
	return 0;
}

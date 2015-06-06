#pragma once
#include "stdafx.h"
#include <windows.h>
#include <Mmsystem.h>//需要 Winmm.lib库的支持 ----timeGetTime()
#include <string>
#include <Kinect.h>
using namespace std;
class CTools
{
public:
	static string DigitToString(int val)
	{
		char char_str[50];
		sprintf_s(char_str, "%d", val);
		return string(char_str);
	}

	//将64bit整数转换为字符串
	static string DigitToString(UINT64 val)
	{
		char char_str[50];
		sprintf_s(char_str, "%ld", val);
		return string(char_str);
	}

	//将32bit浮点数转换为字符串
	static string DigitToString(float val)
	{
		char char_str[50];
		sprintf_s(char_str, "%.2f", val);
		return string(char_str);
	}

	//将leap向量转换为字符串
	static string DigitToString(Vector4 vec)
	{
		char char_str[100];
		sprintf_s(char_str, "%.2f\t%.2f\t%.2f\t%.2f", vec.x, vec.y,vec.z,vec.w);
		return string(char_str);
	}

	//将标志转换为字符串
	static string DigitToString(bool flag)
	{
		return (flag == true) ? "true" : "false";
	}

	//将系统时间转换为字符串
	static string GetTimeString()
	{
		SYSTEMTIME sys;
		GetLocalTime(&sys);
		char char_str[100];
		sprintf_s(char_str, "%02d:%02d:%02d:%03d", sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds);
		return string(char_str);
	}
};


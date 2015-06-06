#pragma once
#include <fstream>
#include <string>
using namespace std;
class CFileWriter
{
public:
	void Write(string str);
	CFileWriter();
	~CFileWriter();
private:
	ofstream g_file;
};


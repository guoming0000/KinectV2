#include "stdafx.h"
#include "FileWriter.h"
CFileWriter::CFileWriter()
{
	g_file.open("data.txt", ofstream::out);
}


CFileWriter::~CFileWriter()
{
	g_file.close();
}

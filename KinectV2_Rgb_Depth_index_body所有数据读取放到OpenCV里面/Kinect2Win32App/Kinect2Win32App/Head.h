#pragma once
#include <iostream>
using namespace std;
inline void SAFE_RELEASE(void *p)
{
	if (p)
	{
		delete p;
		p = NULL;
	}
}
inline void SAFE_RELEASE_VEC(void *p)
{
	if (p)
	{
		delete []p;
		p = NULL;
	}
}
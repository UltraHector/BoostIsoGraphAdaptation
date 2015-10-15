#include <windows.h>
#include<iostream>


#ifndef TIME_UTILITY
#define TIME_UTILITY

class TimeUtility{
public:
	static void StartCounterMicro();
	static double GetCounterMicro();

	static void StartCounterMill();
	static double GetCounterMill();
};

#endif
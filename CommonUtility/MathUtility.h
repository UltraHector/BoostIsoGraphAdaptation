/**
* This file wraps the utility functions for MATH operations
*/

#ifndef MATH_UTILITY
#define MATH_UTILITY
#include<vector>
class Math_Utility{
public:

	static long combinations(int n, int k);
	static long factorial(int n);
	static void combinations(int n, int k, std::vector<std::vector<int>> & combinations);
};



#endif
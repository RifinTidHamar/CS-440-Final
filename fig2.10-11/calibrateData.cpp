#include <iostream>
#include <vector>
#include <string>

#include "calibrateData.h"

using namespace std;

// split data by delimiter ","
bool tokenize(string str, vector<string>& array)
{
	string delim = ",";
	size_t pos = 0;
	bool empty = true;

	while ((pos = str.find(delim)) != string::npos)
	{
		string token = str.substr(0, pos);

		// check for BOM (Byte Order Mark)
		if (token[0] == 'ï' && token[1] == '»' && token[2] == '¿')
			token.erase(0, 3);

		array.push_back(token);
		str.erase(0, pos + delim.length());

		empty = false;
	}
	if (str.length() > 0)
	{
		array.push_back(str);
		empty = false;
	}


	return empty;
}

// gets and then Min - Max normalizes data
void normalizeData(vector<string> array, double* allNum)
{
	size_t size = array.size();

	double max = numeric_limits<double>::lowest();
	double min = numeric_limits<double>::max();

	try
	{
		for (size_t i = 0; i < size; i++)
		{
			double n = stod(array.at(i));
			allNum[i] = n;

			if (allNum[i] > max) max = allNum[i];
			if (allNum[i] < min) min = allNum[i];
		}
	}
	catch (exception e)
	{
		// data can not be converted to double
		allNum[0] = -1;
		return;
	}

	double range = max - min;

	// normalize data
	if (range != 0)
	{
		for (int i = 0; i < size; i++)
			allNum[i] = (allNum[i] - min) / range;

	}
	else
	{
		for (int i = 0; i < size; i++)
			allNum[i] = 0.5;
	}
}
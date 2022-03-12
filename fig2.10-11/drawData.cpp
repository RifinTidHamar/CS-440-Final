// C++ libraries
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <limits>
#include <cmath>
#include <algorithm>

// OpenGL
#include <windows.h>
#include <gl/Gl.h>
#include <gl/Glu.h>
#include "glut.h"

// Group 5
#include "drawdata.h"

using namespace std;

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

void getData(vector<string> array, double* allNum)
{
	size_t size = array.size();

	double sum = 0;

	try
	{
		for (size_t i = 0; i < size; i++)
		{
			double n = stod(array.at(i));
			allNum[i] = n;
			sum += n;
		}
	}
	catch (exception e)
	{
		// data double not be converted to double
		// potentially inconsistent data (bad dataset)
		allNum[0] = -1;
		return;
	}
}

void drawData()
{
	string path;
	string line;

	cout << "Enter csv file path: ";
	getline(cin, path);

	cout << "\nIs the first column for ID? (Y/N)" << endl;

	bool done = false;
	bool id = false;

	while (!done)
	{
		string temp;

		cin >> temp;

		if (temp.compare("Y") == 0 || temp.compare("y") == 0)
		{
			id = true;
			done = true;
		}
		else if (temp.compare("N") == 0 || temp.compare("n") == 0)
			done = true;
		else
			cout << "\nPlease enter a (Y/N).\nIs the first column for ID? (Y/N)" << endl;
	}

	cout << "\nIs the last column for class labels? (Y/N)" << endl;
	
	done = false;
	bool c_label = false;

	while (!done)
	{
		string temp;

		cin >> temp;

		if (temp.compare("Y") == 0 || temp.compare("y") == 0)
		{
			c_label = true;
			done = true;
		}
		else if (temp.compare("N") == 0 || temp.compare("n") == 0)
			done = true;
		else
			cout << "\nPlease enter a (Y/N).\nIs the last column for class labels? (Y/N)" << endl;
	}

	cout << "\nIs the first row for labels? (Y/N)" << endl;

	done = false;
	bool label = false;

	while (!done)
	{
		string temp;

		cin >> temp;

		if (temp.compare("Y") == 0 || temp.compare("y") == 0)
		{
			label = true;
			done = true;
		}
		else if (temp.compare("N") == 0 || temp.compare("n") == 0)
			done = true;
		else
			cout << "\nPlease enter a (Y/N).\nIs the first row for labels? (Y/N)" << endl;
	}

	ifstream file(path);
	if (!file)
	{
		cout << "File not found" << endl;

		exit(-1);
	}

	vector<string> labels;
	
	if (label && getline(file, line))
		tokenize(line, labels);

	vector<string>* cols = NULL;

	bool first = true;
	size_t len = 0;

	while (getline(file, line))
	{
		vector<string> row;

		bool empty = tokenize(line, row);

		if (empty) continue;

		if (first)
		{
			len = row.size();

			cols = new vector<string>[len];

			for (int i = 0; i < len; i++)
				cols[i].push_back(row.at(i));

			first = false;
			continue;
		}

		for (int i = 0; i < len; i++)
			cols[i].push_back(row.at(i));
	}

	size_t size;

	if (cols == NULL)
	{
		cout << "File is empty" << endl;

		exit(-1);
	}

	else
		size = cols[0].size();

	double** normData = new double* [len];

	for (int i = 0; i < len; i++)
		normData[i] = new double[size];

	string** stringData = new string * [len];

	for (int i = 0; i < len; i++)
		stringData[i] = new string[size];

	for (int i = 0; i < len; i++)
	{
		if (c_label && (i + 1) == len)
			normData[i][0] = -1;
		else if (!id)
			getData(cols[i], normData[i]);
		else
		{
			normData[i][0] = -1;
			id = false;
		}

		if (normData[i][0] == -1)
			stringData[i] = &cols[i][0];
	}

	srand(time(0));
	
	// get data coefficients
	double angles[10] = { 0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9 };
	random_shuffle(angles, angles + 10);

	// get angles in radians
	for (int i = 0; i < 10; i++)
		angles[i] = acos(angles[i]);

	int last = len - 1;
	
	vector<string> classes;

	if (c_label)
	{
		classes.push_back(stringData[last][0]);

		for (int i = 1; i < size; i++)
		{
			string temp = stringData[last][i];
			int count = 0;

			for (int j = 0; j < classes.size(); j++)
			{
				if (temp.compare(classes[j]) != 0)
					count++;
			}

			if (count == classes.size())
				classes.push_back(stringData[last][i]);
		}
	}

	int add = 1;
	if (c_label)
		add = 2;

	// calculate x and y
	for (int i = 0; i < size; i++)
	{
		double x = 50;
		double y = 250;

		int direction = 0;
		int color;

		if (c_label)
		{
			string temp = stringData[last][i];

			for (color = 0; color < classes.size(); color++)
			{
				if (temp.compare(classes[color]) == 0)
				{
					direction = color % 2;
					break;
				}
			}
		}
		else
		{
			color = 0;
		}

		if (direction == 1)
			y -= 20;

		for (int j = 0; j < len; j++)
		{
			if (normData[j][0] == -1)
				continue;
			else
			{
				double new_x = x + 3 * (cos(angles[j]) * normData[j][i]);
				double new_y;
				double dot;

				if (direction == 0)
				{
					new_y = y + 3 * (sin(angles[j]) * normData[j][i]);

					dot = 245;
				}
				else
				{
					new_y = y - 3 * (sin(angles[j]) * normData[j][i]);

					dot = 235;
				}

				switch (color)
				{
				case 0:
					glColor3f(1, 0, 0);
					break;
				case 1:
					glColor3f(0, 1, 0);
					break;
				case 2:
					glColor3f(0, 0, 1);
					break;
				case 3:
					glColor3f(1, 1, 0);
					break;
				case 4:
					glColor3f(0, 1, 1);
					break;
				case 5:
					glColor3f(1, 0, 1);
					break;
				case 6:
					glColor3f(0.5, 0, 0);
					break;
				case 7:
					glColor3f(0, 0.5, 0);
					break;
				case 8:
					glColor3f(0, 0, 0.5);
					break;
				case 9:
					glColor3f(0.5, 0.5, 0);
					break;
				}
					
				glBegin(GL_LINES);
					glVertex2d(x, y);
					glVertex2d(new_x, new_y);
				glEnd();

				glBegin(GL_POINTS);
					if (j + add == len)
						glVertex2d(new_x, dot);
					glColor3f(0, 0, 0);
					glVertex2d(new_x, new_y);
				glEnd();

				glFlush();

				x = new_x;
				y = new_y;
			}
		}
	}

	cout << "\nVisualization complete." << endl;

	return;
}

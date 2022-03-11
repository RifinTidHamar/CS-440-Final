// C++ libraries
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <limits>
#include <cmath>

// OpenGL
#define NOMINMAX
#include <windows.h>
#include <gl/Gl.h>
#include <gl/Glu.h>
#include "glut.h"

// Group 5
#include "drawdata.h"

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

// gets and then Min-Max normalizes data
void getData(vector<string> array, double* allNum)
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
		// data double not be converted to double
		// potentially inconsistent data (bad dataset)
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

// gets user input for data then draws it
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
	
	// get labels then discard
	if (label && getline(file, line))
	{
		vector<string> labels;
		tokenize(line, labels);
	}
		
	// holds columns
	vector<string>* cols = NULL;

	bool first = true;
	size_t len = 0;

	// puts all data into columns
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

	// get data coefficients
	double angles[10] = { 0.94, 0.53, 0.61, 0.73, 0.55, 0.91, 0.94, 0.91, 0.62, 0.93 };

	int id_label = 0;

	if (c_label)
		id_label++;

	if (id)
		id_label++;
	
	cout << "\nNumber of Dimensions: " << len - id_label << "\nCoefficients Used : ";

	int check = 0;

	// print coefficients
	for (int i = 0; i < len; i++)
	{
		if (i == 0 && id)
		{
			check = 1;
			continue;
		}

		cout << angles[i - check];

		if (c_label)
		{
			if (i != len - 2)
				cout << ", ";
			else
			{
				cout << endl;
				break;
			}		
		}
		else
		{
			if (i != len - 1)
				cout << ", ";
			else
				cout << endl;	
		}
	}

	// get angles in radians
	for (int i = 0; i < 10; i++)
		angles[i] = acos(angles[i]);

	// get all data
	double** normData = new double* [len];

	for (int i = 0; i < len; i++)
		normData[i] = new double[size];

	string** stringData = new string * [len];

	for (int i = 0; i < len; i++)
		stringData[i] = new string[size];

	for (int i = 0; i < len; i++)
	{
		if (c_label && i == len - 1)
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

	int scale;

	if (len - id_label == 1)
		scale = 50;
	else if (len - id_label == 2)
		scale = 25;
	else if (len - id_label == 3)
		scale = 20;
	else if (len - id_label == 4)
		scale = 15;
	else if (len - id_label == 5)
		scale = 13;
	else if (len - id_label == 6)
		scale = 12;
	else if (len - id_label == 7)
		scale = 11;
	else if (len - id_label == 8)
		scale = 10;
	else if (len - id_label == 9)
		scale = 9;
	else if (len - id_label == 10)
		scale = 8;
	
	// calculate x and y
	for (int i = 0; i < size; i++)
	{
		double x = 1;
		double y = 50.75;

		int direction = 0;
		int color;

		// if no labels, then draw all same color
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
			y -= 1.75;

		for (int j = 0; j < len; j++)
		{
			if (normData[j][0] == -1)
				continue;
			else
			{
				double new_x = x + scale * (cos(angles[j]) * normData[j][i]);
				double new_y;
				double dot;

				if (direction == 0)
				{
					new_y = y + scale * (sin(angles[j]) * normData[j][i]);

					dot = 50.25;
				}
				else
				{
					new_y = y - scale * (sin(angles[j]) * normData[j][i]);

					dot = 49.75;
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
					
				// draw lines
				glBegin(GL_LINES);
					glVertex2d(x, y);
					glVertex2d(new_x, new_y);
				glEnd();

				// draw points
				glBegin(GL_POINTS);
					if (c_label && j == len - 1)
						glVertex2d(new_x, dot);

					glColor3f(0, 0, 0);
					glVertex2d(new_x, new_y);
				glEnd();

				glFlush();

				// get new starting points
				x = new_x;
				y = new_y;
			}
		}
	}
	
	cout << "\nVisualization complete." << endl;

	return;
}
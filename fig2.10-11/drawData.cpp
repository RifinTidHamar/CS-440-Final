#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <limits>
#include <cmath>

#define NOMINMAX
#include <windows.h>
#include <gl/Gl.h>
#include <gl/Glu.h>
#include "glut.h"

#include "calibrateData.h"
#include "drawdata.h"

using namespace std;

// user input for data then draws it
void drawData()
{
	string path;
	string line;

	// get user input for file path
	cout << "Enter csv file path: ";
	getline(cin, path);

	// user input for if dataset has id column
	cout << "\nIs the first column for ID? (Y/N)" << endl;

	bool done = false;
	int id = 0;

	while (!done)
	{
		string temp;

		cin >> temp;

		if (temp.compare("Y") == 0 || temp.compare("y") == 0)
		{
			id = 1;
			done = true;
		}
		else if (temp.compare("N") == 0 || temp.compare("n") == 0)
			done = true;
		else
			cout << "\nPlease enter a (Y/N).\nIs the first column for ID? (Y/N)" << endl;
	}

	// user input for if dataset has class column
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

	// user input for if dataset has label row
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

	// open file
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

	// number of rows
	size_t size;

	if (cols == NULL)
	{
		cout << "File is empty" << endl;

		exit(-1);
	}
	else
	{
		size = cols[0].size();
	}

	// holds data
	double** normData = new double* [len];

	for (int i = 0; i < len; i++)
		normData[i] = new double[size];

	// holds classes
	string* stringData;
	
	for (int i = 0; i < len; i++)
	{
		if (c_label && i == len - 1)
		{
			normData[i][0] = -1;
		}	
		else if (id < 1)
		{
			normalizeData(cols[i], normData[i]);
		}
		else
		{
			normData[i][0] = -1;
			id = -1;
		}

		if (i == len - 1 && normData[i][0] == -1)
			stringData = &cols[i][0];
	}
	
	// holds unique classes
	vector<string> classes;

	if (c_label)
	{
		classes.push_back(stringData[0]);

		for (int i = 1; i < size; i++)
		{
			string temp = stringData[i];
			int count = 0;

			for (int j = 0; j < classes.size(); j++)
			{
				if (temp.compare(classes[j]) != 0)
					count++;
			}

			if (count == classes.size())
				classes.push_back(stringData[i]);
		}
	}

	int id_label = 0;

	if (c_label) id_label++;
	if (id < 0) id_label++;

	if (len - id_label > 10)
	{
		cout << "INVALID DIMENSIONS: dimensions must be between 1-D and 10-D." << endl;
		exit(-1);
	}

	double scale;

	//scale visualization depending on number of dimensions
	switch (len - id_label)
	{
		case 1:
			scale = 50;
			break;
		case 2:
			scale = 25;
			break;
		case 3:
			scale = 20;
			break;
		case 4:
			scale = 15;
			break;
		case 5:
			scale = 13;
			break;
		case 6:
			scale = 12;
			break;
		case 7:
			scale = 11;
			break;
		case 8:
			scale = 9.5;
			break;
		case 9:
			scale = 8.75;
			break;
		case 10:
			scale = 8.75;
			break;
		default:
			cout << "INVALID DIMENSIONS: dimensions must be between 1-D and 10-D." << endl;
			exit(-1);
	}
	
	cout << "\nMin-Max Normalizing Data from [0, 1]." << endl;
	cout << "\nNumber of Dimensions: " << len - id_label << "\nCoefficients Used : ";

	// get data coefficients
	double angles[11] = { 0.94, 0.53, 0.61, 0.73, 0.55, 0.91, 0.94, 0.91, 0.62, 0.93, 1};
	
	bool printed = false;
	
	// calculate x and y
	for (int i = 0; i < size; i++)
	{
		double x = 1;
		double y = 50.75;

		int direction = 0;
		int color;

		// get color
		if (c_label)
		{
			string temp = stringData[i];

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
				// print used coefficients
				if (!printed)
				{
					if (c_label)
					{
						if (j != len - 2)
						{
							cout << angles[j] << ", ";
							angles[j] = acos(angles[j]);
						}	
						else
						{
							cout << angles[j] << endl;
							angles[j] = acos(angles[j]);
							printed = true;
						}
					}	
					else
					{
						if (j != len - 1)
						{
							cout << angles[j] << ", ";
							angles[j] = acos(angles[j]);
						}	
						else
						{
							cout << angles[j] << endl;
							angles[j] = acos(angles[j]);
							printed = true;
						}
					}	
				}
				
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

				// select color
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
					if (c_label && j == len - 2)
						glVertex2d(new_x, dot);
					else if (j == len - 1)
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

	cout << endl;

	if (c_label)
	{
		cout << "Class Colors are: " << endl;
		
		for (int i = 0; i < classes.size(); i++)
		{
			switch (i)
			{
				case 0: 
					cout << "\t" << classes.at(i) << " = red" << endl;
					break;
				case 1:
					cout << "\t" << classes.at(i) << " = green" << endl;
					break;
				case 2:
					cout << "\t" << classes.at(i) << " = blue" << endl;
					break;
				case 3:
					cout << "\t" << classes.at(i) << " = yellow" << endl;
					break;
				case 4:
					cout << "\t" << classes.at(i) << " = cyan" << endl;
					break;
				case 5:
					cout << "\t" << classes.at(i) << " = purple" << endl;
					break;
				case 6:
					cout << "\t" << classes.at(i) << " = dark red" << endl;
					break;
				case 7:
					cout << "\t" << classes.at(i) << " = dark green" << endl;
					break;
				case 8:
					cout << "\t" << classes.at(i) << " = dark blue" << endl;
					break;
				case 9:
					cout << "\t" << classes.at(i) << " = dark yellow" << endl;
					break;
			}
		}
	}
	else
		cout << "No classes: all data is red." << endl;
	
	cout << "\nVisualization complete." << endl;

	return;
}
#include "glut.h"
#include "viewing.h"

void setWindow(float left, float right, float bottom, float top)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(left, right, bottom, top);
}

void setViewport(float left, float right, float bottom, float top)
{
	glViewport(left, bottom, right - left, top - bottom);
}
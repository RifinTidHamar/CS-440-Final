// OpenGl
#include <windows.h>
#include <gl/Gl.h>
#include <gl/Glu.h>
#include "glut.h"

// Group 5
#include "drawData.h"

void GLC_L();
void Init(void);

void main (int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(1000, 800);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("Visualization: GLC-L ");
	glutDisplayFunc(GLC_L);
	Init();
	glutMainLoop();
}
void GLC_L()
{
	glClear(GL_COLOR_BUFFER_BIT);
	drawData();
	glFlush();
}

void Init()
{
	glClearColor(1.0,1.0,1.0,0.0);
	glColor3f(0.0f,0.0f,0.0f);
	glPointSize(4.0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(1.0,640.0,0.0,480.0);
}


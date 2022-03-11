#include <windows.h>
#include <gl/Gl.h>
#include <gl/Glu.h>
#include "glut.h"

#include "viewing.h"
#include "drawData.h"

void GLC_L();
void Init(void);

void main (int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(1000, 1000);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("Visualization: GLC-L ");
	glutDisplayFunc(GLC_L);
	Init();
	glutMainLoop();
}

// gets data from dataset, normalizes data, then draws data
void GLC_L()
{
	setWindow(0, 100, 0, 100);
	setViewport(0, 1000, 0, 1000);
	
	glClear(GL_COLOR_BUFFER_BIT);

	drawData();

	glFlush();
}

void Init()
{
	glClearColor(1.0,1.0,1.0,0.0);
	glPointSize(4.0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
}
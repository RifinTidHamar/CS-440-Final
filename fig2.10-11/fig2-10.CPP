#include <windows.h>
#include <gl/Gl.h>
#include <gl/Glu.h>
/*#include <iostream.h> */
#include "glut.h"
#include <math.h>
#include <stdio.h>
#include <iostream>
static double eyex = 0, eyey = 0, eyez = 30;
static double refx = 0, refy = 0, refz = 0;
static double upx = 0, upy = 1, upz = 0;

static double xminRequested = -5, xmaxRequested = 5;
static double yminRequested = -5, ymaxRequested = 5;
static double zmin = -10, zmax = 10;
static int orthographic = 0;
static int preserveAspect = 1;

static double xminActual, xmaxActual, yminActual, ymaxActual;


// At least produces a rectangle
void drawRect() {
    glColor3f(1, 1, 1);
    glBegin(GL_QUADS);
    glVertex2f(-5.0f, 5.0f);
    glVertex2f(-5.0f, 0.0f);
    glVertex2f(5.0f, 0.0f);
    glVertex2f(5.0f, 5.0f);
    glEnd();
}

struct rectangularPrism
{
    rectangularPrism(float px, float py, float pz, float sx, float sy, float sz)
    {
        float x = sx + px;
        float nx = -sx + px;
        float y = sy + py;
        float ny = -sy + py;
        float z = sz + pz;
        float nz = -sz + pz;
        vc[0]= vc[3]= vc[6]= vc[9] = x; 
        vc[1]= vc[4]= vc[13]= vc[16] = y; 
        vc[2]= vc[11]= vc[14]= vc[23] = z; 
        vc[12]= vc[15]= vc[18]= vc[21] = nx;
        vc[7]= vc[10]= vc[19]= vc[22] = ny;
        vc[5]= vc[8]= vc[17]= vc[20] = nz;

    }
    float vc[24];/* = {x,y,z,   x,y,nz,   x,ny,nz,   x,ny,z,
                         nx,y,z,  nx,y,nz,  nx,ny,nz,  nx,ny,z };*/
};

rectangularPrism rectCent(0, 0, -5, 5, 5, 0.3);
rectangularPrism rectLeft(-6, 0, -4, 1, 5, 1.3);
rectangularPrism rectTop(0, 6, -4, 7, 1, 1.3);
rectangularPrism rectRight(6, 0, -4, 1, 5, 1.3);
rectangularPrism rectBot(0, -6, -4, 7, 1, 1.3);
rectangularPrism rectPic(0, 0, -4.7, 4, 4, 0.5);




int elementArray[24] = {  // Vertex number for the six faces.
          0,1,2,3, 0,3,7,4, 0,4,5,1,
          6,2,1,5, 6,5,4,7, 6,7,3,2 };

// Not currently working -------

void drawRectangularPrism(rectangularPrism rect) {

  /*  glMatrixMode(GL_PROJECTION);
    glOrtho(-4, 4, -2, 2, -2, 2);  
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.5, 0.5, 0.5, 1);*/

    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //glLoadIdentity();             // Set up modelview transform, first cube.
    //glTranslatef(0.5, 0, 0);     // Move cube to left half of window.
    //glTranslatef(0, 0, 0);      // Move cube to right half of window.
    glVertexPointer(3, GL_FLOAT, 0, rect.vc);  // Set data type and location, first cube.
    //glColorPointer(3, GL_FLOAT, 0, vertexColors);

    glEnableClientState(GL_VERTEX_ARRAY);
    //glEnable(GL_NORMALIZE);
    //glEnableClientState(GL_COLOR_ARRAY);
    //glEnable(GL_COLOR_MATERIAL);
    glDrawElements(GL_QUADS, 24, GL_UNSIGNED_INT, elementArray);
    
    //glDisableClientState(GL_COLOR_ARRAY);  // Don't use color array for the edges.
    //glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, edgeElementArray);  // Draw the edges!

    //glutSwapBuffers();

}


// wall of untouched code begins here**************************************
void cameraSetOrthographic(int ortho) {
    orthographic = ortho;
}

void cameraSetPreserveAspect(int preserve) {
    preserveAspect = preserve;
}

void cameraSetLimits(double xmin, double xmax, double ymin, double ymax, double zmin1, double zmax1) {
    xminRequested = xminActual = xmin;
    xmaxRequested = xmaxActual = xmax;
    yminRequested = yminActual = ymin;
    ymaxRequested = ymaxActual = ymax;
    zmin = zmin1;
    zmax = zmax1;
}

void cameraSetScale(double limit) {
    cameraSetLimits(-limit, limit, -limit, limit, -2 * limit, 2 * limit);
}

void cameraLookAt(double eyeX, double eyeY, double eyeZ,
    double viewCenterX, double viewCenterY, double viewCenterZ,
    double viewUpX, double viewUpY, double viewUpZ) {
    eyex = eyeX;
    eyey = eyeY;
    eyez = eyeZ;
    refx = viewCenterX;
    refy = viewCenterY;
    refz = viewCenterZ;
    upx = viewUpX;
    upy = viewUpY;
    upz = viewUpZ;
}

static double norm(double v0, double v1, double v2) {
    double norm2 = v0 * v0 + v1 * v1 + v2 * v2;
    return sqrt(norm2);
}

static void normalize(double v[]) {
    double n = norm(v[0], v[1], v[2]);
    v[0] /= n;
    v[1] /= n;
    v[2] /= n;
}

void cameraApply() {
    int viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    xminActual = xminRequested;
    xmaxActual = xmaxRequested;
    yminActual = yminRequested;
    ymaxActual = ymaxRequested;
    if (preserveAspect) {
        double viewWidth = viewport[2];
        double viewHeight = viewport[3];
        double windowWidth = xmaxActual - xminActual;
        double windowHeight = ymaxActual - yminActual;
        double aspect = viewHeight / viewWidth;
        double desired = windowHeight / windowWidth;
        if (desired > aspect) { //expand width
            double extra = (desired / aspect - 1.0) * (xmaxActual - xminActual) / 2.0;
            xminActual -= extra;
            xmaxActual += extra;
        }
        else if (aspect > desired) {
            double extra = (aspect / desired - 1.0) * (ymaxActual - yminActual) / 2.0;
            yminActual -= extra;
            ymaxActual += extra;
        }
    }
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    double viewDistance = norm(refx - eyex, refy - eyey, refz - eyez);
    if (orthographic) {
        glOrtho(xminActual, xmaxActual, yminActual, ymaxActual, viewDistance - zmax, viewDistance - zmin);
    }
    else {
        double var = viewDistance - zmax;
        if (var < 0.1)
            var = 0.1;
        double centerx = (xminActual + xmaxActual) / 2;
        double centery = (yminActual + ymaxActual) / 2;
        double newwidth = (var / viewDistance) * (xmaxActual - xminActual);
        double newheight = (var / viewDistance) * (ymaxActual - yminActual);
        double x1 = centerx - newwidth / 2;
        double x2 = centerx + newwidth / 2;
        double y1 = centery - newheight / 2;
        double y2 = centery + newheight / 2;
        glFrustum(x1, x2, y1, y2, var, viewDistance - zmin);
    }
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(eyex, eyey, eyez, refx, refy, refz, upx, upy, upz);
}

static void reflectInAxis(double axis[], double source[], double destination[]) {
    double s = 2 * (axis[0] * source[0] + axis[1] * source[1] + axis[2] * source[2]);
    destination[0] = s * axis[0] - source[0];
    destination[1] = s * axis[1] - source[1];
    destination[2] = s * axis[2] - source[2];
}

static void transformToViewCoords(double v[], double x[], double y[], double z[], double out[]) {
    out[0] = v[0] * x[0] + v[1] * y[0] + v[2] * z[0];
    out[1] = v[0] * x[1] + v[1] * y[1] + v[2] * z[1];
    out[2] = v[0] * x[2] + v[1] * y[2] + v[2] * z[2];
}

static void applyTransvection(double from[3], double to[3]) {
    // rotate vector e1 onto e2; must be 3D *UNIT* vectors.
    double zDirection[3] = { eyex - refx, eyey - refy, eyez - refz };
    double viewDistance = norm(zDirection[0], zDirection[1], zDirection[2]);
    normalize(zDirection);
    double yDirection[3] = { upx, upy, upz };
    double upLength = norm(yDirection[0], yDirection[1], yDirection[2]);
    double proj = yDirection[0] * zDirection[0] + yDirection[1] * zDirection[1] + yDirection[2] * zDirection[2];
    yDirection[0] = yDirection[0] - proj * zDirection[0];
    yDirection[1] = yDirection[1] - proj * zDirection[1];
    yDirection[2] = yDirection[2] - proj * zDirection[2];
    normalize(yDirection);
    double xDirection[3] = { yDirection[1] * zDirection[2] - yDirection[2] * zDirection[1],
            yDirection[2] * zDirection[0] - yDirection[0] * zDirection[2],
            yDirection[0] * zDirection[1] - yDirection[1] * zDirection[0] };
    double temp[3], e1[3], e2[3];
    transformToViewCoords(from, xDirection, yDirection, zDirection, e1);
    transformToViewCoords(to, xDirection, yDirection, zDirection, e2);
    double e[3] = { e1[0] + e2[0], e1[1] + e2[1], e1[2] + e2[2] };
    normalize(e);
    reflectInAxis(e, zDirection, temp);
    reflectInAxis(e1, temp, zDirection);
    reflectInAxis(e, xDirection, temp);
    reflectInAxis(e1, temp, xDirection);
    reflectInAxis(e, yDirection, temp);
    reflectInAxis(e1, temp, yDirection);
    eyex = refx + viewDistance * zDirection[0];
    eyey = refy + viewDistance * zDirection[1];
    eyez = refz + viewDistance * zDirection[2];
    upx = upLength * yDirection[0];
    upy = upLength * yDirection[1];
    upz = upLength * yDirection[2];
}

static int dragging = 0;
static int dragButton;
static double prevRay[3];

static void mousePointToRay(int x, int y, double out[3]) {
    double dx, dy, dz, len;
    int centerX = glutGet(GLUT_WINDOW_WIDTH) / 2;
    int centerY = glutGet(GLUT_WINDOW_HEIGHT) / 2;
    double scale = 0.8 * (centerX < centerY ? centerX : centerY);
    dx = (x - centerX);
    dy = (centerY - y);
    len = sqrt(dx * dx + dy * dy);
    if (len >= scale)
        dz = 0;
    else
        dz = sqrt(scale * scale - dx * dx - dy * dy);
    double length = sqrt(dx * dx + dy * dy + dz * dz);
    out[0] = dx / length;
    out[1] = dy / length;
    out[2] = dz / length;
}

void trackballMouseFunction(int button, int buttonState, int x, int y) {
    if (button != GLUT_LEFT_BUTTON) {
        return;
    }
    if (buttonState == GLUT_DOWN) {  // a mouse button was pressed
        if (dragging)
            return;  // ignore a second button press during a draw operation
        dragging = 1;  // Might not want to do this in all cases
        dragButton = button;
        mousePointToRay(x, y, prevRay);
    }
    else {  // a mouse button was released
        if (!dragging || button != dragButton)
            return; // this mouse release does not end a drag operation
        dragging = 0;
    }
}

void trackballMotionFunction(int x, int y) {
    if (!dragging)
        return;
    double thisRay[3];
    mousePointToRay(x, y, thisRay);
    applyTransvection(prevRay, thisRay);
    prevRay[0] = thisRay[0];
    prevRay[1] = thisRay[1];
    prevRay[2] = thisRay[2];
    glutPostRedisplay();
}


const double PI = 3.141592654;

//int animating = 0;       // Is the animation running? 0 = animating
int ambientLight = 1;    // Should global ambient light be on?
int viewpointLight = 1;  // Should the viewpoint light be on?
int light = 1;           // Should the red light be on?
int showLightModel = 1;  // should the light model be showing?

int frameNumber = 0;  // Frame number for use in animation.

char message[200];  // Used for displaying a message at the bottom of the display.
int width, height;  // Size of the display, set in reshape().


//------------------- Drawing functions ----------------------------------------

/*  Sets the positions of the colored lights and turns them on and off, depending on
 *  the state of the light options.  Draws a small
 *  sphere at the location of each light.
 */
void lights() {

    glColor3d(0.5, 0.5, 0.5);
    float zero[] = { 0, 0, 0, 1 };
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, zero);

    if (viewpointLight)
        glEnable(GL_LIGHT0);
    else
        glDisable(GL_LIGHT0);

    if (light) {
        float light[] = { 0.5, 0.5, 0.5, 1 };
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, light);
        glEnable(GL_LIGHT1);
    }
    else {
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, zero);
        glDisable(GL_LIGHT1);
    }

    glPushMatrix();
    glRotated(-frameNumber, 0, 1, 0);
    glTranslated(0, 3, 1);
    glLightfv(GL_LIGHT1, GL_POSITION, zero);
    if(showLightModel)
        glutSolidSphere(0.5, 16, 8);
    glPopMatrix();

    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, zero); // Turn off emission color!
} // end lights()


/*  Fills the array with RGBA color components corresponding the given HSV color
 *  components, where h, s, and v are in the range 0.0 to 1.0.
 */
void colorArrayForHue(double array[4], double h, double s, double v) {
    double r, g, b;
    double c, x;
    h = h * 359;
    c = v * s;
    x = (h < 120) ? h / 60 : (h < 240) ? (h - 120) / 60 : (h - 240) / 60;
    x = c * (1 - fabs(x - 1));
    x += (v - c);
    switch ((int)(h / 60)) {
    case 0: r = v; g = x; b = v - c; break;
    case 1: r = x; g = v; b = v - c; break;
    case 2: r = v - c; g = v; b = x; break;
    case 3: r = v - c; g = x; b = v; break;
    case 4: r = x; g = v - c; b = v; break;
    case 5: r = v; g = v - c; b = x; break;
    }
    array[0] = r;
    array[1] = g;
    array[2] = b;
    array[3] = 1;
}


// wall of untouched code ends here***************************************

/* Draws the scene
 */
void display() {

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glViewport(0, 60, width, height - 60);  // viewport for the drawing

    cameraApply();

    lights();

    float zero[] = { 0, 0, 0, 1 };
    float amb[] = { 0.15, 0.15, 0.15, 1 };
    float spec[] = { 0.2, 0.2, 0.2, 1 };

    if (ambientLight) {
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, amb);
    }
    else {
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, zero);
    }

  
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, amb);
    glPushMatrix();
    drawRectangularPrism(rectCent);
    drawRectangularPrism(rectLeft);
    drawRectangularPrism(rectRight);
    drawRectangularPrism(rectTop);
    drawRectangularPrism(rectBot);
    drawRectangularPrism(rectPic);


    //glutSolidCube(8); // draws a cube - working
    //drawRect();
    glPopMatrix();

    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);

    glutSwapBuffers();

}

/* Called when the window size changes; just record the new size for use in
 * setting the viewports in the display method.
 */
void reshape(int w, int h) {
    width = w;
    height = h;
}

/* Initialization, including setting up a camera and configuring the four lights.
 */
void initGL() {

    glColor3f(0, 0, 0);  // The edges will be black.
    glLineWidth(2);
    glClearColor(0, 0, 0, 1);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 1);
    glMateriali(GL_FRONT_AND_BACK, GL_SHININESS, 32);

    float dim[] = { 0.5F, 0.5F, 0.5F, 1 };
    glLightfv(GL_LIGHT0, GL_DIFFUSE, dim);
    glLightfv(GL_LIGHT0, GL_SPECULAR, dim);

    float light[] = { 0.5F, 0.5F, 0.5F, 1 };
    float lighta[] = { 0.1F, 0.1F, 0.1F, 1 };
    glLightfv(GL_LIGHT1, GL_AMBIENT, lighta);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light);
    glLightfv(GL_LIGHT1, GL_SPECULAR, light);
}


// --------------- support for animation ------------------------------------------

/*
void timerFunction(int timerID) {
    if (animating) {
        frameNumber++;
        glutTimerFunc(30, timerFunction, 0);  // Next frame in 30 milliseconds.
        glutPostRedisplay(); // Causes display() to be called.
    }
}


void toggleAnimation() {
    if (!animating) {
        animating = 1;
        glutTimerFunc(30, timerFunction, 0);
    }
    else {
        animating = 0;
    }
}


// --------------------- Menu Keyboard handling --------------------------------
/*
void charTyped(unsigned char ch, int x, int y) {
    switch (ch) {
    case 'a': case 'A': toggleAnimation(); break;
    case 'm': case 'M': ambientLight = 1 - ambientLight; break;
    case 'v': case 'V': viewpointLight = 1 - viewpointLight; break;
    case 'r': case 'R': redLight = 1 - redLight; break;
    case 27: exit(0);
    }
    glutPostRedisplay();
}


void doMenu(int itemNum) {  // menu handler -- change render mode.
    switch (itemNum) {
    case 1: toggleAnimation(); break;
    case 3: ambientLight = 1 - ambientLight; break;
    case 4: viewpointLight = 1 - viewpointLight; break;
    case 5: redLight = 1 - redLight; break;
    case 8: exit(0);
    }
    glutPostRedisplay();
}



void createMenus() {  // make a menu containing the five render modes.
    int menu = glutCreateMenu(doMenu);
    glutAddMenuEntry("Toggle Animation (A key)", 1);
    glutAddMenuEntry("Toggle Global Ambient Light (M key)", 3);
    glutAddMenuEntry("Toggle Viewpoint Light (V key)", 4);
    glutAddMenuEntry("Toggle Red Light (R key)", 5);
    glutAddMenuEntry("Toggle Green Light (G key)", 6);
    glutAddMenuEntry("Toggle Blue Light (B key)", 7);
    glutAddMenuEntry("Quit (ESC key)", 8);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
}*/


//---------------------------------------------------------------------------


int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(600, 660);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("CS 440 final");
    initGL();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    cameraLookAt(5, 10, 30, 0, 0, 0, 0, 1, 0);
    cameraSetScale(10);
    glutMouseFunc(trackballMouseFunction);
    glutMotionFunc(trackballMotionFunction);
    //glutKeyboardFunc(charTyped);
    //createMenus();
    //toggleAnimation();
    glutMainLoop();
    return 0;
}



#include <windows.h>
#include <gl/Gl.h>
#include <gl/Glu.h>
/*#include <iostream.h> */
#include "glut.h"
#include <math.h>
#include <stdio.h>
#include <iostream>
#include "readrgb.h"

#include "imageTexture.h"
#include "camera.h"


using namespace std;

// Baseline for our code: https://math.hws.edu/graphicsbook/source/glut/four-lights.c

struct rectangularPrism
{
    rectangularPrism(float px = 0, float py = 0, float pz = 0, float sx = 0, float sy = 0, float sz = 0, float r = 1, float g = 1, float b = 1, bool tex = false)
    {
        float x = sx + px;
        float nx = -sx + px;
        float y = sy + py;
        float ny = -sy + py;
        float z = sz + pz;
        float nz = -sz + pz;
        vCoord[0]= vCoord[3]= vCoord[6]= vCoord[9] = x; 
        vCoord[1]= vCoord[4]= vCoord[13]= vCoord[16] = y; 
        vCoord[2]= vCoord[11]= vCoord[14]= vCoord[23] = z; 
        vCoord[12]= vCoord[15]= vCoord[18]= vCoord[21] = nx;
        vCoord[7]= vCoord[10]= vCoord[19]= vCoord[22] = ny;
        vCoord[5]= vCoord[8]= vCoord[17]= vCoord[20] = nz;
        //std::cout << !tex;
        if (!tex)
        {
            int i = 1;
            for (int cnt = 0; cnt < 24; cnt++)
            {
                if (i % 3 == 0)
                {
                    vCol[cnt] = b;
                }
                else if (i % 2 == 0)
                {
                    vCol[cnt] = g;
                }
                else
                {
                    vCol[cnt] = r;
                }
                if (i == 3)
                {
                    i = 1;
                }
                else
                    i++;
            }
        }
        else
        {
            vTex[0] = vTex[3] = vTex[6] = vTex[9] = 0;
            vTex[1] = vTex[4] = vTex[13] = vTex[16] = 0;
            vTex[2] = vTex[11] = vTex[14] = vTex[23] = 0;
            vTex[12] = vTex[15] = vTex[18] = vTex[21] = 0;
            vTex[7] = vTex[10] = vTex[19] = vTex[22] = 0;
            vTex[5] = vTex[8] = vTex[17] = vTex[20] = 0;
        }
    }
    float vCoord[24];/* = {x,y,z,   x,y,nz,   x,ny,nz,   x,ny,z,
                         nx,y,z,  nx,y,nz,  nx,ny,nz,  nx,ny,z };*/
    float vCol[24];

    float vTex[24];
};

float brown[3] = { 0.5, 0.2, 0 };
rectangularPrism rectCent(0, 0, 0, 5, 5, 0.3, 0.5, 0.6, 0.5);
rectangularPrism rectLeft(-6, 0, 1, 1, 5, 1.3, brown[0], brown[1], brown[2]);
rectangularPrism rectTop(0, 6, 1, 7, 1, 1.3, brown[0], brown[1], brown[2]);
rectangularPrism rectRight(6, 0, 1, 1, 5, 1.3, brown[0], brown[1], brown[2]);
rectangularPrism rectBot(0, -6, 1, 7, 1, 1.3, brown[0], brown[1], brown[2]);
rectangularPrism rectBack(0, 0, -0.6, 7, 7, 0.3, brown[0], brown[1], brown[2]);
const int numRects = 40;
rectangularPrism outerRectsX[numRects];
rectangularPrism outerRectsY[numRects];
rectangularPrism outerRectsZ[numRects];

void fillOuterRectsArrayData(float sx, float sy, float sz, float r, int dir)
{
    float pz = 0;
    float px = 0;
    float radToAdd = 6.28 / numRects;
    float currentRad = 0;
    for (int i = 0; i < numRects; i++)
    {
        float px = r*cos(currentRad);
        float pz = r*sin(currentRad);
        if(dir == 1) //x
            outerRectsX[i] = rectangularPrism(px, 0.0, pz, sx, sy, sz);
        else if (dir == 2)//y
            outerRectsY[i] = rectangularPrism(px, pz, 0, sx, sy, sz);
        else if (dir == 3)//z
            outerRectsZ[i] = rectangularPrism(0, pz, px, sx, sy, sz);
        currentRad += radToAdd;
    }
}

int elementArray[24] = {  // Vertex number for the six faces.
          0,1,2,3, 0,3,7,4, 0,4,5,1,
          6,2,1,5, 6,5,4,7, 6,7,3,2 };

void drawRectangularPrism(rectangularPrism rect) {

    glVertexPointer(3, GL_FLOAT, 0, rect.vCoord);  // Set data type and location, first cube.
    glColorPointer(3, GL_FLOAT, 0, rect.vCol);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glDrawElements(GL_QUADS, 24, GL_UNSIGNED_INT, elementArray);
}

void square(double side, float z, float s) {
    double radius = side/2;
    glBegin(GL_POLYGON);
    glNormal3f(0, 0, 1);
    glTexCoord2d(0, 0);
    glVertex3d(-radius - s, -radius, z); // topRight
    glTexCoord2d(1, 0);
    glVertex3d(radius + s, -radius, z); // topLeft
    glTexCoord2d(1, 1);
    glVertex3d(radius + s, radius, z);// bottomLeft
    glTexCoord2d(0, 1);
    glVertex3d(-radius - s, radius, z);// bottomRight
    glEnd();
} // end square

void cube(double side, float z, float sz) {
    glPushMatrix();
    glRotatef(90, 1, 0, 0);  // bottomFace
    glPushMatrix();
    glTranslated(0, 0, side / 2);
    square(side, z + sz, 0);
    glPopMatrix();
    glPushMatrix();
    glRotatef(90, 0, 1, 0);
    glTranslated(0, 0, side / 2);
    square(side, z, sz); // rightFace
    glPopMatrix();
    glPushMatrix();
    glRotatef(180, 0, 1, 0);
    glTranslated(0, 0, side / 2);
    square(side, z + sz, 0); // topFace
    glPopMatrix();
    glPushMatrix();
    glRotatef(270, 0, 1, 0);
    glTranslated(0, 0, side / 2);
    square(side, z, sz); // leftFace
    glPopMatrix();
    glPushMatrix();
    glRotatef(-90, 1, 0, 0);
    glRotatef(90, 0, 0, 1);
    glTranslated(0, 0, side / 2);
    square(side, z, sz); // frontFace
    glPopMatrix();
    glPushMatrix();
    glRotatef(90, 1, 0, 0);
    glRotatef(90, 0, 0, 1);
    glTranslated(0, 0, side / 2);
    square(side, z , sz); // backFace
    glPopMatrix();
    glPopMatrix();
}



const double PI = 3.141592654;
int ambientLight = 1;    // Should global ambient light be on?
int viewpointLight = 1;  // Should the viewpoint light be on?
int light = 1;           // Should the light be on?
int showLightModel = 1;  // should the light model be showing?
int width, height;  // Size of the display, set in reshape().


//------------------- Drawing functions ----------------------------------------

/*  Sets the positions of the light and turns it on and off, depending on
 *  the state of the light options.  Draws a small
 *  sphere at the location of each light.
 */
void lights() {

    glColor3d(1, 1, 1);
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
    glTranslated(0, 10, 6);
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


const int textNum = 2;
GLuint texID[textNum];  // Texture ID's for the three textures.

char* textureFileNames[textNum] = {   // file names for the files from which texture images are loaded
            "textures-rgb/castle.rgb", 
            "textures-rgb/woodGrain.rgb"
};


// rotations for special key function
float rotateY = 0;     
float rotateX = 0;
float rotateZ = 0;
float radius = 25;
float cameraZoom = 10; // camera zoom

void specialKeyFunction(int key, int x, int y) {
    // called when a special key is pressed 
    if (key == GLUT_KEY_LEFT)
        rotateY -= 15;
    else if (key == GLUT_KEY_RIGHT)
        rotateY += 15;
    else if (key == GLUT_KEY_DOWN)
        rotateX += 15;
    else if (key == GLUT_KEY_UP)
        rotateX -= 15;
    else if (key == GLUT_KEY_PAGE_UP)
        rotateZ += 15;
    else if (key == GLUT_KEY_PAGE_DOWN)
        rotateZ -= 15;
    else if (key == GLUT_KEY_HOME)
        rotateX = rotateY = rotateZ = 0;
    glutPostRedisplay();
}

/* Draws the scene
 */

void display() {

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glViewport(0, 0, width, height);  // viewport for the drawing
    cameraSetScale(cameraZoom);
    cameraApply();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPushMatrix();
  
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
  
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, amb);

    glPopMatrix();
   
    radius = cameraZoom * 2.5;
    float s1 = cameraZoom * 0.05;
    float s2 = cameraZoom * 0.05;
    fillOuterRectsArrayData(s1, s2, s1, radius, 1);
    fillOuterRectsArrayData(s1, s1, s2, radius, 2);
    fillOuterRectsArrayData(s1, s1, s2, radius, 3);

    for (int i = 0; i < numRects; i++)
    {
        drawRectangularPrism(outerRectsX[i]);
        drawRectangularPrism(outerRectsY[i]);
        drawRectangularPrism(outerRectsZ[i]);
    }
  
    glPushMatrix();

    glRotatef(rotateZ, 0, 0, 1);     // Apply rotations.
    glRotatef(rotateY, 0, 1, 0);
    glRotatef(rotateX, 1, 0, 0);

    drawRectangularPrism(rectCent);

    //makes picture in the middle
    glEnable(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, texID[0]);  // Bind texture #0 
    square(8, 0.31, 0);

    //makes wooden frame
    glBindTexture(GL_TEXTURE_2D, texID[1]);  // Bind texture #1

    glTranslatef(-6, 0, 0.75);
    cube(2, 0, 4);
    glTranslatef(12, 0, 0);
    cube(2, 0, 4);
    glTranslatef(-6, 0, 0);
    glTranslatef(0, 6, 0);
    glRotatef(90, 0, 0, 1);
    cube(2, 0, 6);
    glTranslatef(-12, 0, 0);
    cube(2, 0, 6);

    glDisable(GL_TEXTURE_2D);

    glPopMatrix();
    glutSwapBuffers();

}

/* Called when the window size changes; just record the new size for use in
 * setting the viewports in the display method.
 */
void reshape(int w, int h) {
    width = w;
    height = h;
}

void loadTextures() {
    glGenTextures(textNum, texID);  // Get the texture object IDs.
    for (int i = 0; i < textNum; i++) {
        void* imgData;  // Pointer to image color data read from the file.
        int imgWidth;   // The width of the image that was read.
        int imgHeight;  // The height.
        int components; // Number of color components in the file (not used).

        imgData = read_texture(textureFileNames[i], &imgWidth, &imgHeight, &components);

        if (imgData) {
            printf("Texture image loaded from file %s, size %dx%d\n",
                textureFileNames[i], imgWidth, imgHeight);
            int format; // The format of the color data in memory, depends on platform.

            format = GL_RGBA;
            glBindTexture(GL_TEXTURE_2D, texID[i]);  // Will load image data into texture object #i
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imgWidth, imgHeight, 0, format,
                GL_UNSIGNED_BYTE, imgData);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Required since there are no mipmaps.
        }
        else {
            printf("Failed to get texture data from %s\n", textureFileNames[i]);
        }
    }
}

/* Initialization, including setting up a camera and configuring the four lights.
 */
void initGL() {
    glShadeModel(GL_FLAT);
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


// --------------------- Menu Keyboard handling --------------------------------

void charTyped(unsigned char ch, int x, int y) {
    switch (ch) {
    case 'm': case 'M': ambientLight = 1 - ambientLight; break;
    case 'v': case 'V': viewpointLight = 1 - viewpointLight; break;
    case 'r': case 'R': light = 1 - light; break;
    case 's': case 'S': if (cameraZoom < 100) cameraZoom += 1; break;
    case 'w': case 'W': if (cameraZoom > 1) cameraZoom -= 1; break;
    case 27: exit(0);
    }
    glutPostRedisplay();
}

void doMenu(int itemNum) {  // menu handler -- change render mode.
    switch (itemNum) {
    case 1: ambientLight = 1 - ambientLight; break;
    case 2: viewpointLight = 1 - viewpointLight; break;
    case 3: light = 1 - light; break;
    case 5: if (cameraZoom < 100) cameraZoom += 10; break;
    case 4: if (cameraZoom > 10) cameraZoom -= 10; break;
    case 6: exit(0);
    }
    glutPostRedisplay();
}

void createMenus() {  // make a menu containing the six render modes.
    int menu = glutCreateMenu(doMenu);
    glutAddMenuEntry("Toggle Global Ambient Light (M key)", 1);
    glutAddMenuEntry("Toggle Viewpoint Light (V key)", 2);
    glutAddMenuEntry("Toggle Light (R key)", 3);
    glutAddMenuEntry("Quit (ESC key)", 6);
    glutAddMenuEntry("Zoom-In (W key)", 4);
    glutAddMenuEntry("Zoom-Out (S key)", 5);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
    cout << "\n Menu of Controls: \n Toggle Global Ambient Light (M key) \n Toggle Viewpoint Light (V key) \n Toggle Light (R key) \n Quit (ESC key) \n Zoom-In (W key) \n Zoom-Out (S key)";
}


//---------------------------------------------------------------------------

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(600, 660);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("CS 440 final - right click to see options");
    initGL();
    loadTextures();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    cameraLookAt(5, 10, 30, 0, 0, 0, 0, 1, 0);
    cameraSetScale(cameraZoom);
    glutMouseFunc(trackballMouseFunction);
    glutMotionFunc(trackballMotionFunction);
    glutKeyboardFunc(charTyped);
    glutSpecialFunc(specialKeyFunction);  
    createMenus();
    glutMainLoop();
    return 0;
}



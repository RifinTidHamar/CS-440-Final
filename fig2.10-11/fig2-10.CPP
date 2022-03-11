#include <windows.h>
#include <gl/Gl.h>
#include <gl/Glu.h>
/*#include <iostream.h> */
#include "glut.h"
#include <math.h>
#include <stdio.h>
#include <iostream>
#include "readrgb.h"

//for image things
void bwtorgba(unsigned char* b, unsigned char* l, int n)
{
    while (n--) {
        l[0] = *b;
        l[1] = *b;
        l[2] = *b;
        l[3] = 0xff;
        l += 4; b++;
    }
}

void latorgba(unsigned char* b, unsigned char* a, unsigned char* l, int n)
{
    while (n--) {
        l[0] = *b;
        l[1] = *b;
        l[2] = *b;
        l[3] = *a;
        l += 4; b++; a++;
    }
}

void rgbtorgba(unsigned char* r, unsigned char* g,
    unsigned char* b, unsigned char* l, int n)
{
    while (n--) {
        l[0] = r[0];
        l[1] = g[0];
        l[2] = b[0];
        l[3] = 0xff;
        l += 4; r++; g++; b++;
    }
}

void rgbatorgba(unsigned char* r, unsigned char* g,
    unsigned char* b, unsigned char* a, unsigned char* l, int n)
{
    while (n--) {
        l[0] = r[0];
        l[1] = g[0];
        l[2] = b[0];
        l[3] = a[0];
        l += 4; r++; g++; b++; a++;
    }
}

typedef struct _ImageRec {
    unsigned short imagic;
    unsigned short type;
    unsigned short dim;
    unsigned short xsize, ysize, zsize;
    unsigned int min, max;
    unsigned int wasteBytes;
    char name[80];
    unsigned long colorMap;
    FILE* file;
    unsigned char* tmp, * tmpR, * tmpG, * tmpB;
    unsigned long rleEnd;
    unsigned int* rowStart;
    int* rowSize;
} ImageRec;

static void ConvertShort(unsigned short* array, long length)
{
    unsigned b1, b2;
    unsigned char* ptr;

    ptr = (unsigned char*)array;
    while (length--) {
        b1 = *ptr++;
        b2 = *ptr++;
        *array++ = (b1 << 8) | (b2);
    }
}

static void ConvertLong(unsigned* array, long length)
{
    unsigned b1, b2, b3, b4;
    unsigned char* ptr;

    ptr = (unsigned char*)array;
    while (length--) {
        b1 = *ptr++;
        b2 = *ptr++;
        b3 = *ptr++;
        b4 = *ptr++;
        *array++ = (b1 << 24) | (b2 << 16) | (b3 << 8) | (b4);
    }
}

static ImageRec* ImageOpen(const char* fileName)
{
    union {
        int testWord;
        char testByte[4];
    } endianTest;
    ImageRec* image;
    int swapFlag;
    int x;

    endianTest.testWord = 1;
    if (endianTest.testByte[0] == 1) {
        swapFlag = 1;
    }
    else {
        swapFlag = 0;
    }

    image = (ImageRec*)malloc(sizeof(ImageRec));
    if (image == NULL) {
        fprintf(stderr, "Out of memory!\n");
        exit(1);
    }
    if ((image->file = fopen(fileName, "rb")) == NULL) {
        perror(fileName);
        exit(1);
    }

    fread(image, 1, 12, image->file);

    if (swapFlag) {
        ConvertShort(&image->imagic, 6);
    }

    image->tmp = (unsigned char*)malloc(image->xsize * 256);
    image->tmpR = (unsigned char*)malloc(image->xsize * 256);
    image->tmpG = (unsigned char*)malloc(image->xsize * 256);
    image->tmpB = (unsigned char*)malloc(image->xsize * 256);
    if (image->tmp == NULL || image->tmpR == NULL || image->tmpG == NULL ||
        image->tmpB == NULL) {
        fprintf(stderr, "Out of memory!\n");
        exit(1);
    }

    if ((image->type & 0xFF00) == 0x0100) {
        x = image->ysize * image->zsize * sizeof(unsigned);
        image->rowStart = (unsigned*)malloc(x);
        image->rowSize = (int*)malloc(x);
        if (image->rowStart == NULL || image->rowSize == NULL) {
            fprintf(stderr, "Out of memory!\n");
            exit(1);
        }
        image->rleEnd = 512 + (2 * x);
        fseek(image->file, 512, SEEK_SET);
        fread(image->rowStart, 1, x, image->file);
        fread(image->rowSize, 1, x, image->file);
        if (swapFlag) {
            ConvertLong(image->rowStart, x / (int)sizeof(unsigned));
            ConvertLong((unsigned*)image->rowSize, x / (int)sizeof(int));
        }
    }
    else {
        image->rowStart = NULL;
        image->rowSize = NULL;
    }
    return image;
}

static void ImageClose(ImageRec* image)
{
    fclose(image->file);
    free(image->tmp);
    free(image->tmpR);
    free(image->tmpG);
    free(image->tmpB);
    free(image->rowSize);
    free(image->rowStart);
    free(image);
}

static void ImageGetRow(ImageRec* image,
    unsigned char* buf, int y, int z)
{
    unsigned char* iPtr, * oPtr, pixel;
    int count;

    if ((image->type & 0xFF00) == 0x0100) {
        fseek(image->file, (long)image->rowStart[y + z * image->ysize], SEEK_SET);
        fread(image->tmp, 1, (unsigned int)image->rowSize[y + z * image->ysize],
            image->file);

        iPtr = image->tmp;
        oPtr = buf;
        for (;;) {
            pixel = *iPtr++;
            count = (int)(pixel & 0x7F);
            if (!count) {
                return;
            }
            if (pixel & 0x80) {
                while (count--) {
                    *oPtr++ = *iPtr++;
                }
            }
            else {
                pixel = *iPtr++;
                while (count--) {
                    *oPtr++ = pixel;
                }
            }
        }
    }
    else {
        fseek(image->file, 512 + (y * image->xsize) + (z * image->xsize * image->ysize),
            SEEK_SET);
        fread(buf, 1, image->xsize, image->file);
    }
}

unsigned* read_texture(char* name,
    int* width, int* height, int* components)
{
    unsigned* base, * lptr;
    unsigned char* rbuf, * gbuf, * bbuf, * abuf;
    ImageRec* image;
    int y;

    image = ImageOpen(name);

    if (!image)
        return NULL;
    (*width) = image->xsize;
    (*height) = image->ysize;
    (*components) = image->zsize;
    base = (unsigned*)malloc(image->xsize * image->ysize * sizeof(unsigned));
    rbuf = (unsigned char*)malloc(image->xsize * sizeof(unsigned char));
    gbuf = (unsigned char*)malloc(image->xsize * sizeof(unsigned char));
    bbuf = (unsigned char*)malloc(image->xsize * sizeof(unsigned char));
    abuf = (unsigned char*)malloc(image->xsize * sizeof(unsigned char));
    if (!base || !rbuf || !gbuf || !bbuf)
        return NULL;
    lptr = base;
    for (y = 0; y < image->ysize; y++) {
        if (image->zsize >= 4) {
            ImageGetRow(image, rbuf, y, 0);
            ImageGetRow(image, gbuf, y, 1);
            ImageGetRow(image, bbuf, y, 2);
            ImageGetRow(image, abuf, y, 3);
            rgbatorgba(rbuf, gbuf, bbuf, abuf, (unsigned char*)lptr, image->xsize);
            lptr += image->xsize;
        }
        else if (image->zsize == 3) {
            ImageGetRow(image, rbuf, y, 0);
            ImageGetRow(image, gbuf, y, 1);
            ImageGetRow(image, bbuf, y, 2);
            rgbtorgba(rbuf, gbuf, bbuf, (unsigned char*)lptr, image->xsize);
            lptr += image->xsize;
        }
        else if (image->zsize == 2) {
            ImageGetRow(image, rbuf, y, 0);
            ImageGetRow(image, abuf, y, 1);
            latorgba(rbuf, abuf, (unsigned char*)lptr, image->xsize);
            lptr += image->xsize;
        }
        else {
            ImageGetRow(image, rbuf, y, 0);
            bwtorgba(rbuf, (unsigned char*)lptr, image->xsize);
            lptr += image->xsize;
        }
    }
    ImageClose(image);
    free(rbuf);
    free(gbuf);
    free(bbuf);
    free(abuf);

    return (unsigned*)base;
}
/// ////////////////////////////end of image things////////////////////////////////////////////////////////////
static double eyex = 0, eyey = 0, eyez = 30;
static double refx = 0, refy = 0, refz = 0;
static double upx = 0, upy = 1, upz = 0;

static double xminRequested = -5, xmaxRequested = 5;
static double yminRequested = -5, ymaxRequested = 5;
static double zmin = -10, zmax = 10;
static int orthographic = 0;
static int preserveAspect = 1;

static double xminActual, xmaxActual, yminActual, ymaxActual;


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

  /*  glMatrixMode(GL_PROJECTION);
    glOrtho(-4, 4, -2, 2, -2, 2);  
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.5, 0.5, 0.5, 1);*/

    ////glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ////glLoadIdentity();             // Set up modelview transform, first cube.
    ////glTranslatef(0.5, 0, 0);     // Move cube to left half of window.
    glVertexPointer(3, GL_FLOAT, 0, rect.vCoord);  // Set data type and location, first cube.
    glColorPointer(3, GL_FLOAT, 0, rect.vCol);
    //glTexCoordPointer(3, GL_FLOAT, 0, rect.vTex);
    glEnableClientState(GL_VERTEX_ARRAY);
    //glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    ////glEnable(GL_NORMALIZE);
    ////glEnable(GL_COLOR_MATERIAL);
    glDrawElements(GL_QUADS, 24, GL_UNSIGNED_INT, elementArray);
    
    ////glDisableClientState(GL_COLOR_ARRAY);  // Don't use color array for the edges.
    ////glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, edgeElementArray);  // Draw the edges!

    //glutSwapBuffers();

}

void square(double side, float z, float s) {
    double radius = side/2;
    glBegin(GL_POLYGON);
    glNormal3f(0, 0, 1);
    glTexCoord2d(0, 0);
    glVertex3d(-radius - s, -radius, z); //toRight
    glTexCoord2d(1, 0);
    glVertex3d(radius + s, -radius, z); //topLeft
    glTexCoord2d(1, 1);
    glVertex3d(radius + s, radius, z);//bottomLeft
    glTexCoord2d(0, 1);
    glVertex3d(-radius - s, radius, z);//bottomRight
    glEnd();
} // end square
void cube(double side, float z, float sz) {
    glPushMatrix();
    glRotatef(90, 1, 0, 0);  //bottomFace
    glPushMatrix();
    glTranslated(0, 0, side / 2);
    square(side, z + sz, 0);
    glPopMatrix();
    glPushMatrix();
    glRotatef(90, 0, 1, 0);
    glTranslated(0, 0, side / 2);
    square(side, z, sz); //rightFace
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
    glRotatef(-90, 1, 0, 0);//
    glRotatef(90, 0, 0, 1);//
    glTranslated(0, 0, side / 2);
    square(side, z, sz);//front Face
    glPopMatrix();
    glPushMatrix();
    glRotatef(90, 1, 0, 0);
    glRotatef(90, 0, 0, 1);
    glTranslated(0, 0, side / 2);
    square(side, z , sz); // backFace
    glPopMatrix();
    glPopMatrix();
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
        glFrustum(x1, x2, y1, y2, var, viewDistance * 3 - zmin);
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

    //not showing
    glPushMatrix();
    glRotated(-frameNumber, 0, 1, 0);
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


// wall of untouched code ends here***************************************
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
float cameraZoom = 10;

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
    
    //glTranslatef(10, 0, 0);     // Move cube to left half of window.
    
    //glTranslatef(-10, 0, 0);     // Move cube to left half of window.
    
    /*//glutSolidTeapot(2.0);
    ////glTranslatef(0.5, 0, 0);     // Move cube to left half of window.
    glDisable(GL_TEXTURE_2D);
    drawRectangularPrism(rectLeft);
    drawRectangularPrism(rectRight);
    drawRectangularPrism(rectTop);
    drawRectangularPrism(rectBot);
    drawRectangularPrism(rectBack);*/
    //glDisable(GL_TEXTURE_2D);


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
    //fillOuterRectsArrayData(1, 2, 1, 25, true);
    //fillOuterRectsArrayData(1, 1, 2, 25, false);
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
}*/


// --------------------- Menu Keyboard handling --------------------------------

void charTyped(unsigned char ch, int x, int y) {
    switch (ch) {
    //case 'a': case 'A': toggleAnimation(); break;
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



void createMenus() {  // make a menu containing the five render modes.
    int menu = glutCreateMenu(doMenu);
    //glutAddMenuEntry("Toggle Animation (A key)", 1);
    glutAddMenuEntry("Toggle Global Ambient Light (M key)", 1);
    glutAddMenuEntry("Toggle Viewpoint Light (V key)", 2);
    glutAddMenuEntry("Toggle Light (R key)", 3);
    glutAddMenuEntry("Quit (ESC key)", 6);
    glutAddMenuEntry("Zoom-In (W key)", 4);
    glutAddMenuEntry("Zoom-Out (S key)", 5);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
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
    //toggleAnimation();
    glutMainLoop();
    return 0;
}



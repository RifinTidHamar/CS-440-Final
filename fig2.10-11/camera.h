#pragma once


void cameraSetOrthographic(int ortho);

void cameraSetPreserveAspect(int preserve);

void cameraSetLimits(double xmin, double xmax, double ymin, double ymax, double zmin1, double zmax1);

void cameraSetScale(double limit);

void cameraLookAt(double eyeX, double eyeY, double eyeZ,
    double viewCenterX, double viewCenterY, double viewCenterZ,
    double viewUpX, double viewUpY, double viewUpZ);

static double norm(double v0, double v1, double v2);

static void normalize(double v[]);

void cameraApply();

static void reflectInAxis(double axis[], double source[], double destination[]);

static void transformToViewCoords(double v[], double x[], double y[], double z[], double out[]);

static void applyTransvection(double from[3], double to[3]);

static void mousePointToRay(int x, int y, double out[3]);

void trackballMouseFunction(int button, int buttonState, int x, int y);

void trackballMotionFunction(int x, int y);


#ifndef RENDERER_H
#define RENDERER_H

#include<iostream>
#include<limits>
#include"objLoader.h"
#include"extraMath.h"
#include"tgaimage.h"
#include <cstdlib>
#include<cmath>
#include<sstream>
using namespace std;

class Render
{
    public:
        mat<4, 4> F;
        mat<4, 4> modelView;
        Vec3 eye, center, up;
        int x, y, w, h; 
        float f;
        float modelScale;
        Vec3 modelCenter;
        ObjModel obj;

        Render(int width, int height, int topX, int topY);

        mat<4, 4> setViewport();
        mat<4, 4> setPerspective();

        void loadOBJ(string path);

        void bringCamera(Vec3 cameraCoordinates, Vec3 upwardDir, Vec3 centerCoordinates = {0,0,0});
        void changeEye(Vec3 newEye);
        void setModelView();

        void line(int ax, int ay, int bx, int by, TGAColor color, TGAImage& framebuffer);

        void renderHelper(Vec3 &A, Vec3 &B, Vec3 &C, vector<double> &zBuffer, TGAImage& framebuffer);

        TGAImage renderFrame();

        void getScreenCoordinates(Vec3 &ndcCoordinates);

        void drawXZ(TGAImage &framebuffer);

        void phongReflectionHelper(Vec3 &A, Vec3 &B, Vec3 &C, float brightness, vector<double> &zBuffer, TGAImage& framebuffer);

        TGAImage phongReflection(Vec3 sun);
};

#endif

#include<iostream>
#include"tgaimage.h"
#include<vector>
#include"extraMath.h"
#include"basicTGA.h"
using namespace std;


void plotTriangle(int ax, int ay, int az, int bx, int by, int bz, int cx, int cy, int cz, TGAColor color, vector<double> &zBuffer, int width, TGAImage &framebuffer) {
    //if (az < 0 || bz < 0 || cz < 0)  return;

    int bbminx = min(min(ax, bx), cx); // bounding box for the triangle
    int bbminy = min(min(ay, by), cy); // defined by its top left and bottom right corners
    int bbmaxx = max(max(ax, bx), cx);
    int bbmaxy = max(max(ay, by), cy);
    double total_area = signed_triangle_area(ax, ay, bx, by, cx, cy);
    if(total_area<-1) return;
    for (int x=bbminx; x<=bbmaxx; x++) {
        for (int y=bbminy; y<=bbmaxy; y++) {
            double alpha = signed_triangle_area(x, y, bx, by, cx, cy) / total_area;
            double beta  = signed_triangle_area(x, y, cx, cy, ax, ay) / total_area;
            double gamma = signed_triangle_area(x, y, ax, ay, bx, by) / total_area;
            if (alpha<0 || beta<0 || gamma<0) continue; // negative barycentric coordinate => the pixel is outside the triangle
            double z = (alpha * az + beta * bz + gamma * cz);
            if(z <= zBuffer[x+y*width]) continue;          // z buffer.
            //z = zBuffer[x+y*width];
            zBuffer[x+y*width] = z;

            //cout << "did it rn \n";
            framebuffer.set(x, y, color);
    
        }
    }

    /*
    cout << "Plotting at " << ax << ", " << ay << "   ";
    cout << bx << ", " << by << "  ";
    cout << cx << ", " << cy << "   \n";

    */


    
    
}

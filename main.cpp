#include <cmath>
#include "lib/renderer.h"

int main()
{
    const int WIDTH  = 2000;
    const int HEIGHT = 2000;
    const int FRAMES = 500;

    Render r(WIDTH, HEIGHT, 0, 0);
    r.loadOBJ("assets/your_file.obj");
    Vec3 eye = { 4.0f, 0.0f, -2.0f};
    Vec3 center = {0, 0, 0};
    Vec3 up     = {0, -1, 0};
    

    r.bringCamera(eye, up, center);
    TGAImage frame = r.phongReflection(eye);
    frame.write_tga_file("output.tga");


    return 0;
}

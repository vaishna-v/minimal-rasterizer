// This is my approach to make renderer.cpp
// this will expose a class
// u can set up camera, change perspective, change resolution
// and basically make all the changes in this file.
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
        // pipeline for any rendering is -
        // T = ViewPort * Perspective * ModelView * (x)
        // T = Viewport * F * x


        // pipelining for my program-
        // 1. constructor sets up the frame to be rendered in (ie rectangle, or the complete screen)
        // 2. Load the obj file using loadObj(path);
        // 3. Setup the camera manually using bringCamera(Vec3 camera, int focallength, Vec3 up (direction to denote UP), Vec3 center (0,0,0 by default));
        //      bringCamera will automatically setup modelView matrix, and normalize up
        // 4. in Order to change camera, u can then do moveCamera(x, y, z) 
        //      this will also automatically setup modelView matrix;
        // 5. View final image using renderFrame() function -> it calls buildF(), render the frame
        //        

        mat<4, 4> F;        // Final transformation function
        mat<4, 4> modelView;
        Vec3 eye, center, up;
        int x, y, w, h; 
        float f; // Focal length of camera
        ObjModel obj; // Object containing all the data

        Render(int width, int height, int topX, int topY): w(width), h(height), x(topX), y(topY)    {}

        mat<4, 4> setViewport()
        {
            mat<4, 4> result = {{{ w/2.f, 0.f, 0.f, x + w/2.f },{ 0.f, h/2.f, 0.f, y + h/2.f },{ 0.f, 0.f, 1, 0},{ 0.f, 0.f, 0.f, 1.f }}};
            return result;
        }

        mat<4, 4> setPerspective()
        {
            mat<4, 4> result = {{{1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0,-1.f/f,1.}}};
            return result;
        }

        void loadOBJ(string path)
        {
            obj = buildOBJ(path);
        }

        void bringCamera(Vec3 cameraCoordinates, Vec3 upwardDir, Vec3 centerCoordinates = {0,0,0})
        {
            eye = cameraCoordinates;
            center = centerCoordinates;
            f = norm(eye-center);
            up = upwardDir;
            up = normalize(up);
            setModelView();
        }
        void changeEye(Vec3 newEye)
        {
            eye = newEye;
            setModelView();
        }
        void setModelView()
        {
            // based on the camera, we need to calculate l, m, n;
            Vec3 n = normalize(eye - center);
            Vec3 l = normalize(up * n);
            Vec3 m = normalize(n * l);

            mat<4,4> R = {{{l.x, l.y, l.z, 0}, {m.x, m.y, m.z, 0}, {n.x, n.y, n.z, 0}, {0,0,0,1}}};
            mat<4,4> T = {{{1 , 0, 0, -center.x}, {0,1,0,-center.y}, {0,0,1,-center.z}, {0,0,0,1}}};
            
            modelView = R*T;
        }
        void line(int ax, int ay, int bx, int by, TGAColor color, TGAImage& framebuffer)
        {
            // check for steepness-
            bool steep = false;
            if(bx - ax < by - ay)
            {
                swap(ax, ay);
                swap(bx, by);
                steep = true;
            }
        
            // always plot from left to right-
            if(ax > bx)
            {
                swap(ax, bx);
                swap(ay, by);
            }
        
            int y = ay;
            int ierror = 0;
            for(int x = ax; x<=bx; x++)
            {
                if(steep)
                    framebuffer.set(y, x, color);
                else
                    framebuffer.set(x, y, color);
                ierror += 2*abs(by - ay);
                y += (by>ay? 1: -1) * (ierror > bx - ax);
                ierror -= 2*(bx-ax)*(ierror>bx-ax);
            }
        
        }


        void renderHelper(Vec3 &A, Vec3 &B, Vec3 &C, vector<double> &zBuffer, TGAImage& framebuffer)
        {   
            auto transform = [&](Vec3 &v, Vec3 &finalView)
            {
                // clip space
                mat<4,1> temp = {{{v.x}, {v.y}, {v.z}, {1}}};
                // view space
                mat<4,1> view = modelView * temp;      
                

                // projection
                mat<4,1> clip = setPerspective() * view;     // exists in ndc
                if(clip(3,0)==0)    clip(3,0) = 1e-6;
                clip(0,0) /= clip(3,0);
                clip(1,0) /= clip(3,0);
                clip(2,0) /= clip(3,0);
                clip(3,0) = 1;
                finalView = {clip(0,0), clip(1,0), clip(2,0)};   //copyFinal view

                // viewport AFTER divide
                mat<4,1> screen = setViewport() * clip;
                v.x = screen(0,0);
                v.y = screen(1,0);
                v.z = screen(2,0);


                //cout << "Screen coordinates: " << v.x << " " << v.y << " " << v.z << " \n";
            };
            Vec3 vA, vB, vC;
            transform(A, vA); transform(B, vB); transform(C, vC);

            
            TGAColor rnd(rand()%255, rand()%255, rand()%255, 255);

            // to calculate barycentric coordinates, i will use view - coordinates, however to plot, i will use A.x, A.y
            int bbminx = (int)min(min(A.x, B.x), C.x); // bounding box for the triangle
            int bbminy = (int)min(min(A.y, B.y), C.y); // defined by its top left and bottom right corners
            int bbmaxx = (int)max(max(A.x, B.x), C.x);
            int bbmaxy = (int)max(max(A.y, B.y), C.y);

            // fixing outofbound check ( as per chatgpt)
            bbminx = max(0, bbminx);
            bbminy = max(0, bbminy);
            bbmaxx = min(w-1, bbmaxx);
            bbmaxy = min(h-1, bbmaxy);

            double total_area = signed_triangle_area(A.x, A.y, B.x, B.y, C.x, C.y);
            if(fabs(total_area)<1e-6) return;       //reject incredibly small triangle
            for (int x=bbminx; x<=bbmaxx; x++) 
            {
                for (int y=bbminy; y<=bbmaxy; y++) 
                {
                    double alpha = signed_triangle_area(x, y, B.x, B.y, C.x, C.y) / total_area;
                    double beta  = signed_triangle_area(x, y, C.x, C.y, A.x, A.y) / total_area;
                    double gamma = signed_triangle_area(x, y, A.x, A.y, B.x, B.y) / total_area;
                    if (alpha<0 || beta<0 || gamma<0) continue; // negative barycentric coordinate => the pixel is outside the triangle
                    double z = (alpha * vA.z + beta * vB.z + gamma * vC.z);
                    if(z <= zBuffer[x+y*w]) continue;          // z buffer.
                    zBuffer[x+y*w] = z;
                    
                    // for depth color
                    // map [-1,1] → [0,1]
                    double znorm = (z + 1.0) * 0.5;

                    // clamp just in case
                    znorm = std::min(1.0, std::max(0.0, znorm));

                    // map to [0,255]
                    unsigned char zn = static_cast<unsigned char>(znorm * 255.0);
                    TGAColor depth = {zn, zn, zn, 255};


                    framebuffer.set(x, y, depth);
                }
            }

            
            
            
        }
        TGAImage renderFrame()
        {
            vector<double> zBuffer(w*h, -numeric_limits<double>::max());
            TGAImage framebuffer(w, h, TGAImage::RGB);
            for(auto face: obj.faces)
            {
                Vec3 a = obj.vertices[face.a];
                Vec3 b = obj.vertices[face.b];
                Vec3 c = obj.vertices[face.c];

                renderHelper(a, b, c, zBuffer, framebuffer);
            }

            return framebuffer;
        }
        void getScreenCoordinates(Vec3 &ndcCoordinates)
        {
            mat<4, 1> ndc = {{{ndcCoordinates.x}, {ndcCoordinates.y}, {ndcCoordinates.z}, 1}};
            mat<4,1> view = modelView * ndc;
            mat<4,1> clip = setPerspective() * view;     // exists in ndc
            if(clip(3,0)==0)    clip(3,0) = 1e-6;
            clip(0,0) /= clip(3,0);
            clip(1,0) /= clip(3,0);
            clip(2,0) /= clip(3,0);
            clip(3,0) = 1;
            mat<4,1> screen = setViewport() * clip;

            // copy 
            ndcCoordinates.x =   screen(0,0);
            ndcCoordinates.y = screen(1,0);
            ndcCoordinates.z = screen(2,0);

        }
        


        //doesnt work due to various reasons
        void drawXZ(TGAImage &framebuffer)
        {
            for (float x = -1; x <= 1; x += 0.1f)
            {
                Vec3 A = {x, -1, -1};
                Vec3 B = {x, -1,  1};
            
                getScreenCoordinates(A);
                getScreenCoordinates(B);
            
                line(A.x, A.y, B.x, B.y, {255,255,255,1}, framebuffer);
            }
        
            for (float z = -1; z <= 1; z += 0.1f)
            {
                Vec3 A = {-1, -0.4, z};
                Vec3 B = { 1, -0.4, z};
            
                getScreenCoordinates(A);
                getScreenCoordinates(B);
            
                line(A.x, A.y, B.x, B.y, {255,255,255,1}, framebuffer);
            }
        }




        void phongReflectionHelper(Vec3 &A, Vec3 &B, Vec3 &C, float brightness, vector<double> &zBuffer, TGAImage& framebuffer)
        {
            auto transform = [&](Vec3 &v, Vec3 &finalView)
            {
                // clip space
                mat<4,1> temp = {{{v.x}, {v.y}, {v.z}, {1}}};
                // view space
                mat<4,1> view = modelView * temp;      
                

                // projection
                mat<4,1> clip = setPerspective() * view;     // exists in ndc
                if(clip(3,0)==0)    clip(3,0) = 1e-6;
                clip(0,0) /= clip(3,0);
                clip(1,0) /= clip(3,0);
                clip(2,0) /= clip(3,0);
                clip(3,0) = 1;
                finalView = {clip(0,0), clip(1,0), clip(2,0)};   //copyFinal view

                // viewport AFTER divide
                mat<4,1> screen = setViewport() * clip;
                v.x = screen(0,0);
                v.y = screen(1,0);
                v.z = screen(2,0);


                //cout << "Screen coordinates: " << v.x << " " << v.y << " " << v.z << " \n";
            };
            Vec3 vA, vB, vC;
            transform(A, vA); transform(B, vB); transform(C, vC);

            // Calculate brightness beforehand-
            // map to [0,255]
            unsigned char zn = static_cast<unsigned char>(brightness * 255.0);
            TGAColor reflection = {zn, zn, zn, 255};


            // to calculate barycentric coordinates, i will use view - coordinates, however to plot, i will use A.x, A.y
            int bbminx = (int)min(min(A.x, B.x), C.x); // bounding box for the triangle
            int bbminy = (int)min(min(A.y, B.y), C.y); // defined by its top left and bottom right corners
            int bbmaxx = (int)max(max(A.x, B.x), C.x);
            int bbmaxy = (int)max(max(A.y, B.y), C.y);

            // fixing outofbound check ( as per chatgpt)
            bbminx = max(0, bbminx);
            bbminy = max(0, bbminy);
            bbmaxx = min(w-1, bbmaxx);
            bbmaxy = min(h-1, bbmaxy);

            double total_area = signed_triangle_area(A.x, A.y, B.x, B.y, C.x, C.y);
            if(fabs(total_area)<1e-6) return;       //reject incredibly small triangle
            for (int x=bbminx; x<=bbmaxx; x++) 
            {
                for (int y=bbminy; y<=bbmaxy; y++) 
                {
                    double alpha = signed_triangle_area(x, y, B.x, B.y, C.x, C.y) / total_area;
                    double beta  = signed_triangle_area(x, y, C.x, C.y, A.x, A.y) / total_area;
                    double gamma = signed_triangle_area(x, y, A.x, A.y, B.x, B.y) / total_area;
                    if (alpha<0 || beta<0 || gamma<0) continue; // negative barycentric coordinate => the pixel is outside the triangle
                    double z = (alpha * vA.z + beta * vB.z + gamma * vC.z);
                    if(z <= zBuffer[x+y*w]) continue;          // z buffer.
                    zBuffer[x+y*w] = z;
                    framebuffer.set(x, y, reflection);
                }
            }
        }

        TGAImage phongReflection(Vec3 sun)
        {
            vector<double> zBuffer(w*h, -numeric_limits<double>::max());
            float ambient = 0.15;        // fix ambient term
            sun = normalize(sun);
            TGAImage framebuffer(w, h, TGAImage::RGB);
            for(auto face: obj.faces)
            {
                Vec3 a = obj.vertices[face.a];
                Vec3 b = obj.vertices[face.b];
                Vec3 c = obj.vertices[face.c];


                Vec3 normal = Vec3{b.x-a.x, b.y-a.y, b.z-a.z} * Vec3{c.x-a.x, c.y-a.y, c.z-a.z};
                normal = normalize(normal);

                Vec3 center_of_triangle = {(a.x+b.x+c.x)/3, (a.y+b.y+c.y)/3, (a.z+b.z+c.z)/3};
                Vec3 towards_sun = sun - center;


                // This is diffuse term- (angle between normal and sun)
                float diffuse = center_of_triangle.x * towards_sun.x + center_of_triangle.y * towards_sun.y + center_of_triangle.z * towards_sun.z; 
                diffuse = fmax(0,  diffuse);

                // Find specular term- (angle between perfect reflection and viewwer)
                Vec3 viewer_angle = {center_of_triangle.x - eye.x, center_of_triangle.y - eye.y, center_of_triangle.y - eye.y};
                // now i have to calculate perfect reflection angle-
                // will derive this formula later: r = 2n(n.l) - l(towards sun)
                normal = normalize(normal);
                towards_sun = normalize(towards_sun);
                Vec3 perfectReflection = 2*diffuse*normal - towards_sun;
                perfectReflection = normalize(perfectReflection);
                Vec3 normalized_eye = normalize(eye - center_of_triangle);
                float specular = perfectReflection.x * normalized_eye.x + perfectReflection.y * normalized_eye.y + normalized_eye.z * eye.z;

                specular = fmax(0, specular);
                phongReflectionHelper(a, b, c, ambient+diffuse+0, zBuffer, framebuffer);
            }

            return framebuffer;
        }


};



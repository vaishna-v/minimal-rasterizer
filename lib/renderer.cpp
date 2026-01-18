// This is my approach to make renderer.cpp
// this will expose a class
// u can set up camera, change perspective, change resolution
// and basically make all the changes in this file.
#include "renderer.h"
#include<cmath>
#include<limits>
const float INF = std::numeric_limits<float>::infinity();
Render::Render(int width, int height, int topX, int topY): w(width), h(height), x(topX), y(topY)    {}

mat<4, 4> Render::setViewport()
{
    mat<4, 4> result = {{{ w/2.f, 0.f, 0.f, x + w/2.f },{ 0.f, h/2.f, 0.f, y + h/2.f },{ 0.f, 0.f, 1, 0},{ 0.f, 0.f, 0.f, 1.f }}};
    return result;
}

mat<4, 4> Render::setPerspective()
{
    mat<4, 4> result = {{{1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0,-1.f/f,1.}}};
    return result;
}

void Render::loadOBJ(string path)
{
    obj = buildOBJ(path);

    Vec3 minv = {+INF,+INF,+INF};
    Vec3 maxv = {-INF,-INF,-INF};

    for (auto &v : obj.vertices) {
        minv.x = std::min(minv.x, v.x);
        minv.y = std::min(minv.y, v.y);
        minv.z = std::min(minv.z, v.z);

        maxv.x = std::max(maxv.x, v.x);
        maxv.y = std::max(maxv.y, v.y);
        maxv.z = std::max(maxv.z, v.z);
    }

    Vec3 size = maxv - minv;
    float maxExtent = std::max(size.x, size.y);  maxExtent = std::max(maxExtent, size.z);
    modelScale = 1.5f/maxExtent;
    modelCenter = 0.5f * (minv + maxv);


}

void Render::bringCamera(Vec3 cameraCoordinates, Vec3 upwardDir, Vec3 centerCoordinates)
{
    eye = cameraCoordinates;
    center = centerCoordinates;
    f = norm(eye-center);
    up = upwardDir;
    up = normalize(up);
    setModelView();
}

void Render::changeEye(Vec3 newEye)
{
    eye = newEye;
    setModelView();
}

void Render::setModelView()
{
    // based on the camera, we need to calculate l, m, n;
    Vec3 n = normalize(eye - center);
    Vec3 l = normalize(up * n);
    Vec3 m = normalize(n * l);

    mat<4,4> R = {{{l.x, l.y, l.z, 0}, {m.x, m.y, m.z, 0}, {n.x, n.y, n.z, 0}, {0,0,0,1}}};
    mat<4,4> T = {{{1 , 0, 0, -center.x}, {0,1,0,-center.y}, {0,0,1,-center.z}, {0,0,0,1}}};
    
    // for scaling too big objects
    mat<4,4> M_translate = {{
        {1,0,0,-modelCenter.x},
        {0,1,0,-modelCenter.y},
        {0,0,1,-modelCenter.z},
        {0,0,0,1}
    }}; 

    mat<4,4> M_scale = {{
        {modelScale,0,0,0},
        {0,modelScale,0,0},
        {0,0,modelScale,0},
        {0,0,0,1}
    }};

    modelView = R*T*M_scale*M_translate;
}

void Render::line(int ax, int ay, int bx, int by, TGAColor color, TGAImage& framebuffer)
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

void Render::renderFramezHelper(Vec3 &A, Vec3 &B, Vec3 &C, vector<double> &zBuffer, TGAImage& framebuffer)
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

    // outbound chekc
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
            unsigned char zn = static_cast<unsigned char>(znorm * 200.0);
            TGAColor depth = {zn, zn, zn, 255};


            framebuffer.set(x, y, depth);
        }
    }

    
    
    
}

TGAImage Render::renderFramez()
{
    vector<double> zBuffer(w*h, -numeric_limits<double>::max());
    TGAImage framebuffer(w, h, TGAImage::RGB);
    for(auto face: obj.faces)
    {
        Vec3 a = obj.vertices[face.a];
        Vec3 b = obj.vertices[face.b];
        Vec3 c = obj.vertices[face.c];

        renderFramezHelper(a, b, c, zBuffer, framebuffer);
    }

    return framebuffer;
}


void Render::phongReflectionHelper(Vec3 &A, Vec3 &B, Vec3 &C, float brightness, vector<double> &zBuffer, TGAImage& framebuffer)
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
    Vec3 As = A;
    Vec3 Bs = B;
    Vec3 Cs = C;

    transform(As, vA);
    transform(Bs, vB);
    transform(Cs, vC);

    // Calculate brightness beforehand-
    // map to [0,255]
    brightness = std::min(1.0f, std::max(0.0f, brightness));
    unsigned char zn = static_cast<unsigned char>(brightness * 200);
    TGAColor reflection = {zn, zn, zn, 255};


    // to calculate barycentric coordinates, i will use view - coordinates, however to plot, i will use A.x, A.y
    int bbminx = (int)min(min(As.x, Bs.x), Cs.x); // bounding box for the triangle
    int bbminy = (int)min(min(As.y, Bs.y), Cs.y); // defined by its top left and bottom right corners
    int bbmaxx = (int)max(max(As.x, Bs.x), Cs.x);
    int bbmaxy = (int)max(max(As.y, Bs.y), Cs.y);

    // outbound chekc
    bbminx = max(0, bbminx);
    bbminy = max(0, bbminy);
    bbmaxx = min(w-1, bbmaxx);
    bbmaxy = min(h-1, bbmaxy);

    double total_area = signed_triangle_area(As.x, As.y, Bs.x, Bs.y, Cs.x, Cs.y);
    if(fabs(total_area)<1e-6) return;       //reject incredibly small triangle
    for (int x=bbminx; x<=bbmaxx; x++) 
    {
        for (int y=bbminy; y<=bbmaxy; y++) 
        {
            double alpha = signed_triangle_area(x, y, Bs.x, Bs.y, Cs.x, Cs.y) / total_area;
            double beta  = signed_triangle_area(x, y, Cs.x, Cs.y, As.x, As.y) / total_area;
            double gamma = signed_triangle_area(x, y, As.x, As.y, Bs.x, Bs.y) / total_area;
            if (alpha<0 || beta<0 || gamma<0) continue; // negative barycentric coordinate => the pixel is outside the triangle
            double z = (alpha * vA.z + beta * vB.z + gamma * vC.z);
            if(z <= zBuffer[x+y*w]) continue;          // z buffer.
            zBuffer[x+y*w] = z;
            framebuffer.set(x, y, reflection);
        }
    }
}

TGAImage Render::phongReflection(Vec3 sun)
{
    setModelView();
    vector<double> zBuffer(w*h, -numeric_limits<double>::max());
    float ambient = 0.1;        // fix ambient term
    sun = normalize(sun);
    TGAImage framebuffer(w, h, TGAImage::RGB);
    for(auto face: obj.faces)
    {
        Vec3 a = obj.vertices[face.a];
        Vec3 b = obj.vertices[face.b];
        Vec3 c = obj.vertices[face.c];


        Vec3 normal =  Vec3{b - a} * Vec3{c - a} ;
        normal = normalize(normal);
        Vec3 center_of_triangle = {(1/3.) * (a + b + c)};
        Vec3 towards_sun = sun - center_of_triangle;
        towards_sun = normalize(towards_sun);



        // This is diffuse term- (angle between normal and sun)
        float diffuse = normal.x * towards_sun.x + normal.y * towards_sun.y + normal.z * towards_sun.z;
        diffuse = fmax(0.0f,  diffuse);

        // for specular
        Vec3 viewDir = normalize(eye - center_of_triangle);
        float NL = normal.x * towards_sun.x + normal.y * towards_sun.y + normal.z * towards_sun.z;
        Vec3 reflection = (2.0f * NL) * normal - towards_sun;
        reflection = normalize(reflection);
        float RV = reflection.x * viewDir.x+ reflection.y * viewDir.y+ reflection.z * viewDir.z;    
        RV = fmax(0.0f, RV);
        float shininess = 32.0f;   // try 16, 32, 64
        float specular = pow(RV, shininess);
        float specularStrength = 0.3f;
        specular *= specularStrength;
        specular = fmax(0, specular);
        phongReflectionHelper(a, b, c, ambient+diffuse+specular, zBuffer, framebuffer);
    }

    return framebuffer;
}

#include "extraMath.h"
#include<iostream>

#include<cmath>

Vec3 operator / (Vec3 &A, float b)
{
    return {A.x / b, A.y / b, A.z / b}; 
}

Vec3 normalize(Vec3 a)
{
    float len = sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
    return a/len;
}

Vec3 operator - (const Vec3 &A, const Vec3 &B)
{
    return {A.x - B.x, A.y - B.y, A.z - B.z};
}

Vec3 operator * (const Vec3 &A, const Vec3 &B)
{
    return {A.y*B.z - A.z*B.y, A.z*B.x - A.x*B.z, A.x*B.y - A.y*B.x};
}

 
double signed_triangle_area(int ax, int ay, int bx, int by, int cx, int cy) {
    return .5*((by-ay)*(bx+ax) + (cy-by)*(cx+bx) + (ay-cy)*(ax+cx));
}

double norm(Vec3 a)
{
    return sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
}

Vec3 operator * (float B, Vec3 A)
{
    return {A.x*B, A.y*B, A.z*B};
}

Vec3 operator + (const Vec3 &A, const Vec3 &B)
{
    return {A.x + B.x, A.y + B.y, A.z + B.z};
}




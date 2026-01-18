#include<iostream>
#pragma once
struct Vec3
{
    float x, y, z;
};
struct Vec4
{
    float x, y, z, e;
};
struct Face
{
    int a, b, c;
};
template<int m, int n>
struct mat
{
    float data[m][n];
    float& operator()(int i, int j)
    {
        return data[i][j];
    }
    const float& operator()(int i, int j) const 
    {
        return data[i][j];
    }
};

template<int m, int n, int p>
mat<m, p> operator * (const mat<m, n> &A, const mat<n, p> &B)
{
    mat<m, p> result{};

    for(int i = 0; i < m; i++)
    {
        for(int j = 0; j < p; j++)
        {
            for(int k = 0; k< n; k++)
            {
                result(i, j) += A(i, k) * B(k, j);
            }
        }
    }

    return result;
}

template<int m, int n>
void displayMat(mat<m,n> &matrix, std::string message)
{
    std::cout << std::endl << message << std::endl;
    for(int i = 0; i < m; i++)
    {
        for(int j = 0; j < n; j++)
            std::cout << matrix(i, j) << " ";
        std::cout << "\n";
    }
    std::cout << std::endl;
}




Vec3 operator / (Vec3 &A, float b);
Vec3 operator * (float b, Vec3 B);

Vec3 normalize(Vec3 a);
Vec3 operator - (const Vec3 &A, const Vec3 &B);
Vec3 operator + (const Vec3 &A, const Vec3 &B);
Vec3 operator * (const Vec3 &A, const Vec3 &B);
Vec3 operator * (const mat<4,4> &A, const Vec4 &B);
double signed_triangle_area(int ax, int ay, int bx, int by, int cx, int cy);
double norm(Vec3 a);



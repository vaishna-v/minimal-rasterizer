#include<iostream>
#include<vector>
#include<string>
#include"extraMath.h"



// This structure is meant to be returned when an .obj file is parsed
struct ObjModel
{
    std::vector<Vec3> vertices;  // contain all vertex
    std::vector<Face> faces;     // contains indices where vertices are stored
};



ObjModel buildOBJ(std::string path);
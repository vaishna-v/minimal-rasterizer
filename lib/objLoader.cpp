#include<iostream>
#include<fstream>
#include<sstream>
#include"extraMath.h"
#include<vector>
#include<string>
#include"objLoader.h"

using namespace std;

ObjModel buildOBJ(string path)
{
    ObjModel result;
    // read the file
    ifstream file(path);
    if(!file)
    {
        cerr << "File could not be opened";
        return result;
    }
    string line;
    while(getline(file, line))
    {
        stringstream ss(line);
        string type;
        ss >> type;
        if(type == "v")
        {
            Vec3 v;
            ss >> v.x >> v.y >> v.z;
            result.vertices.push_back(v);
        }
        else if(type == "f")
        {
            Face f;
            string s1, s2, s3;
            ss >> s1 >> s2 >> s3;
            f.a = stoi(s1.substr(0, s1.find('/'))) - 1;
            f.b = stoi(s2.substr(0, s2.find('/'))) - 1;
            f.c = stoi(s3.substr(0, s3.find('/'))) - 1;
            result.faces.push_back(f);
        }
    }

    return result;

}
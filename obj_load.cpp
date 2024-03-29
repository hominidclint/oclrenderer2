#include "obj_load.hpp"
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <string.h>
#include "triangle.hpp"
#include "texture.hpp"
#include <math.h>
#include <list>
#include <boost/bind.hpp>

std::string retrieve_diffuse_new(std::vector<std::string> file, std::string name)
{
    bool found = false;
    for(unsigned int i=0; i<file.size(); i++)
    {
        if(strncmp(file[i].c_str(), "newmtl ", 7)==0 && file[i].substr(file[i].find_last_of(" ")+1, name.size()) == name)
        {
            found = true;
        }
        if(found && strncmp(file[i].c_str(), "map_Kd ", 7)==0)
        {
            return file[i].substr(file[i].find_last_of(" ")+1, std::string::npos);
        }
    }
    return std::string("");
}

std::string retrieve_bumpmap(std::vector<std::string> file, std::string name)
{
    bool found = false;
    for(unsigned int i=0; i<file.size(); i++)
    {
        ///found newmtl + name of material
        if(strncmp(file[i].c_str(), "newmtl ", 7)==0 && file[i].substr(file[i].find_last_of(" ")+1, name.size()) == name)
        {
            found = true;
            continue;
        }
        if(found && strncmp(file[i].c_str(), "map_Bump ", 9)==0)
        {
            return file[i].substr(file[i].find_last_of(" ")+1, std::string::npos);
        }
        if(found && strncmp(file[i].c_str(), "newmtl ", 7)==0)
        {
            return std::string("None");
        }
    }
    return std::string("None");
}



///vertex, texture coordinate, normal
///remember, offset by one for faces

template <typename T>
void decompose_attribute(const std::string &str, T a[], int n)
{
    size_t pos = str.find(".");
    int s[n+1];
    ///initialise first element to be initial position
    s[0]={str.find(" ")};
    for(int i=1; i<n+1; i++)
    {
        s[i] = str.find(" ", s[i-1]+1); ///implicitly finds end, so correct for n despite no /
        std::string p = str.substr(s[i-1]+1, (s[i] - s[i-1] - 1));

        if(pos==std::string::npos)
        {
            a[i-1] = atoi(p.c_str());
        }
        else
        {
            a[i-1] = atof(p.c_str());
        }
    }
}

void decompose_face(const std::string &str, int v[3], int vt[3], int vn[3])
{
    ///assume valid str because there is no sensible fail case where this isn't a bug
    int start = 2;

    for(int i=0; i<3; i++)
    {
        int s1 = str.find("/", start);
        int s2 = str.find("/", s1+1);
        int s3 = str.find(" ", s2+1);

        std::string p1 = str.substr(start, s1-start);
        std::string p2 = str.substr(s1+1, (s2 - s1 - 1));
        std::string p3 = str.substr(s2+1, (s3 - s2 - 1));

        v[i]  = atoi(p1.c_str()) - 1;
        vt[i] = atoi(p2.c_str()) - 1;
        vn[i] = atoi(p3.c_str()) - 1;

        start = s3 + 1;
    }
}

struct component
{
    float x, y, z;
};

struct indices
{
    int v[3];
    int vt[3];
    int vn[3];
};

void obj_load(objects_container* pobj)
{
    std::string filename = pobj->file;
    std::string mtlname;
    int tp = filename.find_last_of(".");
    mtlname = filename.substr(0, tp) + std::string(".mtl");

    int lslash = filename.find_last_of("/");

    std::string dir = filename.substr(0, lslash);

    std::ifstream file;
    std::ifstream mtlfile;
    file.open(filename.c_str());
    mtlfile.open(mtlname.c_str());
    std::vector<std::string> file_contents;
    std::vector<std::string> mtlf_contents;

    int vc=0, vnc=0, fc=0, vtc=0;

    if(!file.is_open())
    {
        std::cout << filename << " could not be opened" << std::endl;
        //return NULL;
        return;
    }

    if(!mtlfile.is_open())
    {
        std::cout << mtlname << " could not be found" << std::endl;
    }

    while(file.good())
    {
        std::string str;
        std::getline(file, str);
        file_contents.push_back(str);
    }

    while(mtlfile.good())
    {
        std::string str;
        std::getline(mtlfile, str);
        mtlf_contents.push_back(str);
    }

    ///find number of different types of things - vertices, faces, uv coordinates, normals
    for(size_t i=0; i<file_contents.size(); i++)
    {
        std::string ln = file_contents[i];
        if(ln.size() < 2)
        {
            continue;
        }

        if(ln[0]=='v' && ln[1]!=' ')
        {
            vc++;
        }
        if(ln[0]=='v' && ln[1]=='n')
        {
            vnc++;
        }
        if(ln[0]=='v' && ln[1]=='t')
        {
            vtc++;
        }
        if(ln[0]=='f' && ln[1] == ' ')
        {
            fc++;
        }
    }


    std::vector<int> usemtl_pos;
    std::vector<std::string> usemtl_name;
    std::vector<component> vl, vnl, vtl;
    std::vector<indices> fl;

    vl.reserve(vc);
    vnl.reserve(vnc);
    vtl.reserve(vtc);
    fl.reserve(fc);

    int usefc=0;

    for(size_t i=0; i<file_contents.size(); i++)
    {
        if(strncmp(file_contents[i].c_str(), "f ", 2)==0)
        {
            usefc++;
            int v[3];
            int vt[3];
            int vn[3];
            decompose_face(file_contents[i], v, vt, vn);
            indices f;
            for(int j=0; j<3; j++)
            {
                f.v[j]  = v[j];
                f.vt[j] = vt[j];
                f.vn[j] = vn[j];
            }
            fl.push_back(f);
            continue;
        }
        ///if == n then push normals etc
        if(strncmp(file_contents[i].c_str(), "v ", 2)==0)
        {
            float v[3];
            decompose_attribute(file_contents[i], v, 3);
            component t;
            t.x = v[0];
            t.y = v[1];
            t.z = v[2];
            vl.push_back(t);
            continue;
        }
        if(strncmp(file_contents[i].c_str(), "vt ", 3)==0)
        {
            float vt[3];
            decompose_attribute(file_contents[i], vt, 2);
            component t;
            t.x = vt[0];
            t.y = vt[1];
            //t.z = vt[2];
            vtl.push_back(t);
            continue;
        }
        if(strncmp(file_contents[i].c_str(), "vn ", 3)==0)
        {
            float vn[3];
            decompose_attribute(file_contents[i], vn, 3);
            component t;
            t.x = vn[0];
            t.y = vn[1];
            t.z = vn[2];
            vnl.push_back(t);
            continue;
        }
        if(strncmp(file_contents[i].c_str(), "usemtl", 6)==0)
        {
            usemtl_pos.push_back(usefc);
            usemtl_name.push_back(file_contents[i].substr(file_contents[i].find_last_of(" ")+1, std::string::npos));
            continue;
        }
    }

    file_contents.clear();

    ///now, resolve

    std::vector<triangle> tris;
    std::vector<vertex> fr;
    fr.reserve(fc);

    tris.reserve(fc);

    for(size_t i=0; i<fl.size(); i++)
    {
        vertex vert[3];
        indices index;
        index = fl[i];
        for(int j=0; j<3; j++)
        {
            component v, vt, vn;
            v  = vl [index.v [j]];
            vt = vtl[index.vt[j]];
            vn = vnl[index.vn[j]];

            vert[j].pos[0] = v.x;
            vert[j].pos[1] = v.y;
            vert[j].pos[2] = v.z;

            vert[j].vt[0] = vt.x;
            vert[j].vt[1] = vt.y;

            vert[j].normal[0] = vn.x;
            vert[j].normal[1] = vn.y;
            vert[j].normal[2] = vn.z;
        }
        triangle t;
        t.vertices[0] = vert[0];
        t.vertices[1] = vert[1];
        t.vertices[2] = vert[2];
        tris.push_back(t);
    }

    vl.clear();
    vtl.clear();
    vnl.clear();
    fl.clear();


    objects_container *c = pobj;

    usemtl_pos.push_back(tris.size());

    for(unsigned int i=0; i<usemtl_pos.size()-1; i++)
    {
        std::string texture_name = retrieve_diffuse_new(mtlf_contents, usemtl_name[i]);
        std::string bumpmap_name = retrieve_bumpmap    (mtlf_contents, usemtl_name[i]);
        std::string full = dir + std::string("/") + texture_name;

        texture tex;
        tex.type = 0;
        tex.init();
        tex.set_texture_location(full);
        tex.get_id();

        if(texture::idquerytexture(tex.id)==-1)
        {
            tex.push();
        }

        cl_uint isbump = 0;
        cl_uint b_id = -1;

        if(bumpmap_name!=std::string("None"))
        {
            isbump = 1;
            texture bumpmap;
            std::string bump_full = dir + std::string("/") + bumpmap_name;

            bumpmap.type = 1;
            bumpmap.init();
            bumpmap.set_texture_location(bump_full);
            b_id = bumpmap.get_id();

            if(texture::idquerytexture(bumpmap.id)==-1)
            {
                bumpmap.push();
            }
        }

        object obj;
        obj.tri_list.reserve(usemtl_pos[i+1]-usemtl_pos[i]);

        for(int j=usemtl_pos[i]; j<usemtl_pos[i+1]; j++)
        {
            obj.tri_list.push_back(tris[j]);
        }

        obj.tri_num = obj.tri_list.size(); ///needs to be removed

        obj.tid = tex.id; ///this is potentially bad if textures are removed
        obj.bid = b_id;
        obj.has_bump = isbump;

        obj.pos = c->pos;
        obj.rot = c->rot;

        c->objs.push_back(obj);
    }

    c->isloaded = true;
}

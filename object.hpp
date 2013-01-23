#ifndef INCLUDED_HPP_OBJECT
#define INCLUDED_HPP_OBJECT
#include <vector>
#include "triangle.hpp"
#include <string>
#include <cl/cl.h>
struct object
{
    //double x, y, z;
    cl_float4 pos;
    cl_float4 rot;
    bool onvector;
    //std::vector<triangle*> tri_list;

    cl_mem g_mem;
    cl_mem g_tri_num;

    std::string mtlname;
    std::string tex_name;
    ///texture
    triangle *tri_list;
    int tri_num;

    cl_uint tid; ///texture id

    object();
    void alloc(int num);

    void g_alloc();

    //void gettexname();



};




#endif

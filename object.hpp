#ifndef INCLUDED_HPP_OBJECT
#define INCLUDED_HPP_OBJECT
#include <vector>
#include "triangle.hpp"
#include <string>
#include <cl/cl.h>
struct object
{
    cl_float4 pos;
    cl_float4 rot;

    bool isactive;
    int tri_num;

    std::vector<triangle> tri_list;

    cl_uint tid; ///texture id
    cl_uint atid; ///texture id in the active texturelist

    cl_mem g_mem;
    cl_mem g_tri_num;

    object();
    void set_active(bool param);
    void set_pos_rot(cl_float4, cl_float4);
};




#endif

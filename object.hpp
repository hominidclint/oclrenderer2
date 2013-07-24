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

    int graphics_obj_g_descriptor_id;

    std::vector<triangle> tri_list;

    cl_uint tid; ///texture id
    cl_uint atid; ///texture id in the active texturelist
    cl_uint bid; ///bumpmap_id
    cl_uint abid; ///active bumpmap_id

    cl_mem g_mem;
    cl_mem g_tri_num;

    cl_uint has_bump;

    object();
    void set_active(bool param);
    void set_pos   (cl_float4);
    void set_rot   (cl_float4);
};




#endif

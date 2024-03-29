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
    cl_uint bid; ///bumpmap_id
    cl_uint abid; ///active bumpmap_id

    cl_uint object_g_id; ///obj_g_descriptor id

    cl_uint object_sub_position; ///position in array

    cl_mem g_mem;
    cl_mem g_tri_num;

    cl_uint has_bump;

    object();
    void set_active     (bool param);
    void set_pos        (cl_float4);
    void set_rot        (cl_float4);

    void g_flush(cl_uint); ///flush position (currently just) etc to gpu memory
};




#endif

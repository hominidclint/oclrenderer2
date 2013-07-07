#ifndef INCLUDED_OBJ_MEM_MANAGER_HPP
#define INCLUDED_OBJ_MEM_MANAGER_HPP

#include "object.hpp"
#include <vector>

struct texture_array_descriptor;

struct temporaries
{
    cl_uint tri_num;

    cl_mem g_tri_mem;
    cl_mem g_tri_num;

    cl_mem g_obj_desc;
    cl_mem g_obj_num;

    cl_mem g_light_mem;
    cl_mem g_light_num;
    cl_mem g_light_buf;

    cl_mem g_cut_tri_mem;
    cl_mem g_cut_tri_num;

    cl_mem g_texture_array;
    cl_mem g_texture_sizes;
    cl_mem g_texture_nums;
};


struct obj_mem_manager
{
    static temporaries temporary_objects[2];

    static texture_array_descriptor tdescrip;

    static cl_uint tri_num;

    static std::vector<object*> obj_list;

    static cl_mem g_tri_mem;
    static cl_mem g_tri_num;

    static cl_mem g_obj_desc;
    static cl_mem g_obj_num;

    ///screenspace depth buffer for shadow casting lights. This is going to be slow
    static cl_mem g_light_mem;
    static cl_mem g_light_num;
    static cl_mem g_light_buf;
    ///array of lights in g_mem


    static cl_mem g_cut_tri_mem;
    static cl_mem g_cut_tri_num;


    static cl_uchar4* c_texture_array;

    static cl_mem g_texture_array;
    static cl_mem g_texture_sizes;
    static cl_mem g_texture_nums;

    static int which_temp_object;

    void g_arrange_textures();

    void g_arrange_mem();

    void g_changeover();
};



#endif

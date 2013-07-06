#ifndef INCLUDED_OBJ_MEM_MANAGER_HPP
#define INCLUDED_OBJ_MEM_MANAGER_HPP

#include "object.hpp"
#include <vector>

static cl_uint max_tex_size=2048;

struct texture_array_descriptor;


struct obj_mem_manager
{


    obj_mem_manager* temporary_objects;

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

    void init();

    void g_arrange_textures();

    void g_arrange_mem();

    void g_changeover();
};



#endif

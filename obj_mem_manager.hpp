#ifndef INCLUDED_OBJ_MEM_MANAGER_HPP
#define INCLUDED_OBJ_MEM_MANAGER_HPP

#include "object.hpp"
#include <vector>

static cl_uint max_tex_size=2048;

struct texture_array_descriptor
{

    std::vector<int> texture_nums;
    std::vector<int> texture_sizes;


} tad;

struct obj_mem_manager
{

    static texture_array_descriptor tdescrip;

    static cl_uint tri_num;

    static std::vector<object*> obj_list;

    static cl_mem g_tri_mem;

    static cl_mem g_tri_fstorage;

    static cl_mem g_tri_num;

    static cl_mem g_obj_desc;

    static cl_mem g_obj_num;

    static cl_uchar4* c_texture_array;

    static cl_mem g_texture_array;

    static cl_mem i256;
    static cl_mem i512;
    static cl_mem i1024;
    static cl_mem i2048;




    void g_arrange_mem();



};



#endif

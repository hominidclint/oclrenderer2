#ifndef INCLUDED_OBJ_MEM_MANAGER_HPP
#define INCLUDED_OBJ_MEM_MANAGER_HPP

#include "object.hpp"
#include <vector>

struct obj_mem_manager
{

    static cl_uint tri_num;

    static std::vector<object*> obj_list;

    static cl_mem g_tri_mem;

    static cl_mem g_tri_fstorage;

    static cl_mem g_tri_num;

    static cl_mem g_obj_desc;

    static cl_mem g_obj_num;

    static cl_mem i256;
    static cl_mem i512;
    static cl_mem i1024;
    static cl_mem i2048;




    void g_arrange_mem();



};



#endif
